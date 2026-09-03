#!/usr/bin/env python3
import pathlib
import sys

src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")

# Remove a complete C/C++ function by its exact signature. This lets the
# generated engine retain the model-specific hot loop while moving reusable
# subsystems into independently compiled modules.
def remove_function(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        return source
    brace = source.find("{", start)
    if brace < 0:
        raise SystemExit(f"function body not found: {signature}")
    depth = 0
    for i in range(brace, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                while end < len(source) and source[end] in " \t":
                    end += 1
                if end < len(source) and source[end] == "\n":
                    end += 1
                return source[:start] + source[end:]
    raise SystemExit(f"unclosed function: {signature}")

# Correct GPT-2 byte-level BPE mapping.
text = text.replace("if(i>=189)cp=256+(uint32_t)(i-189);", "if(i>=188)cp=256+(uint32_t)(i-188);")

# Correct grouped-query attention mapping for SmolLM (9 Q heads / 3 KV heads).
text = text.replace("size_t kh=h%arch_.kv_heads;", "size_t kh=(h*arch_.kv_heads)/arch_.heads;")

# Respect GGUF's explicit RoPE dimension.
old = "for(size_t h=0;h<arch_.heads;h++)rope(q.data()+h*hd,hd,pos,arch_.rope_theta);for(size_t h=0;h<arch_.kv_heads;h++)rope(kk.data()+h*hd,hd,pos,arch_.rope_theta);"
new = "const size_t rd=arch_.rope_dim?arch_.rope_dim:hd;for(size_t h=0;h<arch_.heads;h++)rope(q.data()+h*hd,rd,pos,arch_.rope_theta);for(size_t h=0;h<arch_.kv_heads;h++)rope(kk.data()+h*hd,rd,pos,arch_.rope_theta);"
text = text.replace(old, new)

# SmolLM-135M-Instruct ChatML format.
old = "std::vector<int32_t>ids;if(!vocab_.encode(prompt,ids)||ids.empty())return false;"
new = "std::string actual_prompt=prompt;if(prompt.find(\"<|im_start|>\")==std::string::npos){std::vector<int32_t>probe;if(vocab_.encode(\"<|im_start|>\",probe)&&!probe.empty())actual_prompt=\"<|im_start|>system\\nYou are a helpful AI assistant named SmolLM, trained by Hugging Face<|im_end|>\\n<|im_start|>user\\n\"+prompt+\"<|im_end|>\\n<|im_start|>assistant\\n\";}std::vector<int32_t>ids;if(!vocab_.encode(actual_prompt,ids)||ids.empty())return false;"
text = text.replace(old, new)

# Stop cleanly at EOS.
old = "int next=sample(logits,ids,sampling_,rng_);if(next<0)return false;ids.push_back(next);out.push_back(vocab_.decode(next));if(on_token&&!on_token(out.back()))return false;"
new = "int next=sample(logits,ids,sampling_,rng_);if(next<0)return false;uint32_t eos_id=UINT32_MAX;if(u32v(model_->metadata(\"tokenizer.ggml.eos_token_id\"),eos_id)&&next==(int)eos_id)break;ids.push_back(next);out.push_back(vocab_.decode(next));if(on_token&&!on_token(out.back()))return false;"
text = text.replace(old, new)

# Keep the generic half conversion for quantized/scalar paths. Dense F16 dot
# products are handled by the architecture-specific compute module.

# Reuse generation scratch buffers. The old code allocated vectors inside every
# token and every layer, which is especially expensive on the Pi 2 allocator.
old = "cache_.clear();size_t hd=arch_.hidden/arch_.heads;std::vector<float>x(arch_.hidden),z(arch_.hidden),q(arch_.hidden),kk(arch_.kv_heads*hd),vv(arch_.kv_heads*hd),a(arch_.hidden),logits(arch_.vocab);"
new = "cache_.clear();size_t hd=arch_.hidden/arch_.heads;std::vector<float>x(arch_.hidden),z(arch_.hidden),q(arch_.hidden),kk(arch_.kv_heads*hd),vv(arch_.kv_heads*hd),a(arch_.hidden),logits(arch_.vocab),sc,ffng,ffnu,ffnd;sc.reserve(arch_.context);ffng.resize(arch_.intermediate);ffnu.resize(arch_.intermediate);ffnd.resize(arch_.hidden);"
text = text.replace(old, new)
text = text.replace("std::vector<float>sc(pos+1);float mx=", "sc.resize(pos+1);float mx=")
text = text.replace("std::vector<float>g(arch_.intermediate),u(arch_.intermediate),d(arch_.hidden);mv(ffn1_[l],z.data(),g.data(),arch_.intermediate,arch_.hidden);if(ffn3_[l].tensor)mv(ffn3_[l],z.data(),u.data(),arch_.intermediate,arch_.hidden);else u=g;", "mv(ffn1_[l],z.data(),ffng.data(),arch_.intermediate,arch_.hidden);if(ffn3_[l].tensor)mv(ffn3_[l],z.data(),ffnu.data(),arch_.intermediate,arch_.hidden);else std::copy(ffng.begin(),ffng.end(),ffnu.begin());")
text = text.replace("for(size_t i=0;i<g.size();i++){float t=g[i];float sig=", "for(size_t i=0;i<ffng.size();i++){float t=ffng[i];float sig=")
text = text.replace("g[i]=(t*sig)*u[i];}mv(ffn2_[l],g.data(),d.data(),arch_.hidden,arch_.intermediate);for(size_t i=0;i<arch_.hidden;i++)x[i]+=d[i];", "ffng[i]=(t*sig)*ffnu[i];}mv(ffn2_[l],ffng.data(),ffnd.data(),arch_.hidden,arch_.intermediate);for(size_t i=0;i<arch_.hidden;i++)x[i]+=ffnd[i];")

# Compute attention scores with the shared NEON/scalar dot kernel.
old = "sc[pp]=0;for(size_t j=0;j<hd;j++)sc[pp]+=q[h*hd+j]*cache_.key(l,pp,kh)[j];sc[pp]/=std::sqrt((float)hd);"
new = "sc[pp]=dot_f32(reinterpret_cast<const uint8_t*>(q.data()+h*hd),cache_.key(l,pp,kh),hd)/std::sqrt((float)hd);"
text = text.replace(old, new)

# Accumulate attention values position-major. This keeps each KV vector
# contiguous and is friendlier to ARM NEON than repeatedly walking positions
# for every output lane.
old = "for(size_t j=0;j<hd;j++){float u=0;for(size_t pp=0;pp<=pos;pp++)u+=sc[pp]*cache_.value(l,pp,kh)[j];a[h*hd+j]=u;}"
new = "std::fill(a.begin()+h*hd,a.begin()+(h+1)*hd,0.f);for(size_t pp=0;pp<=pos;pp++){const float*vvv=cache_.value(l,pp,kh);float wgt=sc[pp];size_t j=0;for(;j+4<=hd;j+=4){float32x4_t av=vld1q_f32(a.data()+h*hd+j);av=vmlaq_n_f32(av,vld1q_f32(vvv+j),wgt);vst1q_f32(a.data()+h*hd+j,av);}for(;j<hd;j++)a[h*hd+j]+=wgt*vvv[j];}"
text = text.replace(old, new)

# The cache is now an independently compiled module. Remove its duplicate
# implementation from the generated engine after all source transformations.
for signature in (
    "bool KVCache::init(",
    "void KVCache::clear(",
    "float*KVCache::key(",
    "float*KVCache::value(",
):
    text = remove_function(text, signature)

# The generated source calls the modular cache API declared by inference.hpp.
dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text, encoding="utf-8")

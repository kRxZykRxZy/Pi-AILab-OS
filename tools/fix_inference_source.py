#!/usr/bin/env python3
import pathlib
import sys

src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")

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
rope_old = "for(size_t h=0;h<arch_.heads;h++)rope(q.data()+h*hd,hd,pos,arch_.rope_theta);for(size_t h=0;h<arch_.kv_heads;h++)rope(kk.data()+h*hd,hd,pos,arch_.rope_theta);"
rope_new = "const size_t rd=arch_.rope_dim?arch_.rope_dim:hd;for(size_t h=0;h<arch_.heads;h++)rope(q.data()+h*hd,rd,pos,arch_.rope_theta);for(size_t h=0;h<arch_.kv_heads;h++)rope(kk.data()+h*hd,rd,pos,arch_.rope_theta);"
text = text.replace(rope_old, rope_new)

# SmolLM-135M-Instruct ChatML format.
prompt_old = "std::vector<int32_t>ids;if(!vocab_.encode(prompt,ids)||ids.empty())return false;"
prompt_new = "std::string actual_prompt=prompt;if(prompt.find(\"<|im_start|>\")==std::string::npos){std::vector<int32_t>probe;if(vocab_.encode(\"<|im_start|>\",probe)&&!probe.empty())actual_prompt=\"<|im_start|>system\\nYou are a helpful AI assistant named SmolLM, trained by Hugging Face<|im_end|>\\n<|im_start|>user\\n\"+prompt+\"<|im_end|>\\n<|im_start|>assistant\\n\";}std::vector<int32_t>ids;if(!vocab_.encode(actual_prompt,ids)||ids.empty())return false;"
text = text.replace(prompt_old, prompt_new)

# Stop cleanly at EOS.
eos_old = "int next=sample(logits,ids,sampling_,rng_);if(next<0)return false;ids.push_back(next);out.push_back(vocab_.decode(next));if(on_token&&!on_token(out.back()))return false;"
eos_new = "int next=sample(logits,ids,sampling_,rng_);if(next<0)return false;uint32_t eos_id=UINT32_MAX;if(u32v(model_->metadata(\"tokenizer.ggml.eos_token_id\"),eos_id)&&next==(int)eos_id)break;ids.push_back(next);out.push_back(vocab_.decode(next));if(on_token&&!on_token(out.back()))return false;"
text = text.replace(eos_old, eos_new)

# Keep the exact FP16 lookup-table optimization for the scalar/quantized element
# path. Dense F16 dot products are routed to piai::compute::dot_f16 instead.
hf_old = 'static float hf(uint16_t h){uint32_t s=h>>15,e=(h>>10)&31,f=h&1023;if(!e)return(s?-1.f:1.f)*std::ldexp((float)f,-24);if(e==31)return f?NAN:(s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);return(s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);}'
hf_new = '''static const std::array<float,65536>& hf_table(){
    static const std::array<float,65536> t=[](){
        std::array<float,65536> a{};
        for(uint32_t h=0;h<65536;h++){
            uint32_t s=h>>15,e=(h>>10)&31,f=h&1023;
            if(!e)a[h]=(s?-1.f:1.f)*std::ldexp((float)f,-24);
            else if(e==31)a[h]=f?NAN:(s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);
            else a[h]=(s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);
        }
        return a;
    }();
    return t;
}
static inline float hf(uint16_t h){return hf_table()[h];}'''
text = text.replace(hf_old, hf_new)

# Cache RoPE sin/cos per position instead of evaluating trig functions for every
# layer and head.
rope_fn_old = 'static void rope(float*x,size_t d,size_t pos,float theta){for(size_t i=0;i+1<d;i+=2){float a=pos*std::pow(theta,-2.f*(i/2.f)/d),c=std::cos(a),s=std::sin(a),u=x[i],v=x[i+1];x[i]=u*c-v*s;x[i+1]=u*s+v*c;}}'
rope_fn_new = '''static void rope(float*x,size_t d,size_t pos,float theta){
    struct RC{size_t d=0,pos=SIZE_MAX;float theta=0;std::vector<float>c,s;};
    static thread_local RC rc;
    if(rc.d!=d||rc.pos!=pos||rc.theta!=theta){
        rc.d=d;rc.pos=pos;rc.theta=theta;rc.c.resize(d/2);rc.s.resize(d/2);
        for(size_t i=0;i<d/2;i++){float a=pos*std::pow(theta,-2.f*((float)i)/d);rc.c[i]=std::cos(a);rc.s[i]=std::sin(a);}
    }
    for(size_t i=0,j=0;i+1<d;i+=2,j++){float c=rc.c[j],s=rc.s[j],u=x[i],v=x[i+1];x[i]=u*c-v*s;x[i+1]=u*s+v*c;}
}'''
text = text.replace(rope_fn_old, rope_fn_new)

# Reuse generation scratch buffers instead of allocating per token/layer.
scratch_old = "cache_.clear();size_t hd=arch_.hidden/arch_.heads;std::vector<float>x(arch_.hidden),z(arch_.hidden),q(arch_.hidden),kk(arch_.kv_heads*hd),vv(arch_.kv_heads*hd),a(arch_.hidden),logits(arch_.vocab);"
scratch_new = "cache_.clear();size_t hd=arch_.hidden/arch_.heads;std::vector<float>x(arch_.hidden),z(arch_.hidden),q(arch_.hidden),kk(arch_.kv_heads*hd),vv(arch_.kv_heads*hd),a(arch_.hidden),logits(arch_.vocab),sc,ffng,ffnu,ffnd;sc.reserve(arch_.context);ffng.resize(arch_.intermediate);ffnu.resize(arch_.intermediate);ffnd.resize(arch_.hidden);"
text = text.replace(scratch_old, scratch_new)
text = text.replace("std::vector<float>sc(pos+1);float mx=", "sc.resize(pos+1);float mx=")
text = text.replace("std::vector<float>g(arch_.intermediate),u(arch_.intermediate),d(arch_.hidden);mv(ffn1_[l],z.data(),g.data(),arch_.intermediate,arch_.hidden);if(ffn3_[l].tensor)mv(ffn3_[l],z.data(),u.data(),arch_.intermediate,arch_.hidden);else u=g;", "mv(ffn1_[l],z.data(),ffng.data(),arch_.intermediate,arch_.hidden);if(ffn3_[l].tensor)mv(ffn3_[l],z.data(),ffnu.data(),arch_.intermediate,arch_.hidden);else std::copy(ffng.begin(),ffng.end(),ffnu.begin());")
text = text.replace("for(size_t i=0;i<g.size();i++){float t=g[i];float sig=", "for(size_t i=0;i<ffng.size();i++){float t=ffng[i];float sig=")
text = text.replace("g[i]=(t*sig)*u[i];}mv(ffn2_[l],g.data(),d.data(),arch_.hidden,arch_.intermediate);for(size_t i=0;i<arch_.hidden;i++)x[i]+=d[i];", "ffng[i]=(t*sig)*ffnu[i];}mv(ffn2_[l],ffng.data(),ffnd.data(),arch_.hidden,arch_.intermediate);for(size_t i=0;i<arch_.hidden;i++)x[i]+=ffnd[i];")

# Shared dense dot kernel for attention scores.
text = text.replace(
    "sc[pp]=0;for(size_t j=0;j<hd;j++)sc[pp]+=q[h*hd+j]*cache_.key(l,pp,kh)[j];sc[pp]/=std::sqrt((float)hd);",
    "sc[pp]=dot_f32(reinterpret_cast<const uint8_t*>(q.data()+h*hd),cache_.key(l,pp,kh),hd)/std::sqrt((float)hd);"
)

# Shared architecture-specific AXPY kernel for attention values.
text = text.replace(
    "for(size_t j=0;j<hd;j++){float u=0;for(size_t pp=0;pp<=pos;pp++)u+=sc[pp]*cache_.value(l,pp,kh)[j];a[h*hd+j]=u;}",
    "std::fill(a.begin()+h*hd,a.begin()+(h+1)*hd,0.f);for(size_t pp=0;pp<=pos;pp++)piai::compute::axpy_f32(a.data()+h*hd,cache_.value(l,pp,kh),sc[pp],hd);"
)

# Cache is now independently compiled under src/inference/cache.cpp.
for signature in (
    "bool KVCache::init(",
    "void KVCache::clear(",
    "float*KVCache::key(",
    "float*KVCache::value(",
):
    text = remove_function(text, signature)

dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text, encoding="utf-8")

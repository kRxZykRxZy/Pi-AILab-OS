#!/usr/bin/env python3
import pathlib
import sys
src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")

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

# Cortex-A7 has no native FP16 arithmetic. Avoid repeated ldexp() calls by using
# an exact 65536-entry IEEE half -> float lookup table. The table is only 256 KiB.
old = 'static float hf(uint16_t h){uint32_t s=h>>15,e=(h>>10)&31,f=h&1023;if(!e)return(s?-1.f:1.f)*std::ldexp((float)f,-24);if(e==31)return f?NAN:(s?-INFINITY:INFINITY);return(s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);}'
new = '''static const std::array<float,65536>& hf_table(){
    static const std::array<float,65536> t=[](){
        std::array<float,65536> a{};
        for(uint32_t h=0;h<65536;h++){
            uint32_t s=h>>15,e=(h>>10)&31,f=h&1023;
            if(!e) a[h]=(s?-1.f:1.f)*std::ldexp((float)f,-24);
            else if(e==31) a[h]=f?NAN:(s?-INFINITY:INFINITY);
            else a[h]=(s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);
        }
        return a;
    }();
    return t;
}
static inline float hf(uint16_t h){return hf_table()[h];}'''
text = text.replace(old, new)

# Tighter NEON FP16 dot kernel. It avoids scalar half conversion in the hot loop,
# uses two independent accumulators, and keeps the Cortex-A7 NEON pipeline fed.
old_start = 'static inline float dot_f16(const uint8_t*wp,const float*x,size_t n){'
start = text.find(old_start)
if start >= 0:
    end = text.find('\n}\n\nstatic inline float dot_f32', start)
    if end >= 0:
        new_dot = r'''static inline float dot_f16(const uint8_t*wp,const float*x,size_t n){
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    const uint16_t*w=reinterpret_cast<const uint16_t*>(wp);
    const auto&ht=hf_table();
    float32x4_t acc0=vdupq_n_f32(0.f),acc1=vdupq_n_f32(0.f);
    size_t i=0;
    for(;i+8<=n;i+=8){
        float32x4_t a=vld1q_f32(x+i),b=vld1q_f32(x+i+4);
        float32x4_t wa={ht[w[i]],ht[w[i+1]],ht[w[i+2]],ht[w[i+3]]};
        float32x4_t wb={ht[w[i+4]],ht[w[i+5]],ht[w[i+6]],ht[w[i+7]]};
        acc0=vmlaq_f32(acc0,a,wa);acc1=vmlaq_f32(acc1,b,wb);
    }
    acc0=vaddq_f32(acc0,acc1);
    float32x2_t p=vadd_f32(vget_low_f32(acc0),vget_high_f32(acc0));
    p=vpadd_f32(p,p);float out=vget_lane_f32(p,0);
    for(;i<n;i++)out+=ht[w[i]]*x[i];
    return out;
#else
    const uint16_t*w=reinterpret_cast<const uint16_t*>(wp);const auto&ht=hf_table();
    float out=0.f;for(size_t i=0;i<n;i++)out+=ht[w[i]]*x[i];return out;
#endif
}'''
        text = text[:start] + new_dot + text[end+2:]

# Cache RoPE values for each position. A position is transformed for many heads
# and every layer, so computing sin/cos once per position removes a large amount
# of scalar libm work from every generated token.
old = 'static void rope(float*x,size_t d,size_t pos,float theta){for(size_t i=0;i+1<d;i+=2){float a=pos*std::pow(theta,-2.f*(i/2.f)/d),c=std::cos(a),s=std::sin(a),u=x[i],v=x[i+1];x[i]=u*c-v*s;x[i+1]=u*s+v*c;}}'
new = r'''static void rope(float*x,size_t d,size_t pos,float theta){
    struct RC{size_t d=0,pos=SIZE_MAX;float theta=0;std::vector<float>c,s;};
    static thread_local RC rc;
    if(rc.d!=d||rc.pos!=pos||rc.theta!=theta){
        rc.d=d;rc.pos=pos;rc.theta=theta;rc.c.resize(d/2);rc.s.resize(d/2);
        for(size_t i=0;i<d/2;i++){float a=pos*std::pow(theta,-2.f*((float)i)/d);rc.c[i]=std::cos(a);rc.s[i]=std::sin(a);}
    }
    for(size_t i=0,j=0;i+1<d;i+=2,j++){float c=rc.c[j],s=rc.s[j],u=x[i],v=x[i+1];x[i]=u*c-v*s;x[i+1]=u*s+v*c;}
}'''
text = text.replace(old, new)

dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text, encoding="utf-8")

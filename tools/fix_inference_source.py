#!/usr/bin/env python3
import pathlib
import sys
src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")
text = text.replace("if(i>=189)cp=256+(uint32_t)(i-189);", "if(i>=188)cp=256+(uint32_t)(i-188);")
old = "size_t kh=h%arch_.kv_heads;"
new = "size_t kh=(h*arch_.kv_heads)/arch_.heads;"
if old in text:
    text = text.replace(old, new)
old = "for(size_t h=0;h<arch_.heads;h++)rope(q.data()+h*hd,hd,pos,arch_.rope_theta);for(size_t h=0;h<arch_.kv_heads;h++)rope(kk.data()+h*hd,hd,pos,arch_.rope_theta);"
new = "const size_t rd=arch_.rope_dim?arch_.rope_dim:hd;for(size_t h=0;h<arch_.heads;h++)rope(q.data()+h*hd,rd,pos,arch_.rope_theta);for(size_t h=0;h<arch_.kv_heads;h++)rope(kk.data()+h*hd,rd,pos,arch_.rope_theta);"
if old in text:
    text = text.replace(old, new)
old = "std::vector<int32_t>ids;if(!vocab_.encode(prompt,ids)||ids.empty())return false;"
new = "std::string actual_prompt=prompt;if(prompt.find(\"<|im_start|>\")==std::string::npos){std::vector<int32_t>probe;if(vocab_.encode(\"<|im_start|>\",probe)&&!probe.empty())actual_prompt=\"<|im_start|>system\\nYou are a helpful AI assistant named SmolLM, trained by Hugging Face<|im_end|>\\n<|im_start|>user\\n\"+prompt+\"<|im_end|>\\n<|im_start|>assistant\\n\";}std::vector<int32_t>ids;if(!vocab_.encode(actual_prompt,ids)||ids.empty())return false;"
if old in text:
    text = text.replace(old, new)
old = "int next=sample(logits,ids,sampling_,rng_);if(next<0)return false;ids.push_back(next);out.push_back(vocab_.decode(next));if(on_token&&!on_token(out.back()))return false;"
new = "int next=sample(logits,ids,sampling_,rng_);if(next<0)return false;uint32_t eos_id=UINT32_MAX;if(u32v(model_->metadata(\"tokenizer.ggml.eos_token_id\"),eos_id)&&next==(int)eos_id)break;ids.push_back(next);out.push_back(vocab_.decode(next));if(on_token&&!on_token(out.back()))return false;"
if old in text:
    text = text.replace(old, new)
dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text, encoding="utf-8")

#!/usr/bin/env python3
import pathlib,sys
src=pathlib.Path(sys.argv[1]);dst=pathlib.Path(sys.argv[2])
t=src.read_text(encoding='utf-8')
t=t.replace('#include "inference.hpp"','#include "inference.hpp"\n#include "piai/compute/dot.hpp"\n#include "piai/platform/pi_models.hpp"',1)
for name in ('dot_f16','dot_f32'):
    marker=f'static inline float {name}('
    start=t.find(marker)
    if start<0: continue
    brace=t.find('{',start); depth=0; end=-1
    for i in range(brace,len(t)):
        if t[i]=='{': depth+=1
        elif t[i]=='}':
            depth-=1
            if depth==0: end=i+1; break
    if end<0: raise SystemExit(f'unclosed {name}')
    if name=='dot_f16': repl='static inline float dot_f16(const uint8_t*wp,const float*x,size_t n){return piai::compute::dot_f16(wp,x,n);}'
    else: repl='static inline float dot_f32(const uint8_t*wp,const float*x,size_t n){return piai::compute::dot_f32(wp,x,n);}'
    t=t[:start]+repl+t[end:]
t=t.replace('unsigned n=std::thread::hardware_concurrency();if(n==0)n=1;','unsigned n=piai::platform::detect_pi().recommended_threads;if(n==0)n=1;')
dst.parent.mkdir(parents=True,exist_ok=True);dst.write_text(t,encoding='utf-8')

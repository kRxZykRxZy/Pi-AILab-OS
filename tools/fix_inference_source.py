#!/usr/bin/env python3
import pathlib
import sys

src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")
old1 = "if(i>=189)cp=256+(uint32_t)(i-189);"
new1 = "if(i>=188)cp=256+(uint32_t)(i-188);"
old2 = "if(i>=189)cp=256+(uint32_t)(i-189);"
if old1 in text:
    text = text.replace(old1, new1)
elif old2 in text:
    text = text.replace(old2, new1)
else:
    raise SystemExit("expected GPT-2 byte mapping was not found")
dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text, encoding="utf-8")

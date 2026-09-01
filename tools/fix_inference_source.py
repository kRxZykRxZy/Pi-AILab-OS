#!/usr/bin/env python3
import pathlib
import sys

src = pathlib.Path(sys.argv[1])
dst = pathlib.Path(sys.argv[2])
text = src.read_text(encoding="utf-8")

# GPT-2 byte-level BPE maps the 188 printable byte entries directly and
# assigns the remaining 68 bytes to Unicode code points starting at U+0100.
# The old source used 189 here, shifting every fallback byte by one and
# turning U+0120 into byte 0x7f (DEL), which appeared as boxed x characters.
old = "if(i>=189)cp=256+(uint32_t)(i-189);"
new = "if(i>=188)cp=256+(uint32_t)(i-188);"
if old in text:
    text = text.replace(old, new)
elif "if(i>=188)cp=256+(uint32_t)(i-188);" not in text:
    raise SystemExit("expected GPT-2 byte mapping was not found")

dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(text, encoding="utf-8")

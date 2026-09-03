# Pi-AILab-OS — Q4_K_M garbage-output debugging notes

## Status: WIP (bug NOT yet resolved)

Repo: `https://github.com/kRxZykRxZy/Pi-AILab-OS` (branch `main`)
Local mirror: `C:\Users\ymonz\Pi-AILab-OS`
RPi: `admin@raspberrypi.local` (password `Hm361485%`), service `piai` on port `5453`.
RPi repo mirror: `~/Pi-AILab-OS`. Only `src/gguf_v3.cpp` is compiled; `src/inference_v4.cpp`
is copied to `build/generated/inference_v4.cpp` by `tools/fix_inference_source.py` at build time
(that script does NOT touch the `elem` dequant, so source edits carry through).

---

## Symptom

- `SmolLM-135M.Q4_K_M.gguf` LOADS fine (`architecture:llama layers:30 hidden:576 heads:9
  kv_heads:3 vocab:49152 context:2048`) but GENERATES identical deterministic word-salad every run:
  `"esi or server Smokingtheless Titlerstripenzaricult W W R but offers new W orapache nor Q any ..."`
- `SmolLM2-135M-Instruct-f16.gguf` (a DIFFERENT base model) generates coherent text:
  `"Good day. What can I do for you today?"`  <- shared pipeline works.

## What is PROVEN correct

1. **Dequant formulas in `elem` (src/inference_v4.cpp line 28) are correct.**
   Built `deq_check.cpp` on the Pi (ties against the project's own `piai::gguf::Model`),
   independently re-deriving Q5_0 / Q8_0 / Q4_K / Q5_K / Q6_K from llama.cpp. `elem` == reference
   for every formula, and values are sane / in-range.
2. **Tensor types actually used by Q4_K_M:** only `F32(0)` norms, `Q5_0(6)`, `Q8_0(8)`,
   `Q4_K(12)`, `Q6_K(14)` — all handled by `elem`.
3. **GGUF tensor-type enum matches the standard** (Q4_K=12, Q5_K=13, Q6_K=14) — no mapping bug.
4. **Layout** for Q8_0 = 34B/block, Q5_0 = 22B/block (d,qh,qs), Q4_K = 144B, Q6_K = 210B (d at
   B+208, scales at B+192 s8), Q5_K = 176B. Verified against reference.
5. The prior Q6_K scale-index bug was fixed (commit `45802f0`: `scidx=(local/16)+grp*2`).
   Commits `fc80dca` + `45802f0` add the K-quant dequant support.

## Since dequant is correct but output is flat/random

The identical repeated word-salad strongly suggests **flat / degenerate logits**, meaning the
forward compute path itself (not the weight formulas) is going wrong for quantized tensors.
Candidates to investigate next:

- The matvec fast paths in `MVWorkers` dispatch on `type==F16` / `type==F32`; all quant types
  use `elem`. Confirm nothing accidentally routes a quant tensor through `dot_f16`/`dot_f32`.
- The output head: `Engine::load` sets `output_=embedding_` when there is no `output.weight`
  (Q4_K_M has NO `output.weight`; output is TIED to `token_embd` which is Q8_0). Check the
  576x49152 tied matmul orientation for the logits.
- Confirm the logits are flat by dumping top-5 logits after token 0 (proposed instrumentation;
  was staged then reverted on 2026-09-03 — see `git reflog` for the temporary commit
  `453e4e1`). Peaked logits => sampling/decode bug. Flat logits => compute-path bug.

## Next steps

1. Re-instrument `Engine::generate` (src/inference_v4.cpp ~line 115, right after
   `mv(output_,z.data(),logits.data(),arch_.vocab,arch_.hidden)`) to write top-5 logits for
   `pos==0` to `/tmp/piai_logits.txt`. Commit + push, deploy via
   `python deploy_piai.py` (does `git pull --ff-only`, `sudo ./install.sh`, `sudo systemctl restart piai`),
   then re-test with the HTTP test script. (NOTE: deploy pulls from GitHub, so edits MUST be
   committed + pushed first.)
2. Diagnose flat-vs-peaked logits; fix the responsible path.
3. On success: re-verify with `max_tokens:5` first (generation is slow / server is single-threaded
   and blocks during load; the model may be auto-loaded or need `/models/<name>/load`).
4. Add `.gitignore` for the local `build/` directory (currently untracked).

## Repro / tooling

- HTTP test: load then generate on `http://raspberrypi.local:5453/api/v1/models/<name>/...`
  (see `C:\Users\ymonz\AppData\Local\Temp\opencode\test_gen.py`).
- Windows console: set `$env:PYTHONIOENCODING='utf-8'` before running Python that prints RPi output.
- SSH only via paramiko (no sshpass/plink on the Windows host). DNS for `raspberrypi.local` is
  occasionally flaky; retry on `getaddrinfo failed` / `10061 refused`.
- Dequant checker (recompile on Pi): `g++ -O2 -std=c++17 -I include deq_check.cpp src/gguf_v3.cpp -o deq_check`
  (source kept at `C:\Users\ymonz\AppData\Local\Temp\opencode\deq_check.cpp`).
- Model paths on Pi: `/var/lib/piai/models/SmolLM-135M.Q4_K_M.gguf`,
  `/var/lib/piai/models/SmolLM2-135M-Instruct-f16.gguf`.
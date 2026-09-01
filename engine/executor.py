from __future__ import annotations
import asyncio
import os
import signal
import shutil
from dataclasses import dataclass
from pathlib import Path


class EngineError(RuntimeError):
    pass


@dataclass
class RunRequest:
    model: str
    prompt: str
    max_tokens: int = 128
    threads: int = 0
    temperature: float = 0.7
    timeout: int = 300


class AIEngine:
    def __init__(self, models_dir: str, runtime: str = 'llama-cli', max_prompt_chars: int = 32768):
        self.models_dir = Path(models_dir).resolve()
        self.runtime = runtime
        self.max_prompt_chars = max_prompt_chars

    def runtime_path(self) -> str:
        path = shutil.which(self.runtime) or self.runtime
        if not os.path.isfile(path) or not os.access(path, os.X_OK):
            raise EngineError(f'Inference runtime not found: {self.runtime}')
        return path

    def validate_model(self, model: str) -> Path:
        p = Path(model).expanduser().resolve()
        try:
            p.relative_to(self.models_dir)
        except ValueError:
            raise EngineError('Model must be inside the configured models directory')
        if not p.is_file():
            raise EngineError('Model file does not exist')
        if p.suffix.lower() != '.gguf':
            raise EngineError('V0.1 supports GGUF models')
        return p

    async def run(self, req: RunRequest) -> dict:
        if not req.prompt.strip():
            raise EngineError('Prompt is empty')
        if len(req.prompt) > self.max_prompt_chars:
            raise EngineError('Prompt is too large')
        if not 1 <= req.max_tokens <= 8192:
            raise EngineError('max_tokens must be between 1 and 8192')
        model = self.validate_model(req.model)
        runtime = self.runtime_path()
        cmd = [runtime, '-m', str(model), '-p', req.prompt, '-n', str(req.max_tokens), '--temp', str(req.temperature), '--no-display-prompt']
        if req.threads > 0:
            cmd += ['-t', str(min(req.threads, os.cpu_count() or 1))]
        process = await asyncio.create_subprocess_exec(*cmd, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE, start_new_session=True)
        try:
            stdout, stderr = await asyncio.wait_for(process.communicate(), timeout=req.timeout)
        except asyncio.TimeoutError:
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            raise EngineError('Inference timed out')
        text = stdout.decode('utf-8', 'replace').strip()
        err = stderr.decode('utf-8', 'replace').strip()
        if process.returncode != 0:
            raise EngineError(err or f'Runtime exited with code {process.returncode}')
        return {'text': text, 'exit_code': process.returncode, 'model': str(model), 'runtime': runtime}

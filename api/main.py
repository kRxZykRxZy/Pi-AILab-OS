from __future__ import annotations
import asyncio
import json
import os
import sys
import time
import uuid
from pathlib import Path

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field

ROOT = Path(os.getenv('PILAB_ROOT', '/var/lib/pi-ai-lab'))
MODELS = ROOT / 'models'
MODELS.mkdir(parents=True, exist_ok=True)
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from engine.executor import AIEngine, EngineError, RunRequest
from monitor.system import snapshot

app = FastAPI(title='Pi AI Lab OS', version='0.1.0')
engine = AIEngine(str(MODELS), os.getenv('PILAB_RUNTIME', 'llama-cli'))
jobs: dict[str, dict] = {}
semaphore = asyncio.Semaphore(int(os.getenv('PILAB_MAX_CONCURRENT_JOBS', '1')))

class InferenceRequest(BaseModel):
    model: str
    prompt: str = Field(min_length=1)
    max_tokens: int = Field(default=128, ge=1, le=8192)
    threads: int = Field(default=0, ge=0, le=128)
    temperature: float = Field(default=0.7, ge=0.0, le=2.0)
    timeout: int = Field(default=300, ge=1, le=3600)

class ModelRequest(BaseModel):
    path: str
    name: str | None = None

@app.get('/api/v1/health')
async def health():
    return {'status': 'ok', 'service': 'pi-ai-lab-os', 'version': '0.1.0'}

@app.get('/api/v1/system')
async def system():
    return snapshot()

@app.get('/api/v1/models')
async def models():
    items = []
    for p in sorted(MODELS.glob('*.gguf')):
        stat = p.stat()
        items.append({'id': p.name, 'name': p.stem, 'path': str(p), 'size_bytes': stat.st_size, 'modified': stat.st_mtime})
    return {'models': items}

@app.post('/api/v1/models')
async def add_model(req: ModelRequest):
    p = Path(req.path).expanduser().resolve()
    try:
        p.relative_to(MODELS.resolve())
    except ValueError:
        raise HTTPException(400, 'Model path must be inside the models directory')
    if not p.is_file() or p.suffix.lower() != '.gguf':
        raise HTTPException(400, 'Expected an existing GGUF file')
    return {'id': p.name, 'name': req.name or p.stem, 'path': str(p), 'size_bytes': p.stat().st_size}

@app.delete('/api/v1/models/{model_id}')
async def delete_model(model_id: str):
    p = (MODELS / model_id).resolve()
    try:
        p.relative_to(MODELS.resolve())
    except ValueError:
        raise HTTPException(400, 'Invalid model id')
    if not p.exists():
        raise HTTPException(404, 'Model not found')
    p.unlink()
    return {'deleted': model_id}

async def execute(job_id: str, req: InferenceRequest):
    jobs[job_id].update(status='running', started_at=time.time())
    try:
        async with semaphore:
            result = await engine.run(RunRequest(**req.model_dump()))
        jobs[job_id].update(status='completed', result=result, finished_at=time.time())
    except (EngineError, Exception) as exc:
        jobs[job_id].update(status='failed', error=str(exc), finished_at=time.time())

@app.post('/api/v1/inference')
async def inference(req: InferenceRequest):
    job_id = uuid.uuid4().hex
    jobs[job_id] = {'id': job_id, 'status': 'queued', 'created_at': time.time()}
    asyncio.create_task(execute(job_id, req))
    return {'job_id': job_id, 'status': 'queued'}

@app.get('/api/v1/jobs/{job_id}')
async def get_job(job_id: str):
    job = jobs.get(job_id)
    if not job:
        raise HTTPException(404, 'Job not found')
    return job

@app.post('/api/v1/jobs/{job_id}/cancel')
async def cancel_job(job_id: str):
    job = jobs.get(job_id)
    if not job:
        raise HTTPException(404, 'Job not found')
    if job.get('status') in {'completed', 'failed', 'cancelled'}:
        return job
    job['status'] = 'cancelled'
    return job

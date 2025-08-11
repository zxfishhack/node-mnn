import os from 'node:os';
import path from 'node:path';
import fs from 'node:fs';
import { LLM } from '../lib/index.js';

const model_dir = path.join(os.homedir(), ".llm/Qwen3-30B-A3B-Instruct-2507-MNN/");

function benchmark() {
  const llm = new LLM();

  const prompt = fs.readFileSync(path.join("test", "test.js"), {encoding: "utf-8"})

  const start = process.hrtime.bigint();
  llm.load(model_dir);

  const end = process.hrtime.bigint();
  const timeInMs = Number(end - start) / 1000000; // 转换为毫秒


  const metrics = []

  for(let i=0; i<10; i++) {
    const gen = llm.generate(prompt)
    for (const v of gen) {}

    metrics.push(llm.metrics())
  } 

  const metric = metrics.reduce((pv, cv) => {
    return {
      prefill_tokens: pv.prefill_tokens + cv.prefill_tokens,
      decode_tokens: pv.decode_tokens + cv.decode_tokens,
      prefill_us: pv.prefill_us + cv.prefill_us,
      decode_us: pv.decode_us + cv.decode_us,
    }
  })
  console.log("=====================")
  console.log(`加载时间: ${timeInMs.toFixed(3)} ms`);
  console.log(`prefill tokens: ${metric.prefill_tokens}`)
  console.log(` decode tokens: ${metric.decode_tokens}`)
  console.log(` prefill tok/s: ${(metric.prefill_tokens / metric.prefill_us * 1e6).toFixed(3)}`)
  console.log(`  decode tok/s: ${(metric.decode_tokens / metric.decode_us * 1e6).toFixed(3)}`)
  console.log("=====================")
}

benchmark();
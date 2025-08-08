import { LLM } from '../lib/index.js';
import path from 'node:path';
import os from 'node:os';

async function testBenchmark() {
  const llm = new LLM(path.join(os.homedir(), ".llm/Qwen3-0.6B-MNN/"));

  llm.load();

  const gen = await llm.generateAsync("今天天气真不错");

  let resp = '';

  for await (const v of gen) {
    resp += v;
  }

  console.log(resp);
}

function testChatMessages() {
  const llm = new LLM(path.join(os.homedir(), ".llm/Qwen3-0.6B-MNN/"));

  llm.load();

  const prompts = [
    {
      role: "system",
      message: "将用户的所有输入都翻译成英文",
    },
    {
      role: "user",
      message: "今天天气真不错"
    }
  ]

  const gen = llm.generate(prompts);

  let resp = ''

  for (const v of gen) {
    resp += v
  }

  console.log(resp);
}

async function runAllTests() {
  try {
    await testBenchmark()

    testChatMessages()

    console.log('\n所有测试完成!');
  } catch (error) {
    console.error('测试错误:', error);
  }
}

runAllTests();

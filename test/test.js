import os from 'node:os';
import path from 'node:path';
import { LLM } from '../lib/index.js';

const model_dir = path.join(os.homedir(), ".llm/gpt-oss-20b-MNN/");

async function testBenchmark() {
  const llm = new LLM();

  llm.load(model_dir);

  const gen = await llm.generateAsync("今天天气真不错");

  let resp = '';

  for await (const v of gen) {
    process.stdout.write(v)
  }

  console.log(llm.metrics());

  llm.unload();
}

function testChatMessages() {
  const llm = new LLM();

  llm.load(model_dir);

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
    process.stdout.write(v)
  }

  console.log(llm.metrics());

  llm.unload();
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

const { LLM } = require('../lib');

function sleepSync(ms) {
  const start = Date.now();
  while (Date.now() - start < ms) {
    // 空循环，CPU 全占用
  }
}

function testBenchmark() {
  const llm = new LLM("/Users/zxfishhack/.llm/Qwen3-0.6B-MNN/");

  llm.load();

  const gen = llm.generate("今天天气真不错");

  let resp = '';

  for (const v of gen) {
    resp += v;
  }

  console.log(resp);
}

async function runAllTests() {
  try {
    testBenchmark()

    console.log('\n所有测试完成!');
  } catch (error) {
    console.error('测试错误:', error);
  }
}

runAllTests();

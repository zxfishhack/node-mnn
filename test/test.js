const { StreamGenerator } = require('../lib');

async function testSyncGenerator() {
  console.log('=== 测试同步生成器 ===');
  const generator = new StreamGenerator(5, 50);
  generator.start();

  // 使用迭代器协议
  for (const value of generator) {
    if (value === undefined) break;
    console.log('同步值:', value);
  }
}

async function testAsyncGenerator() {
  console.log('\n=== 测试异步生成器 ===');
  const generator = new StreamGenerator(5, 100);

  // 使用异步迭代器协议
  for await (const value of generator.asyncGenerator()) {
    console.log('异步值:', value);
  }
}

async function testStream() {
  console.log('\n=== 测试流式输出 ===');
  const generator = new StreamGenerator(5, 80);
  generator.start();

  const stream = generator.createReadableStream();

  stream.on('data', (data) => {
    console.log('流数据:', data);
  });

  stream.on('end', () => {
    console.log('流结束');
  });

  stream.on('error', (err) => {
    console.error('流错误:', err);
  });

  // 等待流结束
  return new Promise((resolve, reject) => {
    stream.on('end', resolve);
    stream.on('error', reject);
  });
}

async function testManualAsync() {
  console.log('\n=== 测试手动异步调用 ===');
  const generator = new StreamGenerator(3, 200);
  generator.start();

  try {
    while (true) {
      const result = await generator.nextAsync();
      if (result.done) {
        console.log('生成器完成');
        break;
      }
      console.log('手动异步值:', result.value);
    }
  } finally {
    generator.stop();
  }
}

async function runAllTests() {
  try {
    await testSyncGenerator();
    await testAsyncGenerator();
    await testStream();
    await testManualAsync();

    console.log('\n所有测试完成!');
  } catch (error) {
    console.error('测试错误:', error);
  }
}

runAllTests();

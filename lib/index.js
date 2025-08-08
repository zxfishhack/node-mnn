const binary = require('../build/Release/node-mnn');

class StreamGenerator {
  constructor(count = 10, delay = 100) {
    this.native = new binary.StreamGenerator(count, delay);
    this.started = false;
  }

  start() {
    this.started = true;
    return this.native.start();
  }

  stop() {
    this.started = false;
    return this.native.stop();
  }

  // 同步获取下一个值
  next() {
    if (!this.started) {
      return { value: undefined, done: true };
    }
    return this.native.getNext();
  }

  // 异步获取下一个值
  nextAsync() {
    return new Promise((resolve, reject) => {
      if (!this.started) {
        resolve({ value: undefined, done: true });
        return;
      }

      this.native.getNext((err, result) => {
        if (err) {
          reject(err);
        } else {
          resolve(result);
        }
      });
    });
  }

  // 实现迭代器协议
  [Symbol.iterator]() {
    return this;
  }

  // 实现异步迭代器协议
  [Symbol.asyncIterator]() {
    return {
      next: () => this.nextAsync()
    };
  }

  // 创建 Node.js 可读流
  createReadableStream() {
    const { Readable } = require('stream');
    const generator = this;

    return new Readable({
      objectMode: true,
      read() {
        generator.nextAsync()
          .then(result => {
            if (result.done) {
              this.push(null); // 结束流
            } else {
              this.push(result.value);
            }
          })
          .catch(err => {
            this.destroy(err);
          });
      }
    });
  }

  // 创建异步生成器函数
  async* asyncGenerator() {
    this.start();

    try {
      while (true) {
        const result = await this.nextAsync();
        if (result.done) {
          break;
        }
        yield result.value;
      }
    } finally {
      this.stop();
    }
  }
}

module.exports = {
  StreamGenerator,
  createGenerator: (count, delay) => new StreamGenerator(count, delay)
};

import { createRequire } from 'module';
const require = createRequire(import.meta.url);

const binary = require('../build/Release/node-mnn.node');

export class LLM {
    constructor(model_dir) {
        this.native = new binary.LLM(model_dir);
    }

    load() {
        return this.native.load();
    }


    generate(msg) {
        return this.native.generate(msg);
    }

    async generateAsync(msg) {
        return this.native.generateAsync(msg);
    }
}
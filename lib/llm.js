import { createRequire } from 'module';
const require = createRequire(import.meta.url);

const binary = require('../build/Release/node-mnn.node');

export class LLM {
    constructor() {
        this.native = new binary.LLM();
    }

    load(model_dir) {
        return this.native.load(model_dir);
    }

    unload() {
        return this.native.unload();
    }


    generate(msg, max_token) {
        return this.native.generate(msg, max_token);
    }

    async generateAsync(msg, max_token) {
        return this.native.generateAsync(msg, max_token);
    }

    metrics() {
        return this.native.metrics();
    }
}
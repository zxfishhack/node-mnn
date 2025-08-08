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


    generate(msg) {
        return this.native.generate(msg);
    }

    async generateAsync(msg) {
        return this.native.generateAsync(msg);
    }
}
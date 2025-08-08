#include "ss_generator.h"
#include "llm.h"


// 模块初始化
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    UnifiedStreamGenerator::Init(env, exports);
    
    LLM::Init(env, exports);
    
    return exports;
}

NODE_API_MODULE(mnn, Init)
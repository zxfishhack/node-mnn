#include <napi.h>
#include <MNN/llm/llm.hpp>
#include <iostream>

class LLM : public Napi::ObjectWrap<LLM> {
public:
    LLM(const Napi::CallbackInfo& info);
    ~LLM() {
        std::cout << "~LLM" << std::endl;
    }
    static void Init(Napi::Env env, Napi::Object exports);
private:
    Napi::Value Load(const Napi::CallbackInfo& info);
    Napi::Value Unload(const Napi::CallbackInfo& info);

    Napi::Value Generate(const Napi::CallbackInfo& info);
    Napi::Value GenerateAsync(const Napi::CallbackInfo& info);
private:
    Napi::Value generate(const Napi::CallbackInfo& info, bool async);

    std::shared_ptr<MNN::Transformer::Llm> llm_;
};
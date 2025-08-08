#include <napi.h>
#include <MNN/llm/llm.hpp>

class LLM : public Napi::ObjectWrap<LLM> {
public:
    LLM(const Napi::CallbackInfo& info);
    static void Init(Napi::Env env, Napi::Object exports);
private:
    Napi::Value SetConfig(const Napi::CallbackInfo& info);
    Napi::Value Load(const Napi::CallbackInfo& info);

    Napi::Value Generate(const Napi::CallbackInfo& info);
    Napi::Value GenerateAsync(const Napi::CallbackInfo& info);
private:
    std::shared_ptr<MNN::Transformer::Llm> llm_;
};
#include <string>
#include <thread>
#include "llm.h"
#include "closable_deque.h"
#include "ss_generator.h"

LLM::LLM(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<LLM>(info)
    , llm_(nullptr)
{
    Napi::Env env = info.Env();

    // 1. 参数检查
    if (info.Length() < 1 || !info[0].IsString()) {
      Napi::TypeError::New(env, "model_dir is required").ThrowAsJavaScriptException();

      // 2. 立刻退出，触发析构函数
      return;
    }

    // 3. 安全进行实际初始化
    std::string modelDir = info[0].As<Napi::String>();
    llm_.reset(MNN::Transformer::Llm::createLLM(modelDir));
    if (!llm_) {
      Napi::Error::New(env, "failed to load model").ThrowAsJavaScriptException();
      // 立即 return，让 JS 层看到这个异常
      return;
    }
}

Napi::Value LLM::SetConfig(const Napi::CallbackInfo &info)
{
    // TODO: 完成配置
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value LLM::Load(const Napi::CallbackInfo &info)
{
    llm_->load(); // 目前无法得知模型是否正确加载
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value LLM::Generate(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::shared_ptr<closable_deque> q(new closable_deque());
    std::string prompt = info[0].As<Napi::String>();;

    std::thread executor([this, q, prompt]() {
        llm_->response(prompt, new std::ostream(q.get()));

        q->close();
    });

    executor.detach();

    return UnifiedStreamGenerator::CreateFromDeque(env, q, false);
}


Napi::Value LLM::GenerateAsync(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::shared_ptr<closable_deque> q(new closable_deque());
    std::string prompt = info[0].As<Napi::String>();;

    std::thread executor([this, q, prompt]() {
        llm_->response(prompt, new std::ostream(q.get()));

        q->close();
    });

    executor.detach();

    return UnifiedStreamGenerator::CreateFromDeque(env, q, true);
}

void LLM::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "LLM", {
        InstanceMethod("load", &LLM::Load),
        InstanceMethod("setConfig", &LLM::SetConfig),
        InstanceMethod("generate", &LLM::Generate),
        InstanceMethod("generateAsync", &LLM::GenerateAsync),
    });

    exports.Set("LLM", func);
}

#include <string>
#include <thread>
#include "llm.h"
#include "closable_deque.h"
#include "ss_generator.h"

std::set<std::string> valid_role{"system", "user", "assistant"};

LLM::LLM(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<LLM>(info)
    , llm_(nullptr)
{
}

Napi::Value LLM::Unload(const Napi::CallbackInfo &info)
{
    llm_.reset();

    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value LLM::Load(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    // 1. 参数检查
    if (info.Length() < 1 || !info[0].IsString()) {
      Napi::TypeError::New(env, "model_dir is required").ThrowAsJavaScriptException();

      // 2. 立刻退出，触发析构函数
      return Napi::Boolean::New(info.Env(), false);
    }

    // 3. 安全进行实际初始化
    std::string modelDir = info[0].As<Napi::String>();
    llm_.reset(MNN::Transformer::Llm::createLLM(modelDir));
    if (!llm_) {
      Napi::Error::New(env, "failed to load model").ThrowAsJavaScriptException();
      // 立即 return，让 JS 层看到这个异常
      return Napi::Boolean::New(info.Env(), false);
    }
    llm_->load(); // 目前无法得知模型是否正确加载
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value LLM::Generate(const Napi::CallbackInfo &info)
{
    return generate(info, false);
}


Napi::Value LLM::GenerateAsync(const Napi::CallbackInfo &info)
{
    return generate(info, true);
}

Napi::Value LLM::generate(const Napi::CallbackInfo &info, bool async)
{
    Napi::Env env = info.Env();
    std::shared_ptr<closable_deque> q(new closable_deque());
    if (info.Length() < 1) {
        Napi::TypeError::New(env, "prompt is required").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    if (info[0].IsString()) {
        std::string prompt = info[0].As<Napi::String>();

        std::thread executor([this, q, prompt]() {
            llm_->response(prompt, new std::ostream(q.get()));

            q->close();
        });

        executor.detach();
    } else if (info[0].IsArray()) {
        Napi::Array arr = info[0].As<Napi::Array>();
        MNN::Transformer::ChatMessages prompt;
        for (uint32_t i=0; i<arr.Length(); i++) {
            Napi::Value element = arr[i];
            if (!element.IsObject()) {
                Napi::TypeError::New(env, "prompt must be string or array of Message").ThrowAsJavaScriptException();
                return env.Undefined();
            }
            Napi::Object obj = element.As<Napi::Object>();
            if (!obj.Has("role") || !obj.Get("role").IsString()) {
                Napi::TypeError::New(env, "prompt must be string or array of Message").ThrowAsJavaScriptException();
                return env.Undefined();
            }
            std::string role = obj.Get("role").As<Napi::String>();
            if (valid_role.find(role) == valid_role.end()) {
                Napi::TypeError::New(env, "prompt must be string or array of Message").ThrowAsJavaScriptException();
                return env.Undefined();
            }
            if (!obj.Has("message") || !obj.Get("message").IsString()) {
                Napi::TypeError::New(env, "prompt must be string or array of Message").ThrowAsJavaScriptException();
                return env.Undefined();
            }
            std::string message = obj.Get("message").As<Napi::String>();
            prompt.push_back(MNN::Transformer::ChatMessage(role, message));
        }
        std::thread executor([this, q, prompt]() {
            llm_->response(prompt, new std::ostream(q.get()));

            q->close();
        });

        executor.detach();
    } else {
        Napi::TypeError::New(env, "prompt must be string or array of Message").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    return UnifiedStreamGenerator::CreateFromDeque(env, q, async);
}

void LLM::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "LLM", {
        InstanceMethod("load", &LLM::Load),
        InstanceMethod("unload", &LLM::Unload),
        InstanceMethod("generate", &LLM::Generate),
        InstanceMethod("generateAsync", &LLM::GenerateAsync),
    });

    exports.Set("LLM", func);
}

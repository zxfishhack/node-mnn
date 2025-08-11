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

Napi::Value LLM::Metrics(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    auto ctx = llm_->getContext();
    auto res = Napi::Object::New(env);
    res.Set("prefill_tokens", Napi::Number::New(env, ctx->prompt_len));
    res.Set("decode_tokens", Napi::Number::New(env, ctx->gen_seq_len));
    res.Set("prefill_us", Napi::Number::New(env, ctx->prefill_us));
    res.Set("decode_us", Napi::Number::New(env, ctx->decode_us));

    return res;
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
    if (info.Length() >= 2 && info[1].IsString()) {
        std::string config = info[1].As<Napi::String>();
        llm_->set_config(config);
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
    int max_token = -1;
    if (info.Length() < 1) {
        Napi::TypeError::New(env, "prompt is required").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    if (info.Length() >= 2 && info[1].IsNumber()) {
        max_token = info[1].As<Napi::Number>().Int32Value();
        if (max_token < 0) {
            max_token = -1;
        } 
    }

    if (info[0].IsString()) {
        std::string prompt = info[0].As<Napi::String>();

        std::thread executor([this, q, prompt, max_token]() {
            llm_->response(prompt, new std::ostream(q.get()), "", max_token);

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
        std::thread executor([this, q, prompt, max_token]() {
            llm_->response(prompt, new std::ostream(q.get()), "", max_token);

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
        InstanceMethod("metrics", &LLM::Metrics),
    });

    exports.Set("LLM", func);
}

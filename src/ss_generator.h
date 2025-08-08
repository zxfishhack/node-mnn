#include <napi.h>
#include <thread>
#include <memory>
#include <chrono>
#include "closable_deque.h"

class UnifiedStreamGenerator : public Napi::ObjectWrap<UnifiedStreamGenerator> {
private:
    std::shared_ptr<closable_deque> deque_;
    bool isAsync;
    std::chrono::milliseconds timeout_;

    static Napi::FunctionReference constructor;

public:
    UnifiedStreamGenerator(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<UnifiedStreamGenerator>(info), 
        isAsync(false),
        timeout_(std::chrono::milliseconds::max()) {
    }
    static void Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "UnifiedStreamGenerator", {
            InstanceMethod("next", &UnifiedStreamGenerator::Next),
            InstanceMethod("return", &UnifiedStreamGenerator::Return),
            InstanceMethod("throw", &UnifiedStreamGenerator::Throw)
        });

        constructor = Napi::Persistent(func);
    }

    // 静态工厂方法：从 close_deque 创建实例
    static Napi::Object CreateFromDeque(Napi::Env env, std::shared_ptr<closable_deque> deque, bool isAsync = false) {        
        // 创建空实例
        Napi::Object instance = constructor.New({});
        
        // 获取 C++ 对象并初始化
        UnifiedStreamGenerator* generator = Napi::ObjectWrap<UnifiedStreamGenerator>::Unwrap(instance);
        generator->deque_ = deque;
        generator->isAsync = isAsync;
        
        // 添加适当的 Symbol
        if (isAsync) {
            Napi::Symbol asyncIteratorSymbol = Napi::Symbol::WellKnown(env, "asyncIterator");
            instance.Set(asyncIteratorSymbol, Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
                return info.This();
            }));
        } else {
            Napi::Symbol iteratorSymbol = Napi::Symbol::WellKnown(env, "iterator");
            instance.Set(iteratorSymbol, Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
                return info.This();
            }));
        }
        
        return instance;
    }

    Napi::Value Next(const Napi::CallbackInfo& info) {
        if (isAsync) {
            return NextAsync(info);
        } else {
            return NextSync(info);
        }
    }

    Napi::Value NextAsync(const Napi::CallbackInfo& info);

private:
    Napi::Value NextSync(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Object result = Napi::Object::New(env);
        
        std::string data;
        bool hasData = deque_->wait_and_pop(data, std::chrono::milliseconds::max());
        
        if (hasData) {
            result.Set("done", false);
            result.Set("value", Napi::String::New(env, data));
        } else if (deque_->is_finished()) {
            result.Set("done", true);
            result.Set("value", env.Undefined());
        } else {
            // 没有数据但未关闭，返回 undefined（可以根据需要调整这个行为）
            result.Set("done", false);
            result.Set("value", env.Undefined());
        }
        
        return result;
    }


public:
    Napi::Value Return(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        // 关闭 deque
        if (deque_) {
            deque_->close();
        }
        
        if (isAsync) {
            auto deferred = Napi::Promise::Deferred::New(env);
            Napi::Object result = Napi::Object::New(env);
            result.Set("done", true);
            result.Set("value", info.Length() > 0 ? info[0] : env.Undefined());
            deferred.Resolve(result);
            return deferred.Promise();
        } else {
            Napi::Object result = Napi::Object::New(env);
            result.Set("done", true);
            result.Set("value", info.Length() > 0 ? info[0] : env.Undefined());
            return result;
        }
    }

    Napi::Value Throw(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        // 关闭 deque
        if (deque_) {
            deque_->close();
        }
        
        if (isAsync) {
            auto deferred = Napi::Promise::Deferred::New(env);
            if (info.Length() > 0) {
                deferred.Reject(info[0]);
            } else {
                deferred.Reject(Napi::String::New(env, "Generator throw"));
            }
            return deferred.Promise();
        } else {
            if (info.Length() > 0) {
                Napi::Error::New(env, info[0].As<Napi::String>()).ThrowAsJavaScriptException();
            }
            return env.Undefined();
        }
    }

private:
};
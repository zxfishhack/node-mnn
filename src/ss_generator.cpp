#include "ss_generator.h"

class DequeAsyncWorker : public Napi::AsyncWorker {
public:
    DequeAsyncWorker(Napi::Env env, 
                     Napi::Promise::Deferred deferred, 
                     std::shared_ptr<closable_deque> deque,
                     std::chrono::milliseconds timeout)
        : Napi::AsyncWorker(env),
          deferred_(std::move(deferred)),
          deque_(deque),
          timeout_(timeout),
          hasData_(false),
          isFinished_(false) {}

protected:
    void Execute() override {
        try {
            hasData_ = deque_->wait_and_pop(data_, timeout_);
            isFinished_ = deque_->is_finished();
        } catch (const std::exception& e) {
            SetError(e.what());
        }
    }

    void OnOK() override {
        Napi::Env env = deferred_.Env();
        Napi::Object result = Napi::Object::New(env);
        
        if (hasData_) {
            result.Set("done", false);
            result.Set("value", Napi::String::New(env, data_));
        } else if (isFinished_) {
            result.Set("done", true);
            result.Set("value", env.Undefined());
        } else {
            // 超时但未结束
            result.Set("done", false);
            result.Set("value", env.Undefined());
        }
        
        deferred_.Resolve(result);
    }

    void OnError(const Napi::Error& error) override {
        deferred_.Reject(error.Value());
    }

private:
    Napi::Promise::Deferred deferred_;
    std::shared_ptr<closable_deque> deque_;
    std::chrono::milliseconds timeout_;
    std::string data_;
    bool hasData_;
    bool isFinished_;
};

Napi::Value UnifiedStreamGenerator::NextAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto deferred = Napi::Promise::Deferred::New(env);
    auto promise = deferred.Promise(); // 先获取 Promise
    
    // 创建并启动 AsyncWorker
    auto worker = new DequeAsyncWorker(env, std::move(deferred), deque_, timeout_);
    worker->Queue();
    
    return promise; // 返回之前获取的 Promise
}

Napi::FunctionReference UnifiedStreamGenerator::constructor;
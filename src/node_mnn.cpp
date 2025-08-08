#include <nan.h>
#include <thread>
#include <chrono>
#include <memory>

class StreamGenerator : public Nan::ObjectWrap {
public:
    static void Init(v8::Local<v8::Object> exports);

private:
    explicit StreamGenerator(int count = 10, int delay = 100);
    ~StreamGenerator();

    static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void Start(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void Stop(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void GetNext(const Nan::FunctionCallbackInfo<v8::Value>& info);

    // 异步工作器
    class GeneratorWorker : public Nan::AsyncWorker {
    public:
        GeneratorWorker(Nan::Callback* callback, StreamGenerator* generator, int value)
            : Nan::AsyncWorker(callback), generator_(generator), value_(value) {}

        void Execute() override {
            // 模拟一些工作延迟
            std::this_thread::sleep_for(std::chrono::milliseconds(generator_->delay_));
        }

        void HandleOKCallback() override {
            Nan::HandleScope scope;

            v8::Local<v8::Object> result = Nan::New<v8::Object>();
            Nan::Set(result, Nan::New("value").ToLocalChecked(), Nan::New(value_));
            Nan::Set(result, Nan::New("done").ToLocalChecked(),
                    Nan::New(generator_->current_ >= generator_->count_));

            v8::Local<v8::Value> argv[] = { Nan::Null(), result };
            callback->Call(2, argv, async_resource);
        }

    private:
        StreamGenerator* generator_;
        int value_;
    };

    int count_;
    int delay_;
    int current_;
    bool running_;
};

StreamGenerator::StreamGenerator(int count, int delay)
    : count_(count), delay_(delay), current_(0), running_(false) {}

StreamGenerator::~StreamGenerator() {}

void StreamGenerator::Init(v8::Local<v8::Object> exports) {
    Nan::HandleScope scope;

    // 准备构造函数模板
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("StreamGenerator").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // 原型方法
    Nan::SetPrototypeMethod(tpl, "start", Start);
    Nan::SetPrototypeMethod(tpl, "stop", Stop);
    Nan::SetPrototypeMethod(tpl, "getNext", GetNext);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(exports, Nan::New("StreamGenerator").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

void StreamGenerator::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.IsConstructCall()) {
        // 使用 new 调用
        int count = info[0]->IsUndefined() ? 10 : Nan::To<int>(info[0]).FromJust();
        int delay = info[1]->IsUndefined() ? 100 : Nan::To<int>(info[1]).FromJust();

        StreamGenerator* obj = new StreamGenerator(count, delay);
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    } else {
        // 普通函数调用，转换为构造调用
        const int argc = 2;
        v8::Local<v8::Value> argv[argc] = { info[0], info[1] };
        v8::Local<v8::Function> cons = Nan::New(constructor());
        info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
    }
}

void StreamGenerator::Start(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    StreamGenerator* obj = ObjectWrap::Unwrap<StreamGenerator>(info.Holder());
    obj->running_ = true;
    obj->current_ = 0;
    info.GetReturnValue().Set(Nan::True());
}

void StreamGenerator::Stop(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    StreamGenerator* obj = ObjectWrap::Unwrap<StreamGenerator>(info.Holder());
    obj->running_ = false;
    info.GetReturnValue().Set(Nan::True());
}

void StreamGenerator::GetNext(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    StreamGenerator* obj = ObjectWrap::Unwrap<StreamGenerator>(info.Holder());

    if (!obj->running_ || obj->current_ >= obj->count_) {
        v8::Local<v8::Object> result = Nan::New<v8::Object>();
        Nan::Set(result, Nan::New("value").ToLocalChecked(), Nan::Undefined());
        Nan::Set(result, Nan::New("done").ToLocalChecked(), Nan::New(true));
        info.GetReturnValue().Set(result);
        return;
    }

    if (info[0]->IsFunction()) {
        // 异步模式
        Nan::Callback* callback = new Nan::Callback(info[0].As<v8::Function>());
        GeneratorWorker* worker = new GeneratorWorker(callback, obj, obj->current_++);
        Nan::AsyncQueueWorker(worker);
    } else {
        // 同步模式
        int value = obj->current_++;
        v8::Local<v8::Object> result = Nan::New<v8::Object>();
        Nan::Set(result, Nan::New("value").ToLocalChecked(), Nan::New(value));
        Nan::Set(result, Nan::New("done").ToLocalChecked(),
                Nan::New(obj->current_ >= obj->count_));
        info.GetReturnValue().Set(result);
    }
}

static Nan::Persistent<v8::Function> constructor;

void StreamGenerator::constructor() {
    return constructor;
}

void InitAll(v8::Local<v8::Object> exports) {
    StreamGenerator::Init(exports);
}

NODE_MODULE(stream_generator, InitAll)

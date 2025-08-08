#include <nan.h>

class JsonHelper {
public:
    static std::string Stringify(v8::Isolate* isolate, v8::Local<v8::Value> value) {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        
        v8::MaybeLocal<v8::String> maybe_json = v8::JSON::Stringify(context, value);
        v8::Local<v8::String> json;
        
        if (!maybe_json.ToLocal(&json)) {
            return ""; // 序列化失败
        }
        
        v8::String::Utf8Value utf8_json(isolate, json);
        return std::string(*utf8_json);
    }
};
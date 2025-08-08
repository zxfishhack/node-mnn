template<typename T>
class ObjectConverter {
public:
    static T FromV8Object(v8::Isolate* isolate, v8::Local<v8::Object> obj);
};

#define GET_STRING_PROPERTY(prop_name, target) \
        do { \
            v8::Local<v8::Value> val; \
            if (obj->Get(context, v8::String::NewFromUtf8(isolate, #prop_name).ToLocalChecked()) \
                .ToLocal(&val) && val->IsString()) { \
                v8::String::Utf8Value utf8(isolate, val); \
                target = std::string(*utf8); \
            } \
        } while(0)
    
#define GET_INT_PROPERTY(prop_name, target) \
        do { \
            v8::Local<v8::Value> val; \
            if (obj->Get(context, v8::String::NewFromUtf8(isolate, #prop_name).ToLocalChecked()) \
                .ToLocal(&val) && val->IsNumber()) { \
                target = val->Int32Value(context).FromJust(); \
            } \
        } while(0)
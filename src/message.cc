#include "gss.h"

namespace node_gss {

v8::Handle<v8::Value> Wrap(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 4) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, ContextHandle, context_handle);
  int conf_req_flag = args[1]->ToInteger()->Value();
  gss_qop_t qop_req = args[2]->ToUint32()->Value();
  BUFFER_ARGUMENT(3, input);

  OM_uint32 minor;
  int conf_state;
  gss_buffer_desc output;
  OM_uint32 major = gss_wrap(&minor, context_handle->get(), conf_req_flag,
                             qop_req, &input, &conf_state, &output);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("confState"), v8::Integer::New(conf_state));
  ret->Set(v8::String::NewSymbol("output"),
           node::Buffer::New(static_cast<char*>(output.value),
                             output.length)->handle_);
  return scope.Close(ret);
}

v8::Handle<v8::Value> Unwrap(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 2) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, ContextHandle, context_handle);
  BUFFER_ARGUMENT(1, input);

  OM_uint32 minor;
  gss_buffer_desc output;
  int conf_state;
  gss_qop_t qop_state;
  OM_uint32 major = gss_unwrap(&minor, context_handle->get(), &input,
                               &output, &conf_state, &qop_state);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("output"),
           node::Buffer::New(static_cast<char*>(output.value),
                             output.length)->handle_);
  ret->Set(v8::String::NewSymbol("confState"), v8::Integer::New(conf_state));
  ret->Set(v8::String::NewSymbol("qopState"),
           v8::Integer::NewFromUnsigned(qop_state));
  return scope.Close(ret);
}

void MessageInit(v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("wrap"),
               v8::FunctionTemplate::New(Wrap)->GetFunction());
  exports->Set(v8::String::NewSymbol("unwrap"),
               v8::FunctionTemplate::New(Unwrap)->GetFunction());
}

}  // namespace node_gss

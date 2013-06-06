#include "gss.h"

namespace node_gss {

v8::Handle<v8::Value> ImportName(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 3) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  BUFFER_ARGUMENT(0, buffer);
  HANDLE_ARGUMENT(1, OidHandle, oid);
  HANDLE_ARGUMENT(2, NameHandle, name);

  OM_uint32 minor;
  OM_uint32 major = gss_import_name(&minor, &buffer, oid->get(), &name->get());
  
  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::New(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::New(minor));
  return scope.Close(ret);
}

v8::Handle<v8::Value> DisplayName(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 1) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, NameHandle, name);

  gss_buffer_desc buffer;
  gss_OID oid;

  OM_uint32 minor;
  OM_uint32 major = gss_display_name(&minor, name->get(), &buffer, &oid);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::New(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::New(minor));
  ret->Set(v8::String::NewSymbol("buffer"),
           node::Buffer::New((const char*)buffer.value,
                             buffer.length)->handle_);
  ret->Set(v8::String::NewSymbol("oid"),
           OidHandle::New(oid)->handle_);

  gss_release_buffer(NULL, &buffer);

  return scope.Close(ret);
}

void NameInit(v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("importName"),
               v8::FunctionTemplate::New(ImportName)->GetFunction());
  exports->Set(v8::String::NewSymbol("displayName"),
               v8::FunctionTemplate::New(DisplayName)->GetFunction());
}

}  // namespace node_gss

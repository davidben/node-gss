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
  OID_ARGUMENT(1, oid);
  HANDLE_ARGUMENT(2, NameHandle, name);

  OM_uint32 minor;
  OM_uint32 major = gss_import_name(&minor, &buffer, oid, &name->get());
  
  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
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
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("buffer"),
           node::Buffer::New((const char*)buffer.value,
                             buffer.length)->handle_);
  ret->Set(v8::String::NewSymbol("oid"),
           OidHandle::New(oid)->handle_);
  gss_release_buffer(NULL, &buffer);
  return scope.Close(ret);
}

v8::Handle<v8::Value> CompareName(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 2) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, NameHandle, name1);
  HANDLE_ARGUMENT(1, NameHandle, name2);

  int name_equal;
  OM_uint32 minor;
  OM_uint32 major = gss_compare_name(&minor,
                                     name1->get(), name2->get(), &name_equal);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("nameEqual"), v8::Integer::New(name_equal));
  return scope.Close(ret);
}

v8::Handle<v8::Value> CanonicalizeName(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 3) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, NameHandle, name);
  OID_ARGUMENT(1, oid);
  HANDLE_ARGUMENT(2, NameHandle, output);

  OM_uint32 minor;
  OM_uint32 major = gss_canonicalize_name(&minor,
                                          name->get(), oid, &output->get());

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  return scope.Close(ret);
}

v8::Handle<v8::Value> ExportName(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 1) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, NameHandle, name);

  gss_buffer_desc buffer;
  OM_uint32 minor;
  OM_uint32 major = gss_export_name(&minor, name->get(), &buffer);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("buffer"),
           node::Buffer::New((const char*)buffer.value,
                             buffer.length)->handle_);
  gss_release_buffer(NULL, &buffer);
  return scope.Close(ret);
}

void NameInit(v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("importName"),
               v8::FunctionTemplate::New(ImportName)->GetFunction());
  exports->Set(v8::String::NewSymbol("displayName"),
               v8::FunctionTemplate::New(DisplayName)->GetFunction());
  exports->Set(v8::String::NewSymbol("compareName"),
               v8::FunctionTemplate::New(CompareName)->GetFunction());
  exports->Set(v8::String::NewSymbol("canonicalizeName"),
               v8::FunctionTemplate::New(CanonicalizeName)->GetFunction());
  exports->Set(v8::String::NewSymbol("exportName"),
               v8::FunctionTemplate::New(ExportName)->GetFunction());
}

}  // namespace node_gss

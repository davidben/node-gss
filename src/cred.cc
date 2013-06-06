#include "gss.h"

namespace node_gss {

v8::Handle<v8::Value> AcquireCred(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 5) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, NameHandle, name);
  OM_uint32 time_req = args[1]->ToUint32()->Value();
  OID_SET_ARGUMENT(2, desired_mechs);
  int cred_usage = args[3]->ToInteger()->Value();
  HANDLE_ARGUMENT(4, CredHandle, output_cred_handle);

  //  gss_OID_set actual_mechs;
  //  OM_uint32 time_rec;
  OM_uint32 minor;
  OM_uint32 major = gss_acquire_cred(&minor, name->get(), time_req,
                                     desired_mechs.oid_set(), cred_usage,
                                     &output_cred_handle->get(),
                                     NULL, NULL);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  //  ret->Set(v8::String::NewSymbol("timeRec"),
  //           v8::Integer::NewFromUnsigned(time_rec));
  //  ret->Set(v8::String::NewSymbol("actualMechs"), OidSetToArray(actual_mechs));
  //  gss_release_oid_set(&minor, &actual_mechs);
  return scope.Close(ret);
}

void CredInit(v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("acquireCred"),
               v8::FunctionTemplate::New(AcquireCred)->GetFunction());
}

}  // namespace node_gss

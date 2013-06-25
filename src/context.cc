#include "gss.h"

namespace node_gss {

v8::Handle<v8::Value> InitSecContext(const v8::Arguments& args) {
  v8::HandleScope scope;

  /*
   OM_uint32 gss_init_sec_context (
     OM_uint32                    *minor_status,
     const gss_cred_id_t          initiator_cred_handle,
     gss_ctx_id_t                 *context_handle,
     const gss_name_t             target_name,
     const gss_OID                mech_type,
     OM_uint32                    req_flags,
     OM_uint32                    time_req,
     const gss_channel_bindings_t input_chan_bindings,
     const gss_buffer_t           input_token
     gss_OID                      *actual_mech_type,
     gss_buffer_t                 output_token,
     OM_uint32                    *ret_flags,
     OM_uint32                    *time_rec )
  */

  if (args.Length() < 8) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, CredHandle, cred_handle);
  HANDLE_ARGUMENT(1, ContextHandle, context_handle);
  HANDLE_ARGUMENT(2, NameHandle, target_name);
  OID_ARGUMENT(3, mech_type);
  OM_uint32 req_flags = args[4]->ToUint32()->Value();
  OM_uint32 time_req = args[5]->ToUint32()->Value();
  if (!args[6]->IsNull()) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Channel bindings not implemented")));
    return scope.Close(v8::Undefined());
  }
  BUFFER_ARGUMENT(7, input_token); 

  OM_uint32 minor;
  gss_OID actual_mech_type;
  gss_buffer_desc output_token;
  OM_uint32 ret_flags;
  OM_uint32 time_rec;
  OM_uint32 major = gss_init_sec_context(
      &minor, cred_handle->get(), &context_handle->get(), target_name->get(),
      mech_type, req_flags, time_req, NULL, &input_token,
      &actual_mech_type, &output_token, &ret_flags, &time_rec);

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("actualMechType"),
           OidHandle::New(actual_mech_type)->handle_);
  ret->Set(v8::String::NewSymbol("outputToken"),
           node::Buffer::New(static_cast<char*>(output_token.value),
                             output_token.length)->handle_);
  ret->Set(v8::String::NewSymbol("retFlags"),
           v8::Integer::NewFromUnsigned(ret_flags));
  ret->Set(v8::String::NewSymbol("timeRec"),
           v8::Integer::NewFromUnsigned(time_rec));
  return scope.Close(ret);
}


v8::Handle<v8::Value> AcceptSecContext(const v8::Arguments& args) {
  v8::HandleScope scope;

  /*
   OM_uint32 gss_accept_sec_context (
     OM_uint32           *minor_status,
     gss_ctx_id_t        *context_handle,
     const gss_cred_id_t acceptor_cred_handle,
     const gss_buffer_t  input_token_buffer,
     const gss_channel_bindings_t  input_chan_bindings,
     const gss_name_t    *src_name,
     gss_OID             *mech_type,
     gss_buffer_t        output_token,
     OM_uint32           *ret_flags,
     OM_uint32           *time_rec,
     gss_cred_id_t       *delegated_cred_handle)
  */

  if (args.Length() < 6) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  HANDLE_ARGUMENT(0, ContextHandle, context_handle);
  HANDLE_ARGUMENT(1, CredHandle, cred_handle);
  BUFFER_ARGUMENT(2, input_token); 
  if (!args[3]->IsNull()) {
    v8::ThrowException(v8::Exception::Error(v8::String::New(
        "Channel bindings not implemented")));
    return scope.Close(v8::Undefined());
  }
  HANDLE_ARGUMENT(4, NameHandle, src_name);
  HANDLE_ARGUMENT(5, CredHandle, delegated_cred_handle);

  OM_uint32 minor;
  gss_OID mech_type;
  gss_buffer_desc output_token;
  OM_uint32 ret_flags;
  OM_uint32 time_rec;
  OM_uint32 major = gss_accept_sec_context(
      &minor, &context_handle->get(), cred_handle->get(), &input_token,
      NULL, &src_name->get(), &mech_type, &output_token, &ret_flags,
      &time_rec, &delegated_cred_handle->get());

  v8::Local<v8::Object> ret = v8::Object::New();
  ret->Set(v8::String::NewSymbol("major"), v8::Integer::NewFromUnsigned(major));
  ret->Set(v8::String::NewSymbol("minor"), v8::Integer::NewFromUnsigned(minor));
  ret->Set(v8::String::NewSymbol("mechType"),
           OidHandle::New(mech_type)->handle_);
  ret->Set(v8::String::NewSymbol("outputToken"),
           node::Buffer::New(static_cast<char*>(output_token.value),
                             output_token.length)->handle_);
  ret->Set(v8::String::NewSymbol("retFlags"),
           v8::Integer::NewFromUnsigned(ret_flags));
  ret->Set(v8::String::NewSymbol("timeRec"),
           v8::Integer::NewFromUnsigned(time_rec));
  return scope.Close(ret);
}

void ContextInit(v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("initSecContext"),
               v8::FunctionTemplate::New(InitSecContext)->GetFunction());
  exports->Set(v8::String::NewSymbol("acceptSecContext"),
               v8::FunctionTemplate::New(AcceptSecContext)->GetFunction());
}

}  // namespace node_gss

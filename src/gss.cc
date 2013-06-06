#include "gss.h"

namespace node_gss {

void AddOidConstant(v8::Handle<v8::Object> exports,
                    gss_OID oid,
                    const char* name) {
  exports->Set(v8::String::NewSymbol(name), OidHandle::New(oid)->handle_);
}

void Init(v8::Handle<v8::Object> exports) {
  ContextHandle::Init(exports, "ContextHandle");
  CredHandle::Init(exports, "CredHandle");
  NameHandle::Init(exports, "NameHandle");
  OidHandle::Init(exports, "OID");

  // Attach constants separately, so it's easy to export the all.
  v8::Local<v8::Object> constants = v8::Object::New();
  exports->Set(v8::String::NewSymbol("constants"), constants);

  AddOidConstant(constants, GSS_C_NT_USER_NAME, "C_NT_USER_NAME");
  AddOidConstant(constants, GSS_C_NT_MACHINE_UID_NAME, "C_NT_MACHINE_UID_NAME");
  AddOidConstant(constants, GSS_C_NT_STRING_UID_NAME, "C_NT_STRING_UID_NAME");
  AddOidConstant(constants,
                 GSS_C_NT_HOSTBASED_SERVICE_X, "C_NT_HOSTBASED_SERVICE_X");
  AddOidConstant(constants,
                 GSS_C_NT_HOSTBASED_SERVICE, "C_NT_HOSTBASED_SERVICE");
  AddOidConstant(constants, GSS_C_NT_ANONYMOUS, "C_NT_ANONYMOUS");
  AddOidConstant(constants, GSS_C_NT_EXPORT_NAME, "C_NT_EXPORT_NAME");

  AddOidConstant(constants,
                 const_cast<gss_OID>(GSS_KRB5_NT_PRINCIPAL_NAME),
                 "KRB5_NT_PRINCIPAL_NAME");
  AddOidConstant(constants,
                 const_cast<gss_OID>(gss_mech_krb5),
                 "KRB5_MECHANISM");
}

}  // namespace node_gss

NODE_MODULE(gss, node_gss::Init)

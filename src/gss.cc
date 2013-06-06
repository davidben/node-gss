#include "gss.h"

namespace node_gss {

void AddOidConstant(v8::Handle<v8::Object> exports,
                    gss_OID oid,
                    const char* name) {
  exports->Set(v8::String::NewSymbol(name), OidHandle::New(oid)->handle_);
}

void AddConstant(v8::Handle<v8::Object> exports,
                 int value,
                 const char* name) {
  exports->Set(v8::String::NewSymbol(name), v8::Integer::New(value));
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

  AddConstant(constants, GSS_C_DELEG_FLAG, "C_DELEG_FLAG");
  AddConstant(constants, GSS_C_MUTUAL_FLAG, "C_MUTUAL_FLAG");
  AddConstant(constants, GSS_C_REPLAY_FLAG, "C_REPLAY_FLAG");
  AddConstant(constants, GSS_C_SEQUENCE_FLAG, "C_SEQUENCE_FLAG");
  AddConstant(constants, GSS_C_CONF_FLAG, "C_CONF_FLAG");
  AddConstant(constants, GSS_C_INTEG_FLAG, "C_INTEG_FLAG");
  AddConstant(constants, GSS_C_ANON_FLAG, "C_ANON_FLAG");
  AddConstant(constants, GSS_C_PROT_READY_FLAG, "C_PROT_READY_FLAG");
  AddConstant(constants, GSS_C_TRANS_FLAG, "C_TRANS_FLAG");
  AddConstant(constants, GSS_C_DELEG_POLICY_FLAG, "C_DELEG_POLICY_FLAG");

  AddConstant(constants, GSS_C_BOTH, "C_BOTH");
  AddConstant(constants, GSS_C_INITIATE, "C_INITIATE");
  AddConstant(constants, GSS_C_ACCEPT, "C_ACCEPT");

  AddConstant(constants, GSS_C_GSS_CODE, "C_GSS_CODE");
  AddConstant(constants, GSS_C_MECH_CODE, "C_MECH_CODE");

  AddConstant(constants, GSS_C_QOP_DEFAULT, "C_QOP_DEFAULT");

  AddConstant(constants,
              GSS_S_CALL_INACCESSIBLE_READ, "S_CALL_INACCESSIBLE_READ");
  AddConstant(constants,
              GSS_S_CALL_INACCESSIBLE_WRITE, "S_CALL_INACCESSIBLE_WRITE");
  AddConstant(constants,
              GSS_S_CALL_BAD_STRUCTURE, "S_CALL_BAD_STRUCTURE");

  AddConstant(constants, GSS_S_BAD_MECH, "S_BAD_MECH");
  AddConstant(constants, GSS_S_BAD_NAME, "S_BAD_NAME");
  AddConstant(constants, GSS_S_BAD_NAMETYPE, "S_BAD_NAMETYPE");
  AddConstant(constants, GSS_S_BAD_BINDINGS, "S_BAD_BINDINGS");
  AddConstant(constants, GSS_S_BAD_STATUS, "S_BAD_STATUS");
  AddConstant(constants, GSS_S_BAD_SIG, "S_BAD_SIG");
  AddConstant(constants, GSS_S_NO_CRED, "S_NO_CRED");
  AddConstant(constants, GSS_S_NO_CONTEXT, "S_NO_CONTEXT");
  AddConstant(constants, GSS_S_DEFECTIVE_TOKEN, "S_DEFECTIVE_TOKEN");
  AddConstant(constants, GSS_S_DEFECTIVE_CREDENTIAL, "S_DEFECTIVE_CREDENTIAL");
  AddConstant(constants, GSS_S_CREDENTIALS_EXPIRED, "S_CREDENTIALS_EXPIRED");
  AddConstant(constants, GSS_S_CONTEXT_EXPIRED, "S_CONTEXT_EXPIRED");
  AddConstant(constants, GSS_S_FAILURE, "S_FAILURE");
  AddConstant(constants, GSS_S_BAD_QOP, "S_BAD_QOP");
  AddConstant(constants, GSS_S_UNAUTHORIZED, "S_UNAUTHORIZED");
  AddConstant(constants, GSS_S_DUPLICATE_ELEMENT, "S_DUPLICATE_ELEMENT");
  AddConstant(constants, GSS_S_NAME_NOT_MN, "S_NAME_NOT_MN");
  AddConstant(constants, GSS_S_BAD_MECH_ATTR, "S_BAD_MECH_ATTR");

  AddConstant(constants, GSS_S_CONTINUE_NEEDED, "S_CONTINUE_NEEDED");
  AddConstant(constants, GSS_S_DUPLICATE_TOKEN, "S_DUPLICATE_TOKEN");
  AddConstant(constants, GSS_S_OLD_TOKEN, "S_OLD_TOKEN");
  AddConstant(constants, GSS_S_UNSEQ_TOKEN, "S_UNSEQ_TOKEN");
  AddConstant(constants, GSS_S_GAP_TOKEN, "S_GAP_TOKEN");

  NameInit(exports);
}

}  // namespace node_gss

NODE_MODULE(gss, node_gss::Init)

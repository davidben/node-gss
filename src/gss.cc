#include <node.h>
#include <v8.h>

#include <gssapi/gssapi.h>

namespace node_gss {

// This code assumes GSS_C_NO_CONTEXT, etc., are all 0. It seems C++
// doesn't like passing them as template arguments.
template <class T, OM_uint32 (*Deleter)(OM_uint32*, T*)>
class GssHandle : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports, const char* ctor) {
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(New);
    tpl->SetClassName(v8::String::NewSymbol(ctor));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    exports->Set(v8::String::NewSymbol(ctor), tpl->GetFunction());
  }

  const T& get() const { return gss_obj_; }
  T& get() { return gss_obj_; }

 private:
  GssHandle() : gss_obj_(0) { }
  ~GssHandle() {
    if (gss_obj_) {
      OM_uint32 dummy;
      // TODO(davidben): Check output and print something to stderr?
      // Shouldn't actually happen, but...
      Deleter(&dummy, &gss_obj_);
    }
  }

  static v8::Handle<v8::Value> New(const v8::Arguments& args) {
    v8::HandleScope scope;
    GssHandle* obj = new GssHandle();
    obj->Wrap(args.This());

    return args.This();
  }

  T gss_obj_;
};

OM_uint32
ContextDeleter(OM_uint32* minor_status, gss_ctx_id_t* ctx) {
  return gss_delete_sec_context(minor_status, ctx, GSS_C_NO_BUFFER);
}

typedef GssHandle<gss_ctx_id_t, ContextDeleter> ContextHandle;
typedef GssHandle<gss_cred_id_t, gss_release_cred> CredHandle;
typedef GssHandle<gss_name_t, gss_release_name> NameHandle;
typedef GssHandle<gss_OID_set, gss_release_oid_set> OidSetHandle;
typedef GssHandle<gss_OID, gss_release_oid> OidHandle;

void Init(v8::Handle<v8::Object> exports) {
  ContextHandle::Init(exports, "ContextHandle");
  CredHandle::Init(exports, "CredHandle");
  NameHandle::Init(exports, "NameHandle");
  OidSetHandle::Init(exports, "OidSetHandle");
  OidHandle::Init(exports, "OidHandle");
}

}  // namespace node_gss

NODE_MODULE(gss, node_gss::Init)

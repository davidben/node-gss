#ifndef NODE_GSS_GSS_H_
#define NODE_GSS_GSS_H_

#include <node.h>
#include <v8.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_krb5.h>

namespace node_gss {

template <class T> OM_uint32
NoopDeleter(OM_uint32* minor_status, T* obj) {
  return GSS_S_COMPLETE;
}

// This code assumes GSS_C_NO_CONTEXT, etc., are all 0. It seems C++
// doesn't like passing them as template arguments.
template <class T, OM_uint32 (*Deleter)(OM_uint32*, T*) = NoopDeleter<T> >
class GssHandle : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports, const char* ctor) {
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(New);
    tpl->SetClassName(v8::String::NewSymbol(ctor));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor_ = v8::Persistent<v8::Function>::New(tpl->GetFunction());
    exports->Set(v8::String::NewSymbol(ctor), constructor_);
  }

  static GssHandle* New(T gss_obj = NULL) {
    v8::HandleScope scope;
    v8::Local<v8::Object> obj = constructor_->NewInstance();
    if (obj.IsEmpty()) return NULL;
    GssHandle* handle = ObjectWrap::Unwrap<GssHandle>(obj);
    handle->gss_obj_ = gss_obj;
    return handle;
  }

  const T& get() const { return gss_obj_; }
  T& get() { return gss_obj_; }

 private:
  static v8::Persistent<v8::Function> constructor_;

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
template <class T, OM_uint32 (*Deleter)(OM_uint32*, T*)>
v8::Persistent<v8::Function> GssHandle<T, Deleter>::constructor_;


OM_uint32
ContextDeleter(OM_uint32* minor_status, gss_ctx_id_t* ctx) {
  return gss_delete_sec_context(minor_status, ctx, GSS_C_NO_BUFFER);
}

typedef GssHandle<gss_ctx_id_t, ContextDeleter> ContextHandle;
typedef GssHandle<gss_cred_id_t, gss_release_cred> CredHandle;
typedef GssHandle<gss_name_t, gss_release_name> NameHandle;

// TODO(davidben): Subclass or wrap or something so we can provide a
// equals() method for OIDs.
typedef GssHandle<gss_OID> OidHandle;

}  // namespace node_gss

#endif  // NODE_GSS_GSS_H_

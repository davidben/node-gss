#ifndef NODE_GSS_GSS_H_
#define NODE_GSS_GSS_H_

#include <node.h>
#include <node_buffer.h>
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

    exports->Set(v8::String::NewSymbol(ctor), tpl->GetFunction());
    template_ = v8::Persistent<v8::FunctionTemplate>::New(tpl);
  }

  static GssHandle* New(T gss_obj = NULL) {
    v8::HandleScope scope;
    v8::Local<v8::Object> obj = template_->GetFunction()->NewInstance();
    if (obj.IsEmpty()) return NULL;
    GssHandle* handle = ObjectWrap::Unwrap<GssHandle>(obj);
    handle->gss_obj_ = gss_obj;
    return handle;
  }

  static bool HasInstance(v8::Handle<v8::Value> value) {
    if (!value->IsObject()) return false;
    return template_->HasInstance(value->ToObject());
  }

  const T& get() const { return gss_obj_; }
  T& get() { return gss_obj_; }

 private:
  static v8::Persistent<v8::FunctionTemplate> template_;

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
v8::Persistent<v8::FunctionTemplate> GssHandle<T, Deleter>::template_;

inline OM_uint32
ContextDeleter(OM_uint32* minor_status, gss_ctx_id_t* ctx) {
  return gss_delete_sec_context(minor_status, ctx, GSS_C_NO_BUFFER);
}
inline OM_uint32
OwnOidDeleter(OM_uint32* minor_status, gss_OID* oid) {
  delete[] static_cast<char*>((*oid)->elements);
  *oid = GSS_C_NO_OID;
  return GSS_S_COMPLETE;
}

typedef GssHandle<gss_ctx_id_t, ContextDeleter> ContextHandle;
typedef GssHandle<gss_cred_id_t, gss_release_cred> CredHandle;
typedef GssHandle<gss_name_t, gss_release_name> NameHandle;

// OIDs in GSSAPI are really annoying. Some are owned by the library
// and static. But the ones in a gss_OID_set are dynamic. For sanity,
// make a copy of those. In that case, we own them.
typedef GssHandle<gss_OID> OidHandle;
typedef GssHandle<gss_OID, OwnOidDeleter> OwnOidHandle;

// A bunch of macros for extracting types.
#define BUFFER_ARGUMENT(index, name)                                    \
  gss_buffer_desc name;                                                 \
  if (!NodeBufferAsGssBuffer(args[index], &name)) {                     \
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(        \
      "Expected Buffer as argument " #index)));                         \
    return scope.Close(v8::Undefined());                                \
  }
// This one is all kinds of special...
#define OID_ARGUMENT(index, name)                                       \
  gss_OID name;                                                         \
  if (OidHandle::HasInstance(args[index])) {                            \
    name = node::ObjectWrap::Unwrap<OidHandle>(                         \
        args[index]->ToObject())->get();                                \
  } else if (OwnOidHandle::HasInstance(args[index])) {                  \
    name = node::ObjectWrap::Unwrap<OwnOidHandle>(                      \
        args[index]->ToObject())->get();                                \
  } else if (args[index]->IsNull() || args[index]->IsUndefined()) {     \
    name = GSS_C_NO_OID;                                                \
  } else  {                                                             \
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(        \
        "Expected OID as argument " #index)));                          \
    return scope.Close(v8::Undefined());                                \
  }
#define HANDLE_ARGUMENT(index, type, name)                              \
  if (!type::HasInstance(args[index])) {                                \
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(        \
        "Expected " #type " as argument " #index)));                    \
    return scope.Close(v8::Undefined());                                \
  }                                                                     \
  type* name = node::ObjectWrap::Unwrap<type>(args[index]->ToObject());


bool NodeBufferAsGssBuffer(v8::Handle<v8::Value> value, gss_buffer_t out);

// Various initialization functions. Groups by how functions are
// organized in Section 2 of RFC2744
void NameInit(v8::Handle<v8::Object> exports);
void MiscInit(v8::Handle<v8::Object> exports);

}  // namespace node_gss

#endif  // NODE_GSS_GSS_H_

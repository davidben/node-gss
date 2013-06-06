#include "gss.h"

#include <string.h>

namespace node_gss {

bool ObjectToOidSet(v8::Handle<v8::Value> value, scoped_OID_set* out) {
  if (!value->IsArray())
    return false;
  v8::Handle<v8::Array> array = value.As<v8::Array>();
  for (unsigned i = 0; i < array->Length(); i++) {
    v8::Local<v8::Value> elem = array->Get(i);
    // Blegh.
    if (OidHandle::HasInstance(elem)) {
      out->Append(node::ObjectWrap::Unwrap<OidHandle>(
          elem->ToObject())->get());
    } else if (OwnOidHandle::HasInstance(elem)) {
      out->Append(node::ObjectWrap::Unwrap<OwnOidHandle>(
          elem->ToObject())->get());
    } else {
      return false;
    }
  }
  return true;
}

v8::Local<v8::Array> OidSetToArray(gss_OID_set oid_set) {
  if (!oid_set)
    return v8::Array::New(0);
  v8::Local<v8::Array> ret = v8::Array::New(oid_set->count);
  for (unsigned i = 0; i < oid_set->count; i++) {
    gss_OID oid = new gss_OID_desc;
    oid->length = oid_set->elements[i].length;
    oid->elements = new char[oid->length];
    memcpy(oid->elements, oid_set->elements[i].elements, oid->length);
    ret->Set(i, OwnOidHandle::New(oid)->handle_);
  }
  return ret;
}

bool NodeBufferAsGssBuffer(v8::Handle<v8::Value> value, gss_buffer_t out) {
  if (!node::Buffer::HasInstance(value))
    return false;
  v8::Local<v8::Object> obj = value->ToObject();
  out->value = node::Buffer::Data(obj);
  out->length = node::Buffer::Length(obj);
  return true;
}

}  // namespace node_gss

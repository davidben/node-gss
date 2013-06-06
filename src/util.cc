#include "gss.h"

namespace node_gss {

bool NodeBufferAsGssBuffer(v8::Handle<v8::Value> value, gss_buffer_t out) {
  if (!node::Buffer::HasInstance(value))
    return false;
  v8::Local<v8::Object> obj = value->ToObject();
  out->value = node::Buffer::Data(obj);
  out->length = node::Buffer::Length(obj);
  return true;
}

}  // namespace node_gss

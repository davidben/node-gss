#include "gss.h"

namespace node_gss {

v8::Handle<v8::Value> DisplayStatus(const v8::Arguments& args) {
  v8::HandleScope scope;

  if (args.Length() < 3) {
    v8::ThrowException(v8::Exception::TypeError(v8::String::New(
        "Wrong number of arguments")));
    return scope.Close(v8::Undefined());
  }

  OM_uint32 status_value = args[0]->ToUint32()->Value();
  int status_type = args[1]->ToInteger()->Value();
  OID_ARGUMENT(2, mech_type);

  // Okay, what is this monstrousity of an API? I'll just return the
  // list, argh.
   v8::Local<v8::Array> ret = v8::Array::New();
  OM_uint32 minor;
  gss_buffer_desc status_string;
  OM_uint32 message_context = 0;
  const int kMaxDisplayIterations = 8;
  int iter = 0;
  do {
    gss_display_status (
        &minor,
        status_value,
        status_type,
        mech_type,
        &message_context,
        &status_string);

    ret->Set(ret->Length(), node::Buffer::New((const char*)status_string.value,
                                              status_string.length)->handle_);
    gss_release_buffer(&minor, &status_string);

    // Take a leaf from Chromium's use of this function and have a
    // maximum iteration count.
    if (++iter >= kMaxDisplayIterations)
      break;
   } while (message_context != 0);

  return scope.Close(ret);
}

void MiscInit(v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("displayStatus"),
               v8::FunctionTemplate::New(DisplayStatus)->GetFunction());
}

}  // namespace node_gss

var util = require('util');

var internal = require('./build/Release/gss');

// Export all the constants, as-is.
Object.keys(internal.constants).forEach(function(key) {
  exports[key] = internal.constants[key];
});

var GSS_C_CALLING_ERROR_OFFSET = 24;
var GSS_C_ROUTINE_ERROR_OFFSET = 16;
var GSS_C_SUPPLEMENTARY_OFFSET = 0;
var GSS_C_CALLING_ERROR_MASK = 0377;
var GSS_C_ROUTINE_ERROR_MASK = 0377;
var GSS_C_SUPPLEMENTARY_MASK = 0177777;

function GSS_CALLING_ERROR(x) {
  return (x & (GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET)) >>> 0;
}
function GSS_ROUTINE_ERROR(x) {
  return (x & (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET)) >>> 0;
}
function GSS_SUPPLEMENTARY_INFO(x) {
  return (x & (GSS_C_SUPPLEMENTARY_MASK << GSS_C_SUPPLEMENTARY_OFFSET)) >>> 0;
}
function GSS_ERROR(x) {
  return (x & ((GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET) |
               (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET))) >>> 0;
}

function GSSError(major, minor) {
  // Apparently you're supposed to do this?
  Error.captureStackTrace(this, GSSError);
  this.major = major;
  this.minor = minor;
}
util.inherits(GSSError, Error);
GSSError.prototype.callingError = function() {
  return GSS_CALLING_ERROR(this.major);
}
GSSError.prototype.routineError = function() {
  return GSS_ROUTINE_ERROR(this.major);
}
GSSError.prototype.supplementaryInfo = function() {
  return GSS_SUPPLEMENTARY_INFO(this.major);
}
GSSError.prototype.toString = function() {
  // TODO(davidben): Wrap gss_display_status and give human-readable
  // status messages.
  return "GSS error: " + this.major + ", " + this.minor;
};
exports.Error = GSSError;

function gssCall(fn) {
  var args = [].slice.call(arguments, 1);
  var ret = fn.apply(internal, args);
  if (GSS_ERROR(ret.major)) {
    throw new GSSError(ret.major, ret.minor);
  }
  return ret;
}

function Name(handle) {
  this.handle_ = handle;
}
Name.prototype.toString = function() {
  // TODO(davidben): Care about the OID? OIDs are pretty useless right
  // now.
  return gssCall(internal.displayName, this.handle_).buffer.toString('utf8');
};
Name.prototype.exportName = function() {
  return gssCall(internal.exportName, this.handle_).buffer;
};
Name.prototype.compareName = function(name) {
  return gssCall(internal.compareName,
                 this.handle_, name.handle_).nameEqual != 0;
};
Name.prototype.canonicalize = function(oid) {
  var handle = new internal.NameHandle();
  gssCall(internal.canonicalizeName, this.handle_, oid, handle);
  return new Name(handle);
};

exports.importName = function(buffer, oid) {
  if (typeof buffer === "string")
    buffer = new Buffer(buffer, "utf8");
  var handle = new internal.NameHandle();
  gssCall(internal.importName, buffer, oid, handle);
  return new Name(handle);
};

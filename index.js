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

function GSSError(major, minor, mechanism) {
  // Apparently you're supposed to do this?
  Error.captureStackTrace(this, GSSError);
  this.major = major;
  this.minor = minor;
  this.mechanism = mechanism;
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
  var statuses;
  if (this.major != exports.S_FAILURE) {
    statuses = internal.displayStatus(this.major, exports.C_GSS_CODE,
                                      this.mechanism);
  } else {
    statuses = [];
  }
  if (this.minor) {
    statuses = statuses.concat(
      internal.displayStatus(this.minor, exports.C_MECH_CODE, this.mechanism));
  }
  statuses = statuses.map(function(status) {
    return status.toString('utf8');
  });
  return 'GSS error: ' + statuses.join('; ');
};
exports.Error = GSSError;

function gssCall(mech, fn) {
  var args = [].slice.call(arguments, 2);
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
  return gssCall(null, internal.displayName,
                 this.handle_).buffer.toString('utf8');
};
Name.prototype.exportName = function() {
  return gssCall(null, internal.exportName, this.handle_).buffer;
};
Name.prototype.compareName = function(name) {
  return gssCall(null, internal.compareName,
                 this.handle_, name.handle_).nameEqual != 0;
};
Name.prototype.canonicalize = function(oid) {
  var handle = new internal.NameHandle();
  gssCall(null, internal.canonicalizeName, this.handle_, oid, handle);
  return new Name(handle);
};

exports.importName = function(buffer, oid) {
  if (typeof buffer === "string")
    buffer = new Buffer(buffer, "utf8");
  var handle = new internal.NameHandle();
  gssCall(null, internal.importName, buffer, oid, handle);
  return new Name(handle);
};

function Credential(handle) {
  this.handle_ = handle;
}

exports.acquireCredential = function(name,
                                     timeReq,
                                     desiredMechs,
                                     credUsage) {
  var nameHandle = name ? name.handle_ : new internal.NameHandle();
  var handle = new internal.CredHandle();
  var ret = gssCall(null, internal.acquireCred,
                    nameHandle, timeReq, desiredMechs, credUsage, handle);
  // TODO(davidben): Actually do something with the return value? We
  // went through all this trouble to plumb the the gss_OID_set
  // through...
  return new Credential(handle);
};

function Context(credential) {
  this.credHandle_ =
    credential ? credential.handle_ : new internal.CredHandle();
  this.handle_ = new internal.ContextHandle();
  this.established_ = false;
  this.retFlags_ = 0;
}
Context.prototype.isEstablished = function() {
  return this.established_;
};
Context.prototype.flags = function() {
  return this.retFlags_;
};
Context.prototype.wrap = function(input, confidential, qop) {
  if (qop == undefined)
    qop = exports.C_QOP_DEFAULT;
  var ret = gssCall(null, internal.wrap,
                    this.handle_, confidential ? 1 : 0, qop, input);
  // Just require the bits we requested. Not getting them is silly.
  if (confidential && !ret.confState)
    throw new Error('Did not provide confidentiality');
  return ret.output;
};
Context.prototype.unwrap = function(input) {
  var ret = gssCall(null, internal.unwrap, this.handle_, input);
  return {
    output: ret.output,
    confidential: ret.confState,
    qop: ret.qopState
  };
};

function AcceptContext(credential, opts) {
  Context.call(this, credential);
  // TODO(davidben): Pull channel bindings out of opts.
}
util.inherits(AcceptContext, Context);
AcceptContext.prototype.acceptSecContext = function(token) {
  var srcNameHandle = new internal.NameHandle();
  // TODO(davidben): Do something with this.
  var delegatedCredHandle = new internal.CredHandle();
  var ret = gssCall(null, internal.acceptSecContext,
                    this.handle_, this.credHandle_, token, null, srcNameHandle,
                    delegatedCredHandle);
  this.retFlags_ = ret.retFlags;
  if (!(ret.major & exports.S_CONTINUE_NEEDED)) {
    this.established_ = true;
    this.srcName_ = new Name(srcNameHandle);
  }
  return ret.outputToken;
}
AcceptContext.prototype.srcName = function() {
  return this.srcName_;
};

exports.createAcceptor = function(credential, opts) {
  return new AcceptContext(credential, opts);
};

function InitContext(credential, target, opts) {
  Context.call(this, credential);
  opts = opts || { };
  this.target_ = target;
  this.flags_ = opts.flags;
  this.mechanism_ = opts.mechanism || new internal.OidHandle();
  this.lifetime_ = opts.lifetime || 0;
}
util.inherits(InitContext, Context);
InitContext.prototype.initSecContext = function(token) {
  // C++ code can't deal with NULL buffers, but an empty buffer works
  // too, according to RFC.
  token = token || new Buffer(0);
  var ret = gssCall(null, internal.initSecContext,
                    this.credHandle_, this.handle_, this.target_.handle_,
                    this.mechanism_, this.flags_, this.lifetime_,
                    null, token);
  this.actualMechanism_ = ret.actualMechType;
  this.retFlags_ = ret.retFlags;
  if (!(ret.major & exports.S_CONTINUE_NEEDED)) {
    this.established_ = true;
  }
  return ret.outputToken;

}

exports.createInitiator = function(credential, target, opts) {
  return new InitContext(credential, target, opts);
};

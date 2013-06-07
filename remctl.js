var Buffer = require('buffer').Buffer;
var net = require('net');
var Q = require('q');

var gss = require('./index.js');

if (process.argv.length < 4) {
  console.error('USAGE: %s %s HOST COMMAND..', process.argv[0], process.argv[1]);
  process.exit();
}

var TOKEN_NOOP = 0x01;
var TOKEN_CONTEXT = 0x02;
var TOKEN_DATA = 0x04;
var TOKEN_CONTEXT_NEXT = 0x10;
var TOKEN_PROTOCOL = 0x40;

var MESSAGE_COMMAND = 1;
var MESSAGE_QUIT = 2;
var MESSAGE_OUTPUT = 3;
var MESSAGE_STATUS = 4;
var MESSAGE_ERROR = 5;
var MESSAGE_VERSION = 6;
var MESSAGE_NOOP = 7;

function formatPacket(flags, data) {
  var buf = new Buffer(1 + 4 + data.length);
  buf.writeUInt8(flags, 0);
  buf.writeUInt32BE(data.length, 1);
  data.copy(buf, 5);
  return buf;
}

function readFromSocket(socket, size) {
  var deferred = Q.defer();
  function tryRead() {
    var buf = socket.read(size);
    if (buf == null) {
      socket.once('readable', tryRead);
      return;
    }
    deferred.resolve(buf);
  }
  tryRead();
  return deferred.promise;
}

function readPacket(socket) {
  return readFromSocket(socket, 5).then(function(buf) {
    var flags = buf.readUInt8(0);
    var length = buf.readUInt32BE(1);
    return readFromSocket(socket, length).then(function(buf) {
      return [flags, buf];
    });
  });
}

var host = process.argv[2];
var argv = process.argv.slice(3);

var socket = net.connect({host: host, port: 4373});

var name = gss.importName("host@" + host, gss.C_NT_HOSTBASED_SERVICE);
var context = gss.createInitiator(null, name, {
  flags: gss.C_MUTUAL_FLAG | gss.C_CONF_FLAG |
    gss.C_INTEG_FLAG | gss.C_REPLAY_FLAG | gss.C_SEQUENCE_FLAG,
  mechanism: gss.KRB5_MECHANISM,
});

socket.write(formatPacket(TOKEN_NOOP|TOKEN_CONTEXT_NEXT|TOKEN_PROTOCOL,
                          new Buffer(0)));
function initializeContext(token) {
  var output = context.initSecContext(token);
  if (output.length)
    socket.write(formatPacket(TOKEN_CONTEXT|TOKEN_PROTOCOL, output));
  if (context.isEstablished()) {
    return Q();
  } else {
    return readPacket(socket).then(function(ret) {
      var flags = ret[0], buf = ret[1];
      if (flags != (TOKEN_CONTEXT|TOKEN_PROTOCOL))
        throw "Bad flags!";
      return initializeContext(buf);
    });
  }
}

initializeContext(null).then(function() {
  // Send the command packet:
  var argvBufs = argv.map(function(arg) { return new Buffer(arg, 'utf8'); });
  var argLen = 0;
  argvBufs.forEach(function(arg) {
    argLen += 4 + arg.length;
  });
  var plain = new Buffer(1 + 1 + 1 + 1 + 4 + argLen);
  plain.writeUInt8(2, 0);  // protocol version
  plain.writeUInt8(MESSAGE_COMMAND, 1);
  plain.writeUInt8(0, 2);  // keepalive
  plain.writeUInt8(0, 3);  // continue
  plain.writeUInt32BE(argv.length, 4);
  var offset = 8;
  argvBufs.forEach(function(arg) {
    plain.writeUInt32BE(arg.length, offset);
    offset += 4;
    arg.copy(plain, offset);
    offset += arg.length;
  });

  var wrapped = context.wrap(plain, true);
  socket.write(formatPacket(TOKEN_DATA|TOKEN_PROTOCOL, wrapped));

  function readUntilDone() {
    return readPacket(socket).then(function(ret) {
      var flags = ret[0], buf = ret[1];
      if (flags != (TOKEN_DATA|TOKEN_PROTOCOL))
        throw "Bad flags!";
      var unwrapped = context.unwrap(buf).output;
      if (unwrapped.length < 2)
        throw "Bad packet!";
      var version = unwrapped.readUInt8(0);
      var command = unwrapped.readUInt8(1);
      if (command == MESSAGE_OUTPUT) {
        var stream = unwrapped.readUInt8(2);
        var length = unwrapped.readUInt32BE(3);
        if (stream == 1) {
          process.stdout.write(unwrapped.slice(7, 7 + length));
        } else if (stream == 2) {
          process.stderr.write(unwrapped.slice(7, 7 + length));
        } else {
          throw "Bad stream";
        }
      } else if (command == MESSAGE_STATUS) {
        var status = unwrapped.readUInt8(2);
        return Q(status);
      } else if (command == MESSAGE_ERROR) {
        var code = unwrapped.readUInt32BE(2);
        var length = unwrapped.readUInt32BE(6);
        var msg = unwrapped.slice(10, 10 + length);
        throw Error(msg.toString('utf8'));
      }
      return readUntilDone();
    });
  }
  return readUntilDone();
}).done();

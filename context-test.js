var Buffer = require('buffer').Buffer;
var readline = require('readline');

var gss = require('./index.js');

var creds = gss.acquireCredential(null, 0,
                                  [gss.KRB5_MECHANISM], gss.C_INITIATE);
var zsrName = gss.importName('host@zsr.mit.edu', gss.C_NT_HOSTBASED_SERVICE);
var context = gss.createInitiator(creds, zsrName, {
  flags: gss.C_MUTUAL_FLAG | gss.C_CONF_FLAG |
    gss.C_INTEG_FLAG | gss.C_REPLAY_FLAG | gss.C_SEQUENCE_FLAG,
  mechanism: gss.KRB5_MECHANISM,
});

var rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

function step(token) {
  console.log(context.initSecContext(token).toString('base64'));
  console.log('isEstablished', context.isEstablished());
  if (context.isEstablished()) {
    console.log('flags', context.flags());
    rl.close();
  } else {
    rl.question('next token? ', function(answer) {
      step(new Buffer(answer, 'base64'));
    });
  }
};
step(null);
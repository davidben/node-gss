var gss = require('./index.js');

console.dir(gss);

var name = gss.importName("host@linerva.mit.edu",
                          gss.C_NT_HOSTBASED_SERVICE);
console.log(name.toString());
var canonicalized = name.canonicalize(gss.KRB5_MECHANISM);
console.log(canonicalized.toString());
console.log(name.compareName(canonicalized));

var exported = canonicalized.exportName();
var imported = gss.importName(exported, gss.C_NT_EXPORT_NAME);
console.log(imported.toString());

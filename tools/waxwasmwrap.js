var _wax_mem={};
var _wax_is_node = false;
if (typeof window == "undefined"){
  _wax_is_node = true;
  module.exports = WAXWASMWRAP;
}
function WAXWASMWRAP(wasmpath,onloaded){
  let prom;
  function getStr(offset,length){
    var bytes = new Uint8Array(_wax_mem[wasmpath].buffer, offset, length);
    var str = new TextDecoder('utf8').decode(bytes);
    return str;
  }
  let imports = {
    console:{
      log : function(offset, length){
        console.log(getStr(offset,length));
      }
    },
    Math,
    debug:{logi32: function(x){console.log(x);}},
  }
  if (!_wax_is_node){
    prom = fetch(wasmpath).then(response=>response.arrayBuffer()).then(
      bytes=>WebAssembly.instantiate(bytes,imports)
    );
  }else{
    const fs = require('fs');
    const buf = fs.readFileSync(wasmpath);
    prom = WebAssembly.instantiate(new Uint8Array(buf),imports);
  }
  prom.then(results => {
    let lib = results.instance.exports;
    _wax_mem[wasmpath] = lib.mem;
    onloaded(lib);
  });
}
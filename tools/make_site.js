const fs = require("fs");
const icon = fs.readFileSync("../assets/wax.svg").toString();

var exampleNames = fs.readdirSync('../examples').filter(x=>x.endsWith(".wax"));
exampleNames.sort();
var exampleTexts = exampleNames.map(x=>fs.readFileSync('../examples/'+x).toString())
var examples = {};
exampleNames.map((x,i)=>examples[x]=exampleTexts[i]);

var lblselstyle = `position:absolute;font-size:11px;color:gray`

var html = `
<style>
  body{
    margin:0px;
    overflow:hidden;
    background:rgb(30,30,35);
    font-family:sans-serif;
    color:white;
  }
  select, button{
    background:none;
    -webkit-appearance:none;
    font-size:12px;
    color:silver;
    border: 1px solid gray;
    border-radius:2px;
    padding:1px 5px 1px 5px;
    margin:0px;
    outline: none;
    cursor:pointer;
  }
  select:hover, button:hover{
    outline: none;
    background:rgba(255,255,255,0.1);
  }
  div::-webkit-scrollbar {
    display: none;
  }
  div {
    -ms-overflow-style: none;
    scrollbar-width: none;
  }
</style>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/codemirror.min.css">
<link rel="stylesheet" href="https://codemirror.net/theme/dracula.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/codemirror.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/mode/simple.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/edit/matchbrackets.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.58.1/addon/comment/comment.min.js"></script>
<script src="https://codemirror.net/mode/javascript/javascript.js"></script>
<script src="https://codemirror.net/mode/clike/clike.js"></script>
<script src="https://codemirror.net/mode/python/python.js"></script>
<script src="https://codemirror.net/mode/swift/swift.js"></script>

<script src="https://unpkg.com/typescript@4.0.3/lib/typescriptServices.js"></script>

<script src="waxc.js"></script>
<style>.CodeMirror { height: 100%; }</style>
<body>
  <div id="hd" style="position:absolute;left:0px;top:0px;width:100%;height:50px;overflow-x:scroll;overflow-y:hidden">
    <div id="hdin" style="width:2000px">
      <div style="position:absolute;left:9px;top:9px;">
        ${icon.replace(/width=".*?"/,'width="32"').replace(/height=".*?"/,'height="32"')}
      </div>
      <div style="position:absolute;left:50px;top:8px;font-size:16px;font-weight:bold;">
        wax compiler
      </div>
      <div style="position:absolute;left:50px;top:27px;font-size:11px;color:gray">
        A tiny language designed to transpile to other languages.
      </div>

      <div style="position:absolute;left:365px;top:27px;${lblselstyle}">
        Example:
      </div>
      <select id="selexam" style="position:absolute;left:420px;top:25px;">
        ${
          Object.keys(examples).map(x=>`<option>${x}</option>`).join("")
        }
      </select>

      <div style="position:absolute;left:550px;top:27px;${lblselstyle}">
        Transpile&nbsp;to:
      </div>
      <select id="seltarg" style="position:absolute;left:620px;top:25px;" value="ts">
        <option value="c">C</option>
        <option value="cpp">C++</option>
        <option value="java">Java</option>
        <option selected="selected" value="ts">TypeScript</option>
        <option value="py">Python</option>
        <option value="cs">C#</option>
        <option value="swift">Swift</option>
        <option value="json">JSON</option>
        <!--<option value="ast">Syntax Tree</option>-->
      </select>


      <div style="position:absolute;left:715px;top:27px;${lblselstyle}">
        Realtime:
      </div>
      <select id="selreal" style="position:absolute;left:770px;top:25px;">
        <option>yes</option>
        <option>no</option>
      </select>

      <button id="butcomp" style="position:absolute;left:850px;top:25px;">
        Compile
      </button>

      <button id="butrunc" style="position:absolute;left:910px;top:25px;">
        Run
      </button>

    </div>
  </div>
  <div id="cl" style="position:absolute;left:0px;top:50px;width:50%;height:calc(100% - 50px)">
    
  </div>
  <div id="cr" style="position:absolute;left:50%;top:50px;width:50%;height:calc(100% - 50px)">
    
  </div>
</body>
<script>
  var examples = ${JSON.stringify(examples)}
</script>
`;

function main(){

  CodeMirror.defineSimpleMode("wax", {
    meta:{
      lineComment: ';',
      comment: ';',
    },
    start: [
      {regex: /"(?:[^\\]|\\.)*?(?:"|$)/smi, token: "string"},
      {regex: /(?:return|result|insert|remove|extern|param|local|while|alloc|slice|print|break|func|then|else|call|cast|free|null|get|set|let|for|asm|if|do)\b/,
       token: "keyword"},
      {regex: /(?:@include|@if|@define|@pragma)\b/,
       token: "def"},
      {regex: /(?:int|float|arr|vec|struct|map)\b/,
       token: "type"},
      {regex: /0x[a-f\d]+|[-+]?(?:\.\d+|\d+\.?\d*)(?:e[-+]?\d+)?/i,
       token: "number"},
      {regex: /;.*/, token: "comment"},
      {regex: /[-+\/*=<>!\?\&\|\^\%\~\#]+/, token: "keyword"},
      {regex: /[\{\[\(]/, indent: true, token: "bracket"},
      {regex: /[\}\]\)]/, dedent: true, token: "bracket"},
      {regex: /[a-z$][\w$]*/, token: "variable-2"},
    ],
  });

  var CML = CodeMirror(document.getElementById("cl"), {
    lineNumbers:true,
    matchBrackets: true,
    theme:"dracula",
    mode:  "wax",
    indentWithTabs: true,
    indentUnit: 4,
    extraKeys:{
      'Ctrl-/': 'toggleComment',
      'Cmd-/': 'toggleComment'
    }

  });
  window.CML = CML;

  var CMR = CodeMirror(document.getElementById("cr"), {
    lineNumbers:true,
    matchBrackets: true,
    theme:"dracula",
    mode:  "javascript",
    typescript:true,
  });
  window.CMR = CMR;

  CML.setSize(null,null);
  CMR.setSize(null,null);


  

  window.console_content = "";
  window.print = console.log;
  window.console.log = function(x){console_content+=x+"\n"}


  var seltarg = document.getElementById("seltarg");
  var selexam = document.getElementById("selexam");
  var selreal = document.getElementById("selreal");
  var butcomp = document.getElementById("butcomp");
  var butrunc = document.getElementById("butrunc");

  CML.on("change",function(e){
    if (selreal.value == "yes"){
      doCompile();
    }
  })

  butcomp.onclick = function(){
    doCompile();
  }
  butrunc.onclick = function(){
    doRun();
  }

  selexam.onchange = function(){
    CML.setValue(examples[selexam.value]);
    doCompile();
  }

  selexam.value = "turing.wax";
  selexam.onchange();

  seltarg.onchange = function(){
    doCompile();
  }

  function doCompile(){
    if (window.transpile){
      var out;
      var targ = seltarg.value;
      var mode = ({
          "ts":"javascript",
          "c":"text/x-csrc",
          "cpp":"text/x-c++src",
          "java":"text/x-java",
          "py":"text/x-python",
          "cs":"text/x-csharp",
          "swift":"text/x-swift",
          "json":"javascript",
        })[targ];
      // print(mode);
      try{
        out = transpile(targ,selexam.value,CML.getValue(),0);
        CMR.setOption("mode", mode);
        CMR.setOption("lineWrapping",false);
      }catch(e){
        out = console_content;
        CMR.setOption("mode", "text");
        CMR.setOption("lineWrapping",true);
      }
      console_content = "";
      CMR.setValue(out);
    }else{
      setTimeout(doCompile,1000);
    }
  }

  function doRun(){
    if (window.transpile){
      var tsCode = transpile('ts',selexam.value,CML.getValue(),0);
      var jsCode = window.ts.transpile(tsCode);
      console_content = "";
      eval(jsCode);
      CMR.setOption("mode", "text");
      CMR.setOption("lineWrapping",false);
      CMR.setValue(console_content);

    }else{
      setTimeout(doRun,1000);
    }
  }

  WAXC().then(function(waxc){
    window.transpile = waxc.cwrap('transpile', 'string', ['string','string','string','number'])
  })

}


html += `<script>

${main.toString()}
main();
</script>`



fs.writeFileSync("../site/index.html",html);


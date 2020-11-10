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
  option{
    background:rgb(30,30,35);
    color:silver;
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
<script src="https://codemirror.net/mode/lua/lua.js"></script>
<script src="https://codemirror.net/mode/wast/wast.js"></script>

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
          Object.keys(examples).map(x=>`<option value="${x}">${x}</option>`).join("")
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
        <option value="lua">Lua</option>
        <option value="wat">WebAssembly</option>
        <option value="json">JSON</option>
        <!--<option value="ast">Syntax Tree</option>-->
      </select>


      <div style="position:absolute;left:730px;top:27px;${lblselstyle}">
        Realtime:
      </div>
      <select id="selreal" style="position:absolute;left:785px;top:25px;">
        <option>yes</option>
        <option>no</option>
      </select>

      <button id="butcomp" style="position:absolute;left:865px;top:25px;">
        Compile
      </button>

      <button id="butrunc" style="position:absolute;left:925px;top:25px;">
        Run
      </button>

    </div>
  </div>
  <div id="cl" style="position:absolute;left:0px;top:50px;width:50%;height:calc(100% - 50px)">
    
  </div>
  <div id="cr" style="position:absolute;left:50%;top:50px;width:50%;height:calc(100% - 50px)">
  </div>
  <div id="crimg" style="position:absolute;left:calc(50% + 30px);top:80px;z-index:1000;filter:invert(100%);"></div>
  <div id="ghub" style="opacity:50%;position:absolute;right:9px;top:8px;cursor:pointer;z-index:1000">
<svg fill="white" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0" y="0" width="32" height="32" viewBox="0, 0, 32, 32">
    <path d="M15.999,-0 C7.163,-0 0,7.345 0,16.405 C0,23.653 4.584,29.802 10.942,31.972 C11.743,32.122 12.034,31.615 12.034,31.18 C12.034,30.792 12.021,29.759 12.013,28.391 C7.562,29.382 6.623,26.191 6.623,26.191 C5.895,24.296 4.846,23.791 4.846,23.791 C3.394,22.774 4.956,22.794 4.956,22.794 C6.562,22.91 7.407,24.485 7.407,24.485 C8.834,26.992 11.152,26.268 12.064,25.848 C12.209,24.788 12.622,24.065 13.079,23.655 C9.527,23.242 5.791,21.834 5.791,15.547 C5.791,13.757 6.415,12.292 7.438,11.145 C7.273,10.73 6.724,9.063 7.595,6.804 C7.595,6.804 8.938,6.363 11.995,8.486 C13.271,8.121 14.64,7.94 16,7.934 C17.359,7.94 18.728,8.121 20.006,8.486 C23.061,6.363 24.401,6.804 24.401,6.804 C25.275,9.063 24.726,10.73 24.561,11.145 C25.586,12.292 26.206,13.757 26.206,15.547 C26.206,21.85 22.465,23.236 18.9,23.642 C19.475,24.149 19.986,25.15 19.986,26.681 C19.986,28.873 19.967,30.643 19.967,31.18 C19.967,31.619 20.255,32.13 21.067,31.97 C27.42,29.796 32,23.651 32,16.405 C32,7.345 24.836,-0 15.999,-0"/>
</svg>
  </div>
</body>
<script>
var ghub = document.getElementById("ghub");
ghub.onmouseenter=function(){ghub.style.opacity=0.8}
ghub.onmouseleave=function(){ghub.style.opacity=0.5}
ghub.onclick=function(){document.location.href="https://github.com/LingDong-/wax"}
function ghubresize(){
  if (window.innerWidth<1024){
    ghub.style.top="58px";
  }else{
    ghub.style.top="8px";
  }
}
ghubresize();
window.addEventListener('resize', ghubresize);
</script>
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
      {regex: /"(?:[^\\]|\\.)*?(?:"|$)/mi, token: "string"},
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


  
  var seltarg = document.getElementById("seltarg");
  var selexam = document.getElementById("selexam");
  var selreal = document.getElementById("selreal");
  var butcomp = document.getElementById("butcomp");
  var butrunc = document.getElementById("butrunc");

  selexam.value = "turing.wax";

  try{
    if (window.location.href.split("?")[1]){
      var args = {};
      window.location.href.split("?")[1].split("&").map(x=>x.split("=")).map(x=>{args[x[0]]=x[1]})
      console.log(args);
      if (args.example){
        document.querySelector('#selexam [value="' + args.example + '"]').selected = true;
      }
      if (args.to){
        document.querySelector('#seltarg [value="' + args.to + '"]').selected = true;
      }
    }
  }catch(e){
    console.log("cannot parse url query")
  }


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

  seltarg.onchange = function(){
    doCompile();
  }


  selexam.onchange();

  window.console_content = "";
  window.print = console.log;
  window.console.log = function(x){console_content+=x+"\n"}




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
          "lua":"text/x-lua",
          "wat":"text/webassembly",
        })[targ];
      // print(mode);
      document.getElementById("crimg").innerHTML = "";
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
      
      if (console_content.startsWith("<svg")){
        document.getElementById("crimg").innerHTML = console_content;
        // CMR.setOption("lineWrapping",true);
      }
      // }else{
        CMR.setOption("lineWrapping",false);
      // }
      // }else{
        // document.getElementById("crimg").innerHTML = "";
        CMR.setOption("mode", "text");
        
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


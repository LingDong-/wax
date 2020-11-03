# script for running examples/
# compile waxc -> transpile example -> compile transpiled -> run compiled
import sys
import os
targ = sys.argv[1]
name = sys.argv[2]
perf = "";
try:
  sys.argv[3]
  perf = "time"
except:
  pass
cmd = f'cd ../; make text; make c; ./waxc examples/{name}.wax --{targ} out/{name}.{targ}; cd out;'
if (targ == 'ts'):
  cmd += f'tsc {name}.{targ}; {perf} node {name}.js;'
elif (targ == 'c'):
  cmd += f'gcc {name}.{targ}; {perf} ./a.out;'
elif (targ == 'cpp'):
  cmd += f'g++ {name}.{targ} -std=c++11; {perf} ./a.out;'
elif (targ == 'cs'):
  cmd += f'csc {name}.{targ}; {perf} mono {name}.exe;'
elif (targ == 'swift'):
  cmd += f'swiftc {name}.{targ}; {perf} ./{name};'
elif (targ == 'lua'):
  cmd += f'{perf} luajit {name}.{targ};'
elif (targ == 'wat'):
  cmd += f'./wat2wasm {name}.{targ}; ./wasm-opt {name}.wasm -O4 -o {name}.O4.wasm; {perf} node driver.js {name}.O4.wasm;'
elif (targ == 'java'):
  cmd += f'javac {name}.{targ}; {perf} java {name};'
elif (targ == 'py'):
  cmd += f'{perf} python3 {name}.{targ};'

cmd += "cd ../;";
os.system(cmd);
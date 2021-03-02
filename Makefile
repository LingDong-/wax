all: co emlib

_:


co: _
	gcc src/waxc.c -o waxc -O3 -std=c99 -pedantic -Wall

c: _
	gcc -g src/waxc.c -DEBUG -o waxc

em: _
	../emsdk/upstream/emscripten/emcc src/waxc.c -s WASM=0 -lnodefs.js -s NODERAWFS=1 -O3 -o waxc_cli.js --memory-init-file 0

emlib: _
	../emsdk/upstream/emscripten/emcc \
	src/waxc_lib.c -s WASM=0 -O3 -o site/waxc.js --memory-init-file 0 \
	-s EXPORTED_FUNCTIONS='["_transpile"]' \
	-s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
	-s MODULARIZE=1 -s 'EXPORT_NAME="WAXC"'

text: _
	cd tools; python3 concat.py; cd ../
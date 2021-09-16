#!/bin/bash

set -e

echo "Using $(which em++)" 

if [ ! -d build_wasm ]; then
	mkdir -p build_wasm && cd build_wasm
	ln -sf ../src/*.cpp ../src/*.h .
	ln -sf ../resources .
else
	cd build_wasm
fi

em++ \
	-Os \
	-std=c++17 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s USE_SDL=2 \
	-s WASM=1 \
	-DRESOURCES_DIR='"resources/"' \
	-s USE_SDL_IMAGE=2 \
	-s SDL2_IMAGE_FORMATS='["png","bmp"]' \
	--preload-file resources \
	-o index.html \
	*.cpp 

echo "Build files: build_wasm/index*"


#! /bin/sh

emcc \
  -s WASM=1 \
  -std=c++11 \
  -D__SSE__=1 -DBONSAI_INTERNAL=1 -DWASM=1 \
  -I src -I src/datatypes -I examples/ssao_test/engine_constants -I examples/ssao_test \
  tests/m4.cpp \
  -o bin/wasm/game.html

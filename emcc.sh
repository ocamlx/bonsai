#! /bin/sh

emcc                                     \
  -s WASM=1                              \
  -s USE_WEBGL2=1                        \
  -s FULL_ES3=1                          \
  -std=c++11                             \
  -DGL_GLEXT_PROTOTYPES=1                \
  -DWASM=1                               \
  -DBONSAI_INTERNAL=1                    \
  -I src -I src/datatypes                \
  -I examples/ssao_test/engine_constants \
  -I examples/ssao_test                  \
  src/platform.cpp                       \
  -o bin/wasm/platform.html

emcc                                     \
  -s WASM=1                              \
  -s USE_WEBGL2=1                        \
  -s FULL_ES3=1                          \
  -std=c++11                             \
  -DGL_GLEXT_PROTOTYPES=1                \
  -DWASM=1                               \
  -DBONSAI_INTERNAL=1                    \
  -I src -I src/datatypes                \
  -I examples/ssao_test/engine_constants \
  -I examples/ssao_test                  \
  examples/ssao_test/game.cpp            \
  -o bin/wasm/game.js

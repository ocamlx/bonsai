#! /bin/sh

emcc                                     \
  -s WASM=1                              \
  -s USE_WEBGL2=1                        \
  -s FULL_ES3=1                          \
  -s ALLOW_MEMORY_GROWTH=1               \
  -s ASSERTIONS=1                        \
  -s DEMANGLE_SUPPORT=1                  \
  -std=c++1z                             \
  -O2                                    \
  -g4                                    \
  --source-map-base /                    \
  --emrun                                \
  -DGL_GLEXT_PROTOTYPES=1                \
  -DWASM=1                               \
  -DBONSAI_INTERNAL=1                    \
  -I src                                 \
  -I src/datatypes                       \
  -I src/emscripten                      \
  -I examples/ssao_test/engine_constants \
  -I examples/ssao_test                  \
  --embed-file shaders                   \
  --embed-file models                    \
  --embed-file Holstein.DDS              \
  src/platform.cpp                       \
  -o bin/wasm/platform.html

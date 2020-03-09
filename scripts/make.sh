#! /usr/bin/bash

# COMMON_OPTIMIZATION_OPTIONS="-O2"

RED="\x1b[31m"
BLUE="\x1b[34m"
GREEN="\x1b[32m"
YELLOW="\x1b[33m"
WHITE="\x1b[37m"

Delimeter="$RED-----------------------------------------------------------$WHITE"
Success="$GREEN  ✔ $WHITE"
Building="$BLUE  Building $WHITE"
Failed="$RED  ✗ $WHITE"

ROOT="."
SRC="$ROOT/src"
EXAMPLES="$ROOT/examples"
TESTS="$SRC/tests"
BIN="$ROOT/bin"
BIN_TEST="$BIN/tests"
META_OUT="$SRC/metaprogramming/output"

function SetOutputBinaryPathBasename()
{
  base_file="${1##*/}"
  output_basename="$2/${base_file%%.*}"
}

function ColorizeTitle()
{
  echo -e "$YELLOW$1$WHITE"
  echo -e ""
}

INCLUDE_DIRECTORIES="$SRC"
OUTPUT_DIRECTORY="$BIN"

# NOTE(Jesse): -Wno-global-constructors can be turned off when the defaultPallette
# in colors.h gets axed .. I think.

# TODO(Jesse): Investigate -Wcast-align situation

COMMON_COMPILER_OPTIONS="
  -ferror-limit=20000
  -ggdb
  -Weverything
  -Wno-c++98-compat-pedantic
  -Wno-gnu-anonymous-struct
  -Wno-missing-prototypes
  -Wno-zero-as-null-pointer-constant
  -Wno-format-nonliteral
  -Wno-cast-qual
  -Wno-unused-function
  -Wno-four-char-constants
  -Wno-old-style-cast
  -Wno-float-equal
  -Wno-double-promotion
  -Wno-padded
  -Wno-global-constructors
  -Wno-cast-align
  -Wno-switch-enum
  -Wno-undef
  -Wno-covered-switch-default
  -Wno-c99-extensions
  -Wno-reserved-id-macro

  -Wno-nonportable-system-include-path
  -Wno-implicit-int-float-conversion
  -Wno-unused-variable
  -Wno-unused-parameter
  -Wno-reorder-init-list
  -Wno-atomic-implicit-seq-cst
"


COMMON_LINKER_OPTIONS="-lgdi32 -lopengl32 -luser32  "
# COMMON_LINKER_OPTIONS="-lpthread -lX11 -ldl -lGL"

SHARED_LIBRARY_FLAGS="-shared"

EXAMPLES_TO_BUILD="
  $EXAMPLES/world_gen
"

  # $EXAMPLES/animation_test
  # $EXAMPLES/ssao_test
  # $EXAMPLES/space_invaders

EXECUTABLES_TO_BUILD="
  $SRC/platform.cpp
  $SRC/font/ttf.cpp
  $SRC/net/server.cpp
"

# TODO(Jesse): The allocation tests crash in release mode because of some
# ultra-jank-tastic segfault recovery code.  Find another less janky way?
DEBUG_TESTS_TO_BUILD="
  $TESTS/allocation.cpp
"

TESTS_TO_BUILD="
  $TESTS/ui_command_buffer.cpp
  $TESTS/m4.cpp
  $TESTS/colladaloader.cpp
  $TESTS/test_bitmap.cpp
  $TESTS/chunk.cpp
  $TESTS/bonsai_string.cpp
  $TESTS/objloader.cpp
  $TESTS/callgraph.cpp
  $TESTS/heap_allocation.cpp
  $TESTS/preprocessor.cpp
  $TESTS/rng.cpp
  $TESTS/file.cpp
"

function BuildWithClang {

  which clang++ > /dev/null
  [ $? -ne 0 ] && echo -e "Please install clang++" && exit 1

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""

  echo ""
  ColorizeTitle "DebugSystem"
  DEBUG_SRC_FILE="$SRC/debug_system/debug.cpp"
  echo -e "$Building $DEBUG_SRC_FILE"
  clang++                          \
    $COMMON_OPTIMIZATION_OPTIONS   \
    $COMMON_COMPILER_OPTIONS       \
    $SHARED_LIBRARY_FLAGS          \
    $COMMON_LINKER_OPTIONS         \
    -D BONSAI_INTERNAL=1           \
    -I"$SRC"                       \
    -I"$SRC/debug_system"          \
    -o "$BIN/lib_debug_system.so"  \
    "$DEBUG_SRC_FILE" && echo -e "$Success $DEBUG_SRC_FILE" &

  wait

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""

  echo -e ""
}

function BuildWithEmcc {
  which emcc > /dev/null
  [ $? -ne 0 ] && echo -e "Please install emcc" && exit 1

  emcc src/font/ttf.cpp              \
    -D BONSAI_INTERNAL=1             \
    -I src                           \
    -I /usr/include                  \
    -I /usr/include/x86_64-linux-gnu \
    -o bin/emscripten/ttf.html
}


if [ ! -d "$BIN" ]; then
  mkdir "$BIN"
fi

if [ ! -d "$BIN_TEST" ]; then
  mkdir "$BIN_TEST"
fi

if [ ! -d "$BIN_TEST" ]; then
  mkdir "$BIN_TEST"
fi

# SOURCE_FILES="src/metaprogramming/preprocessor.h"


function RunEntireBuild {

  if [ "$EMCC" == "1" ]; then
    BuildWithEmcc
  else
    BuildWithClang
  fi

}

time RunEntireBuild

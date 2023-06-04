#!/usr/bin/env bash

# TODO(Jesse)(build): Revisit this blog and see if there are some tidbits we can use.
# http://ptspts.blogspot.com/2013/12/how-to-make-smaller-c-and-c-binaries.html

BUILD_EVERYTHING=0

RunPoof=0

BuildExecutables=1
BuildExamples=0
BuildDebugSystem=0

BuildTests=0
BuildDebugTests=0

RunTests=0

MakeDebugLibRelease=0

. scripts/preamble.sh
. scripts/setup_for_cxx.sh

OPTIMIZATION_LEVEL="-O2"
EMCC=0


# NOTE(Jesse): Can't do this cause it fucks with the paths poof outputs
# ROOT="$(pwd)"
ROOT="."
SRC="$ROOT/src"
INCLUDE="$ROOT/include"
EXAMPLES="$ROOT/examples"
TESTS="$SRC/tests"
BIN="$ROOT/bin"
BIN_TEST="$BIN/tests"

BONSAI_INTERNAL='-D BONSAI_INTERNAL'



  # $EXAMPLES/asset_picker
  # $EXAMPLES/the_wanderer
  # $EXAMPLES/tile_gen

EXAMPLES_TO_BUILD="
  $EXAMPLES/blank_project
  $EXAMPLES/turn_based
  $EXAMPLES/tools/voxel_synthesis_rule_baker
"
  # $EXAMPLES/wave_function_collapse_terrain

EXECUTABLES_TO_BUILD="
  $SRC/game_loader.cpp
"
  # $SRC/tools/asset_packer.cpp
  # $SRC/font/ttf.cpp
  # $SRC/net/server.cpp


# TODO(Jesse, tags: tests, release): The allocation tests crash in release mode because of some
# ultra-jank-tastic segfault recovery code.  Find another less janky way?
DEBUG_TESTS_TO_BUILD="
  $TESTS/allocation.cpp
"


DEBUG_LIB_RELEASE_DIR="releases/debug_lib"
function MakeDebugLibRelease
{
  [ -d $DEBUG_LIB_RELEASE_DIR ] && rm -Rf $DEBUG_LIB_RELEASE_DIR
  mkdir -p $DEBUG_LIB_RELEASE_DIR

  sync

  BuildDebugSystem
  [ $? -ne 0 ] && exit 1

  wait
  sync

  echo "#pragma once"                             >  $DEBUG_LIB_RELEASE_DIR/api.h
  cat include/bonsai_debug/headers/public.h       >> $DEBUG_LIB_RELEASE_DIR/api.h
  cat include/bonsai_stdlib/headers/primitives.h  >> $DEBUG_LIB_RELEASE_DIR/api.h
  cat include/bonsai_debug/headers/api.h          >> $DEBUG_LIB_RELEASE_DIR/api.h

  sync

  cp texture_atlas_0.bmp $DEBUG_LIB_RELEASE_DIR
  cp -R shaders $DEBUG_LIB_RELEASE_DIR
  cp "$BIN/lib_debug_system""$PLATFORM_LIB_EXTENSION" $DEBUG_LIB_RELEASE_DIR/lib_debug_system"$PLATFORM_LIB_EXTENSION"

  sync
}


function BuildExecutables
{
  echo ""
  ColorizeTitle "Executables"
  for executable in $EXECUTABLES_TO_BUILD; do
    SetOutputBinaryPathBasename "$executable" "$BIN"
    echo -e "$Building $executable"
    clang++                                          \
      $OPTIMIZATION_LEVEL                            \
      $CXX_OPTIONS                                   \
      $BONSAI_INTERNAL                               \
      $PLATFORM_CXX_OPTIONS                          \
      $PLATFORM_LINKER_OPTIONS                       \
      $PLATFORM_DEFINES                              \
      $PLATFORM_INCLUDE_DIRS                         \
      -I "$ROOT"                                     \
      -I "$SRC"                                      \
      -I "$INCLUDE"                             \
      -o "$output_basename""$PLATFORM_EXE_EXTENSION" \
      $executable && echo -e "$Success $executable" &
  done
}

function BuildDebugTests
{
  echo ""
  ColorizeTitle "Debug Tests"
  for executable in $DEBUG_TESTS_TO_BUILD; do
    SetOutputBinaryPathBasename "$executable" "$BIN_TEST"
    echo -e "$Building $executable"
    clang++                                          \
      $CXX_OPTIONS                                   \
      $BONSAI_INTERNAL \
      $PLATFORM_CXX_OPTIONS                          \
      $PLATFORM_LINKER_OPTIONS                       \
      $PLATFORM_DEFINES                              \
      $PLATFORM_INCLUDE_DIRS                         \
      -I "$ROOT"                                     \
      -I"$SRC"                                       \
      -o "$output_basename""$PLATFORM_EXE_EXTENSION" \
      $executable && echo -e "$Success $executable" &
  done
}

function BuildTests
{
  echo ""
  ColorizeTitle "Tests"
  for executable in $TESTS_TO_BUILD; do
    SetOutputBinaryPathBasename "$executable" "$BIN_TEST"
    echo -e "$Building $executable"
    clang++                                          \
      $OPTIMIZATION_LEVEL                            \
      $CXX_OPTIONS                                   \
      $BONSAI_INTERNAL \
      $PLATFORM_CXX_OPTIONS                          \
      $PLATFORM_LINKER_OPTIONS                       \
      $PLATFORM_DEFINES                              \
      $PLATFORM_INCLUDE_DIRS                         \
      -I "$ROOT"                                     \
      -I "$SRC"                                      \
      -I "$INCLUDE"                              \
      -o "$output_basename""$PLATFORM_EXE_EXTENSION" \
      $executable && echo -e "$Success $executable" &
  done
}

function BuildDebugSystem
{
  echo ""
  ColorizeTitle "DebugSystem"
  DEBUG_SRC_FILE="$INCLUDE/bonsai_debug/debug.cpp"
  output_basename="$BIN/lib_debug_system"
  echo -e "$Building $DEBUG_SRC_FILE"
  clang++                                                                           \
    $OPTIMIZATION_LEVEL                                                             \
    $CXX_OPTIONS                                                                    \
    $BONSAI_INTERNAL                                                                \
    $PLATFORM_CXX_OPTIONS                                                           \
    $PLATFORM_LINKER_OPTIONS                                                        \
    $PLATFORM_DEFINES                                                               \
    $PLATFORM_INCLUDE_DIRS                                                          \
    $SHARED_LIBRARY_FLAGS                                                           \
    -I "$ROOT"                                                                      \
    -I "$SRC"                                                                       \
    -I "$INCLUDE"                                                                   \
    -o $output_basename                                                             \
    "$DEBUG_SRC_FILE" &&                                                            \
    mv "$output_basename" "$output_basename""_loadable""$PLATFORM_LIB_EXTENSION" && \
    sync && \
    echo -e "$Success $output_basename" &
}

function BuildExamples
{
  echo ""
  ColorizeTitle "Examples"
  for executable in $EXAMPLES_TO_BUILD; do
    echo -e "$Building $executable"
    SetOutputBinaryPathBasename "$executable" "$BIN"
    clang++                                                                           \
      -D DEBUG_SYSTEM_API=1 \
      $OPTIMIZATION_LEVEL                                                             \
      $CXX_OPTIONS                                                                    \
      $BONSAI_INTERNAL                                                                \
      $PLATFORM_CXX_OPTIONS                                                           \
      $PLATFORM_LINKER_OPTIONS                                                        \
      $PLATFORM_DEFINES                                                               \
      $PLATFORM_INCLUDE_DIRS                                                          \
      $SHARED_LIBRARY_FLAGS                                                           \
      -I "$ROOT"                                                                      \
      -I "$INCLUDE"                                                                   \
      -I "$SRC"                                                                       \
      -I "$executable"                                                                \
      -o "$output_basename"                                                           \
      "$executable/game.cpp" &&                                                       \
      mv "$output_basename" "$output_basename""_loadable""$PLATFORM_LIB_EXTENSION" && \
      echo -e "$Success $executable" &
  done
}

function BuildAllClang
{
  which clang++ > /dev/null
  [ $? -ne 0 ] && echo -e "Please install clang++" && exit 1

  echo -e ""
  echo -e "$Delimeter"

  [[ $BuildExecutables == 1 || $BUILD_EVERYTHING == 1 ]] && BuildExecutables
  [[ $BuildDebugTests == 1  || $BUILD_EVERYTHING == 1 ]] && BuildDebugTests
  [[ $BuildTests == 1       || $BUILD_EVERYTHING == 1 ]] && BuildTests
  [[ $BuildDebugSystem == 1 || $BUILD_EVERYTHING == 1 ]] && BuildDebugSystem
  [[ $BuildExamples == 1    || $BUILD_EVERYTHING == 1 ]] && BuildExamples

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""
  ColorizeTitle "Complete"

  wait

  echo -e ""
  echo -e "$Delimeter"
  echo -e ""

  echo -e ""
}

function BuildAllEMCC {
  which emcc > /dev/null 2&> 1
  [ $? -ne 0 ] && echo -e "$Error Please install emcc" && exit 1

  emcc                              \
    -s WASM=1                       \
    -s LLD_REPORT_UNDEFINED         \
    -s FULL_ES3=1                   \
    -s ALLOW_MEMORY_GROWTH=1        \
    -s ASSERTIONS=1                 \
    -s DEMANGLE_SUPPORT=1           \
    -std=c++17                      \
    -Wno-c99-designator             \
    -Wno-reorder-init-list          \
    -ferror-limit=2000              \
    -fno-exceptions                 \
    -O2                             \
    -g4                             \
    --source-map-base /             \
    --emrun                         \
    -msse                           \
    -msimd128                       \
    -DEMCC=1                        \
    -DWASM=1                        \
    $BONSAI_INTERNAL \
    -I src                          \
    -I src/bonsai_debug             \
    -I examples                     \
    src/game_loader.cpp                \
    -o bin/wasm/platform.html

    # --embed-file shaders     \
    # --embed-file models      \

}


if [ ! -d "$BIN" ]; then
  mkdir "$BIN"
fi

if [ ! -d "$BIN/wasm" ]; then
  mkdir "$BIN/wasm"
fi


if [ ! -d "$BIN_TEST" ]; then
  mkdir "$BIN_TEST"
fi

if [ ! -d "$BIN_TEST" ]; then
  mkdir "$BIN_TEST"
fi

function RunEntireBuild {

  if [ $MakeDebugLibRelease == 1 ]; then
    MakeDebugLibRelease
  fi

  if [ $RunPoof == 1 ]; then
    RunPoof
  fi

  if [ $EMCC == 1 ]; then
    BuildAllEMCC
  else
    BuildAllClang
  fi

  if [ $RunTests == 1 ]; then
    ./scripts/run_tests.sh
  fi

}


# --log-level LogLevel_Debug -I src/ -I include/ -I "C:/Program Files/LLVM/lib/clang/11.0.0/include" -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/include" -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/atlmfc/include" -I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt" -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/shared" -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/um" -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/winrt" -D BONSAI_WIN32 -D BONSAI_INTERNAL -o generated src/game_loader.cpp

function RunPoofHelper {

   # -D _M_CEE_PURE \

   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/include"        \
   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/atlmfc/include" \

   # -I "C:/Program Files/LLVM/lib/clang/11.0.0/include"                                                         \
   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/include"        \
   # -I "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29333/atlmfc/include" \
   # -I "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt"                                       \
   # -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/shared"                                     \
   # -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/um"                                         \
   # -I "C:/Program Files (x86)/Windows Kits/10/include/10.0.18362.0/winrt"                                      \




   # --log-level LogLevel_Debug                                                                                  \
   #
  poof                                                                                                         \
   -I src/                                                                                                     \
   -I include/                                                                                                 \
   -D _M_X64                                                                                                   \
   -D _M_CEE                                                                                                   \
   -D POOF_PREPROCESSOR \
   $PLATFORM_DEFINES                                                                                           \
   $BONSAI_INTERNAL                                                                                            \
   -o generated                                                                                                \
   $1

}

function RunPoof
{
  echo -e ""
  echo -e "$Delimeter"
  echo -e ""

  ColorizeTitle "Poofing"

  # [ -d src/generated ] && rm -Rf src/generated
  # [ -d generated ] && rm -Rf generated

  RunPoofHelper src/game_loader.cpp && echo -e "$Success poofed src/game_loader.cpp" &
  # RunPoofHelper include/bonsai_debug/debug.cpp && echo -e "$Success poofed src/include/bonsai_debug/debug.cpp" &
  # RunPoofHelper examples/asset_picker/game.cpp && echo -e "$Success poofed examples/asset_picker/game.cpp" &
  # RunPoofHelper examples/the_wanderer/game.cpp && echo -e "$Success poofed examples/the_wanderer/game.cpp" &
  # RunPoofHelper examples/turn_based/game.cpp && echo -e "$Success poofed examples/turn_based/game.cpp" &
  RunPoofHelper examples/tools/voxel_synthesis_rule_baker/game.cpp && echo -e "$Success poofed examples/tools/voxel_synthesis_rule_baker/game.cpp" &

  # RunPoofHelper src/tools/asset_packer.cpp && echo -e "$Success poofed src/tools/asset_packer.cpp" &
  # RunPoofHelper src/tools/voxel_synthesis_rule_baker.cpp && echo -e "$Success poofed src/tools/voxel_synthesis_rule_baker.cpp" &


  wait
  sync

  [ -d tmp ] && rmdir tmp
}


TESTS_TO_BUILD="
  $TESTS/chunk.cpp
"

#   $TESTS/ui_command_buffer.cpp
#   $TESTS/m4.cpp
#   $TESTS/colladaloader.cpp
#   $TESTS/test_bitmap.cpp
#   $TESTS/bonsai_string.cpp
#   $TESTS/objloader.cpp
#   $TESTS/callgraph.cpp
#   $TESTS/heap_allocation.cpp
#   $TESTS/rng.cpp
#   $TESTS/file.cpp

# if [[ $BUILD_EVERYTHING == 0 ]]; then
#   TESTS_TO_BUILD="
#   $TESTS/bonsai_string.cpp
#   $TESTS/objloader.cpp
#   $TESTS/callgraph.cpp
#   $TESTS/heap_allocation.cpp
#   $TESTS/rng.cpp
#   $TESTS/file.cpp
#   "
# fi

time RunEntireBuild

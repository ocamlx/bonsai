#! /bin/bash

if [ $WIN32 -eq 1 ]; then

  echo " -- Building Win32"

  BINARY="bin/Debug/Bonsai.exe"
  msbuild.exe /nologo /v:m ./bin/Game.vcxproj

  cp bin/Debug/Game.dll bin/Debug/GameLoadable.dll

else # Unix

  echo " -- Building Unix"

  BINARY="bin/Bonsai"

  cd build
  make "$@" 2>&1 && cp ./bin/Game.so ./bin/GameLoadable.so

  [ $? -eq 0 ] && ./$BINARY > /dev/tty

fi


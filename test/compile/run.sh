#!/bin/sh

TEST_FILE=@TEST_CXX_FILE@
CXX=@CMAKE_CXX_COMPILER@
INCLUDE=@VP_INCLUDE_DIR@

$CXX -c "-I$INCLUDE" "$TEST_FILE"
STATUS=$?

if [ $STATUS -eq 0 ]; then
  exit 1
else
  exit 0
fi

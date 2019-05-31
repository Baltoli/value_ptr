#!/bin/sh

TEST_FILES=(@TEST_LIST_STR@)

CXX=@CMAKE_CXX_COMPILER@
INCLUDE=@VP_INCLUDE_DIR@

for test in "${TEST_FILES[@]}"; do
  $CXX -c "-I$INCLUDE" "$test" 2>/dev/null
  STATUS=$?

  if [ $STATUS -eq 0 ]; then
    exit 1
  fi
done

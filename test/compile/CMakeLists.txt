set(TESTS
  basic.cpp
  non_copy.cpp
)

foreach(file ${TESTS})
  configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

list(TRANSFORM TESTS PREPEND ${CMAKE_CURRENT_BINARY_DIR}/)
string(REPLACE ";" "\n" TEST_LIST_STR "${TESTS}")

configure_file(run.sh
  ${CMAKE_CURRENT_BINARY_DIR}/compile-fails.sh
  @ONLY)

add_test(compile-fails
  bash ${CMAKE_CURRENT_BINARY_DIR}/compile-fails.sh)

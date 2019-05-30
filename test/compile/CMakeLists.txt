set(TESTS
  basic.cpp
)

foreach(test ${TESTS})
  set(TEST_CXX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${test})

  configure_file(run.sh
    ${CMAKE_CURRENT_BINARY_DIR}/${test}.sh)

  add_test(${test}.fail
    sh ${CMAKE_CURRENT_BINARY_DIR}/${test}.sh)
endforeach()

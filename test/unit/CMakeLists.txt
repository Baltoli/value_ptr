add_executable(valueptr-unit
  fixes.cpp
  value_ptr.cpp
  main.cpp)

target_link_libraries(valueptr-unit
  valueptr)

if(CMAKE_CXX_COMPILER MATCHES ".*clang")
  target_compile_options(valueptr-unit
    PRIVATE "-Wno-error=self-assign")
endif()

add_test(
  NAME unit
  COMMAND $<TARGET_FILE:valueptr-unit>)

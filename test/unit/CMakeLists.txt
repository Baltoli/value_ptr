add_executable(valueptr-unit
  fixes.cpp
  value_ptr.cpp
  main.cpp)

target_link_libraries(valueptr-unit
  valueptr)

add_test(
  NAME unit
  COMMAND $<TARGET_FILE:valueptr-unit>)

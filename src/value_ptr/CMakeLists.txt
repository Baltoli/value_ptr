project(libvalueptr VERSION 0.1 LANGUAGES CXX)

add_library(valueptr INTERFACE)

target_include_directories(valueptr INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

install(TARGETS valueptr EXPORT ValuePtrConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ValuePtrConfig DESTINATION share/ValuePtr/cmake)

add_executable(valueptr-unit
  test/value_ptr.cpp
  test/main.cpp)

target_link_libraries(valueptr-unit
  valueptr
  CONAN_PKG::Catch2)

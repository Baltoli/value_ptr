cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(libvalueptr VERSION 0.1 LANGUAGES CXX)

add_compile_options(
  "$<$<COMPILE_LANGUAGE:CXX>:-std=c++17>"
)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.12/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE "conanfile.txt"
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

include(GNUInstallDirs)
enable_testing()

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

add_test(
  NAME unit
  COMMAND $<TARGET_FILE:valueptr-unit>)

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(libvalueptr VERSION 0.1 LANGUAGES CXX)

add_compile_options(
  "$<$<COMPILE_LANGUAGE:CXX>:-std=c++11>"
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

option(BUILD_DOC "Build documentation" OFF)
find_package(Doxygen)
if(DOXYGEN_FOUND)
  if(BUILD_DOC)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    add_custom_target(doc ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM)
  endif()
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)

option(VP_ASAN "Use sanitizers to build" OFF)
if(VP_ASAN)
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -static-libasan -fsanitize=address")
  set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -static-libasan -fsanitize=address")
endif()

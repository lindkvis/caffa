cmake_minimum_required(VERSION 3.20)

set(CAFFA_VERSION 1.3.0)

project(
  caffa
  VERSION ${CAFFA_VERSION}
  DESCRIPTION "Caffa Application Framework")

message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${GIT_HASH}")

add_definitions(-DCAFFA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DCAFFA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
                -DCAFFA_VERSION_PATCH=${PROJECT_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/scripts")

option(CAFFA_REST_INTERFACE "Enable the REST API interface to the Project Data Model" ON)
option(CAFFA_BUILD_EXAMPLES "Build examples" ON)
option(CAFFA_BUILD_UNIT_TESTS "Build unit tests" ON)
option(CAFFA_BUILD_DOCS "Build Doxygen documentation" OFF)

option(CAFFA_BUILD_SHARED "Create a shared Caffa library" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(UNIX)
  set(THREAD_LIBRARY "pthread")
endif()

if(MSVC)
  # Windows 10 minimum
  add_definitions(/D_WIN32_WINNT=0x0A00)
else()
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wno-unused-parameter
    -Wno-conversion
    -Wno-psabi)
endif()

add_definitions(-DGTEST_HAS_STD_WSTRING=0)
set(UUID_USING_CXX20_SPAN
    ON
    CACHE INTERNAL "Use CXX20 span")
set(UUID_BUILD_TESTS
    OFF
    CACHE INTERNAL "Build stduuid tests")

add_subdirectory(ThirdParty/stduuid)
add_subdirectory(ThirdParty/caffa-base)

# libraries
add_subdirectory(Core)
add_subdirectory(RpcBase)

if(CAFFA_REST_INTERFACE)
  add_subdirectory(ThirdParty/cpp-httplib)
  add_subdirectory(RestInterface)
  if(CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory(RestInterface/RestInterface_UnitTests)
  endif()
  if(CAFFA_BUILD_EXAMPLES)
    add_subdirectory(RestInterface/RestInterface_Example)
  endif()
endif()

if(CAFFA_BUILD_UNIT_TESTS)
  add_subdirectory(Core/Core_UnitTests)
  add_subdirectory(Core/IoCore_UnitTests)
  add_subdirectory(Core/ProjectDataModel_UnitTests)
  add_subdirectory(TestData)

  enable_testing()
  add_test(caffaCore_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaCore_UnitTests)
  add_test(caffaIoCore_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaIoCore_UnitTests)
  add_test(caffaProjectDataModel_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaProjectDataModel_UnitTests)

  if(CAFFA_REST_INTERFACE)
    add_test(caffaRestInterface_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaRestInterface_UnitTests)
  endif()
endif()

# Organize sub-projects into folders on Visual Studio Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET caffaCore PROPERTY FOLDER "Core")
if(CAFFA_BUILD_UNIT_TESTS)
  set_property(TARGET caffaCore_UnitTests caffaIoCore_UnitTests PROPERTY FOLDER "Core")
endif()

if(CAFFA_BUILD_DOCS)
  set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen.config.in")
  set(DOXYGEN_OUT "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen.config")
  set(CAFFA_README_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md")
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    message("Doxygen build started")
    add_custom_target(
      doc_doxygen ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR/docs}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM)
  else()
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif()
endif()

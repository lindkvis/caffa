cmake_minimum_required(VERSION 3.12)

include(cmake/VersionFromGit.cmake)
version_from_git(LOG OFF INCLUDE_HASH ON TIMESTAMP "%Y%m%d%H%M%S")

project(
  caffa
  VERSION ${VERSION}
  DESCRIPTION "Caffa Application Framework")

message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${GIT_HASH}")

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/scripts")

option(CAFFA_GRPC_INTERFACE "Enable the gRPC interface to the Project Data Model" OFF)
option(CAFFA_DESKTOP_DEPLOYMENT "Turn this on to build for desktop deployment with the Qt Toolkit" OFF)
option(CAFFA_BUILD_EXAMPLES "Build examples" OFF)
option(CAFFA_BUILD_UNIT_TESTS "Build unit tests" ON)

option(CAFFA_BUILD_SHARED "Create a shared Caffa library" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(UNIX)
  set(THREAD_LIBRARY "pthread")
endif()

add_definitions(-DGTEST_HAS_STD_WSTRING=0)

# Third Party libraries
set(JSON_BuildTests
    OFF
    CACHE INTERNAL "Build Json library with unit tests")
set(JSON_Coverage
    OFF
    CACHE INTERNAL "Build Json library with coverage tests")
set(JSON_Install
    OFF
    CACHE INTERNAL "Add install step to json library")
set(JSON_ImplicitConversions
    ON
    CACHE INTERNAL "Do implicit data type conversions")
set(JSON_MultipleHeaders
    OFF
    CACHE INTERNAL "Split in multiple headers")
set(GSL_CXX_STANDARD
    17
    CACHE INTERNAL "Make GSL confirm to which C++ standard?")
set(GSL_TEST
    OFF
    CACHE INTERNAL "Build GSL tests")
add_subdirectory(ThirdParty/json)
add_subdirectory(ThirdParty/GSL)
if(CAFFA_BUILD_UNIT_TESTS)
  add_subdirectory(ThirdParty/gtest)
endif()

# libraries
add_subdirectory(ProjectDataModel/Core)
add_subdirectory(ProjectDataModel/UiCore)
add_subdirectory(ProjectDataModel/IoCore)
add_subdirectory(ProjectDataModel/Object)

add_subdirectory(ProjectDataModel)

if(CAFFA_GRPC_INTERFACE)
  add_subdirectory(GrpcInterface)
endif()

if(CAFFA_BUILD_UNIT_TESTS)
  add_subdirectory(ProjectDataModel/Core/Core_UnitTests)
  add_subdirectory(ProjectDataModel/IoCore/IoCore_UnitTests)
  add_subdirectory(ProjectDataModel/ProjectDataModel_UnitTests)
endif()

if(CAFFA_GRPC_INTERFACE)
  if(CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory(GrpcInterface/GrpcInterface_UnitTests)
  endif()
  if(CAFFA_BUILD_EXAMPLES)
    add_subdirectory(GrpcInterface/GrpcInterface_Example)
  endif()
endif()

if(CAFFA_DESKTOP_DEPLOYMENT)
  add_subdirectory(UserInterface)
  add_subdirectory(Tests/TestApplication)
  if(CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory(UserInterface/UserInterface_UnitTests)
  endif()
endif()

# Organize sub-projects into folders on Visual Studio Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET caffaCore caffaIoCore caffaUiCore caffaObject PROPERTY FOLDER "Core")
if(CAFFA_BUILD_UNIT_TESTS)
  set_property(TARGET caffaCore_UnitTests caffaIoCore_UnitTests PROPERTY FOLDER "Core")
endif()

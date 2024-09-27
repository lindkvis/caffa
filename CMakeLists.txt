cmake_minimum_required(VERSION 3.22)

set(CAFFA_VERSION 1.6.0)

project(
        caffa
        VERSION ${CAFFA_VERSION}
        DESCRIPTION "Caffa Application Framework")

include(cmake/git_submodule.cmake)

message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${GIT_HASH}")

add_definitions(-DCAFFA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DCAFFA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        -DCAFFA_VERSION_PATCH=${PROJECT_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 20)

option(CAFFA_REST_INTERFACE "Enable the REST API interface to CAFFA" ON)
option(CAFFA_BUILD_EXAMPLES "Build examples" ON)
option(CAFFA_BUILD_UNIT_TESTS "Build unit tests" ON)
option(CAFFA_BUILD_DOCS "Build Doxygen documentation" OFF)

option(CAFFA_BUILD_SHARED "Create a shared Caffa library" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (UNIX)
    set(THREAD_LIBRARY "pthread")
endif ()

if (MSVC)
    # Windows 10 minimum
    add_definitions(/D_WIN32_WINNT=0x0A00)
else ()
    add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wno-unused-parameter
            -Wno-conversion
            -Wno-psabi)
endif ()

add_definitions(-DGTEST_HAS_STD_WSTRING=0)

# libraries
add_git_submodule(DataModel TRUE)
add_subdirectory(Core)

if (CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory(Core/IoCore_UnitTests)
    add_subdirectory(Core/ProjectDataModel_UnitTests)

    enable_testing()
    add_test(caffaIoCore_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaIoCore_UnitTests)
    add_test(caffaProjectDataModel_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaProjectDataModel_UnitTests)
endif ()

if (CAFFA_REST_INTERFACE)
    add_subdirectory(RpcBase)
    add_subdirectory(RestInterface)
    add_git_submodule(RestInterface/Bindings/Java FALSE)
    add_git_submodule(RestInterface/Bindings/Python/caffa FALSE)

    if (CAFFA_BUILD_UNIT_TESTS OR CAFFA_BUILD_EXAMPLES)
        add_subdirectory(RestInterface/RestInterface_TestData)
    endif ()
    if (CAFFA_BUILD_UNIT_TESTS)
        add_subdirectory(RestInterface/RestInterface_UnitTests)
        add_test(caffaRestInterface_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaRestInterface_UnitTests -V debug)
    endif ()
    if (CAFFA_BUILD_EXAMPLES)
        add_subdirectory(RestInterface/RestInterface_Example)
    endif ()
endif ()

if (CAFFA_BUILD_DOCS)
    set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen.config.in")
    set(DOXYGEN_OUT "${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen.config")
    set(CAFFA_README_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/docs/README.md")
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        message("Doxygen build started")
        add_custom_target(
                doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR/docs}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)
    else ()
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif ()
endif ()

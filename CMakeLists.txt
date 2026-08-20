cmake_minimum_required(VERSION 3.22)

set(CAFFA_VERSION 2.0.0)
set(CAFFA_VERSION_PRERELEASE develop-1)
set(CAFFA_VERSION_EXTENDED ${CAFFA_VERSION}-${CAFFA_VERSION_PRERELEASE})
set(CAFFA_SEMVER_REGEX
        "^[0-9]+\\.[0-9]+\\.[0-9]+(-[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*)?(\\+[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*)?$")

if (NOT CAFFA_VERSION_EXTENDED MATCHES "${CAFFA_SEMVER_REGEX}")
    message(FATAL_ERROR "CAFFA_VERSION_EXTENDED is not valid SemVer: ${CAFFA_VERSION_EXTENDED}")
endif ()

project(
        caffa
        VERSION ${CAFFA_VERSION}
        DESCRIPTION "Caffa Introspection and Serialization Library")

include(cmake/git_submodule.cmake)

message(STATUS "Building ${PROJECT_NAME} version ${CAFFA_VERSION_EXTENDED}")

add_compile_definitions(
        CAFFA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        CAFFA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        CAFFA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        CAFFA_VERSION_STRING="${CAFFA_VERSION_EXTENDED}")

set(CMAKE_CXX_STANDARD 20)

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
    add_compile_options(/utf-8)
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
add_subdirectory(DataModel)
add_subdirectory(Core)

if (CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory(Core/IoCore_UnitTests)
    add_subdirectory(Core/ProjectDataModel_UnitTests)

    enable_testing()
    add_test(caffaIoCore_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaIoCore_UnitTests)
    add_test(caffaProjectDataModel_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaProjectDataModel_UnitTests)
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

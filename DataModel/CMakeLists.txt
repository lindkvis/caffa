cmake_minimum_required(VERSION 3.22)

set(CAFFA_DATA_MODEL_VERSION 1.7.1)
project(caffaDataModel VERSION ${CAFFA_DATA_MODEL_VERSION} DESCRIPTION "Caffa Data Model Library")

include(cmake/git_submodule.cmake)

message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${GIT_HASH}")

set(CMAKE_CXX_STANDARD 20)
option(CAFFA_BUILD_EXAMPLES "Build examples" ON)
option(CAFFA_BUILD_UNIT_TESTS "Build unit tests" ON)
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

add_git_submodule(Base TRUE)

set(PUBLIC_HEADERS
        cafAppEnum.h
        cafChildArrayField.h
        cafChildArrayField.inl
        cafChildArrayFieldAccessor.h
        cafChildFieldHandle.h
        cafChildField.h
        cafChildField.inl
        cafChildFieldAccessor.h
        cafDataFieldAccessor.h
        cafDataField.h
        cafField.h
        cafFieldCapability.h
        cafFieldHandle.h
        cafObjectMacros.h
        cafObjectCollector.h
        cafObjectFinder.h
        cafObjectPerformer.h
        cafObjectHandle.h
        cafPortableDataType.h
        cafFieldProxyAccessor.h
        cafChildArrayFieldHandle.h
        cafMethodHandle.h
        cafDefaultObjectFactory.h
        cafObjectFactory.h
        cafObjectHandlePortableDataType.h)

set(PROJECT_FILES
        ${PUBLIC_HEADERS}
        cafChildArrayFieldAccessor.cpp
        cafChildFieldHandle.cpp
        cafFieldHandle.cpp
        cafObjectHandle.cpp
        cafDefaultObjectFactory.cpp
)

find_package(Boost 1.74.0 REQUIRED COMPONENTS regex)

if (CAFFA_BUILD_SHARED)
    message(STATUS "Building ${PROJECT_NAME} shared")
    add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else ()
    add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC caffaBase Boost::regex)

if (CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory(DataModel_UnitTests)

    enable_testing()
    add_test(caffaDataModel_UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/caffaDataModel_UnitTests)

endif ()

install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

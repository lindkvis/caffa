cmake_minimum_required(VERSION 3.22)

project(caffaCore)

set(PUBLIC_HEADERS
        cafJsonDefinitions.h
        cafExtraFieldValidators.hpp
        cafRangeValidator.h
        cafMethod.h
        cafObject.h
        cafObjectJsonConversion.h
        cafDocument.h
        cafApplication.h
        cafSession.h
        cafFieldInitHelper.h
        cafFieldIoCapabilitySpecializations.h
        cafFieldIoCapabilitySpecializations.inl
        cafFieldIoCapability.h
        cafFieldScriptingCapability.h
        cafJsonDataType.h
        cafJsonSerializer.h
        cafStringEncoding.h)

set(PROJECT_FILES
        ${PUBLIC_HEADERS}
        cafObject.cpp
        cafDocument.cpp
        cafApplication.cpp
        cafSession.cpp
        cafFieldIoCapability.cpp
        cafFieldScriptingCapability.cpp
        cafJsonSerializer.cpp
        cafObjectJsonConversion.cpp
        cafStringEncoding.cpp)

if (CAFFA_BUILD_SHARED)
    message(STATUS "Building ${PROJECT_NAME} shared")
    add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else ()
    add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif ()

find_package(Boost 1.83.0 REQUIRED COMPONENTS json)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC caffaBase caffaDataModel Boost::json)

install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

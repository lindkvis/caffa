cmake_minimum_required(VERSION 3.16)

project(caffaCore)

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
    cafFieldInitHelper.h
    cafFieldHandle.h
    cafFieldValidator.h
    cafObjectCollector.h
    cafObjectPerformer.h
    cafObjectHandle.h
    cafPortableDataType.h
    cafFieldProxyAccessor.h
    cafChildArrayFieldHandle.h
    cafTypedField.h
    cafSignal.h
    cafMethodHandle.h
    cafMethod.h
    cafMethodInitHelper.h
    cafObjectMacros.h
    cafDefaultObjectFactory.h
    cafObjectFactory.h
    cafObject.h
    cafObjectJsonConversion.h
    cafObjectPortableDataType.h
    cafFieldScriptingCapability.h
    cafDocument.h
    cafApplication.h
    cafSession.h
    cafFieldJsonCapabilitySpecializations.h
    cafFieldJsonCapabilitySpecializations.inl
    cafFieldJsonCapability.h
    cafJsonDataType.h
    cafJsonSerializer.h
    cafUuidGenerator.h
    cafRangeValidator.h
    cafStringEncoding.h)

set(PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafChildArrayFieldAccessor.cpp
    cafChildFieldAccessor.cpp
    cafChildFieldHandle.cpp
    cafDataField.cpp
    cafFieldCapability.cpp
    cafFieldHandle.cpp
    cafObjectHandle.cpp
    cafSignal.cpp
    cafDefaultObjectFactory.cpp
    cafObject.cpp
    cafFieldScriptingCapability.cpp
    cafDocument.cpp
    cafApplication.cpp
    cafSession.cpp
    cafFieldJsonCapability.cpp
    cafJsonSerializer.cpp
    cafObjectJsonConversion.cpp
    cafStringEncoding.cpp
    cafUuidGenerator.cpp
    cafVisitor.cpp)

if(CAFFA_BUILD_SHARED)
  message(STATUS "Building ${PROJECT_NAME} shared")
  add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else()
  add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC caffaBase nlohmann_json::nlohmann_json
  PRIVATE stduuid)

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

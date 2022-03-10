cmake_minimum_required(VERSION 3.12)

project(caffaCore)

set(PUBLIC_HEADERS
    cafAssert.h
    cafFactory.h
    cafBase.h
    cafChildArrayField.h
    cafChildArrayField.inl
    cafChildArrayFieldAccessor.h
    cafChildField.h
    cafChildField.inl
    cafChildFieldAccessor.h
    cafDataFieldAccessor.h
    cafField.h
    cafFieldCapability.h
    cafFieldHandle.h
    cafObjectCapability.h
    cafObjectHandle.h
    cafObservingPointer.h
    cafPortableDataType.h
    cafFieldProxyAccessor.h
    cafChildArrayFieldHandle.h
    cafValueField.h
    cafSignal.h
    cafFieldIoCapabilitySpecializations.h
    cafFieldIoCapabilitySpecializations.inl
    cafFieldIoCapability.h
    cafSerializer.h
    cafJsonSerializer.h
    cafObjectIoCapability.h
    cafObjectMacros.h
    cafDefaultObjectFactory.h
    cafObjectFactory.h
    cafObject.h
    cafFieldScriptingCapability.h
    cafObjectMethod.h
    cafDocument.h
    cafApplication.h)

set(PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafChildArrayFieldAccessor.cpp
    cafChildFieldAccessor.cpp
    cafFieldHandle.cpp
    cafObjectCapability.cpp
    cafObjectHandle.cpp
    cafObservingPointer.cpp
    cafSignal.cpp
    cafFieldIoCapability.cpp
    cafObjectIoCapability.cpp
    cafSerializer.cpp
    cafJsonSerializer.cpp
    cafDefaultObjectFactory.cpp
    cafObject.cpp
    cafFieldScriptingCapability.cpp
    cafObjectMethod.cpp
    cafDocument.cpp
    cafApplication.cpp)

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

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC caffaLogger nlohmann_json::nlohmann_json
  PRIVATE stduuid)

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

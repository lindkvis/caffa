cmake_minimum_required(VERSION 3.16)

project(caffaCore)

set(PUBLIC_HEADERS
    cafApplication.h
    cafDocument.h
    cafFieldJsonCapabilitySpecializations.h
    cafFieldJsonCapabilitySpecializations.inl
    cafFieldJsonCapability.h
    cafFieldRangeValidator.h
    cafJsonConversionHelpers.h
    cafJsonDataType.h
    cafJsonSerializer.h
    cafMethod.h
    cafObject.h
    cafSerializer.h
    cafSession.h
  )

set(PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafApplication.cpp
    cafDocument.cpp
    cafFieldJsonCapability.cpp
    cafJsonConversionHelpers.cpp
    cafJsonSerializer.cpp
    cafObject.cpp
    cafSerializer.cpp
    cafSession.cpp
)

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
  PUBLIC caffaDataModel nlohmann_json::nlohmann_json)

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

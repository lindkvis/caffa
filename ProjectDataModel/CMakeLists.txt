cmake_minimum_required(VERSION 3.12)

project(caffaProjectDataModel)

set(PUBLIC_HEADERS cafDocument.h cafApplication.h)

set(PROJECT_FILES ${PUBLIC_HEADERS} cafDocument.cpp cafApplication.cpp)

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
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC caffaCore caffaIoCore caffaUiCore caffaObject)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

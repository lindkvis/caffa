cmake_minimum_required(VERSION 3.16)

project(caffaRpcBase)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

set(PUBLIC_HEADERS
    cafRpcServer.h
    cafRpcClient.h
    cafRpcApplication.h
    cafRpcChildFieldAccessor.h
    cafRpcChildArrayFieldAccessor.h
    cafRpcDataFieldAccessor.h
    cafRpcClientPassByRefObjectFactory.h
    cafRpcClientPassByValueObjectFactory.h
    cafRpcMethodAccessor.h)

set(PROJECT_FILES ${PUBLIC_HEADERS} cafRpcApplication.cpp cafRpcClientPassByRefObjectFactory.cpp cafRpcClientPassByValueObjectFactory.cpp)

if(CAFFA_BUILD_SHARED)
  message(STATUS "Building ${PROJECT_NAME} shared")
  add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else()
  add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/Generated")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC caffaCore)

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

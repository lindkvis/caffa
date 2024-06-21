cmake_minimum_required(VERSION 3.22)

project(caffaRpcBase)

set(PUBLIC_HEADERS
    cafRpcServer.h
    cafRpcClient.h
    cafRpcApplication.h
    cafRpcServerApplication.h
    cafRpcChildFieldAccessor.h
    cafRpcChildArrayFieldAccessor.h
    cafRpcDataFieldAccessor.h
    cafRpcClientPassByRefObjectFactory.h
    cafRpcClientPassByValueObjectFactory.h
    cafRpcMethodAccessor.h
    cafRpcObjectConversion.h)

set(PROJECT_FILES ${PUBLIC_HEADERS} cafRpcApplication.cpp cafRpcClientPassByRefObjectFactory.cpp cafRpcClientPassByValueObjectFactory.cpp
                  cafRpcObjectConversion.cpp cafRpcServerApplication.cpp)

if(CAFFA_BUILD_SHARED)
  message(STATUS "Building ${PROJECT_NAME} shared")
  add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else()
  add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif()

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
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

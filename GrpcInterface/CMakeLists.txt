cmake_minimum_required(VERSION 3.12)

project(caffaGrpcInterface)

message(STATUS "GRPC enabled")

add_definitions(-DENABLE_GRPC)

option(CAFFA_GRPC_FETCHCONTENT "Use CMAKE FetchContent to get gRPC" OFF)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Generated)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

set(PUBLIC_HEADERS
    cafGrpcServer.h
    cafGrpcCallbacks.h
    cafGrpcCallbacks.inl
    cafGrpcServiceInterface.h
    cafGrpcAppService.h
    cafGrpcChildFieldAccessor.h
    cafGrpcChildArrayFieldAccessor.h
    cafGrpcDataFieldAccessor.h
    cafGrpcException.h
    cafGrpcFieldService.h
    cafGrpcObjectService.h
    cafGrpcApplication.h
    cafGrpcServerApplication.h
    cafGrpcClient.h
    cafGrpcClientObjectFactory.h
    cafGrpcClientApplication.h
    cafGrpcCodeGenerator.h
    cafGrpcMarkdownGenerator.h
    cafGrpcPythonGenerator.h
    )

set(PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafGrpcServer.cpp
    cafGrpcAppService.cpp
    cafGrpcFieldService.cpp
    cafGrpcObjectService.cpp
    cafGrpcApplication.cpp
    cafGrpcServerApplication.cpp
    cafGrpcClient.cpp
    cafGrpcClientObjectFactory.cpp
    cafGrpcClientApplication.cpp
    cafGrpcMarkdownGenerator.cpp
    cafGrpcPythonGenerator.cpp)

if(CAFFA_GRPC_FETCHCONTENT)
  include(FetchContent)
  FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG v1.43.0)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_MakeAvailable(gRPC)
  set(CAFFA_PROTOC $<TARGET_FILE:protoc>)
  set(CAFFA_GRPC_CPP_PLUGIN $<TARGET_FILE:grpc_cpp_plugin>)
  set(GRPC_LINK_LIBRARIES grpc++ grpc gpr libprotobuf)
else()
  if(PKG_CONFIG_PATH)
    message("Setting environment before calling pkgconfig to: " ${PKG_CONFIG_PATH})
    set(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH})
  endif()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(
    GRPC
    REQUIRED
    grpc++>=1.16
    grpc
    gpr
    protobuf
    NO_CMAKE_ENVIRONMENT_PATH)
  if(EXISTS "${GRPC_protobuf_PREFIX}/bin/protoc")
    set(CAFFA_PROTOC "${GRPC_protobuf_PREFIX}/bin/protoc")
    message("Using protoc: ${GRPC_protobuf_PREFIX}/bin/protoc")
  else()
    message(FATAL_ERROR "Could not find the protobuf compiler (protoc)")
  endif()

  set(CAFFA_GRPC_CPP_PLUGIN "${GRPC_grpc++_PREFIX}/bin/grpc_cpp_plugin" CACHE FILEPATH "Protoc Plugin used to generate Caffa gRPC C++ code")

  if (EXISTS ${CAFFA_GRPC_CPP_PLUGIN})
      message("Using grpc_cpp_plugin: ${CAFFA_GRPC_CPP_PLUGIN}")
  else()
      message(FATAL_ERROR "Could not find the grpc_cpp_plugin ${CAFFA_GRPC_CPP_PLUGIN}")
  endif()

  include_directories(AFTER ${GRPC_INCLUDE_DIRS})
endif()

set(CURRENT_GRPC_VERSION ${GRPC_grpc++_VERSION})
set(CURRENT_PROTOBUF_VERSION ${GRPC_protobuf_VERSION})

message(STATUS "Using protobuf ${CURRENT_PROTOBUF_VERSION}")
message(STATUS "Using gRPC ${CURRENT_GRPC_VERSION}")

set(LINK_LIBRARIES ${GRPC_LINK_LIBRARIES} caffaCore)

# Proto files
set(PROTO_FILES "App" "FieldService" "ObjectService")

foreach(proto_file ${PROTO_FILES})
  get_filename_component(caffa_proto "${CMAKE_CURRENT_LIST_DIR}/Protos/${proto_file}.proto" ABSOLUTE)
  get_filename_component(caffa_proto_path "${caffa_proto}" PATH)

  list(APPEND GRPC_PROTO_FILES_FULL_PATH ${caffa_proto})

  set(caffa_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.cc")
  set(caffa_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.h")
  set(caffa_grpc_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.cc")
  set(caffa_grpc_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.h")

  add_custom_command(
    OUTPUT "${caffa_proto_srcs}" "${caffa_proto_hdrs}" "${caffa_grpc_srcs}" "${caffa_grpc_hdrs}"
    COMMAND ${CAFFA_PROTOC} ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated" --cpp_out "${CMAKE_BINARY_DIR}/Generated" -I
            "${caffa_proto_path}" --plugin=protoc-gen-grpc=${CAFFA_GRPC_CPP_PLUGIN} "${caffa_proto}"
    DEPENDS "${caffa_proto}")
  list(APPEND GRPC_HEADER_FILES ${caffa_proto_hdrs} ${caffa_grpc_hdrs})
  list(APPEND GRPC_CPP_SOURCES ${caffa_proto_srcs} ${caffa_grpc_srcs})
endforeach(proto_file)

if(CAFFA_BUILD_SHARED)
  message(STATUS "Building ${PROJECT_NAME} shared")
  add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES} ${GRPC_HEADER_FILES} ${GRPC_CPP_SOURCES})
else()
  add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES} ${GRPC_HEADER_FILES} ${GRPC_CPP_SOURCES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/Generated")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${LINK_LIBRARIES})

if(MSVC)
  # GRPC generates a lot of harmless warnings on MSVC
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4251 /wd4702 /wd4005 /wd4244 /wd4125 /wd4267")
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  set_target_properties(${PROJECT_NAME} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

else()
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wno-overloaded-virtual")
endif()

source_group("Protos" FILES ${GRPC_PROTO_FILES_FULL_PATH})

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

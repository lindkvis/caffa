cmake_minimum_required(VERSION 3.12)

project(cafGrpcInterface)

message(STATUS "GRPC enabled")

add_definitions(-DENABLE_GRPC)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Generated)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServer.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcCallbacks.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcCallbacks.inl
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServiceInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcAppService.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcDataFieldAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcException.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcFieldService.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcObjectService.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcApplication.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServerApplication.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClient.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcObjectClientCapability.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientObjectFactory.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientApplication.h
    )

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcAppService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcFieldService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcObjectService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcApplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServerApplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientObjectFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientApplication.cpp
    )

find_package(protobuf)
find_package(absl CONFIG)
find_package(gRPC REQUIRED grpc++>=1.25 grpc gpr protobuf)

set(CAFFA_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(CAFFA_GRPC_CPP_PLUGIN $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
set(GRPC_LINK_LIBRARIES gRPC::grpc++ gRPC::grpc gRPC::gpr protobuf::libprotobuf)

set(CURRENT_GRPC_VERSION ${GRPC_grpc++_VERSION})
set(CURRENT_PROTOBUF_VERSION ${GRPC_protobuf_VERSION})

message(STATUS "Using protobuf ${CURRENT_PROTOBUF_VERSION}")
message(STATUS "Using gRPC ${CURRENT_GRPC_VERSION}")

set(LINK_LIBRARIES
    ${GRPC_LINK_LIBRARIES}
    Microsoft.GSL::GSL
    cafProjectDataModel
    cafPdmScriptCore
    )

# Proto files
set(PROTO_FILES
    "AppInfo"
    "FieldService"
    "ObjectService"
)

foreach(proto_file ${PROTO_FILES})
  get_filename_component(
    caffa_proto "${CMAKE_CURRENT_LIST_DIR}/Protos/${proto_file}.proto"
    ABSOLUTE)
  get_filename_component(caffa_proto_path "${caffa_proto}" PATH)

  list(APPEND GRPC_PROTO_FILES_FULL_PATH ${caffa_proto})

  set(caffa_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.cc")
  set(caffa_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.h")
  set(caffa_grpc_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.cc")
  set(caffa_grpc_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.h")

  add_custom_command(
    OUTPUT "${caffa_proto_srcs}" "${caffa_proto_hdrs}" "${caffa_grpc_srcs}"
           "${caffa_grpc_hdrs}"
    COMMAND
      ${CAFFA_PROTOC} ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated"
      --cpp_out "${CMAKE_BINARY_DIR}/Generated" -I "${caffa_proto_path}"
      --plugin=protoc-gen-grpc=${CAFFA_GRPC_CPP_PLUGIN} "${caffa_proto}"
    DEPENDS "${caffa_proto}"
    )
  list(APPEND GRPC_HEADER_FILES ${caffa_proto_hdrs} ${caffa_grpc_hdrs})
  list(APPEND GRPC_CPP_SOURCES ${caffa_proto_srcs} ${caffa_grpc_srcs})  
endforeach(proto_file)

message(STATUS "Configuring Java gradle build file")
configure_file("${CMAKE_CURRENT_LIST_DIR}/Java/build.gradle.cmake" "${CMAKE_CURRENT_LIST_DIR}/Java/build.gradle")

if (CAFFA_BUILD_SHARED)
    message(STATUS "Building ${PROJECT_NAME} shared")
    add_library( ${PROJECT_NAME} SHARED
                 ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
                 ${GRPC_HEADER_FILES} ${GRPC_CPP_SOURCES}
    )
else()
    add_library( ${PROJECT_NAME} STATIC
                 ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
                 ${GRPC_HEADER_FILES} ${GRPC_CPP_SOURCES}
    )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY AUTOMOC OFF)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/Generated")
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})

if(MSVC)
  # GRPC generates a lot of harmless warnings on MSVC
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES COMPILE_FLAGS "/wd4251 /wd4702 /wd4005 /wd4244 /wd4125 /wd4267")
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  set_target_properties(
    ${PROJECT_NAME} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
                               MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

else()
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                                                   "-Wno-overloaded-virtual")
endif()

source_group("Protos" FILES ${GRPC_PROTO_FILES_FULL_PATH})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

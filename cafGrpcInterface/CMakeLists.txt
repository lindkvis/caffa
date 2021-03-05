cmake_minimum_required(VERSION 3.12)

project(cafGrpcInterface)

option(CAFFA_GRPC_PYTHON "Generate Python Client Code" OFF)
if (CAFFA_GRPC_PYTHON)
  option(CAFFA_GRPC_PYTHON_DOWNLOAD "Download Grpc Python Modules" OFF)
endif()

message(STATUS "GRPC enabled")

if(VCPKG_AUTO_INSTALL)
  vcpkg_install(grpc protobuf)
endif()

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
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcObjectService.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServerApplication.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClient.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcObjectClientCapability.h
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientObjectFactory.h
    #${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientApplication.h
    )

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcAppService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcObjectService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcServerApplication.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientObjectFactory.cpp
    #${CMAKE_CURRENT_LIST_DIR}/cafGrpcClientApplication.cpp
    )

find_package(Protobuf 3.0 CONFIG QUIET)

if(Protobuf_FOUND)
  message(STATUS "Using protobuf ${protobuf_VERSION}")

  # Find gRPC installation Looks for gRPCConfig.cmake file installed by gRPC's
  # cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

  set(_GRPC_GRPCPP gRPC::grpc++ gRPC::grpc gRPC::gpr)
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  set(GRPC_LINK_LIBRARIES ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

  set_target_properties(
    ${GRPC_LINK_LIBRARIES} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
                                 MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
else()
  message(STATUS "Using GRPC_INSTALL_PREFIX=${GRPC_INSTALL_PREFIX}")
  set(GRPC_INSTALL_PREFIX
      "/usr"
      CACHE PATH "gRPC : Install prefix for gRPC")
  if(NOT DEFINED GRPC_INSTALL_PREFIX
     OR NOT EXISTS ${GRPC_INSTALL_PREFIX})
    message(
      FATAL_ERROR
        "You need a valid GRPC_INSTALL_PREFIX set to build with gRPC"
    )
  endif()
  set(ENV{PKG_CONFIG_PATH} "${GRPC_INSTALL_PREFIX}/lib/pkgconfig")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GRPC REQUIRED grpc grpc++>=1.16 grpc gpr
                    protobuf)
  if(EXISTS "${GRPC_INSTALL_PREFIX}/bin/protoc")
    set(_PROTOBUF_PROTOC "${GRPC_INSTALL_PREFIX}/bin/protoc")
  elseif(EXISTS "${GRPC_INSTALL_PREFIX}/tools/protobuf/protoc")
    set(_PROTOBUF_PROTOC
        "${GRPC_INSTALL_PREFIX}/tools/protobuf/protoc")
  else()
    message(FATAL_ERROR "Could not find the protobuf compiler (protoc)")
  endif()

  if(EXISTS "${GRPC_INSTALL_PREFIX}/bin/grpc_cpp_plugin")
    set(_GRPC_CPP_PLUGIN_EXECUTABLE
        "${GRPC_INSTALL_PREFIX}/bin/grpc_cpp_plugin")
  elseif(EXISTS "${GRPC_INSTALL_PREFIX}/tools/grpc/grpc_cpp_plugin")
    set(_GRPC_CPP_PLUGIN_EXECUTABLE
        "${GRPC_INSTALL_PREFIX}/tools/grpc/grpc_cpp_plugin")
  endif()
  include_directories(AFTER ${GRPC_INCLUDE_DIRS})
endif()

set(LINK_LIBRARIES
    ${GRPC_LINK_LIBRARIES}
    Microsoft.GSL::GSL
    cafProjectDataModel
    cafPdmScriptCore
    )

# Proto files
set(PROTO_FILES
    "App"
    "Object"
)

set(GRPC_PYTHON_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/Python")

if (CAFFA_GRPC_PYTHON)
  find_package(Python3 COMPONENTS Interpreter)
endif()

add_custom_target(PipInstall)

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
      ${_PROTOBUF_PROTOC} ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated"
      --cpp_out "${CMAKE_BINARY_DIR}/Generated" -I "${caffa_proto_path}"
      --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}" "${caffa_proto}"
    DEPENDS "${caffa_proto}")

  set(caffa_proto_python "caffa/generated/${proto_file}_pb2.py")
  set(caffa_grpc_python "caffa/generated/${proto_file}_pb2_grpc.py")  

  if(Python3_Interpreter_FOUND AND Python3_EXECUTABLE)
    add_custom_command(
      OUTPUT "${GRPC_PYTHON_SOURCE_PATH}/${caffa_proto_python}"
             "${GRPC_PYTHON_SOURCE_PATH}/${caffa_grpc_python}"
      COMMAND
        ${Python3_EXECUTABLE} ARGS -m grpc_tools.protoc -I
        "${caffa_proto_path}" --python_out
        "${GRPC_PYTHON_SOURCE_PATH}/caffa/generated" --grpc_python_out
        "${GRPC_PYTHON_SOURCE_PATH}/caffa/generated" "${caffa_proto}"
      DEPENDS "${caffa_proto}"
      COMMENT "Generating ${caffa_proto_python} and ${caffa_grpc_python}"
      VERBATIM)
    list(APPEND GRPC_PYTHON_GENERATED_SOURCES ${caffa_proto_python}
         ${caffa_grpc_python})
  endif()

  list(APPEND GRPC_HEADER_FILES ${caffa_proto_hdrs} ${caffa_grpc_hdrs})
  list(APPEND GRPC_CPP_SOURCES ${caffa_proto_srcs} ${caffa_grpc_srcs})  
endforeach(proto_file)

if (Python3_Interpreter_FOUND)
  configure_file(${GRPC_PYTHON_SOURCE_PATH}/setup.py.cmake
                 ${GRPC_PYTHON_SOURCE_PATH}/setup.py)
else()
    message(
        STATUS
        "Python3 not found. Will not install Python client code"
    )
endif()
list(
  APPEND
  GRPC_PYTHON_SOURCES
  "caffa/__init__.py"
  "caffa/app.py"
  "caffa/pdmobject.py"
  "caffa/PythonExamples/app_example.py"
  "caffa/PythonExamples/pdm_object_example.py"
  "caffa/tests/test_app.py"
  "caffa/tests/test_pdm_object.py"
  "caffa/tests/conftest.py"
  "requirements.txt"
  "setup.py"
  "README.md"
  "LICENSE")

list(APPEND GRPC_PYTHON_SOURCES ${GRPC_PYTHON_GENERATED_SOURCES})

if(Python3_Interpreter_FOUND)
  foreach(PYTHON_SCRIPT ${GRPC_PYTHON_SOURCES})
    list(APPEND GRPC_PYTHON_SOURCES_FULL_PATH
         "${GRPC_PYTHON_SOURCE_PATH}/${PYTHON_SCRIPT}")
  endforeach()
endif()


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

# install gRPC Python files
if(Python3_EXECUTABLE AND CAFFA_GRPC_PYTHON_DOWNLOAD)
  message(STATUS "Installing Python modules")
  add_custom_command(
    TARGET PipInstall
    COMMAND ${Python3_EXECUTABLE} ARGS -m pip install --upgrade
            pip
    COMMAND ${Python3_EXECUTABLE} ARGS -m pip install --user
            wheel setuptools pytest
    COMMAND ${Python3_EXECUTABLE} ARGS -m pip install --user
            grpcio-tools)
endif()

if(Python3_EXECUTABLE)
  add_custom_target(GeneratedPythonSources ALL
                    DEPENDS ${GRPC_PYTHON_SOURCES_FULL_PATH})
  add_dependencies(${PROJECT_NAME} GeneratedPythonSources)

  if(CAFFA_GRPC_PYTHON_DOWNLOAD)
    add_dependencies(${PROJECT_NAME} PipInstall)
    add_dependencies(GeneratedPythonSources PipInstall)
  endif()
endif()

source_group("Protos" FILES ${GRPC_PROTO_FILES_FULL_PATH})
if(Python3_EXECUTABLE)
  source_group(
    TREE ${GRPC_PYTHON_SOURCE_PATH}
    FILES ${GRPC_PYTHON_SOURCES_FULL_PATH}
    PREFIX "Python")
endif()

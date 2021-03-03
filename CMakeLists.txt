cmake_minimum_required (VERSION 3.12)

project (caffa VERSION 0.1 DESCRIPTION "Caffa Application Framework")

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/scripts")

configure_file(vcpkg.json.cmake vcpkg.json)

option(VCPKG_AUTO_INSTALL "Automatically download build pre-requisites with VCPKG" OFF)
if (VCPKG_AUTO_INSTALL)
    include (AutoVcpkg)
    set (AUTO_VCPKG_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/vcpkg")
endif()

option(CAFFA_GRPC_INTERFACE "Enable the gRPC interface to the Project Data Model" OFF)
option(CAFFA_WEB_DEPLOYMENT "Turn this on to build for web deployment with the Web Toolkit" OFF)
option(CAFFA_DESKTOP_DEPLOYMENT "Turn this on to build for desktop deployment with the Qt Toolkit" OFF)

option(CAFFA_BUILD_SHARED "Create a shared Caffa library" ON)

if (CAFFA_WEB_DEPLOYMENT)
    add_definitions(-DWEB_DEPLOYMENT)
endif(CAFFA_WEB_DEPLOYMENT)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if( UNIX )
    set(THREAD_LIBRARY "pthread")
endif()

#Third Party libraries
add_subdirectory (ThirdParty/json)

#libraries
add_subdirectory (cafProjectDataModel/cafPdmCore)
add_subdirectory (cafProjectDataModel/cafPdmUiCore)
add_subdirectory (cafProjectDataModel/cafPdmIoCore)
add_subdirectory (cafProjectDataModel/cafPdmObject)
add_subdirectory (cafProjectDataModel/cafPdmScriptCore)


add_subdirectory(cafProjectDataModel)

if (CAFFA_GRPC_INTERFACE)
    add_subdirectory(cafGrpcInterface)
    #add_subdirectory(cafGrpcServerApp)
endif()

#executables
if (CAFFA_WEB_DEPLOYMENT)
    add_subdirectory(cafWebInterface)
    #add_subdirectory(cafWebApp)
    add_subdirectory(cafTests/cafTestWebApplication)
endif(CAFFA_WEB_DEPLOYMENT)

if (CAFFA_DESKTOP_DEPLOYMENT)
    add_subdirectory (cafUserInterface)
    #add_subdirectory (cafQtApp)
    add_subdirectory (cafTests/cafTestApplication)
    add_subdirectory (cafUserInterface/cafUserInterface_UnitTests)
endif()

add_subdirectory (cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory (cafProjectDataModel/cafPdmIoCore/cafPdmIoCore_UnitTests)
add_subdirectory (cafProjectDataModel/cafPdmScriptCore/cafPdmScriptCore_UnitTests)
add_subdirectory (cafProjectDataModel/cafProjectDataModel_UnitTests)
if (CAFFA_GRPC_INTERFACE)
    add_subdirectory(cafGrpcInterface/cafGrpcInterface_UnitTests)
endif()

# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET  cafPdmCore cafPdmCore_UnitTests cafPdmIoCore cafPdmIoCore_UnitTests  cafPdmUiCore  cafPdmObject cafPdmScriptCore cafPdmScriptCore_UnitTests PROPERTY FOLDER "PdmCore")

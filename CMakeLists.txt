cmake_minimum_required (VERSION 3.12)

project (caffa VERSION 0.1 DESCRIPTION "Caffa Application Framework")

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/scripts")

option(CAFFA_GRPC_INTERFACE "Enable the gRPC interface to the Project Data Model" OFF)
option(CAFFA_DESKTOP_DEPLOYMENT "Turn this on to build for desktop deployment with the Qt Toolkit" OFF)
option(CAFFA_BUILD_EXAMPLES "Build examples" OFF)
option(CAFFA_BUILD_UNIT_TESTS "Build unit tests" ON)

option(CAFFA_BUILD_SHARED "Create a shared Caffa library" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if( UNIX )
    set(THREAD_LIBRARY "pthread")
endif()

add_definitions(-DGTEST_HAS_STD_WSTRING=0)

#Third Party libraries
set(JSON_BuildTests OFF)
set(JSON_Coverage OFF)
set(JSON_Install OFF)
add_subdirectory (ThirdParty/json)
add_subdirectory (ThirdParty/GSL)

#libraries
add_subdirectory (cafProjectDataModel/cafCore)
add_subdirectory (cafProjectDataModel/cafUiCore)
add_subdirectory (cafProjectDataModel/cafIoCore)
add_subdirectory (cafProjectDataModel/cafObject)

add_subdirectory(cafProjectDataModel)

if (CAFFA_GRPC_INTERFACE)
    add_subdirectory(cafGrpcInterface)
    #add_subdirectory(cafGrpcServerApp)
endif()

if(CAFFA_BUILD_UNIT_TESTS)
    add_subdirectory (cafProjectDataModel/cafCore/cafCore_UnitTests)
    add_subdirectory (cafProjectDataModel/cafIoCore/cafIoCore_UnitTests)
    add_subdirectory (cafProjectDataModel/cafProjectDataModel_UnitTests)
endif()

if (CAFFA_GRPC_INTERFACE)
    if (CAFFA_BUILD_UNIT_TESTS)
        add_subdirectory(cafGrpcInterface/cafGrpcInterface_UnitTests)
    endif()
    if (CAFFA_BUILD_EXAMPLES)
        add_subdirectory(cafGrpcInterface/cafGrpcInterface_Example)
    endif()
endif()

if (CAFFA_DESKTOP_DEPLOYMENT)
    add_subdirectory (cafUserInterface)
    add_subdirectory (cafTests/cafTestApplication)
    if (CAFFA_BUILD_UNIT_TESTS)
        add_subdirectory (cafUserInterface/cafUserInterface_UnitTests)
    endif()
endif()


# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET  cafCore cafIoCore cafUiCore cafObject PROPERTY FOLDER "Core")
if (CAFFA_BUILD_UNIT_TESTS)
    set_property(TARGET  cafCore_UnitTests cafIoCore_UnitTests PROPERTY FOLDER "Core")
endif()

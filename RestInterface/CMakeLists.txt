cmake_minimum_required(VERSION 3.22)

project(caffaRestInterface)

set(PUBLIC_HEADERS
        cafRestAuthenticator.h
        cafRestSession.h
        cafRestServer.h
        cafRestClient.h
        cafRestServerApplication.h
        cafRestClientApplication.h
        cafRestServiceInterface.h
        cafRestAppService.h
        cafRestDocumentService.h
        cafRestObjectService.h
        cafRestSessionService.h
        cafRestRequest.h
        cafRestOpenApiService.h
        cafRestRequest.h
        cafRestServiceFactory.h)

set(PROJECT_FILES
        ${PUBLIC_HEADERS}
        cafRestSession.cpp
        cafRestServer.cpp
        cafRestClient.cpp
        cafRestServerApplication.cpp
        cafRestClientApplication.cpp
        cafRestServiceInterface.cpp
        cafRestAppService.cpp
        cafRestDocumentService.cpp
        cafRestObjectService.cpp
        cafRestSessionService.cpp
        cafRestRequest.cpp
        cafRestOpenApiService.cpp
        cafRestRequest.cpp)

if (CAFFA_BUILD_SHARED)
    message(STATUS "Building ${PROJECT_NAME} shared")
    add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else ()
    add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127 /bigobj")
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost 1.83.0 REQUIRED COMPONENTS system json regex)
find_package(OpenSSL REQUIRED)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC caffaBase
        caffaCore
        caffaRpcBase
        OpenSSL::SSL
        OpenSSL::Crypto
        Boost::system
        Boost::json
        Boost::regex)

install(
        TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

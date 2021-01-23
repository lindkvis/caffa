cmake_minimum_required (VERSION 3.12.0)

project ( cafTestWebApplication )

set( PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    MainWindow.h
)

set(Boost
_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system program_options thread filesystem date_time regex REQUIRED)
set(Fontconfig_LIBRARIES)
set(FREETYPE_LIBRARIES)
set(OTHER_DEPENDENCIES)
if (UNIX)
    find_package(Fontconfig REQUIRED)
    find_package(Freetype REQUIRED)
    list(APPEND OTHER_DEPENDENCIES ffi expat harfbuzz tiff)
endif()
find_package(wt COMPONENTS Wt HTTP REQUIRED)

# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set (EXTERNAL_LINK_LIBRARIES
    ${THREAD_LIBRARY}
	ZLIB::ZLIB
    OpenSSL::Crypto
    ${OTHER_DEPENDENCIES}
    ${Fontconfig_LIBRARIES}
    ${FREETYPE_LIBRARIES}
	Wt::Wt Wt::HTTP
    )

add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_ALL_DYN_LINK)

target_link_libraries ( ${PROJECT_NAME}
    ${EXTERNAL_LINK_LIBRARIES}
    ${Boost_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    cafProjectDataModel
#    cafCommandFeatures
#    cafCommand
    cafWebInterface
)


if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4100 /wd4127 /wd4267 /wd4251 /wd4275")
endif() 


source_group("" FILES ${PROJECT_FILES})

cmake_minimum_required (VERSION 3.12.0)

project ( cafTestWebApplication )

find_package(Qt5 COMPONENTS REQUIRED Core )
set(QT_LIBRARIES Qt5::Core)

option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

if (USE_COMMAND_FRAMEWORK)
    include_directories (
        ${cafCommand_SOURCE_DIR}
    )
    ADD_DEFINITIONS(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)


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
find_package(Boost 1.60 COMPONENTS system program_options thread filesystem date_time regex REQUIRED)
set(Fontconfig_LIBRARIES)
set(FREETYPE_LIBRARIES)
if (UNIX)
    find_package(Fontconfig REQUIRED)
    find_package(Freetype REQUIRED)
endif()
find_package(WT COMPONENTS Wt HTTP REQUIRED)

# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
    ${QRC_FILES_CPP}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set (EXTERNAL_LINK_LIBRARIES
    ${THREAD_LIBRARY}
	ZLIB::ZLIB
    OpenSSL::Crypto
    ${Fontconfig_LIBRARIES}
	Wt::Wt Wt::HTTP
    )

add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_ALL_DYN_LINK)

target_link_libraries ( ${PROJECT_NAME}
    ${EXTERNAL_LINK_LIBRARIES}
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    cafProjectDataModel
    cafCommandFeatures
    cafCommand
    cafCommonInterface
    cafWebInterface
)


if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4267")
endif() 

source_group("" FILES ${PROJECT_FILES})

foreach (qtlib ${QT_LIBRARIES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		           COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach(qtlib)
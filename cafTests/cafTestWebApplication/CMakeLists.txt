cmake_minimum_required (VERSION 3.12.0)

project ( cafTestWebApplication )

find_package(Qt5 COMPONENTS REQUIRED Core )
set(QT_LIBRARIES Qt5::Core)

set(WEBTOOLKIT_PREFIX "" CACHE PATH "WebToolKit : Path to WebToolKit")
if(WEBTOOLKIT_PREFIX)
#	message(STATUS "Using web toolkit prefix: ${WEBTOOLKIT_PREFIX}")
else()
	message(ERROR "Need web toolkit")
endif(WEBTOOLKIT_PREFIX)

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

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.60 COMPONENTS system program_options thread filesystem date_time regex REQUIRED)
#message(STATUS "Boost Library Dir: ${Boost_LIBRARY_DIRS}")
link_directories(${Boost_LIBRARY_DIRS})

# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
    ${QRC_FILES_CPP}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
	${WEBTOOLKIT_PREFIX}/include
	${Boost_INCLUDE_DIRS}
)

find_library(WT_LIB NAMES wtd PATHS ${WEBTOOLKIT_PREFIX}/lib)
find_library(WTHTTP_LIB NAMES wthttpd PATHS ${WEBTOOLKIT_PREFIX}/lib)

#message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
#message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

set (EXTERNAL_LINK_LIBRARIES
    )

if (UNIX)
    list(APPEND EXTERNAL_LINK_LIBRARIES
        rt
        pthread
        z
    )
endif()

add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_ALL_DYN_LINK)

target_link_libraries ( ${PROJECT_NAME}
    ${EXTERNAL_LINK_LIBRARIES}
    cafProjectDataModel
    cafCommand
    cafCommandFeatures
    cafCommonInterface
    cafWebInterface
    ${EXTERNAL_LINK_LIBRARIES}
    ${QT_LIBRARIES}
	${Boost_LIBRARIES}
    ${WT_LIB}
    ${WTHTTP_LIB}
)

source_group("" FILES ${PROJECT_FILES})

foreach (qtlib ${QT_LIBRARIES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		           COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach(qtlib)

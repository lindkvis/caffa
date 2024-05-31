cmake_minimum_required(VERSION 3.16)

project(caffaDataModel_UnitTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_FILES
    cafDataModel_UnitTests.cpp
    cafDataModelBasicTest.cpp
    cafChildArrayFieldHandleTest.cpp
    cafSignalTest.cpp
    Child.cpp
    Child.h
    Parent.cpp
    Parent.h
    TestObj.cpp
    TestObj.h)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("" FILES ${PROJECT_FILES})

find_package(Boost 1.71.0 REQUIRED COMPONENTS program_options)

target_link_libraries(${PROJECT_NAME} gtest caffaDataModel Boost::program_options ${THREAD_LIBRARY})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

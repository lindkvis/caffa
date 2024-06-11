cmake_minimum_required(VERSION 3.16)

project(caffaCore_UnitTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_FILES
    cafCore_UnitTests.cpp
    cafCoreBasicTest.cpp
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

find_package(GTest CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} GTest::gtest caffaCore ${THREAD_LIBRARY})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

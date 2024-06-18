cmake_minimum_required(VERSION 3.16)

project(caffaDataModel_UnitTests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_FILES
    cafDataModel_UnitTests.cpp
    cafDataModelBasicTest.cpp
    cafChildArrayFieldHandleTest.cpp
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

target_link_libraries(${PROJECT_NAME} GTest::gtest caffaDataModel ${THREAD_LIBRARY})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

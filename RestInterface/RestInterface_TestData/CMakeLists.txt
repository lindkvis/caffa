cmake_minimum_required(VERSION 3.22)

project(caffaTestData)

set(PROJECT_FILES
    DemoObject.h
    DemoObject.cpp
    Parent.h
    Parent.cpp
    Child.h
    Child.cpp
    TestObj.h
    TestObj.cpp)

add_library(${PROJECT_NAME} ${PROJECT_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

source_group("" FILES ${PROJECT_FILES})

target_link_libraries(${PROJECT_NAME} caffaCore)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

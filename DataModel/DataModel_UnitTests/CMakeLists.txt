cmake_minimum_required(VERSION 3.22)

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

find_package(Boost 1.74.0 REQUIRED COMPONENTS regex)
find_package(GTest REQUIRED)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("" FILES ${PROJECT_FILES})

target_link_libraries(${PROJECT_NAME} GTest::gtest caffaDataModel ${THREAD_LIBRARY} Boost::regex)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

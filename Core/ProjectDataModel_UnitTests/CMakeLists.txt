cmake_minimum_required(VERSION 3.22)

project(caffaProjectDataModel_UnitTests)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_FILES cafBasicTest.cpp cafProjectDataModel_UnitTests.cpp Child.cpp Parent.cpp TestObj.cpp)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

find_package(Boost 1.83.0 REQUIRED COMPONENTS json)
find_package(GTest REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE caffaCore GTest::gtest ${THREAD_LIBRARY} Boost::json)

source_group("" FILES ${PROJECT_FILES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

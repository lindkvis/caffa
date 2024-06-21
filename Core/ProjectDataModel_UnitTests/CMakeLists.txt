cmake_minimum_required(VERSION 3.22)

project(caffaProjectDataModel_UnitTests)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_FILES cafBasicTest.cpp cafProjectDataModel_UnitTests.cpp cafSignalTest.cpp Child.cpp Parent.cpp TestObj.cpp)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

find_package(nlohmann_json CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE caffaCore GTest::gtest ${THREAD_LIBRARY} nlohmann_json::nlohmann_json)

source_group("" FILES ${PROJECT_FILES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

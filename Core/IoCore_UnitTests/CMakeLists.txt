cmake_minimum_required(VERSION 3.22)

project(caffaIoCore_UnitTests)

# add the executable
add_executable(${PROJECT_NAME} cafIo_UnitTests.cpp cafIoBasicTest.cpp cafAdvancedTemplateTest.cpp cafIoNumberTest.cpp cafReadmeObjects.cpp)

find_package(Boost 1.83.0 REQUIRED COMPONENTS json)
find_package(GTest REQUIRED)

target_link_libraries(${PROJECT_NAME} caffaCore ${THREAD_LIBRARY} Boost::json GTest::gtest)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

source_group("" FILES ${PROJECT_FILES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

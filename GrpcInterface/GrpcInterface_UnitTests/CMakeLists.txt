cmake_minimum_required(VERSION 3.12)

project(caffaGrpcInterface_UnitTests)

set(PROJECT_FILES cafGrpcInterface_UnitTests.cpp cafGrpcInterfaceBasicTest.cpp ServerApp.h ServerApp.cpp)

add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("" FILES ${PROJECT_FILES})

find_package(Boost 1.65.0 REQUIRED COMPONENTS program_options)

target_link_libraries(
  ${PROJECT_NAME}
  Boost::program_options
  ${THREAD_LIBRARY}
  gtest
  caffaProjectDataModel
  caffaGrpcInterface
  caffaGrpcInterface_TestData)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

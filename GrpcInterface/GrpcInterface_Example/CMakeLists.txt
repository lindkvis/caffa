cmake_minimum_required(VERSION 3.12)

project(caffaGrpcInterface_Example)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME}Server ${PROJECT_FILES} ExampleServer.cpp)

add_executable(${PROJECT_NAME}Client ${PROJECT_FILES} ExampleClient.cpp)

add_executable(${PROJECT_NAME}Benchmark ${PROJECT_FILES} Benchmark.cpp)

target_link_libraries(${PROJECT_NAME}Server caffaCore caffaGrpcInterface caffaGrpcInterface_TestData ${THREAD_LIBRARY})

target_link_libraries(${PROJECT_NAME}Client caffaCore caffaGrpcInterface caffaGrpcInterface_TestData ${THREAD_LIBRARY})

target_link_libraries(${PROJECT_NAME}Benchmark caffaCore caffaGrpcInterface caffaGrpcInterface_TestData ${THREAD_LIBRARY})

install(
  TARGETS ${PROJECT_NAME}Client ${PROJECT_NAME}Server ${PROJECT_NAME}Benchmark
  LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

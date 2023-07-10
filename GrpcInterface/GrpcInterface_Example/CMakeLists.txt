cmake_minimum_required(VERSION 3.16)

project(caffaGrpcInterface_Example)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME}Server ${PROJECT_FILES} ExampleServer.cpp)

add_executable(${PROJECT_NAME}Client ${PROJECT_FILES} ExampleClient.cpp)

target_link_libraries(${PROJECT_NAME}Server caffaCore caffaGrpcInterface caffaTestData ${THREAD_LIBRARY})

target_link_libraries(${PROJECT_NAME}Client caffaCore caffaGrpcInterface caffaTestData ${THREAD_LIBRARY})

install(
  TARGETS ${PROJECT_NAME}Client ${PROJECT_NAME}Server
  LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

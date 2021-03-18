cmake_minimum_required (VERSION 3.12)

project ( cafGrpcInterface_Example )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( PROJECT_FILES
    DemoObject.h
    DemoObject.cpp
)

add_executable (${PROJECT_NAME}Server
	${PROJECT_FILES}
    ExampleServer.cpp
)

target_link_libraries ( ${PROJECT_NAME}Server
    cafProjectDataModel
    cafGrpcInterface
    ${THREAD_LIBRARY}
)

add_executable (${PROJECT_NAME}Client
	${PROJECT_FILES}
    ExampleClient.cpp
)

target_link_libraries ( ${PROJECT_NAME}Client
    cafProjectDataModel
    cafGrpcInterface
    ${THREAD_LIBRARY}
)

install(TARGETS ${PROJECT_NAME}Client ${PROJECT_NAME}Server
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

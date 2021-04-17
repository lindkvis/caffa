cmake_minimum_required (VERSION 3.12)

project ( cafGrpcInterface_UnitTests )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( PROJECT_FILES
	cafGrpcInterface_UnitTests.cpp

	cafGrpcInterfaceBasicTest.cpp
    Parent.h
    Parent.cpp
    Child.h
    Child.cpp
    TestObj.h
    TestObj.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
    cafGrpcInterface
    ${THREAD_LIBRARY}
    gtest
)

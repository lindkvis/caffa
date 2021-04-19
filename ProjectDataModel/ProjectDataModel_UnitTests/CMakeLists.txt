cmake_minimum_required (VERSION 3.12)

project ( caffaProjectDataModel_UnitTests )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( PROJECT_FILES
	cafBasicTest.cpp
	cafProjectDataModel_UnitTests.cpp
    Child.cpp
    Parent.cpp
    TestObj.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

target_link_libraries ( ${PROJECT_NAME}
    PRIVATE
	caffaProjectDataModel         
    gtest
    ${THREAD_LIBRARY}
    nlohmann_json::nlohmann_json
)

source_group("" FILES ${PROJECT_FILES})

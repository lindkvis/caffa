cmake_minimum_required (VERSION 3.12)

project ( cafProjectDataModel_UnitTests )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( PROJECT_FILES
	cafPdmBasicTest.cpp
	cafProjectDataModel_UnitTests.cpp
    Child.cpp
    Parent.cpp
    TestObj.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
	gtest/gtest-all.cpp
)

target_link_libraries ( ${PROJECT_NAME}
    PRIVATE
	cafProjectDataModel         
    ${THREAD_LIBRARY}
    nlohmann_json::nlohmann_json
)

source_group("" FILES ${PROJECT_FILES})

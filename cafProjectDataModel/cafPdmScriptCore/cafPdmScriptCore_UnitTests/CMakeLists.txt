cmake_minimum_required (VERSION 3.12)

project ( cafPdmScriptCore_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmScripting_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmScriptingBasicTest.cpp
)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
    cafPdmIoCore
    cafPdmUiCore
    cafPdmObject
	cafPdmScriptCore
    ${THREAD_LIBRARY}
)

source_group("" FILES ${PROJECT_FILES})

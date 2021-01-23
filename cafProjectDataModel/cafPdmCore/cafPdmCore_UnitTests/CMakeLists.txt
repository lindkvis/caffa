cmake_minimum_required (VERSION 3.12)

project ( cafPdmCore_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set( PROJECT_FILES

	cafPdmCore_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmCoreBasicTest.cpp
    cafPdmReferenceHelperTest.cpp
    cafChildArrayFieldHandleTest.cpp
	cafSignalTest.cpp

	Child.cpp
	Child.h
	Parent.cpp
	Parent.h
	TestObj.cpp
	TestObj.h
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
    ${THREAD_LIBRARY}
)

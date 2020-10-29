cmake_minimum_required (VERSION 3.12)

project ( cafPdmCore_UnitTests )

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Gui)

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
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

#Copy Qt Dlls
foreach (qtlib ${QT_LIBRARIES})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
endforeach(qtlib)

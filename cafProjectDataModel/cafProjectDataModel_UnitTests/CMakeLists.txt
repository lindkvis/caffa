cmake_minimum_required (VERSION 3.12)

project ( cafProjectDataModel_UnitTests )

find_package(Qt5 COMPONENTS REQUIRED Core Xml Gui)
set(QT_LIBRARIES Qt5::Core Qt5::Xml Qt5::Gui)

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
	cafProjectDataModel 
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

source_group("" FILES ${PROJECT_FILES})

# Copy Qt Dlls
foreach (qtlib ${QT_LIBRARIES})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
endforeach(qtlib)

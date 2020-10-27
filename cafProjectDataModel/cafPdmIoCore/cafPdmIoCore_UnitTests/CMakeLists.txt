cmake_minimum_required (VERSION 3.12)

project ( cafPdmIoCore_UnitTests )

find_package(Qt5 COMPONENTS REQUIRED Core Xml)
set(QT_LIBRARIES Qt5::Core Qt5::Xml)

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmXml_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmXmlBasicTest.cpp
	cafPdmAdvancedTemplateTest.cpp
	cafPdmXmlNumberTest.cpp
	cafPdmPtrArrayTest.cpp
)

set(QT_LIBRARIES Qt5::Core Qt5::Xml)

target_link_libraries ( ${PROJECT_NAME}
	cafPdmIoCore
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

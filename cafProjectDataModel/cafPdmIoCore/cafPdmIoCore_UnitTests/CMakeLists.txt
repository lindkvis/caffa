cmake_minimum_required (VERSION 3.12)

project ( cafPdmIoCore_UnitTests )

# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmXml_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmXmlBasicTest.cpp
	cafPdmAdvancedTemplateTest.cpp
	cafPdmXmlNumberTest.cpp
	cafPtrArrayTest.cpp
)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmIoCore    
    ${THREAD_LIBRARY}
    nlohmann_json::nlohmann_json
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})

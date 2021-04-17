cmake_minimum_required (VERSION 3.12)

project ( cafIoCore_UnitTests )

# add the executable 
add_executable (${PROJECT_NAME}
	cafXml_UnitTests.cpp

	cafXmlBasicTest.cpp
	cafAdvancedTemplateTest.cpp
	cafXmlNumberTest.cpp
	cafPtrArrayTest.cpp
)

target_link_libraries ( ${PROJECT_NAME}
    cafIoCore    
    ${THREAD_LIBRARY}
    nlohmann_json::nlohmann_json
	gtest
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})

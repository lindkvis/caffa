cmake_minimum_required (VERSION 3.12)

project (cafCommandFeatures)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# These headers need to go through Qt's MOC compiler
set (MOC_HEADER_FILES    
 
)

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)
if (NOT CAFFA_WEB_DEPLOYMENT)
	find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )

set( PROJECT_FILES

    # Default features
    AddAndDelete/cafCmdAddItemExec.cpp
    AddAndDelete/cafCmdAddItemExec.h
    AddAndDelete/cafCmdAddItemExecData.cpp
    AddAndDelete/cafCmdAddItemExecData.h
    AddAndDelete/cafCmdAddItemFeature.cpp
    AddAndDelete/cafCmdAddItemFeature.h
    AddAndDelete/cafCmdDeleteItemExec.cpp
    AddAndDelete/cafCmdDeleteItemExec.h
    AddAndDelete/cafCmdDeleteItemExecData.cpp
    AddAndDelete/cafCmdDeleteItemExecData.h
    AddAndDelete/cafCmdDeleteItemFeature.cpp
    AddAndDelete/cafCmdDeleteItemFeature.h
)

if (NOT CAFFA_WEB_DEPLOYMENT)
	list(APPEND PROJECT_FILES
		ToggleCommands/cafToggleItemsFeature.cpp
		ToggleCommands/cafToggleItemsFeature.h
		ToggleCommands/cafToggleItemsFeatureImpl.cpp
		ToggleCommands/cafToggleItemsFeatureImpl.h
		ToggleCommands/cafToggleItemsOffFeature.cpp
		ToggleCommands/cafToggleItemsOffFeature.h
		ToggleCommands/cafToggleItemsOnFeature.cpp
		ToggleCommands/cafToggleItemsOnFeature.h
		ToggleCommands/cafToggleItemsOnOthersOffFeature.cpp
		ToggleCommands/cafToggleItemsOnOthersOffFeature.h
	)
endif()

# NOTE! Resources in this subfolder appends to the variable QRC_FILES in parent scope
# CMakeList.txt in the application folder (parent scope) must use the following syntax
# to make sure the QRC_FILES variable contains appended files in subfolders

#   set( QRC_FILES
#       ${QRC_FILES} 
#       <Path/To/ApplicationResourceFile.qrc>
#   )

set( QRC_FILES
    ${QRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/cafCommandFeatures.qrc
    PARENT_SCOPE
)

add_library( ${PROJECT_NAME} OBJECT
 	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(INTERFACE_LIB "cafUserInterface")
if (CAFFA_WEB_DEPLOYMENT)
	set(INTERFACE_LIB "cafWebInterface")
endif()

target_link_libraries ( ${PROJECT_NAME}
    cafCommand
	${INTERFACE_LIB}
    ${QT_LIBRARIES}
)


if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})


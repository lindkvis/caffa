cmake_minimum_required (VERSION 3.12)

project (cafCommandFeatures)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)
if (NOT CAFFA_WEB_DEPLOYMENT)
	find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

set (PUBLIC_HEADERS
    # Default features
    AddAndDelete/cafCmdAddItemExec.h
    AddAndDelete/cafCmdAddItemExecData.h
    AddAndDelete/cafCmdAddItemFeature.h
    AddAndDelete/cafCmdDeleteItemExec.h
    AddAndDelete/cafCmdDeleteItemExecData.h
    AddAndDelete/cafCmdDeleteItemFeature.h
)

if (NOT CAFFA_WEB_DEPLOYMENT)
	list(APPEND PUBLIC_HEADERS
		ToggleCommands/cafToggleItemsFeature.h
		ToggleCommands/cafToggleItemsFeatureImpl.h
		ToggleCommands/cafToggleItemsOffFeature.h
		ToggleCommands/cafToggleItemsOnFeature.h
		ToggleCommands/cafToggleItemsOnOthersOffFeature.h
	)
endif()

set( PROJECT_FILES
    ${PUBLIC_HEADERS}
    # Default features
    AddAndDelete/cafCmdAddItemExec.cpp
    AddAndDelete/cafCmdAddItemExecData.cpp
    AddAndDelete/cafCmdAddItemFeature.cpp
    AddAndDelete/cafCmdDeleteItemExec.cpp
    AddAndDelete/cafCmdDeleteItemExecData.cpp
    AddAndDelete/cafCmdDeleteItemFeature.cpp
)

if (NOT CAFFA_WEB_DEPLOYMENT)
	list(APPEND PROJECT_FILES
		ToggleCommands/cafToggleItemsFeature.cpp
		ToggleCommands/cafToggleItemsFeatureImpl.cpp
		ToggleCommands/cafToggleItemsOffFeature.cpp
		ToggleCommands/cafToggleItemsOnFeature.cpp
		ToggleCommands/cafToggleItemsOnOthersOffFeature.cpp
	)
endif()

add_library( ${PROJECT_NAME}
 	${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

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
    cafProjectDataModel
	${INTERFACE_LIB}
    ${QT_LIBRARIES}
)


if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})


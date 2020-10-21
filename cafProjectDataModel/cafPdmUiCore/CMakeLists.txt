cmake_minimum_required (VERSION 3.12)

project (cafPdmUiCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set (PUBLIC_HEADERS
    cafPdmUiCore_export.h

    cafInternalPdmFieldTypeSpecializations.h
    cafInternalPdmUiFieldCapability.h
    cafInternalPdmUiFieldCapability.inl
    cafInternalPdmUiCommandSystemInterface.h

    cafFontTools.h
    cafColorTable.h
    cafColorTables.h
    cafColorTools.h

    cafActionWrapper.h
    cafQActionWrapper.h
    cafPdmUiEditorAttribute.h
    cafPdmUiEditorHandle.h
    cafPdmUiFieldEditorHandle.h
    cafPdmUiFieldHandle.h
    cafPdmUiFieldSpecialization.h
    cafPdmUiItem.h
    cafPdmUiModelChangeDetector.h
    cafPdmUiObjectEditorHandle.h
    cafPdmUiWidgetObjectEditorHandle.h
    cafPdmUi3dObjectEditorHandle.h
    cafPdmUiObjectHandle.h
    cafPdmUiOrdering.h
    cafPdmUiCommandSystemProxy.h
    cafPdmUiTreeOrdering.h
    cafUiTreeItem.h
    cafPdmUiGroup.h

    cafSelectionManager.h
    cafSelectionChangedReceiver.h
    cafSelectionManagerTools.h
    cafPdmUiSelection3dEditorVisualizer.h
    cafIconProvider.h
)

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

include_directories (
    ..
)

add_definitions(-D${PROJECT_NAME}_LIBRARY)

set( PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafFontTools.cpp
    cafColorTable.cpp
    cafColorTables.cpp
    cafColorTools.cpp

    cafQActionWrapper.cpp
    cafPdmUiEditorHandle.cpp
    cafPdmUiFieldEditorHandle.cpp
    cafPdmUiFieldHandle.cpp
    cafPdmUiItem.cpp
    cafPdmUiModelChangeDetector.cpp
    cafPdmUiObjectEditorHandle.cpp
    cafPdmUiWidgetObjectEditorHandle.cpp
    cafPdmUi3dObjectEditorHandle.cpp
    cafPdmUiObjectHandle.cpp
    cafPdmUiOrdering.cpp
    cafPdmUiCommandSystemProxy.cpp
    cafPdmUiTreeOrdering.cpp
    cafPdmUiGroup.cpp

    cafSelectionManager.cpp
    cafSelectionChangedReceiver.cpp
    cafPdmUiSelection3dEditorVisualizer.cpp
    cafIconProvider.cpp
)

add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
	${QT_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

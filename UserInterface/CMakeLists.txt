cmake_minimum_required (VERSION 3.12)

project (caffaUserInterface)

# Unity Build
if (CAFFA_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC)
    # Define this one to tell windows.h to not define min() and max() as macros
    add_definitions(-DNOMINMAX)
endif (MSVC)

add_definitions(-D${PROJECT_NAME}_LIBRARY)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})


find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

set( PROJECT_FILES
    # field editors
    cafUiFieldEditorHandle.h
    cafUiFieldEditorHandle.cpp
    cafUiCheckBoxDelegate.cpp
    cafUiCheckBoxDelegate.h
    cafUiCheckBoxEditor.cpp
    cafUiCheckBoxEditor.h
    cafUiCheckBoxTristateEditor.cpp
    cafUiCheckBoxTristateEditor.h
	cafUiColorEditor.cpp
    cafUiColorEditor.h
	cafUiComboBoxEditor.cpp
    cafUiComboBoxEditor.h
	cafUiDateEditor.cpp
    cafUiDateEditor.h
	cafUiTimeEditor.cpp
    cafUiTimeEditor.h   
    cafUiDoubleSliderEditor.cpp
    cafUiDoubleSliderEditor.h
    cafUiLineEditor.cpp
    cafUiLineEditor.h
    cafUiListEditor.cpp
    cafUiListEditor.h
    cafUiPushButtonEditor.cpp
    cafUiPushButtonEditor.h
    cafUiSliderEditor.cpp
    cafUiSliderEditor.h
    cafUiTextEditor.cpp
    cafUiTextEditor.h
    cafUiToolBarEditor.cpp
    cafUiToolBarEditor.h
    cafUiToolButtonEditor.cpp
    cafUiToolButtonEditor.h
    cafUiTreeSelectionEditor.h
    cafUiTreeSelectionEditor.cpp
    cafUiTreeSelectionQModel.h
    cafUiTreeSelectionQModel.cpp
    cafUiFieldEditorHelper.h
    cafUiFieldEditorHelper.cpp
    cafUiLabelEditor.h
    cafUiLabelEditor.cpp
    cafUiDragDropInterface.h

    # object editors
    cafUiDefaultObjectEditor.cpp
    cafUiDefaultObjectEditor.h
    cafUiWidgetObjectEditorHandle.h
    cafUiWidgetObjectEditorHandle.cpp
    cafUiListView.cpp
    cafUiListView.h
    cafUiListViewEditor.cpp
    cafUiListViewEditor.h
    cafUiTableRowEditor.cpp
    cafUiTableRowEditor.h
    cafUiTableView.cpp
    cafUiTableView.h
    cafUiTableViewDelegate.cpp
    cafUiTableViewDelegate.h
    cafUiTableViewEditor.cpp
    cafUiTableViewEditor.h
    cafUiTableViewQModel.cpp
    cafUiTableViewQModel.h
    cafUiTreeEditorHandle.cpp
    cafUiTreeEditorHandle.h
    cafUiTreeItemEditor.cpp
    cafUiTreeItemEditor.h
    cafUiTreeView.cpp
    cafUiTreeView.h
    cafUiTreeViewEditor.cpp
    cafUiTreeViewEditor.h
    cafUiTreeViewQModel.cpp
    cafUiTreeViewQModel.h
    cafUiPropertyView.cpp
    cafUiPropertyView.h
    cafUiPropertyViewDialog.cpp
    cafUiPropertyViewDialog.h
    cafUiFormLayoutObjectEditor.cpp
    cafUiFormLayoutObjectEditor.h
    cafUiDoubleValueEditor.cpp
    cafUiDoubleValueEditor.h

    # div
    cafAboutDialog.cpp
    cafAboutDialog.h
    cafProgressInfo.cpp
    cafProgressInfo.h
    cafUiProcess.cpp
    cafUiProcess.h
    QMinimizePanel.cpp
    QMinimizePanel.h
    cafQActionWrapper.cpp
    cafQActionWrapper.h
    cafQTreeViewStateSerializer.h
    cafQTreeViewStateSerializer.cpp
    cafMemoryInspector.h
    cafMemoryInspector.cpp
    cafDoubleStringValidator.h
    cafDoubleStringValidator.cpp
    cafPickEventHandler.h
    cafUiPickableLineEditor.h
    cafUiPickableLineEditor.cpp
    cafUiTabbedPropertyViewDialog.h
    cafUiTabbedPropertyViewDialog.cpp

    cafQVariantConverter.cpp
    cafQVariantConverter.h
)

if (CAFFA_BUILD_SHARED)
    message(STATUS "Building ${PROJECT_NAME} shared")
    add_library( ${PROJECT_NAME} SHARED
        ${PROJECT_FILES}
    )
else()
    add_library( ${PROJECT_NAME} STATIC
        ${PROJECT_FILES}
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTORCC ON)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif() 

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    ${QT_LIBRARIES}
    nlohmann_json::nlohmann_json
    caffaProjectDataModel
    caffaUiCore
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

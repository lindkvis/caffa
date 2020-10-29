cmake_minimum_required (VERSION 3.12)

project (cafUserInterface)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC)
    # Define this one to tell windows.h to not define min() and max() as macros
    add_definitions(-DNOMINMAX)
endif (MSVC)

add_definitions(-D${PROJECT_NAME}_LIBRARY)

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

set( PROJECT_FILES
    cafUserInterface_export.h

    # field editors
    cafPdmUiCheckBoxDelegate.cpp
    cafPdmUiCheckBoxDelegate.h
    cafPdmUiCheckBoxEditor.cpp
    cafPdmUiCheckBoxEditor.h
    cafPdmUiCheckBoxTristateEditor.cpp
    cafPdmUiCheckBoxTristateEditor.h
	cafPdmUiColorEditor.cpp
    cafPdmUiColorEditor.h
	cafPdmUiComboBoxEditor.cpp
    cafPdmUiComboBoxEditor.h
	cafPdmUiDateEditor.cpp
    cafPdmUiDateEditor.h
	cafPdmUiTimeEditor.cpp
    cafPdmUiTimeEditor.h   
    cafPdmUiDoubleSliderEditor.cpp
    cafPdmUiDoubleSliderEditor.h
	cafPdmUiFilePathEditor.cpp
    cafPdmUiFilePathEditor.h
    cafPdmUiLineEditor.cpp
    cafPdmUiLineEditor.h
    cafPdmUiListEditor.cpp
    cafPdmUiListEditor.h
    cafPdmUiPushButtonEditor.cpp
    cafPdmUiPushButtonEditor.h
    cafPdmUiSliderEditor.cpp
    cafPdmUiSliderEditor.h
    cafPdmUiTextEditor.cpp
    cafPdmUiTextEditor.h
    cafPdmUiToolBarEditor.cpp
    cafPdmUiToolBarEditor.h
    cafPdmUiToolButtonEditor.cpp
    cafPdmUiToolButtonEditor.h
    cafPdmUiTreeSelectionEditor.h
    cafPdmUiTreeSelectionEditor.cpp
    cafPdmUiTreeSelectionQModel.h
    cafPdmUiTreeSelectionQModel.cpp
    cafUiFieldEditorHelper.h
    cafUiFieldEditorHelper.cpp
    cafPdmUiLabelEditor.h
    cafPdmUiLabelEditor.cpp
	

    # object editors
    cafPdmUiDefaultObjectEditor.cpp
    cafPdmUiDefaultObjectEditor.h
    cafPdmUiListView.cpp
    cafPdmUiListView.h
    cafPdmUiListViewEditor.cpp
    cafPdmUiListViewEditor.h
    cafPdmUiTableRowEditor.cpp
    cafPdmUiTableRowEditor.h
    cafPdmUiTableView.cpp
    cafPdmUiTableView.h
    cafPdmUiTableViewDelegate.cpp
    cafPdmUiTableViewDelegate.h
    cafPdmUiTableViewEditor.cpp
    cafPdmUiTableViewEditor.h
    cafPdmUiTableViewQModel.cpp
    cafPdmUiTableViewQModel.h
    cafPdmUiTreeEditorHandle.cpp
    cafPdmUiTreeEditorHandle.h
    cafPdmUiTreeItemEditor.cpp
    cafPdmUiTreeItemEditor.h
    cafPdmUiTreeView.cpp
    cafPdmUiTreeView.h
    cafPdmUiTreeViewEditor.cpp
    cafPdmUiTreeViewEditor.h
    cafPdmUiTreeViewQModel.cpp
    cafPdmUiTreeViewQModel.h
    cafPdmUiPropertyView.cpp
    cafPdmUiPropertyView.h
    cafPdmUiPropertyViewDialog.cpp
    cafPdmUiPropertyViewDialog.h
    cafPdmUiFormLayoutObjectEditor.cpp
    cafPdmUiFormLayoutObjectEditor.h
    cafPdmUiDoubleValueEditor.cpp
    cafPdmUiDoubleValueEditor.h


    # div
    cafAboutDialog.cpp
    cafAboutDialog.h
    cafProgressInfo.cpp
    cafProgressInfo.h
    cafUiProcess.cpp
    cafUiProcess.h
    QMinimizePanel.cpp
    QMinimizePanel.h
    cafQTreeViewStateSerializer.h
    cafQTreeViewStateSerializer.cpp
    cafMemoryInspector.h
    cafMemoryInspector.cpp
    cafPdmUniqueIdValidator.h
    cafPdmUniqueIdValidator.cpp
    cafPdmDoubleStringValidator.h
    cafPdmDoubleStringValidator.cpp
    cafPickEventHandler.h
    cafPdmUiPickableLineEditor.h
    cafPdmUiPickableLineEditor.cpp
    cafPdmUiTabbedPropertyViewDialog.h
    cafPdmUiTabbedPropertyViewDialog.cpp
)

add_library( ${PROJECT_NAME}
    ${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif() 

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
    cafCommonInterface
    cafPdmUiCore
    ${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

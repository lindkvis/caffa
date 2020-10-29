cmake_minimum_required (VERSION 3.12)

project (cafCommonInterface)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

if (MSVC)
    # Define this one to tell windows.h to not define min() and max() as macros
    add_definitions(-DNOMINMAX)
endif (MSVC)

find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)

set (PUBLIC_HEADERS
    cafUiDragDropInterface.h
 	cafUiColorEditorAttribute.h
 	cafUiComboBoxEditorAttribute.h
 	cafUiDateEditorAttribute.h
 	cafUiFilePathEditorAttribute.h
 	cafUiSliderEditorAttribute.h
 	cafUiTreeViewAttribute.h

    cafPdmCoreColor.h
    cafUiCoreColor.h
    cafPdmXmlColor.h
    cafFieldColor.h

    cafCurveInterface.h
    cafPlotViewerInterface.h
)

set( PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafUiCoreColor.cpp
    cafPdmXmlColor.cpp
    cafCurveInterface.cpp
    cafPlotViewerInterface.cpp
    cafCurveDataTools.cpp

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
    ${QT_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})


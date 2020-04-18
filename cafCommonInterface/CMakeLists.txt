cmake_minimum_required (VERSION 3.12.0)

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

set( PROJECT_FILES
    cafPdmUiDragDropInterface.h
 	cafPdmUiColorEditorAttribute.h
 	cafPdmUiComboBoxEditorAttribute.h
 	cafPdmUiDateEditorAttribute.h
 	cafPdmUiFilePathEditorAttribute.h
 	cafPdmUiSliderEditorAttribute.h
 	cafPdmUiTreeViewAttribute.h
    
    cafColor3.h
    cafColor3.cpp
    
    cafPdmCoreColor3f.h
    cafPdmUiCoreColor3f.cpp
    cafPdmUiCoreColor3f.h
    cafPdmXmlColor3f.cpp
    cafPdmXmlColor3f.h
    cafPdmFieldCafColor.h

)

add_library( ${PROJECT_NAME}
             ${PROJECT_FILES}            
)

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

source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()

cmake_minimum_required(VERSION 3.12)

project(caffaUiCore)

# Unity Build
if(CAFFA_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set(PUBLIC_HEADERS
    cafInternalUiFieldSpecializations.h
    cafInternalUiFieldCapability.h
    cafInternalUiFieldCapability.inl
    cafInternalUiCommandSystemInterface.h
    cafFontTools.h
    cafActionWrapper.h
    cafOptionItemInfo.h
    cafUiEditorAttribute.h
    cafUiEditorHandle.h
    cafFieldUiCapability.h
    cafUiFieldSpecialization.h
    cafUiItem.h
    cafUiObjectEditorHandle.h
    cafObjectUiCapability.h
    cafUiOrdering.h
    cafUiCommandSystemProxy.h
    cafUiTreeOrdering.h
    cafUiTreeItem.h
    cafUiGroup.h
    cafSelectionManager.h
    cafSelectionChangedReceiver.h
    cafSelectionManagerTools.h
    cafUiComboBoxEditorAttribute.h
    cafUiDateEditorAttribute.h
    cafUiFilePathEditorAttribute.h
    cafUiSliderEditorAttribute.h
    cafUiTreeViewAttribute.h)

include_directories(..)

add_definitions(-D${PROJECT_NAME}_LIBRARY)

set(PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafFontTools.cpp
    cafUiEditorHandle.cpp
    cafFieldUiCapability.cpp
    cafOptionItemInfo.cpp
    cafUiItem.cpp
    cafUiObjectEditorHandle.cpp
    cafObjectUiCapability.cpp
    cafUiOrdering.cpp
    cafUiCommandSystemProxy.cpp
    cafUiTreeOrdering.cpp
    cafUiGroup.cpp
    cafSelectionManager.cpp
    cafSelectionChangedReceiver.cpp)

if(CAFFA_BUILD_SHARED)
  message(STATUS "Building ${PROJECT_NAME} shared")
  add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})
else()
  add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_link_libraries(${PROJECT_NAME} caffaCore)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

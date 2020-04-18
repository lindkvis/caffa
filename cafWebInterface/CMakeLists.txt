cmake_minimum_required (VERSION 3.12.0)

project (cafWebInterface)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "-std=c++14")
endif() 

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

if (MSVC)
    # Define this one to tell windows.h to not define min() and max() as macros
    add_definitions(-DNOMINMAX)
endif (MSVC)

find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )	

set(WEBTOOLKIT_PREFIX "" CACHE PATH "WebToolKit : Path to WebToolKit")
if(WEBTOOLKIT_PREFIX)
	message(STATUS "Using web toolkit prefix: ${WEBTOOLKIT_PREFIX}")
else()
	message(FATAL_ERROR "Need web toolkit to build for web deployment. Please set the WEBTOOLKIT_PREFIX")
endif(WEBTOOLKIT_PREFIX)

set( PROJECT_FILES
	cafPdmWebFieldEditorHandle.cpp
	cafPdmWebFieldEditorHandle.h
	cafPdmWebDefaultObjectEditor.cpp
	cafPdmWebDefaultObjectEditor.h
	cafPdmWebFormLayoutObjectEditor.cpp
	cafPdmWebFormLayoutObjectEditor.h
	cafPdmWebWidgetObjectEditorHandle.cpp
	cafPdmWebWidgetObjectEditorHandle.h
	cafPdmWebTreeView.cpp
	cafPdmWebTreeView.h
	cafPdmWebTreeViewEditor.cpp
	cafPdmWebTreeViewEditor.h
	cafPdmWebTreeItemEditor.cpp
	cafPdmWebTreeItemEditor.h
	cafPdmWebTreeEditorHandle.cpp
	cafPdmWebTreeEditorHandle.h
	cafPdmWebTreeViewWModel.cpp
	cafPdmWebTreeViewWModel.h

	cafWebPlotViewer.h
	cafWebPlotViewer.cpp
	cafWebPlotCurve.h
	cafWebPlotCurve.cpp

    # field editors
    cafPdmWebCheckBoxEditor.cpp
    cafPdmWebCheckBoxEditor.h
    cafPdmWebCheckBoxTristateEditor.cpp
    cafPdmWebCheckBoxTristateEditor.h
	cafPdmWebComboBoxEditor.cpp
	cafPdmWebComboBoxEditor.h
    cafPdmWebLineEditor.cpp
    cafPdmWebLineEditor.h
    cafPdmWebColorEditor.cpp
    cafPdmWebColorEditor.h
    cafPdmWebDateEditor.cpp
    cafPdmWebDateEditor.h
	cafPdmWebFileUploadEditor.cpp
    cafPdmWebFileUploadEditor.h
	cafPdmWebSliderEditor.cpp
	cafPdmWebSliderEditor.h
	cafPdmWebDoubleSliderEditor.cpp
	cafPdmWebDoubleSliderEditor.h

	WPopupMenuWrapper.cpp
	WPopupMenuWrapper.h
)

add_library( ${PROJECT_NAME}    
			 ${PROJECT_FILES}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
	set( OTHER_DEPENDENCIES d2d1 Shlwapi windowscodecs Dwrite)
endif() 

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.60 COMPONENTS system regex program_options thread filesystem date_time REQUIRED)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
	${WEBTOOLKIT_PREFIX}/include
	${Boost_INCLUDE_DIRS}
)
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
link_directories(${Boost_LIBRARY_DIRS})

#find_library(WT_LIBD NAMES wtd PATHS ${WEBTOOLKIT_PREFIX}/lib)
#find_library(WTHTTP_LIBD NAMES wthttpd PATHS ${WEBTOOLKIT_PREFIX}/lib)
find_library(WT_LIB NAMES wt PATHS ${WEBTOOLKIT_PREFIX}/lib)
find_library(WTHTTP_LIB NAMES wthttp PATHS ${WEBTOOLKIT_PREFIX}/lib)

message(STATUS "WTHTTP_LIBS: " ${WTHTTP_LIB})

add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_ALL_DYN_LINK)

target_link_libraries ( ${PROJECT_NAME}	
    cafProjectDataModel
	cafCommonInterface
	${OTHER_DEPENDENCIES}
    ${QT_LIBRARIES}
	${WT_LIB}
	${WTHTTP_LIB}
	${Boost_LIBRARIES}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DQT_NO_EMIT)

source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()

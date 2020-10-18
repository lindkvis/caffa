cmake_minimum_required (VERSION 3.12.0)

project (cafWebInterface)

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
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )	

set(WEBTOOLKIT_PREFIX "" CACHE PATH "WebToolKit : Path to WebToolKit")
if(WEBTOOLKIT_PREFIX)
	message(STATUS "Using web toolkit prefix: ${WEBTOOLKIT_PREFIX}")
else()
	message(FATAL_ERROR "Need web toolkit to build for web deployment. Please set the WEBTOOLKIT_PREFIX")
endif(WEBTOOLKIT_PREFIX)

set( PUBLIC_HEADERS
	cafPdmWebFieldEditorHandle.h
	cafPdmWebDefaultObjectEditor.h
	cafPdmWebFormLayoutObjectEditor.h
	cafPdmWebWidgetObjectEditorHandle.h
	cafPdmWebTreeView.h
	cafPdmWebTreeViewEditor.h
	cafPdmWebTreeItemEditor.h
	cafPdmWebTreeEditorHandle.h
	cafPdmWebTreeViewWModel.h

	cafWebPlotViewer.h
	cafWebPlotCurve.h

    # field editors
    cafPdmWebCheckBoxEditor.h
    cafPdmWebCheckBoxTristateEditor.h
	cafPdmWebComboBoxEditor.h
    cafPdmWebLineEditor.h
    cafPdmWebColorEditor.h
    cafPdmWebDateEditor.h
    cafPdmWebFileUploadEditor.h
	cafPdmWebSliderEditor.h
	cafPdmWebDoubleSliderEditor.h

	WPopupMenuWrapper.h
)

set( PROJECT_FILES
	${PUBLIC_HEADERS}
	cafPdmWebFieldEditorHandle.cpp
	cafPdmWebDefaultObjectEditor.cpp
	cafPdmWebFormLayoutObjectEditor.cpp
	cafPdmWebWidgetObjectEditorHandle.cpp
	cafPdmWebTreeView.cpp
	cafPdmWebTreeViewEditor.cpp
	cafPdmWebTreeItemEditor.cpp
	cafPdmWebTreeEditorHandle.cpp
	cafPdmWebTreeViewWModel.cpp

	cafWebPlotViewer.cpp
	cafWebPlotCurve.cpp

    # field editors
    cafPdmWebCheckBoxEditor.cpp
    cafPdmWebCheckBoxTristateEditor.cpp
	cafPdmWebComboBoxEditor.cpp
    cafPdmWebLineEditor.cpp
    cafPdmWebColorEditor.cpp
    cafPdmWebDateEditor.cpp
	cafPdmWebFileUploadEditor.cpp
	cafPdmWebSliderEditor.cpp
	cafPdmWebDoubleSliderEditor.cpp

	WPopupMenuWrapper.cpp
)

add_library( ${PROJECT_NAME}    
			 ${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

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

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

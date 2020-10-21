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

#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

find_package(Boost 1.60 COMPONENTS system regex program_options thread filesystem date_time REQUIRED)
find_package(WT CONFIG REQUIRED)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127 /wd4267")
	set( OTHER_DEPENDENCIES d2d1 Shlwapi windowscodecs Dwrite)
endif() 

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

#add_definitions(-DBOOST_ALL_NO_LIB)
#add_definitions(-DBOOST_ALL_DYN_LINK)

target_link_libraries ( ${PROJECT_NAME}	
    cafProjectDataModel
	cafCommonInterface
    OpenSSL::Crypto Wt::Wt Wt::Dbo Wt::HTTP Wt::DboSqlite3
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

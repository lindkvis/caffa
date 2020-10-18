cmake_minimum_required (VERSION 3.12)

project ( cafTestApplication )

# Qt

# Qt MOC
set (MOC_HEADER_FILES 
	MainWindow.h 
    WidgetLayoutTest.h
    CustomObjectEditor.h
    MenuItemProducer.h
)

# Resource file
set( QRC_FILES  
    ${QRC_FILES} 
    textedit.qrc
)

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
qt5_add_resources( QRC_FILES_CPP ${QRC_FILES} )

include_directories (
    ${cafCommand_SOURCE_DIR}
)
ADD_DEFINITIONS(-DTAP_USE_COMMAND_FRAMEWORK)


set( PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    MainWindow.h
    WidgetLayoutTest.cpp
    WidgetLayoutTest.h
    ManyGroups.cpp
    ManyGroups.h
    CustomObjectEditor.cpp
    CustomObjectEditor.h
    MenuItemProducer.cpp
    MenuItemProducer.h
    TamComboBox.h
    TamComboBox.cpp
)


# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
    ${QRC_FILES_CPP}
	$<TARGET_OBJECTS:cafCommandFeatures> # Needed for cmake version < 3.12. Remove when we can use target_link_libraries with OBJECT libraries
)

set (TAP_LINK_LIBRARIES
    cafUserInterface
    cafCommand
    )

if (UNIX)
    list(APPEND LIST TAP_LINK_LIBRARIES pthread z)
endif()

target_link_libraries ( ${PROJECT_NAME}
    ${TAP_LINK_LIBRARIES}
    ${QT_LIBRARIES}
    ${EXTERNAL_LINK_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})

foreach (qtlib ${QT_LIBRARIES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
endforeach(qtlib)

# cotire
if (COMMAND caf_apply_cotire)
  set_source_files_properties (qrc_textedit.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
  set_source_files_properties (qrc_cafCommandFeatures.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
  set_source_files_properties (qrc_cafAnimControl.cpp PROPERTIES COTIRE_EXCLUDED TRUE)

  caf_apply_cotire("${PROJECT_NAME}")
endif()

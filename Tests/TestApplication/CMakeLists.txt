cmake_minimum_required (VERSION 3.12)

project ( caffaTestApplication )

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

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
)

set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTORCC ON)

if (CAFFA_BUILD_SHARED AND MSVC)
    add_definitions(-DQT_DLL)
endif()

target_link_libraries ( ${PROJECT_NAME}
    ${THREAD_LIBRARY}
    ${QT_LIBRARIES}
    caffaUserInterface
)

source_group("" FILES ${PROJECT_FILES})

foreach (qtlib ${QT_LIBRARIES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
endforeach(qtlib)

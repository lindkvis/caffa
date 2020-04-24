cmake_minimum_required (VERSION 3.12.0)

project (cafUiPlotViewer)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

if (MSVC)
    # Define this one to tell windows.h to not define min() and max() as macros
    add_definitions(-DNOMINMAX)
endif (MSVC)

# These headers need to go through Qt's MOC compiler
set (MOC_HEADER_FILES
)

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )

set( PROJECT_FILES
    cafUiPlotViewer.h
    cafUiPlotViewer.cpp
)

add_library( ${PROJECT_NAME}

 ${PROJECT_FILES}

    ${MOC_SOURCE_FILES}
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
    cafCommonInterface
    cafUserInterface
    qwt
    ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()

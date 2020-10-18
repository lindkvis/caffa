cmake_minimum_required (VERSION 3.12)

project (cafProjectDataModel)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

set(PUBLIC_HEADERS
	cafFactory.h
	cafOmpMutex.h
	cafPdmDocument.h
	cafPdmObjectGroup.h
	cafPdmObject.h
)

set( PROJECT_FILES
	${PUBLIC_HEADERS}
	cafPdmDocument.cpp
	cafPdmObjectGroup.cpp
	cafPdmObject.cpp
)

add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_link_libraries ( ${PROJECT_NAME}
	cafPdmCore
    cafPdmUiCore
    cafPdmXml
    ${QT_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

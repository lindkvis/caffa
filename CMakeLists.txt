cmake_minimum_required (VERSION 3.12)

project (caffa VERSION 0.1 DESCRIPTION "Caffa Application Framework")

set(CMAKE_CXX_STANDARD 17)

option(CAFFA_WEB_DEPLOYMENT "Turn this on to build for web deployment with the Web Toolkit" OFF)
set(CAFFA_DESKTOP_DEPLOYMENT OFF)
if (NOT CAFFA_WEB_DEPLOYMENT)
    set(CAFFA_DESKTOP_DEPLOYMENT ON)
endif()

if (UNIX)
    option(CAFFA_HARDWARE_IO "Build the Caffa Hardware IO library (Raspberry PI)" OFF)
endif()

option(BUILD_SHARED_LIBS "Create a shared Caffa library" ON)
find_package(Qt5 COMPONENTS REQUIRED Core)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (CAFFA_DESKTOP_DEPLOYMENT)
    find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
    add_definitions(-DDESKTOP_DEPLOYMENT)
else()
    set(QT_LIBRARIES Qt5::Core)
endif()

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

if (CAFFA_WEB_DEPLOYMENT)
    add_definitions(-DWEB_DEPLOYMENT)
endif(CAFFA_WEB_DEPLOYMENT)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if( UNIX )
    set(THREAD_LIBRARY "pthread")
endif()

#libraries
add_subdirectory (cafProjectDataModel/cafPdmCore)
add_subdirectory (cafProjectDataModel/cafPdmUiCore)
add_subdirectory (cafProjectDataModel/cafPdmXml)

add_subdirectory(cafProjectDataModel)
add_subdirectory(cafCommand)
add_subdirectory(cafCommandFeatures)
add_subdirectory(cafPdmScripting)
add_subdirectory(cafCommonInterface)

#executables
if (CAFFA_WEB_DEPLOYMENT)
    add_subdirectory(cafWebInterface)
    add_subdirectory(cafTests/cafTestWebApplication)
endif(CAFFA_WEB_DEPLOYMENT)

if (CAFFA_DESKTOP_DEPLOYMENT)
    option(CAFFA_DESKTOP_PLOTTING ON)
    add_subdirectory(cafUserInterface)

    if (CAFFA_DESKTOP_PLOTTING)
        add_subdirectory(ThirdParty/qwt)
        set_property(TARGET qwt PROPERTY FOLDER "ThirdParty")
        add_subdirectory(cafUiPlotViewer)
    endif(CAFFA_DESKTOP_PLOTTING)

    add_subdirectory(cafTests/cafTestApplication)
    add_subdirectory (cafUserInterface/cafUserInterface_UnitTests)
endif()

add_subdirectory (cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory (cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
add_subdirectory (cafProjectDataModel/cafProjectDataModel_UnitTests)
if (CAFFA_HARDWARE_IO AND UNIX)
    add_subdirectory (cafHardwareIO)
endif()
#add_subdirectory(cafPdmScripting/cafPdmScripting_UnitTests)

# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET  cafPdmCore cafPdmCore_UnitTests cafPdmXml cafPdmXml_UnitTests  cafPdmUiCore  PROPERTY FOLDER "PdmCore")

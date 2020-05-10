cmake_minimum_required (VERSION 3.12.0)

project (caffa)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

option(CAFFA_WEB_DEPLOYMENT "Turn this on to build for web deployment with the Web Toolkit" OFF)
option(CAFFA_DESKTOP_DEPLOYMENT "Turn this on to build for desktop deployment with Qt" ON)

find_package(Qt5 COMPONENTS REQUIRED Core)

if (CAFFA_DESKTOP_DEPLOYMENT)
    find_package(Qt5 COMPONENTS REQUIRED Core Gui OpenGL Widgets)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)
else()
    set(QT_LIBRARIES Qt5::Core)
endif()

if (CAFFA_WEB_DEPLOYMENT)
    add_definitions(-DWEB_DEPLOYMENT)
endif(CAFFA_WEB_DEPLOYMENT)

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

if( UNIX )
    set(THREAD_LIBRARY "pthread")
endif()

add_subdirectory (cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory (cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
add_subdirectory (cafProjectDataModel/cafProjectDataModel_UnitTests)
#add_subdirectory(cafPdmScripting/cafPdmScripting_UnitTests)

# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET  cafPdmCore cafPdmCore_UnitTests cafPdmXml cafPdmXml_UnitTests  cafPdmUiCore  PROPERTY FOLDER "PdmCore")

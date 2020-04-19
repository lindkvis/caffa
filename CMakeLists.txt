cmake_minimum_required (VERSION 3.12.0)

project (wcaf)

set(CMAKE_CXX_STANDARD 14)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message("Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

option(WebDeployment "Turn this on to build for the WebToolkit. Off to build for Desktop Qt5" OFF)

find_package(Qt5 COMPONENTS REQUIRED Core)
if (NOT WebDeployment)
    find_package(Qt5 COMPONENTS REQUIRED Core Gui OpenGL Widgets)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)
else()
    add_definitions(-DWEB_DEPLOYMENT)
    set(QT_LIBRARIES Qt5::Core)
endif()

# Improvement Cmake 3.x
# If we have compiler requirements for this library, list them
#target_compile_features(lib
#    PUBLIC cxx_auto_type
#    PRIVATE cxx_variadic_templates)

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
if (WebDeployment)
    add_subdirectory(cafWebInterface)
    add_subdirectory(cafTests/cafTestWebApplication)    
else()
    add_subdirectory(cafUserInterface)
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


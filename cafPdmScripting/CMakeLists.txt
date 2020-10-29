cmake_minimum_required (VERSION 3.12)

project (cafPdmScripting)

option(CAF_CVF_SCRIPTING "Enable CVF-data support in Scripting" OFF)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set( PROJECT_FILES
     cafPdmCodeGenerator.h
     cafPdmAbstractFieldScriptingCapability.h
     cafPdmAbstractFieldScriptingCapability.cpp
     cafFieldScriptingCapability.h
     cafFieldScriptingCapability.cpp
     cafObjectMethod.h
     cafObjectMethod.cpp
     cafObjectScriptingCapabilityRegister.h
     cafObjectScriptingCapabilityRegister.cpp
     cafPdmPythonGenerator.h
     cafPdmPythonGenerator.cpp
     cafPdmScriptIOMessages.h
     cafPdmScriptIOMessages.cpp
     cafObjectScriptingCapability.h
     cafObjectScriptingCapability.cpp
     cafPdmScriptResponse.h
     cafPdmScriptResponse.cpp
     cafPdmMarkdownGenerator.h
     cafPdmMarkdownGenerator.cpp
     cafPdmMarkdownBuilder.h
     cafPdmMarkdownBuilder.cpp
)

set (CAF_LIBS cafProjectDataModel)

if (CAF_CVF_SCRIPTING)
    list(APPEND PROJECT_FILES
     cafFieldScriptingCapabilityCvfColor3.h
     cafFieldScriptingCapabilityCvfColor3.cpp
     cafFieldScriptingCapabilityCvfVec3d.h
     cafFieldScriptingCapabilityCvfVec3d.cpp
    )
    list(APPEND CAF_LIBS cafPdmCvf)
endif()

add_library( ${PROJECT_NAME}
    ${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)




target_link_libraries ( ${PROJECT_NAME}
    ${CAF_LIBS}
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


source_group("" FILES ${PROJECT_FILES})

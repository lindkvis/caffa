cmake_minimum_required (VERSION 3.12)

project (cafPdmScriptCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set( PROJECT_FILES
     cafPdmCodeGenerator.h
     cafAbstractFieldScriptingCapability.h
     cafAbstractFieldScriptingCapability.cpp
     cafFieldScriptingCapability.h
     cafFieldScriptingCapability.cpp
     cafObjectMethod.h
     cafObjectMethod.cpp
     cafObjectScriptingCapabilityRegister.h
     cafObjectScriptingCapabilityRegister.cpp
     cafPdmScriptIOMessages.h
     cafPdmScriptIOMessages.cpp
     cafObjectScriptingCapability.h
     cafObjectScriptingCapability.cpp
     cafPdmScriptResponse.h
     cafPdmScriptResponse.cpp
)


if (CAFFA_BUILD_SHARED)
    message(STATUS "Building ${PROJECT_NAME} shared")
    add_library( ${PROJECT_NAME} SHARED
        ${PROJECT_FILES}
    )
else()
    add_library( ${PROJECT_NAME} STATIC
        ${PROJECT_FILES}
    )
endif()


set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    PRIVATE
    cafPdmCore
    cafPdmIoCore
    cafPdmUiCore
    cafPdmObject
    PRIVATE
    nlohmann_json::nlohmann_json
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


source_group("" FILES ${PROJECT_FILES})

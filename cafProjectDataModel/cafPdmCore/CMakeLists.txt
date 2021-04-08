cmake_minimum_required (VERSION 3.12)

project (cafPdmCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set (PUBLIC_HEADERS 
    cafAssert.h
    cafAppEnum.h
    cafColor.h
    cafFactory.h
    cafPdmBase.h
    cafChildArrayField.h
    cafChildArrayField.inl
    cafChildField.h
    cafChildField.inl
    cafDataValueField.h
    cafDataFieldAccessor.h
    cafException.h
    cafField.h
    cafFieldCapability.h
    cafFieldHandle.h
    cafLogger.h
    cafObjectCapability.h
    cafFieldReorderCapability.h
    cafObjectHandle.h
    cafPdmPointer.h
    cafFieldProxyAccessor.h
    cafPtrArrayField.h
    cafPtrArrayField.inl
    cafPtrArrayFieldHandle.h
    cafPtrField.h
    cafPtrField.inl
    cafPdmReferenceHelper.h
    cafFieldUiCapabilityInterface.h
    cafValueField.h
    cafValueFieldSpecializations.h

    cafSignal.h
    cafStringTools.h
    cafVariant.h

    cafTristate.h
    cafAsyncObjectDeleter.h
    cafAsyncObjectDeleter.inl
	cafAsyncWorkerManager.h
)

set( PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafColor.cpp
    cafChildArrayFieldHandle.cpp
    cafFieldHandle.cpp
    cafFieldReorderCapability.cpp
    cafLogger.cpp
    cafObjectHandle.cpp
    cafPdmPointer.cpp
    cafPdmReferenceHelper.cpp
    cafSignal.cpp
    cafStringTools.cpp
    cafVariant.cpp
    cafTristate.cpp
	cafAsyncWorkerManager.cpp
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

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

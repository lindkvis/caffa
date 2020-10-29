cmake_minimum_required (VERSION 3.12)

project (cafPdmCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

find_package(Qt5 COMPONENTS REQUIRED Core)
set(QT_LIBRARIES Qt5::Core)

set (PUBLIC_HEADERS 
    cafAssert.h
    cafAppEnum.h
    cafClassTypeName.h
    cafPdmBase.h
    cafPdmChildArrayField.h
    cafPdmChildArrayField.inl
    cafPdmChildField.h
    cafPdmChildField.inl
    cafPdmDataValueField.h
    cafField.h
    cafFieldCapability.h
    cafFieldHandle.h
    cafObjectCapability.h
    cafFieldReorderCapability.h
    cafObjectHandle.h
    cafPdmPointer.h
    cafPdmProxyValueField.h
    cafPdmPtrArrayField.h
    cafPdmPtrArrayField.inl
    cafPdmPtrArrayFieldHandle.h
    cafPdmPtrField.h
    cafPdmPtrField.inl
    cafPdmReferenceHelper.h
    cafFieldUiCapabilityInterface.h
    cafPdmValueField.h
    cafPdmValueFieldSpecializations.h

    cafSignal.h

    cafTristate.h
    cafFilePath.h
    cafAsyncObjectDeleter.h
    cafAsyncObjectDeleter.inl
	cafAsyncWorkerManager.h
)

set( PROJECT_FILES
    ${PUBLIC_HEADERS}
    cafPdmChildArrayFieldHandle.cpp
    cafFieldHandle.cpp
    cafFieldReorderCapability.cpp
    cafObjectHandle.cpp
    cafPdmPointer.cpp
    cafPdmReferenceHelper.cpp
    cafSignal.cpp
    cafTristate.cpp
    cafFilePath.cpp
	cafAsyncWorkerManager.cpp
)

add_library( ${PROJECT_NAME}
    ${PROJECT_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

source_group("" FILES ${PROJECT_FILES})

cmake_minimum_required (VERSION 3.12)

project (cafPdmIoCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core Xml)
set(QT_LIBRARIES Qt5::Core Qt5::Xml)

include_directories (
	..
)

set( PUBLIC_HEADERS
	cafInternalPdmFieldIoHelper.h
	cafInternalPdmFilePathStreamOperators.h
	cafInternalPdmStreamOperators.h
	cafPdmFieldIoCapabilitySpecializations.h
	cafPdmFieldIoCapabilitySpecializations.inl
	cafInternalPdmXmlFieldReaderWriter.h

	cafPdmFieldIoCapability.h
	cafPdmObjectXmlCapability.h
	cafPdmObjectIoCapability.h
	cafPdmObjectHandleIoMacros.h

	cafPdmDefaultObjectFactory.h
	cafPdmObjectFactory.h
	
	cafPdmSettings.h

	cafPdmXmlStringValidation.h

)

set( PROJECT_FILES
	${PUBLIC_HEADERS}
	cafInternalPdmFieldIoHelper.cpp
	cafInternalPdmFilePathStreamOperators.cpp
	cafInternalPdmStreamOperators.cpp
	cafInternalPdmXmlFieldReaderWriter.cpp

	cafPdmFieldIoCapability.cpp
	
	cafPdmObjectIoCapability.cpp
	cafPdmObjectXmlCapability.cpp
	cafPdmDefaultObjectFactory.cpp
	cafPdmSettings.cpp
)


add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
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

source_group("" FILES ${PROJECT_FILES})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
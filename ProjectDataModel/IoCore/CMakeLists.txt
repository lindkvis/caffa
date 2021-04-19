cmake_minimum_required (VERSION 3.12)

project (caffaIoCore)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# IO Libraries
include_directories (
	..
)

set( PUBLIC_HEADERS
	cafInternalStreamOperators.h
	cafFieldIoCapabilitySpecializations.h
	cafFieldIoCapabilitySpecializations.inl
	cafInternalIoFieldReaderWriter.h

	cafFieldIoCapability.h
	cafObjectJsonCapability.h
	cafObjectIoCapability.h
	cafObjectHandleIoMacros.h

	cafDefaultObjectFactory.h
	cafObjectFactory.h	
)

set( PROJECT_FILES
	${PUBLIC_HEADERS}
	cafInternalIoFieldReaderWriter.cpp

	cafFieldIoCapability.cpp
	
	cafObjectIoCapability.cpp
	cafObjectJsonCapability.cpp
	cafDefaultObjectFactory.cpp
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

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_link_libraries ( ${PROJECT_NAME}
    PUBLIC
      caffaCore
    PRIVATE
      nlohmann_json::nlohmann_json
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

cmake_minimum_required(VERSION 3.12)

project(caffaUserInterface_UnitTests)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_FILES cafUserInterface_UnitTests.cpp cafUiTreeViewModelTest.cpp cafUiTreeSelectionQModelTest.cpp)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

source_group("" FILES ${PROJECT_FILES})

set(EXTERNAL_LINK_LIBRARIES)

if(UNIX)
  list(APPEND LIST EXTERNAL_LINK_LIBRARIES z)
endif()

target_link_libraries(
  ${PROJECT_NAME}
  ${THREAD_LIBRARY}
  ${QT_LIBRARIES}
  ${EXTERNAL_LINK_LIBRARIES}
  caffaUiCore
  caffaUserInterface
  gtest)

# Copy Qt Dlls
foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach(qtlib)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

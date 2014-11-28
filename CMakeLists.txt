project(torrent-file-editor)
cmake_minimum_required(VERSION 2.8.11)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

option(QT5_BUILD OFF "Force Qt5 build")

if(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5LinguistTools REQUIRED)

  include_directories(${Qt5Core_INCLUDE_DIRS})
  include_directories(${Qt5Gui_INCLUDE_DIRS})
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  add_definitions(${Qt5Core_DEFINITIONS})
  add_definitions(${Qt5Gui_DEFINITIONS})
  add_definitions(${Qt5Widgets_DEFINITIONS})
  add_definitions(-DHAVE_QT5)
else()
  find_package(Qt4 REQUIRED)
  find_package(QJSON REQUIRED)
  include(${QT_USE_FILE})
endif()

configure_file(translations.qrc.in translations.qrc COPYONLY)

set(TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/translations/torrentfileeditor_en.ts
  ${CMAKE_SOURCE_DIR}/translations/torrentfileeditor_ru.ts)

set(HEADERS
  ${CMAKE_SOURCE_DIR}/mainwindow.h
  ${CMAKE_SOURCE_DIR}/bencode.h
  ${CMAKE_SOURCE_DIR}/datewidget.h
  ${CMAKE_SOURCE_DIR}/lineeditwidget.h
  ${CMAKE_SOURCE_DIR}/urledit.h
  ${CMAKE_SOURCE_DIR}/folderedit.h
  ${CMAKE_SOURCE_DIR}/aboutdlg.h)

set(FORMS 
  ${CMAKE_SOURCE_DIR}/mainwindow.ui
  ${CMAKE_SOURCE_DIR}/aboutdlg.ui)

set(SOURCES
  ${CMAKE_SOURCE_DIR}/main.cpp
  ${CMAKE_SOURCE_DIR}/mainwindow.cpp
  ${CMAKE_SOURCE_DIR}/bencode.cpp
  ${CMAKE_SOURCE_DIR}/datewidget.cpp
  ${CMAKE_SOURCE_DIR}/lineeditwidget.cpp
  ${CMAKE_SOURCE_DIR}/urledit.cpp
  ${CMAKE_SOURCE_DIR}/folderedit.cpp
  ${CMAKE_SOURCE_DIR}/aboutdlg.cpp)

set(RESOURCES
  ${CMAKE_BINARY_DIR}/translations.qrc
  resources.qrc)

if(QT5_BUILD)
  qt5_add_translation(QM ${TRANSLATIONS})
  qt5_add_resources(QRC_SOURCES ${RESOURCES})
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
  qt5_wrap_ui(UI_SOURCES ${FORMS})
else()
  QT4_ADD_TRANSLATION(QM ${TRANSLATIONS})
  qt4_add_resources(QRC_SOURCES ${RESOURCES})
  qt4_wrap_cpp(MOC_SOURCES ${HEADERS})
  qt4_wrap_ui(UI_SOURCES ${FORMS})
endif()

if(WIN32)
  add_definitions(-DQT_NODLL)
  add_definitions(-DQJSON_STATIC)

  set(START_STATIC -Wl,-Bstatic)
  set(END_STATIC z jpeg png tiff stdc++ -Wl,-Bdynamic  ws2_32 winmm imm32)
  set(QJSON_LIBRARIES qjson.a)

  # resource compilation for MinGW
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/app.o
                     COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/app.rc
                             -o ${CMAKE_CURRENT_BINARY_DIR}/app.o)
  set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/app.o)
endif()

add_executable(${PROJECT_NAME} WIN32 ${QM} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${UI_SOURCES})

if(QT5_BUILD)
  target_link_libraries(${PROJECT_NAME} ${START_STATIC} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES}  ${END_STATIC})
else()
  target_link_libraries(${PROJECT_NAME} ${START_STATIC} ${QJSON_LIBRARIES} ${START_STATIC} ${QT_LIBRARIES} ${END_STATIC})
endif()

add_subdirectory(translations)

if(UNIX AND NOT APPLE)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
  install(FILES torrent-file-editor.desktop DESTINATION share/applications)
  install(FILES icons/app_16.png DESTINATION share/icons/hicolor/16x16/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_32.png DESTINATION share/icons/hicolor/32x32/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_48.png DESTINATION share/icons/hicolor/48x48/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_256.png DESTINATION share/icons/hicolor/256x256/apps RENAME torrent-file-editor.png)
endif()

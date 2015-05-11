project(torrent-file-editor)
cmake_minimum_required(VERSION 2.8.11)

set(APP_VERSION "0.1.0")
set(BUNDLE_DIR_BASE_NAME "Torrent File Editor")

set(MACOSX_BUNDLE_INFO_STRING "Torrent File Editor ${APP_VERSION}. This program is licensed under the GNU GPL.")
set(MACOSX_BUNDLE_ICON_FILE "application.icns")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.torrent-file-editor")
set(MACOSX_BUNDLE_LONG_VERSION_STRING ${APP_VERSION})
set(MACOSX_BUNDLE_BUNDLE_NAME "Torrent File Editor")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION})
set(MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION})

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# Enable C++11
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

if(APPLE)
  # Force Qt5 build on Mac OS X. Qt4 is not supported
  set(QT5_BUILD ON)
else()
  option(QT5_BUILD OFF "Force Qt5 build")
endif()

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
  find_package(QJSON 0.8.0 REQUIRED)
  include(${QT_USE_FILE})
endif()

configure_file(translations.qrc.in translations.qrc COPYONLY)

set(TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/translations/torrentfileeditor_en.ts
  ${CMAKE_SOURCE_DIR}/translations/torrentfileeditor_ru.ts)

set(HEADERS
  ${CMAKE_SOURCE_DIR}/application.h
  ${CMAKE_SOURCE_DIR}/mainwindow.h
  ${CMAKE_SOURCE_DIR}/bencode.h
  ${CMAKE_SOURCE_DIR}/datewidget.h
  ${CMAKE_SOURCE_DIR}/lineeditwidget.h
  ${CMAKE_SOURCE_DIR}/urledit.h
  ${CMAKE_SOURCE_DIR}/folderedit.h
  ${CMAKE_SOURCE_DIR}/aboutdlg.h
  ${CMAKE_SOURCE_DIR}/bencodemodel.h
  ${CMAKE_SOURCE_DIR}/bencodedelegate.h
  ${CMAKE_SOURCE_DIR}/abstracttreemodel.h
  ${CMAKE_SOURCE_DIR}/abstracttreeitem.h
  ${CMAKE_SOURCE_DIR}/proxystyle.h)

set(FORMS
  ${CMAKE_SOURCE_DIR}/mainwindow.ui
  ${CMAKE_SOURCE_DIR}/aboutdlg.ui)

set(SOURCES
  ${CMAKE_SOURCE_DIR}/application.cpp
  ${CMAKE_SOURCE_DIR}/main.cpp
  ${CMAKE_SOURCE_DIR}/mainwindow.cpp
  ${CMAKE_SOURCE_DIR}/bencode.cpp
  ${CMAKE_SOURCE_DIR}/datewidget.cpp
  ${CMAKE_SOURCE_DIR}/lineeditwidget.cpp
  ${CMAKE_SOURCE_DIR}/urledit.cpp
  ${CMAKE_SOURCE_DIR}/folderedit.cpp
  ${CMAKE_SOURCE_DIR}/aboutdlg.cpp
  ${CMAKE_SOURCE_DIR}/bencodemodel.cpp
  ${CMAKE_SOURCE_DIR}/bencodedelegate.cpp
  ${CMAKE_SOURCE_DIR}/abstracttreemodel.cpp
  ${CMAKE_SOURCE_DIR}/abstracttreeitem.cpp
  ${CMAKE_SOURCE_DIR}/proxystyle.cpp)

if(ENABLE_MODEL_TEST)
  list(APPEND SOURCES
    ${CMAKE_SOURCE_DIR}/modeltest/dynamictreemodel.cpp
    ${CMAKE_SOURCE_DIR}/modeltest/tst_modeltest.cpp
    ${CMAKE_SOURCE_DIR}/modeltest/modeltest.cpp)
endif()

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

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${QM} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${UI_SOURCES})

if(QT5_BUILD)
  target_link_libraries(${PROJECT_NAME} ${START_STATIC} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES}  ${END_STATIC})
else()
  target_link_libraries(${PROJECT_NAME} ${START_STATIC} ${QJSON_LIBRARIES} ${START_STATIC} ${QT_LIBRARIES} ${END_STATIC})
endif()

add_subdirectory(translations)

if(UNIX AND NOT APPLE)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
  install(FILES torrent-file-editor.desktop DESTINATION share/applications)
  install(FILES torrent-file-editor.appdata.xml DESTINATION share/appdata)
  install(FILES icons/app_16.png DESTINATION share/icons/hicolor/16x16/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_32.png DESTINATION share/icons/hicolor/32x32/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_48.png DESTINATION share/icons/hicolor/48x48/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME torrent-file-editor.png)
  install(FILES icons/app_256.png DESTINATION share/icons/hicolor/256x256/apps RENAME torrent-file-editor.png)
endif()

if(APPLE)
  install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION . COMPONENT Runtime)
  set_target_properties(torrent-file-editor PROPERTIES
    OUTPUT_NAME "${BUNDLE_DIR_BASE_NAME}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")

  # Install app icon
  install(FILES icons/application.icns DESTINATION "${BUNDLE_DIR_BASE_NAME}.app/Contents/Resources")

  # Install needed Qt plugins by copying each pluging separately
  find_package(Qt5PrintSupport REQUIRED)
  set(PLATFORM_PLUGINS Qt5::QCocoaIntegrationPlugin)
  set(IMAGE_PLUGINS Qt5::QICNSPlugin)
  set(PRINTSUPPORT_PLUGINS Qt5::QCocoaPrinterSupportPlugin)
  set(PLUGINS_DEST_PATH "${BUNDLE_DIR_BASE_NAME}.app/Contents/Plugins")

  foreach(plugin ${PLATFORM_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    install(FILES "${_loc}" DESTINATION ${PLUGINS_DEST_PATH}/platforms/ COMPONENT Runtime)
  endforeach()

  foreach(plugin ${IMAGE_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    install(FILES "${_loc}" DESTINATION ${PLUGINS_DEST_PATH}/imageformats/ COMPONENT Runtime)
  endforeach()

  foreach(plugin ${PRINTSUPPORT_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    install(FILES "${_loc}" DESTINATION ${PLUGINS_DEST_PATH}/printsupport/ COMPONENT Runtime)
  endforeach()

  # Now the work of copying dependencies into the bundle/package
  # The quotes are escaped and variables to use at install time have their $ escaped
  # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
  # Note that the image plugins depend on QtSvg and QtXml, and it got those copied
  # over.
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${BUNDLE_DIR_BASE_NAME}.app")
  install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${PLUGINS_DEST_PATH}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIRS}\")
    " COMPONENT Runtime)

  # install a qt.conf file
  # this inserts some cmake code into the install script to write the file
  INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${BUNDLE_DIR_BASE_NAME}.app/Contents/Resources/qt.conf\" \"\")
    " COMPONENT Runtime)

  # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
  # where CPackConfig.cmake is created by including CPack
  # And then there's ways to customize this as well
  set(CPACK_PACKAGE_VERSION ${APP_VERSION})
  set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION})
  set(CPACK_BINARY_DRAGNDROP ON)
  include(CPack)

  add_custom_target(dmg COMMAND cpack -G DragNDrop CPackConfig.cmake DEPENDS ${PROJECT_NAME})
endif()

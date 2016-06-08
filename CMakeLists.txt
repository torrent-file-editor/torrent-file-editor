project(torrent-file-editor)

if(APPLE)
  # CPACK_DMG_DS_STORE_SETUP_SCRIPT was introduced only in CMake 3.5
  cmake_minimum_required(VERSION 3.5)
else()
  cmake_minimum_required(VERSION 2.8.11)
endif()

if(POLICY CMP0046)
  cmake_policy(SET CMP0046 OLD)
endif()

set(APP_VERSION "0.2.2")
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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(APPLE)
  # Force Qt5 build on Mac OS X. Qt4 is not supported
  set(QT5_BUILD ON)
else()
  option(QT5_BUILD "Force Qt5 build" OFF)
endif()

if(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5LinguistTools REQUIRED)

  add_definitions(-DHAVE_QT5)

  macro(QT4_ADD_TRANSLATION)
    qt5_add_translation(${ARGN})
  endmacro()

  macro(qt4_add_resources)
    qt5_add_resources(${ARGN})
  endmacro()

  macro(qt4_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
  endmacro()

  macro(qt4_wrap_ui)
    qt5_wrap_ui(${ARGN})
  endmacro()
else()
  find_package(Qt4 REQUIRED)
  find_package(QJSON 0.8.0 REQUIRED)
  include(${QT_USE_FILE})
endif()

macro(qt_add_translation ARG1 ARG2)
  if(QT5_BUILD)
    qt5_add_translation(${ARG1} ${ARG2})
  else()
    qt4_add_translation(${ARG1} ${ARG2})
  endif()
endmacro()

configure_file(translations.qrc.in translations.qrc COPYONLY)

set(LANGS ar cs de en ko ru)

foreach(l ${LANGS})
  list(APPEND TRANSLATIONS ${CMAKE_SOURCE_DIR}/translations/torrentfileeditor_${l}.ts)
endforeach()

set(HEADERS
  ${CMAKE_SOURCE_DIR}/application.h
  ${CMAKE_SOURCE_DIR}/mainwindow.h
  ${CMAKE_SOURCE_DIR}/datewidget.h
  ${CMAKE_SOURCE_DIR}/lineeditwidget.h
  ${CMAKE_SOURCE_DIR}/urledit.h
  ${CMAKE_SOURCE_DIR}/folderedit.h
  ${CMAKE_SOURCE_DIR}/aboutdlg.h
  ${CMAKE_SOURCE_DIR}/bencodemodel.h
  ${CMAKE_SOURCE_DIR}/bencodedelegate.h
  ${CMAKE_SOURCE_DIR}/abstracttreemodel.h
)

set(PLAIN_HEADERS
  ${CMAKE_SOURCE_DIR}/bencode.h
  ${CMAKE_SOURCE_DIR}/abstracttreeitem.h
  ${CMAKE_SOURCE_DIR}/proxystyle.h
)

set(FORMS
  ${CMAKE_SOURCE_DIR}/mainwindow.ui
  ${CMAKE_SOURCE_DIR}/aboutdlg.ui
)

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
  ${CMAKE_SOURCE_DIR}/proxystyle.cpp
)

if(APPLE)
  list(APPEND SOURCES
    ${CMAKE_SOURCE_DIR}/cocoainitializer.mm
    ${CMAKE_SOURCE_DIR}/sparkleautoupdater.mm
  )

  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})  # to find FindSparkle.cmake file
  find_package(Sparkle REQUIRED)
  include_directories(${SPARKLE_INCLUDE_DIR})
endif()

set(RESOURCES
  ${CMAKE_BINARY_DIR}/translations.qrc
  resources.qrc
)

QT4_ADD_TRANSLATION(QM ${TRANSLATIONS})
qt4_add_resources(QRC_SOURCES ${RESOURCES})
qt4_wrap_cpp(MOC_SOURCES ${HEADERS})
qt4_wrap_ui(UI_SOURCES ${FORMS})

if(WIN32)
  add_definitions(-DQT_NODLL)
  add_definitions(-DQJSON_STATIC)

  set(START_STATIC -Wl,-Bstatic)
  set(END_STATIC z jpeg png tiff stdc++ -lwinpthread -Wl,-Bdynamic  ws2_32 winmm imm32)
  set(QJSON_LIBRARIES qjson.a)

  # resource compilation for MinGW
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/app.o
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/app.rc -o ${CMAKE_CURRENT_BINARY_DIR}/app.o
  )
  set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/app.o)
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${QM} ${HEADERS} ${PLAIN_HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${UI_SOURCES})

if(QT5_BUILD)
  target_link_libraries(${PROJECT_NAME} ${START_STATIC} Qt5::Core Qt5::Gui Qt5::Widgets ${END_STATIC})
else()
  target_link_libraries(${PROJECT_NAME} ${START_STATIC} ${QJSON_LIBRARIES} ${START_STATIC} ${QT_LIBRARIES} ${END_STATIC})
endif()

if(APPLE)
  target_link_libraries(${PROJECT_NAME} ${SPARKLE_LIBRARY} "-framework AppKit")
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
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in"
  )

  # Install app icon
  install(FILES icons/application.icns DESTINATION "${BUNDLE_DIR_BASE_NAME}.app/Contents/Resources" COMPONENT Runtime)

  # Install needed Qt plugins by copying each pluging separately
  find_package(Qt5PrintSupport REQUIRED)
  set(PLATFORM_PLUGINS Qt5::QCocoaIntegrationPlugin)
  set(IMAGE_PLUGINS Qt5::QICNSPlugin)
  set(PRINTSUPPORT_PLUGINS Qt5::QCocoaPrinterSupportPlugin)
  set(PLUGINS_DEST_PATH "${BUNDLE_DIR_BASE_NAME}.app/Contents/PlugIns")

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

  # Install public key for Sparkle Updater
  install(FILES "dsa_pub.pem" DESTINATION "${BUNDLE_DIR_BASE_NAME}.app/Contents/Resources" COMPONENT Runtime)

  # Install qt.conf file
  # Qt can't find plugins in standard path on case-sensetive file system
  # Write correct case-sensetive folder name in qt.conf file
  install(FILES "qt.conf" DESTINATION "${BUNDLE_DIR_BASE_NAME}.app/Contents/Resources" COMPONENT Runtime)

  # Now the work of copying dependencies into the bundle/package
  # The quotes are escaped and variables to use at install time have their $ escaped
  # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
  # Note that the image plugins depend on QtSvg and QtXml, and it got those copied
  # over.
  get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
  while(NOT ${QT_LIBRARY_DIR} MATCHES "lib$")
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} DIRECTORY)
  endwhile()
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${BUNDLE_DIR_BASE_NAME}.app")
  install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${PLUGINS_DEST_PATH}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\"
    )
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    file(GLOB BINARIES
      \"${APPS}/Contents/Frameworks/*.framework\"
      \"${APPS}/Contents/Frameworks/Sparkle.framework/Resources/Autoupdate.app\"
      \"${APPS}/Contents/MacOS/*.dylib\"
      \"${APPS}/Contents/PlugIns/*.dylib\"
      \"${APPS}/Contents/PlugIns/*/*.dylib\"
    )
    execute_process(COMMAND codesign --deep -f -s \"Ivan Romanov\" \${BINARIES} \"${APPS}\" WORKING_DIRECTORY \"${APPS}\")
    " COMPONENT Runtime
  )

  # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
  # where CPackConfig.cmake is created by including CPack
  # And then there's ways to customize this as well
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_PACKAGE_VERSION ${APP_VERSION})
  set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION})
  set(CPACK_DMG_VOLUME_NAME ${BUNDLE_DIR_BASE_NAME})
  set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_SOURCE_DIR}/CMakeDMGSetup.scpt")
  set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/DS_Background.png")
  include(CPack)

  add_custom_target("dmg"
    COMMAND ${CMAKE_COMMAND} -DAPP_NAME=${PROJECT_NAME} -DAPP_VERSION=${APP_VERSION} -P "${CMAKE_CURRENT_SOURCE_DIR}/Dmg.cmake"
    DEPENDS "${PROJECT_NAME}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
endif()

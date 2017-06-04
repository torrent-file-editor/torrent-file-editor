project("Torrent File Editor")
set(EXE_NAME "torrent-file-editor")

if(APPLE)
  # CPACK_DMG_DS_STORE_SETUP_SCRIPT was introduced only in CMake 3.5
  cmake_minimum_required(VERSION 3.5)
else()
  cmake_minimum_required(VERSION 2.8.11)
endif()

if(POLICY CMP0046)
  cmake_policy(SET CMP0046 OLD)
endif()

# Fallback version. Will be used when compiling out of git repository.
set(APP_VERSION "0.3.5")

set(MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT_NAME}")
set(MACOSX_BUNDLE_INFO_STRING "Torrent File Editor \${APP_VERSION}. This program is licensed under the GNU GPL.")
set(MACOSX_BUNDLE_ICON_FILE "application.icns")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.torrent-file-editor")
set(MACOSX_BUNDLE_BUNDLE_NAME "Torrent File Editor")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "\${APP_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "\${APP_VERSION}")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# Workaround: On Fedora will be executed cmake not mingw32-cmake for building Windows version
add_custom_target(update_version ALL
  COMMAND ${CMAKE_COMMAND} -DWIN32=${WIN32} -DAPP_VERSION="v${APP_VERSION}" -P "${CMAKE_SOURCE_DIR}/Version.cmake"
)

# Enable C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-DQT_NO_CAST_FROM_ASCII)

if(APPLE)
  # Force Qt5 build on Mac OS X. Qt4 is not supported
  set(QT5_BUILD ON)
else()
  option(QT5_BUILD "Force Qt5 build" OFF)
endif()

option(DISABLE_DONATION "Do not show donation text in About dialog" OFF)
if(DISABLE_DONATION)
  add_definitions(-DNO_DONATION)
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
if(WIN32)
  configure_file(qt_translations.qrc.in qt_translations.qrc COPYONLY)
endif()

set(LANGS ar cs de en es ko ru hu it nl tr zh_CN)

foreach(l ${LANGS})
  list(APPEND TRANSLATIONS ${CMAKE_SOURCE_DIR}/translations/torrentfileeditor_${l}.ts)
endforeach()

if(WIN32 AND NOT QT5_BUILD)
  foreach(l ${LANGS})
    if(EXISTS "/usr/i686-w64-mingw32/sys-root/mingw/share/qt4/translations/qt_${l}.qm")
      configure_file("/usr/i686-w64-mingw32/sys-root/mingw/share/qt4/translations/qt_${l}.qm" "qt_${l}.qm" COPYONLY)
    endif()
  endforeach()
endif()

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
  ${CMAKE_SOURCE_DIR}/tableview.h
  ${CMAKE_SOURCE_DIR}/treeview.h
  ${CMAKE_SOURCE_DIR}/combobox.h
  ${CMAKE_SOURCE_DIR}/searchdlg.h
)

if(WIN32)
  list(APPEND HEADERS
    ${CMAKE_SOURCE_DIR}/checkupdate.h
  )
endif()

set(PLAIN_HEADERS
  ${CMAKE_SOURCE_DIR}/bencode.h
  ${CMAKE_SOURCE_DIR}/abstracttreeitem.h
  ${CMAKE_SOURCE_DIR}/proxystyle.h
  ${CMAKE_BINARY_DIR}/config.h
)

# config.h is a generated file
set_source_files_properties(${CMAKE_BINARY_DIR}/config.h
  PROPERTIES GENERATED TRUE
  HEADER_FILE_ONLY TRUE
)

set(FORMS
  ${CMAKE_SOURCE_DIR}/mainwindow.ui
  ${CMAKE_SOURCE_DIR}/aboutdlg.ui
  ${CMAKE_SOURCE_DIR}/searchdlg.ui
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
  ${CMAKE_SOURCE_DIR}/tableview.cpp
  ${CMAKE_SOURCE_DIR}/treeview.cpp
  ${CMAKE_SOURCE_DIR}/combobox.cpp
  ${CMAKE_SOURCE_DIR}/searchdlg.cpp
)

if(WIN32)
  list(APPEND SOURCES
    ${CMAKE_SOURCE_DIR}/checkupdate.cpp
  )
endif()

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

if(WIN32)
  list(APPEND RESOURCES "${CMAKE_BINARY_DIR}/qt_translations.qrc")
endif()

QT4_ADD_TRANSLATION(QM ${TRANSLATIONS})
qt4_add_resources(QRC_SOURCES ${RESOURCES})
qt4_wrap_cpp(MOC_SOURCES ${HEADERS})
qt4_wrap_ui(UI_SOURCES ${FORMS})

if(WIN32)
  add_definitions(-DQT_NODLL)
  add_definitions(-DQJSON_STATIC)

  set(START_STATIC -Wl,-Bstatic)
  set(END_STATIC z jpeg png tiff stdc++ -lwinpthread -Wl,-Bdynamic  ws2_32 winmm imm32 wininet)
  set(QJSON_LIBRARIES qjson.a)

  # resource compilation for MinGW
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/app.o
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/app.rc
    COMMENT "Building RC object app.o"
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_BINARY_DIR}/app.rc -o ${CMAKE_CURRENT_BINARY_DIR}/app.o
  )
  set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/app.o)
endif()

add_executable(${EXE_NAME} WIN32 MACOSX_BUNDLE ${QM} ${HEADERS} ${PLAIN_HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${UI_SOURCES})

add_dependencies(${EXE_NAME} update_version)

if(QT5_BUILD)
  target_link_libraries(${EXE_NAME} ${START_STATIC} Qt5::Core Qt5::Gui Qt5::Widgets ${END_STATIC})
else()
  target_link_libraries(${EXE_NAME} ${START_STATIC} ${QJSON_LIBRARIES} ${START_STATIC} ${QT_LIBRARIES} ${END_STATIC})
endif()

if(APPLE)
  target_link_libraries(${EXE_NAME} ${SPARKLE_LIBRARY} "-framework AppKit")
endif()

add_subdirectory(translations)

if(UNIX AND NOT APPLE)
  install(TARGETS ${EXE_NAME} DESTINATION bin)
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
  install(TARGETS ${EXE_NAME} BUNDLE DESTINATION . COMPONENT Runtime)
  set_target_properties(torrent-file-editor PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
  )
  configure_file(MacOSXBundleInfo.plist.in "${CMAKE_BINARY_DIR}/MacOSXBundleInfo.plist.in")

  # Install app icon
  install(FILES icons/application.icns DESTINATION "${PROJECT_NAME}.app/Contents/Resources" COMPONENT Runtime)

  # Install needed Qt plugins by copying each pluging separately
  find_package(Qt5PrintSupport REQUIRED)
  set(PLATFORM_PLUGINS Qt5::QCocoaIntegrationPlugin)
  set(IMAGE_PLUGINS Qt5::QICNSPlugin)
  set(PRINTSUPPORT_PLUGINS Qt5::QCocoaPrinterSupportPlugin)
  set(PLUGINS_DEST_PATH "${PROJECT_NAME}.app/Contents/PlugIns")
  set(TRANSLATIONS_DEST_PATH "${PROJECT_NAME}.app/Contents/Resources/translations")

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
  install(FILES "dsa_pub.pem" DESTINATION "${PROJECT_NAME}.app/Contents/Resources" COMPONENT Runtime)

  # Install qt.conf file
  # Qt can't find plugins in standard path on case-sensetive file system
  # Write correct case-sensetive folder name in qt.conf file
  install(FILES "qt.conf" DESTINATION "${PROJECT_NAME}.app/Contents/Resources" COMPONENT Runtime)

  # Now the work of copying dependencies into the bundle/package
  # The quotes are escaped and variables to use at install time have their $ escaped
  # An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
  # Note that the image plugins depend on QtSvg and QtXml, and it got those copied
  # over.
  get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
  while(NOT ${QT_LIBRARY_DIR} MATCHES "lib$")
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} DIRECTORY)
  endwhile()

  foreach(l ${LANGS})
    # In Qt lacks some languages
    if(EXISTS "${QT_LIBRARY_DIR}/../translations/qtbase_${l}.qm")
      install(FILES "${QT_LIBRARY_DIR}/../translations/qtbase_${l}.qm" DESTINATION ${TRANSLATIONS_DEST_PATH} COMPONENT Runtime)
    endif()
  endforeach()

  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
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
  set(CPACK_DMG_VOLUME_NAME ${PROJECT_NAME})
  set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_SOURCE_DIR}/CMakeDMGSetup.scpt")
  set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/DS_Background.png")
  include(CPack)

  add_custom_target("dmg"
    COMMAND ${CMAKE_COMMAND} -DPROJECT_NAME=${PROJECT_NAME} -DAPP_NAME=${EXE_NAME} -P "${CMAKE_CURRENT_SOURCE_DIR}/Dmg.cmake"
    DEPENDS "${EXE_NAME}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
endif()

# collect_info_files function
# Takes collect_info_files - list of non-source files to look for
# Returns INFO_FILES with all files found from the input list.
# Based on macro posted here
# http://lists.qt-project.org/pipermail/qt-creator/2012-August/001191.html [^]
function(collect_info_files)
	list(APPEND _all_found)
	foreach(_it ${ARGN})
		if(NOT IS_DIRECTORY ${_it})
			get_filename_component(_path ${_it} ABSOLUTE)
			if(EXISTS ${_path})
				list(APPEND _all_found ${_it})
				if(NOT ${_it} MATCHES "^/\\\\..*$;~$")
					set_source_files_properties(${_it} PROPERTIES HEADER_FILE_ONLY TRUE)
				endif()
			endif()
		endif()
	endforeach()
	set(INFO_FILES ${_all_found} PARENT_SCOPE)
endfunction()

# List all non-source code files (documents, configuration files, etc.)
set(OTHER_FILES
  .gitignore
  .travis.yml
  README.md
  Dmg.cmake
  appcast.xml.in
  CMakeDMGSetup.scpt
  Version.cmake
)

# Collect all existing files and configure as non-sources.
collect_info_files(${OTHER_FILES})

# Create dummy target/project for an IDE with documents attached
add_custom_target(docs SOURCES ${INFO_FILES})

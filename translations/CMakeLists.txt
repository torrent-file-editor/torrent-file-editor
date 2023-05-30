# hack to update translations files only on 'make lupdate'
unset(QM)
if(QT6_BUILD)
  QT6_CREATE_TRANSLATION(QM ${HEADERS} ${SOURCES} ${FORMS} ${TRANSLATIONS} OPTIONS -no-obsolete)
elseif(QT5_BUILD)
  QT5_CREATE_TRANSLATION(QM ${HEADERS} ${SOURCES} ${FORMS} ${TRANSLATIONS} OPTIONS -no-obsolete)
else()
  QT4_CREATE_TRANSLATION(QM ${HEADERS} ${SOURCES} ${FORMS} ${TRANSLATIONS} OPTIONS -no-obsolete)
endif()

add_custom_target(lupdate_base DEPENDS ${QM})
add_custom_target(lupdate)

foreach(l ${LANGS})
  add_custom_target(
    fix_${l}
    COMMAND "xmllint" "--noblanks" "torrentfileeditor_${l}.ts" "--output" "torrentfileeditor_${l}.ts"
    COMMAND "perl" "-pi" "-e" "s|<context><name>QLineEdit.*?</context>||g" "torrentfileeditor_${l}.ts"
    COMMAND "xmllint" "--format" "torrentfileeditor_${l}.ts" "--output" "torrentfileeditor_${l}.ts"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
  )
  add_dependencies(fix_${l} lupdate_base)
  add_dependencies(lupdate fix_${l})
endforeach()

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

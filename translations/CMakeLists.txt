# hack to update translations files only on 'make lupdate'
unset(QM)
if(QT5_BUILD)
  QT5_CREATE_TRANSLATION(QM ${HEADERS} ${SOURCES} ${FORMS} ${TRANSLATIONS} OPTIONS -no-obsolete)
else()
  QT4_CREATE_TRANSLATION(QM ${HEADERS} ${SOURCES} ${FORMS} ${TRANSLATIONS} OPTIONS -no-obsolete)
endif()
add_custom_target(lupdate DEPENDS ${QM})

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

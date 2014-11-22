# hack to update translations files only on 'make update'

QT4_CREATE_TRANSLATION(QM ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES} ${FORMS} ${TRANSLATIONS})
add_custom_target(lupdate DEPENDS ${QM})
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

set(SOURCES
    debug_new.cpp
)

set(PLAIN_HEADERS
    c++11.h
    debug_new.h
    fast_mutex.h
    _nvwa.h
    static_assert.h
)

add_library(nvwa OBJECT ${SOURCES} ${PLAIN_HEADERS})

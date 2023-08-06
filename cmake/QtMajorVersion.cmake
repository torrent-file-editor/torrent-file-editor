# Get Qt version from QT*_BUILD var
foreach(v 4 5 6)
    if (QT${v}_BUILD)
        set(QT_MAJOR_VERSION ${v})
    endif()
endforeach()

# Check if QT_QMAKE_EXECUTABLE is force to use this Qt version
if(QT_QMAKE_EXECUTABLE AND EXISTS ${QT_QMAKE_EXECUTABLE})
    # Get the Qt version from qmake
    execute_process(
        COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_VERSION
        OUTPUT_VARIABLE QT_VERSION_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Extract the major version number (e.g., 5 from "5.15.10")
    string(REGEX MATCH "[0-9]+" QT_MAJOR_VERSION "${QT_VERSION_STRING}")
endif()

if(QT_MAJOR_VERSION)
    set(QT${QT_MAJOR_VERSION}_BUILD TRUE)
    message(STATUS "Autodetected Qt${QT_MAJOR_VERSION} version")
endif()

# Use Qt5 in none of set
if(NOT QT_MAJOR_VERSION)
    set(QT_MAJOR_VERSION 5)
    set(QT${QT_MAJOR_VERSION}_BUILD TRUE)
    message(STATUS "Qt version is not set. Use Qt${QT_MAJOR_VERSION} version by default")
endif()

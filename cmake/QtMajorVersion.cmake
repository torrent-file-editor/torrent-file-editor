# SPDX-FileCopyrightText: 2023, 2026 Ivan Romanov <drizt72@zoho.eu>
# SPDX-License-Identifier: GPL-3.0-or-later

# Get Qt version from QT*_BUILD var
foreach(v 6 5 4)
    if (QT${v}_BUILD)
        set(QT_MAJOR_VERSION ${v})
        message(STATUS "Use Qt${QT_MAJOR_VERSION} version from args")
    endif()
endforeach()

# Check if QT_QMAKE_EXECUTABLE is force to use this Qt version
if(NOT QT_MAJOR_VERSION)
    if (!QT_QMAKE_EXECUTABLE OR NOT EXISTS ${QT_QMAKE_EXECUTABLE})
        find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake6 qmake-qt5 qmake-qt4)
    endif()

    # Get the Qt version from qmake
    execute_process(
        COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_VERSION
        OUTPUT_VARIABLE QT_VERSION_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Extract the major version number (e.g., 5 from "5.15.10")
    string(REGEX MATCH "[0-9]+" QT_MAJOR_VERSION "${QT_VERSION_STRING}")
    if(QT_MAJOR_VERSION)
        set(QT${QT_MAJOR_VERSION}_BUILD TRUE)
        message(STATUS "Autodetected Qt${QT_MAJOR_VERSION} version")
    endif()
endif()

# Use Qt6 in none of set
if(NOT QT_MAJOR_VERSION)
    set(QT_MAJOR_VERSION 6)
    set(QT${QT_MAJOR_VERSION}_BUILD TRUE)
    message(STATUS "Qt version is not set. Use Qt${QT_MAJOR_VERSION} version by default")
endif()

# CMAKE_EXECUTABLE_SUFFIX is not defined in scripting mode
if(WIN32)
    set(CMAKE_EXECUTABLE_SUFFIX .exe)
endif()

# Compile parse-compile-commands
if(NOT EXISTS ${BINARY_DIR}/parse-compile-commands${CMAKE_EXECUTABLE_SUFFIX})
    include(CheckLanguage)
    # check_language requires CMAKE_GENERATOR
    if(NOT CMAKE_GENERATOR)
        if(UNIX)
            set(CMAKE_GENERATOR "Unix Makefiles")
        elseif("$ENV{MSYSTEM}" STREQUAL MSYS)
            set(CMAKE_GENERATOR "MSYS Makefiles")
        else()
            set(CMAKE_GENERATOR "MinGW Makefiles")
        endif()
    endif()
    check_language(C)
    execute_process(COMMAND ${CMAKE_C_COMPILER} ${CMAKE_CURRENT_LIST_DIR}/parse-compile-commands.c -o ${BINARY_DIR}/parse-compile-commands${CMAKE_EXECUTABLE_SUFFIX})
endif()

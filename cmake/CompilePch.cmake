# Must be set:
# SOURCE_DIR
# BINARY_DIR
# TARGET
# PCH
# SOURCE_FILE
# LANG

include(${CMAKE_CURRENT_LIST_DIR}/JSONParser.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/JSONParser.cmake)

# Check all required variables is set
foreach(VAR SOURCE_DIR;BINARY_DIR;TARGET;PCH;SOURCE_FILE;LANG)
    if(NOT DEFINED ${VAR})
        message(FATAL_ERROR "${VAR} is not set.")
    endif()
endforeach()

# Path to PCH header can be absolue or relative
if(IS_ABSOLUTE ${PCH})
    set(PCH_PATH ${PCH})
else()
    set(PCH_PATH ${SOURCE_DIR}/${PCH})
endif()

# Check PCH header exists
if(NOT EXISTS ${PCH_PATH})
    message(FATAL_ERROR "${PCH_PATH} is not exists.")
endif()

# Check compile_commands.json exists
set(COMPILE_COMMANDS_PATH ${BINARY_DIR}/compile_commands.json)
if(NOT EXISTS ${COMPILE_COMMANDS_PATH})
    message(FATAL_ERROR "${COMPILE_COMMANDS_PATH} is not exists. Probably CMAKE_EXPORT_COMPILE_COMMANDS is not set.")
endif()

# Read and parse compile_commands.json
file(READ ${COMPILE_COMMANDS_PATH} COMPILE_COMMANDS_TEXT)
sbeParseJson(COMMANDS COMPILE_COMMANDS_TEXT)

get_filename_component(BASE_PCH ${PCH} NAME)

# Check GCH file updated
set(GSH_PATH CMakeFiles/${TARGET}_pch.dir/${BASE_PCH}.gch)
if(NOT (${PCH_PATH} IS_NEWER_THAN ${BINARY_DIR}/${GSH_PATH}))
    message(STATUS "Up-to-date ${PCH} PCH header")
    return()
endif()

# Set -x options
if(LANG STREQUAL CXX)
    set(LANG_HEADER c++-header)
elseif(LANG STREQUAL C)
    set(LANG_HEADER c-header)
else()
    message(FATAL_ERROR "${LANG} language is not supported.")
endif()

# Path to PCH header can be absolue or relative
if(NOT IS_ABSOLUTE ${SOURCE_FILE})
    set(SOURCE_FILE ${SOURCE_DIR}/${SOURCE_FILE})
endif()

# Search given SOURCE_FILE in compile_commands.json
set(I 0)
while(COMMANDS_${I}.command)   
    if(COMMANDS_${I}.file STREQUAL ${SOURCE_FILE})
        separate_arguments(ARGS NATIVE_COMMAND ${COMMANDS_${I}.command})
        list(FIND ARGS "-o" POS)
        if(POS STREQUAL -1)
            message(FATAL_ERROR "Wrong args:\n${ARGS}")
        endif()

        list(REMOVE_AT ARGS ${POS})
        list(REMOVE_AT ARGS ${POS})

        list(FIND ARGS "-c" POS)
        if(POS STREQUAL -1)
            message(FATAL_ERROR "Wrong args:\n${ARGS}")
        endif()
        list(REMOVE_AT ARGS ${POS})
        list(REMOVE_AT ARGS ${POS})

        string(REPLACE "-include;${BINARY_DIR}/CMakeFiles/${TARGET}_pch.dir/${BASE_PCH}" "" ARGS "${ARGS}")

        list(APPEND ARGS -x ${LANG_HEADER} -c ${SOURCE_DIR}/${PCH} -o ${GSH_PATH})
        file(MAKE_DIRECTORY CMakeFiles/${TARGET}_pch.dir)
        message(STATUS "Recompile ${PCH} PCH header")
        execute_process(COMMAND ${ARGS} WORKING_DIRECTORY ${BINARY_DIR})
        set(WORKING_DIR ${COMMANDS_${I}.directory})
        set(FOUND_SOURCE TRUE)
        break()
    endif()

    math(EXPR I "${I} + 1")
endwhile()

if(NOT FOUND_SOURCE)
    message(FATAL_ERROR "Source file isn't found in compile_commands.json")
endif()

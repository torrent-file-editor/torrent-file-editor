# Must be set:
# SOURCE_DIR
# BINARY_DIR
# CURRENT_BINARY_DIR
# TARGET
# PCH
# SOURCE_FILE
# LANG

function(compile_pch args pch_path gch_path lang_header binary_dir)
    string(REGEX REPLACE "\\.gch\$" "" include_gch "${gch_path}")
    set(include_gch "${binary_dir}/${include_gch}")

    separate_arguments(args NATIVE_COMMAND ${args})
    list(FIND args "-o" pos)
    if(pos EQUAL -1)
        message(FATAL_ERROR "Wrong args:\n${args}")
    endif()

    list(REMOVE_AT args ${pos})
    list(REMOVE_AT args ${pos})

    list(FIND args "-c" pos)
    if(POS EQUAL -1)
        message(FATAL_ERROR "Wrong args:\n${args}")
    endif()
    list(REMOVE_AT args ${pos})
    list(REMOVE_AT args ${pos})

    string(REPLACE "-include;${include_gch}" "" args "${args}")

    list(APPEND args -x ${lang_header} -c ${pch_path} -o ${gch_path})

    get_filename_component(gch_dir ${gch_path} DIRECTORY)
    file(MAKE_DIRECTORY ${gch_dir})
    message(STATUS "Recompile ${PCH} PCH header")
    execute_process(COMMAND ${args} WORKING_DIRECTORY ${binary_dir})
    set(WORKING_DIR ${COMMANDS_${I}.directory})
endfunction()

# Check all required variables is set
foreach(VAR SOURCE_DIR;BINARY_DIR;TARGET;PCH;SOURCE_FILE;LANG;CURRENT_BINARY_DIR)
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

# Check GCH file updated
get_filename_component(PCH_NAME ${PCH} NAME)
set(GSH_PATH CMakeFiles/${TARGET}_pch.dir/${PCH_NAME}.gch)
if(NOT (${PCH_PATH} IS_NEWER_THAN ${CURRENT_BINARY_DIR}/${GSH_PATH}))
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

# Fix source file path
if("$ENV{MSYSTEM}" STREQUAL MSYS)
    string(REGEX REPLACE "^/([A-Z])/" "\\1:/" SOURCE_FILE ${SOURCE_FILE})
endif()

execute_process(
    COMMAND ${BINARY_DIR}/parse-compile-commands${CMAKE_EXECUTABLE_SUFFIX} ${COMPILE_COMMANDS_PATH} ${SOURCE_FILE}
    OUTPUT_VARIABLE ARGS
    RESULT_VARIABLE RESULT
)

if (NOT ARGS)
    message(FATAL_ERROR "Can't parse compile command for ${SOURCE_FILE}: ${RESULT}")
endif()

# Fix args
if("$ENV{MSYSTEM}" STREQUAL MSYS)
    string(REGEX REPLACE "^/([A-Z])/" "\\1:/" ARGS "${ARGS}")
endif()

compile_pch(${ARGS} ${PCH_PATH} ${GSH_PATH} ${LANG_HEADER} ${CURRENT_BINARY_DIR})

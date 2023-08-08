cmake_minimum_required(VERSION 3.5)

macro(GET_DATE)
  #
  # All macro arguments are optional.
  #   If there's an ARGV0, use it as GD_PREFIX. Default = 'GD_'
  #   If there's an ARGV1, use it as ${GD_PREFIX}VERBOSE. Default = '0'
  #
  # If the date can be retrieved and parsed successfully, this macro
  # will set the following CMake variables:
  #
  #   GD_PREFIX
  #   ${GD_PREFIX}PREFIX (if '${GD_PREFIX}' is not 'GD_'...!)
  #   ${GD_PREFIX}VERBOSE
  #
  #   ${GD_PREFIX}OV
  #
  #   ${GD_PREFIX}REGEX
  #   ${GD_PREFIX}YEAR
  #   ${GD_PREFIX}MONTH
  #   ${GD_PREFIX}DAY
  #   ${GD_PREFIX}HOUR
  #   ${GD_PREFIX}MINUTE
  #   ${GD_PREFIX}SECOND
  #
  # Caller can then use these variables to construct names based on
  # date and time stamps...
  #

  # If there's an ARGV0, use it as GD_PREFIX:
  #
  set(GD_PREFIX "GD_")
  if(NOT "${ARGV0}" STREQUAL "")
    set(GD_PREFIX "${ARGV0}")
  endif()
  if(NOT "${GD_PREFIX}" STREQUAL "GD_")
    set(${GD_PREFIX}PREFIX "${GD_PREFIX}")
  endif()

  # If there's an ARGV1, use it as ${GD_PREFIX}VERBOSE:
  #
  set(${GD_PREFIX}VERBOSE "0")
  if(NOT "${ARGV1}" STREQUAL "")
    set(${GD_PREFIX}VERBOSE "${ARGV1}")
  endif()

  # Retrieve the current date and time in the format:
  #
  # 01/12/2006  08:55:12
  # mm/dd/YYYY HH:MM:SS
  #
  string(TIMESTAMP "${GD_PREFIX}OV" "%m/%d/%Y %H:%M:%S")

  if(${GD_PREFIX}VERBOSE)
    message(STATUS "")
    message(STATUS "<GET_DATE>")
    message(STATUS "")
    message(STATUS "GD_PREFIX='${GD_PREFIX}'")
    if(NOT "${GD_PREFIX}" STREQUAL "GD_")
      message(STATUS "${GD_PREFIX}PREFIX='${${GD_PREFIX}PREFIX}'")
    endif()
    message(STATUS "${GD_PREFIX}VERBOSE='${${GD_PREFIX}VERBOSE}'")
    message(STATUS "")
    message(STATUS "${GD_PREFIX}OV='${${GD_PREFIX}OV}'")
    message(STATUS "")
  endif()

  #
  # Extract six individual components by matching a regex with paren groupings.
  # Use the replace functionality and \\1 thru \\6 to extract components.
  #
  set(${GD_PREFIX}REGEX "([^/]+)/([^/]+)/([^ ]+) +([^:]+):([^:]+):([^\\.]+)")

  string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\1" ${GD_PREFIX}MONTH "${${GD_PREFIX}OV}")
  string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\2" ${GD_PREFIX}DAY "${${GD_PREFIX}OV}")
  string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\3" ${GD_PREFIX}YEAR "${${GD_PREFIX}OV}")
  string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\4" ${GD_PREFIX}HOUR "${${GD_PREFIX}OV}")
  string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\5" ${GD_PREFIX}MINUTE "${${GD_PREFIX}OV}")
  string(REGEX REPLACE "${${GD_PREFIX}REGEX}" "\\6" ${GD_PREFIX}SECOND "${${GD_PREFIX}OV}")

  if(${GD_PREFIX}VERBOSE)
    message(STATUS "${GD_PREFIX}REGEX='${${GD_PREFIX}REGEX}'")
    message(STATUS "${GD_PREFIX}YEAR='${${GD_PREFIX}YEAR}'")
    message(STATUS "${GD_PREFIX}MONTH='${${GD_PREFIX}MONTH}'")
    message(STATUS "${GD_PREFIX}DAY='${${GD_PREFIX}DAY}'")
    message(STATUS "${GD_PREFIX}HOUR='${${GD_PREFIX}HOUR}'")
    message(STATUS "${GD_PREFIX}MINUTE='${${GD_PREFIX}MINUTE}'")
    message(STATUS "${GD_PREFIX}SECOND='${${GD_PREFIX}SECOND}'")
    message(STATUS "")
    message(STATUS "Counters that change...")
    message(STATUS "")
    message(STATUS "        every second : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}${${GD_PREFIX}HOUR}${${GD_PREFIX}MINUTE}${${GD_PREFIX}SECOND}")
    message(STATUS "               daily : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}${${GD_PREFIX}DAY}")
    message(STATUS "             monthly : ${${GD_PREFIX}YEAR}${${GD_PREFIX}MONTH}")
    message(STATUS "            annually : ${${GD_PREFIX}YEAR}")
    message(STATUS "")
  endif()

  if(${GD_PREFIX}VERBOSE)
    message(STATUS "</GET_DATE>")
    message(STATUS "")
  endif()
endmacro()

GET_DATE()

set(APP_COMPILATION_DATE "${${GD_PREFIX}YEAR}-${${GD_PREFIX}MONTH}-${${GD_PREFIX}DAY}")
set(APP_COMPILATION_TIME "${${GD_PREFIX}HOUR}:${${GD_PREFIX}MINUTE}:${${GD_PREFIX}SECOND}")

execute_process(COMMAND git describe --tags --dirty
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  OUTPUT_VARIABLE _VERSION
  ERROR_VARIABLE ERROR_MESSAGE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(_VERSION)
  set(APP_VERSION ${_VERSION})
endif()

unset(_VERSION)

configure_file(${CMAKE_CURRENT_LIST_DIR}/../config.h.in config.h)
if(WIN32)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/../app.rc.in app.rc)
endif()

string(REGEX REPLACE "^v" "" APP_VERSION ${APP_VERSION})
if(WIN32)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/../appcast.json.in appcast.json)
endif()
file(WRITE appversion ${APP_VERSION})
if (APPLE)
  configure_file("${CMAKE_BINARY_DIR}/MacOSXBundleInfo.plist.in" "${CMAKE_BINARY_DIR}/Torrent File Editor.app/Contents/Info.plist")
  file(WRITE "${CMAKE_BINARY_DIR}/CPackProperties.cmake"
    "set(CPACK_PACKAGE_VERSION \"${APP_VERSION}\")\n"
    "set(CPACK_PACKAGE_FILE_NAME \"torrent-file-editor-\${CPACK_PACKAGE_VERSION}\")\n"
  )
endif()

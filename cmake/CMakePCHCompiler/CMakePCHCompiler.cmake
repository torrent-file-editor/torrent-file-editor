#.rst:
# CMakePCH
# --------
#
# Defines following functions:
#
# target_precompiled_header(target [...] header
#                          [REUSE reuse_target]
#                          [TYPE type])
#
# Uses given header as precompiled header for given target.
#
# Optionally it may reuse existing compiled header object from other target, so
# it is precompiled just once. Both targets need to have same compiler
# arguments otherwise compilation will fale.
#
# Also header may be given different type that default "c-header"/"c++-header".
#
# Do not include this file directly, rather specify that project uses CPCH or
# CXXPCH language with:
#
# project(project_name C CPCH)     # plain C project
# project(project_name CXX CXXPCH) # C++ project

# Copyright (c) CMakePCHCompiler Authors. All rights reserved.
# This code is licensed under the MIT License, see LICENSE.

include(CMakeParseArguments)

function(target_precompiled_header) # target [...] header
                                    # [REUSE reuse_target] [TYPE type]
	set(lang ${CMAKE_PCH_COMPILER_LANGUAGE})
	if(NOT MSVC AND
		NOT CMAKE_COMPILER_IS_GNU${lang} AND
		NOT CMAKE_${lang}_COMPILER_ID STREQUAL "GNU" AND
		NOT CMAKE_${lang}_COMPILER_ID STREQUAL "Clang" AND
		NOT CMAKE_${lang}_COMPILER_ID STREQUAL "AppleClang"
		)
		message(WARNING
			"Precompiled headers not supported for ${CMAKE_${lang}_COMPILER_ID}"
			)
		return()
	endif()
	cmake_parse_arguments(ARGS "" "REUSE;TYPE" "" ${ARGN})
	if(ARGS_SHARED)
		set(ARGS_REUSE ${ARGS_SHARED})
	endif()
	list(GET ARGS_UNPARSED_ARGUMENTS -1 header)
	list(REMOVE_AT ARGS_UNPARSED_ARGUMENTS -1)
	if(ARGS_REUSE AND NOT TARGET "${ARGS_REUSE}")
		message(SEND_ERROR "Re-use target \"${ARGS_REUSE}\" does not exist.")
		return()
	endif()
	foreach(target ${ARGS_UNPARSED_ARGUMENTS})
		if(NOT TARGET "${target}")
			message(SEND_ERROR "Target \"${target}\" does not exist.")
			return()
		endif()
		if(ARGS_REUSE)
			set(pch_target ${ARGS_REUSE}.pch)
			set(target_dir
				${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${pch_target}.dir
				)
		else()
			set(pch_target ${target}.pch)
			set(target_dir
				${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${pch_target}.dir
				)
		endif()
		if(MSVC)
			get_filename_component(win_pch "${target_dir}/${header}.pch" ABSOLUTE)
			get_filename_component(win_header "${header}" ABSOLUTE)
		endif()
		if(NOT ARGS_REUSE)
			if(ARGS_TYPE)
				set(header_type ${ARGS_TYPE})
			elseif(lang STREQUAL C)
				set(header_type "c-header")
			elseif(lang STREQUAL CXX)
				set(header_type "c++-header")
			else()
				message(WARNING "Unknown header type for language ${lang}")
				set(header_type "c++-header")
			endif()
			if(MSVC)
				# /Yc - create precompiled header
				# /Fp - exact location for precompiled header
				# /FI - force include of precompiled header
				set(flags "/Yc\"${win_header}\" /Fp\"${win_pch}\" /FI\"${win_header}\"")
				set_source_files_properties(
					${header}
					PROPERTIES
					LANGUAGE ${lang}
					COMPILE_FLAGS ${flags}
					)
				add_library(${pch_target} OBJECT ${header})
			else()
				set(flags "-x ${header_type}")
				set_source_files_properties(
					${header}
					PROPERTIES
					LANGUAGE ${lang}PCH
					COMPILE_FLAGS ${flags}
					)
				add_library(${pch_target} OBJECT ${header})
			endif()
		endif()

		get_target_property(target_libraries ${target} LINK_LIBRARIES)
		set_target_properties(${pch_target} PROPERTIES LINK_LIBRARIES "${target_libraries}")

		add_dependencies(${target} ${pch_target})

		if(MSVC)
			# /Yu - use given include as precompiled header
			# /Fp - exact location for precompiled header
			# /FI - force include of precompiled header
			set(exclude "/Yu${win_header}")
			target_compile_options(
				${target} PRIVATE "/Fp${win_pch}" "/FI${win_header}"
				)
			target_sources(${target} PRIVATE $<TARGET_OBJECTS:${pch_target}>)
		else()
			set(exclude -include ${target_dir}/${header})
		endif()
		target_compile_options(${target} PRIVATE ${exclude})

		if(NOT ARGS_REUSE)
			if(NOT DEFINED CMAKE_PCH_COMPILER_TARGETS)
				# this will be executed in just before makefile generation
				variable_watch(
					CMAKE_BACKWARDS_COMPATIBILITY
					__watch_pch_last_hook
					)
			endif()
			list(APPEND CMAKE_PCH_COMPILER_TARGETS ${target})
			set(CMAKE_PCH_COMPILER_TARGETS
				"${CMAKE_PCH_COMPILER_TARGETS}"
				PARENT_SCOPE
				)
			set_target_properties(${pch_target} PROPERTIES
				PCH_COMPILER_EXCLUDE "${exclude}"
				)
		endif()
	endforeach()
endfunction()

################################################################################
# PRIVATE MACROS
################################################################################

macro(__define_pch_compiler lang)
	if(NOT CMAKE_PCH_COMPILER_LANGUAGE)
		set(CMAKE_PCH_COMPILER_LANGUAGE ${lang})
	endif()

	# copy compiler settings from existing compiler
	set(CMAKE_${lang}PCH_COMPILE_OBJECT ${CMAKE_${lang}_COMPILE_OBJECT})
	set(CMAKE_INCLUDE_FLAG_${lang}PCH ${CMAKE_INCLUDE_FLAG_${lang}})
	set(CMAKE_INCLUDE_FLAG_SEP_${lang}PCH ${CMAKE_INCLUDE_FLAG_SEP_${lang}})
	set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang}PCH ${CMAKE_INCLUDE_SYSTEM_FLAG_${lang}})

	if(CMAKE_COMPILER_IS_GNU${lang} OR
		CMAKE_${lang}_COMPILER_ID STREQUAL "GNU"
		)
		set(CMAKE_${lang}PCH_OUTPUT_EXTENSION .gch)
	else()
		set(CMAKE_${lang}PCH_OUTPUT_EXTENSION .pch)
	endif()

	# setup compiler & platform specific flags same way C/CXX does
	if(CMAKE_${lang}_COMPILER_ID)
		if(CMAKE_VERSION VERSION_LESS 3.12)
			include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_${lang}_COMPILER_ID}-${lang}PCH
				OPTIONAL
				)
		else()
			include(Platform/${CMAKE_EFFECTIVE_SYSTEM_NAME}-${CMAKE_${lang}_COMPILER_ID}-${lang}PCH
				OPTIONAL
				)
		endif()
	endif()

	# just use all settings from C/CXX compiler
	string(REPLACE "${lang}PCH" "${lang}"
		CMAKE_${lang}PCH_COMPILE_OBJECT
		${CMAKE_${lang}PCH_COMPILE_OBJECT}
		)

	if(MSVC)
		# redirect object file to NUL and just create precompiled header
		# /FoNUL - do not write output object file file
		# /Fp - specify location for precompiled header
		string(REPLACE " /Fo" " /FoNUL /Fp"
			CMAKE_${lang}PCH_COMPILE_OBJECT
			${CMAKE_${lang}PCH_COMPILE_OBJECT}
			)
		# disable pdb, we point to later to different location
		string(REPLACE " /Fd<TARGET_COMPILE_PDB>" ""
			CMAKE_${lang}PCH_COMPILE_OBJECT
			${CMAKE_${lang}PCH_COMPILE_OBJECT}
			)
		# force /Z7 for MSVC2010 and forward to make it work,
		# as with /Zi Microsoft.Cpp.Win32.targets deletes the PCH file
		string(APPEND CMAKE_${lang}_FLAGS_DEBUG " /Z7")
		string(APPEND CMAKE_${lang}PCH_FLAGS_DEBUG " /Z7")
		string(APPEND CMAKE_${lang}_FLAGS_RELWITHDEBINFO " /Z7")
		string(APPEND CMAKE_${lang}PCH_FLAGS_RELWITHDEBINFO " /Z7")
	endif()

	# copy all initial settings for C/CXXPCH from C/CXX & watch them
	# NOTE: marking INTERNAL instead of STRING, so these do not appear in cmake-gui
	set(CMAKE_${lang}PCH_FLAGS "${CMAKE_${lang}_FLAGS_INIT}"
		CACHE INTERNAL #STRING
		"Flags used by the compiler during all build types."
		)
	variable_watch(CMAKE_${lang}_FLAGS __watch_pch_variable)

	if(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
		set(CMAKE_${lang}PCH_FLAGS_DEBUG "${CMAKE_${lang}_FLAGS_DEBUG_INIT}"
			CACHE INTERNAL #STRING
			"Flags used by the compiler during debug builds."
			)
		set(CMAKE_${lang}PCH_FLAGS_MINSIZEREL "${CMAKE_${lang}_FLAGS_MINSIZEREL_INIT}"
			CACHE INTERNAL #STRING
			"Flags used by the compiler during release builds for minimum size."
			)
		set(CMAKE_${lang}PCH_FLAGS_RELEASE "${CMAKE_${lang}_FLAGS_RELEASE_INIT}"
			CACHE INTERNAL #STRING
			"Flags used by the compiler during release builds."
			)
		set(CMAKE_${lang}PCH_FLAGS_RELWITHDEBINFO "${CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT}"
			CACHE INTERNAL #STRING
			"Flags used by the compiler during release builds with debug info."
			)
		variable_watch(CMAKE_${lang}_FLAGS_DEBUG          __watch_pch_variable)
		variable_watch(CMAKE_${lang}_FLAGS_MINSIZEREL     __watch_pch_variable)
		variable_watch(CMAKE_${lang}_FLAGS_RELEASE        __watch_pch_variable)
		variable_watch(CMAKE_${lang}_FLAGS_RELWITHDEBINFO __watch_pch_variable)
	endif()
endmacro()

# copies all compile definitions, flags and options to .pch subtarget
function(__watch_pch_last_hook variable access value)
	list(LENGTH CMAKE_PCH_COMPILER_TARGETS length)
	if(length EQUAL 0)
		return()
	endif()
	foreach(index RANGE -${length} -1)
		list(GET CMAKE_PCH_COMPILER_TARGETS ${index} target)
		set(pch_target ${target}.pch)
		foreach(property
			# NOTE: this list is likely incomplete
			COMPILE_DEFINITIONS
			COMPILE_DEFINITIONS_DEBUG
			COMPILE_DEFINITIONS_MINSIZEREL
			COMPILE_DEFINITIONS_RELEASE
			COMPILE_DEFINITIONS_RELWITHDEBINFO
			COMPILE_FLAGS
			COMPILE_OPTIONS
			INCLUDE_DIRECTORIES
			# commented out items are handled exceptionally:
			# CXX_STANDARD
			# POSITION_INDEPENDENT_CODE
			CXX_STANDARD_REQUIRED
			)
			get_target_property(value ${target} ${property})
			if(NOT value STREQUAL "value-NOTFOUND")
				set_target_properties(${pch_target} PROPERTIES
					"${property}" "${value}"
					)
			endif()
		endforeach()

		# HACK: remove target_precompiled_header inserted flags
		get_target_property(exclude ${pch_target} PCH_COMPILER_EXCLUDE)
		get_target_property(value ${pch_target} COMPILE_OPTIONS)
		string(REPLACE "${exclude}" "" value "${value}")
		set_target_properties(${pch_target} PROPERTIES
			COMPILE_OPTIONS "${value}"
			)

		# difficult cases
		if(NOT MSVC)
			get_target_property(value "${target}" CXX_STANDARD)
			if(value)
				target_compile_options("${pch_target}" PRIVATE
					-std=gnu++${value}
					)
			endif()
			# NOTE: setting POSITION_INDEPENDENT_CODE here has no effect
			get_target_property(value "${target}" POSITION_INDEPENDENT_CODE)
			if(value AND NOT CYGWIN)
				target_compile_options("${pch_target}" PRIVATE -fPIC)
			endif()
		endif()
	endforeach()
endfunction()

# copies all custom compiler settings to PCH compiler
macro(__watch_pch_variable variable access value)
	string(REPLACE _C_ _CPCH_ pchvariable ${variable})
	string(REPLACE _CXX_ _CXXPCH_ pchvariable ${pchvariable})
	set(${pchvariable} ${${variable}}) # because ${value} expands backslashes
endmacro()

macro(__configure_pch_compiler lang)
	set(CMAKE_${lang}PCH_COMPILER_ENV_VAR "${lang}PCH")
	set(CMAKE_${lang}PCH_COMPILER ${CMAKE_${lang}_COMPILER})

	if(SET_MSVC_${lang}PCH_ARCHITECTURE_ID)
		string(REPLACE _${lang}_ _${lang}PCH_
			${SET_MSVC_${lang}_ARCHITECTURE_ID}
			SET_MSVC_${lang}PCH_ARCHITECTURE_ID
			)
	endif()
	if(CMAKE_${lang}_SYSROOT_FLAG_CODE)
		string(REPLACE _${lang}_ _${lang}PCH_
			${CMAKE_${lang}_SYSROOT_FLAG_CODE}
			CMAKE_${lang}PCH_SYSROOT_FLAG_CODE
			)
	endif()
	if(CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG_CODE)
		string(REPLACE _${lang}_ _${lang}PCH_
			${CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG_CODE}
			CMAKE_${lang}PCH_OSX_DEPLOYMENT_TARGET_FLAG_CODE
			)
	endif()

	configure_file(
		${CMAKE_CURRENT_LIST_DIR}/CMake${lang}PCHCompiler.cmake.in
		${CMAKE_PLATFORM_INFO_DIR}/CMake${lang}PCHCompiler.cmake
		)
endmacro()

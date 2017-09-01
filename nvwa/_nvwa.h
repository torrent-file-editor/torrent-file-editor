// -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
// vim:tabstop=4:shiftwidth=4:expandtab:

/*
 * Copyright (C) 2013-2015 Wu Yongwei <adah at users dot sourceforge dot net>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute
 * it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software.  If you use this
 *    software in a product, an acknowledgement in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * This file is part of Stones of Nvwa:
 *      http://sourceforge.net/projects/nvwa
 *
 */

/**
 * @file  _nvwa.h
 *
 * Common definitions for preprocessing.
 *
 * @date  2015-10-28
 */

#ifndef NVWA_NVWA_H
#define NVWA_NVWA_H

/**
 * @namespace nvwa
 * Namespace of the nvwa project.  Most functions and global variables
 * are defined in this namespace.
 */

#ifndef NVWA_USE_NAMESPACE
#ifdef __cplusplus
#define NVWA_USE_NAMESPACE 1
#else
#define NVWA_USE_NAMESPACE 0
#endif // __cplusplus
#endif // NVWA_USE_NAMESPACE

#if NVWA_USE_NAMESPACE
#define NVWA_NAMESPACE_BEGIN namespace nvwa {
#define NVWA_NAMESPACE_END }
#define NVWA nvwa
#else  // NVWA_USE_NAMESPACE
#define NVWA_NAMESPACE_BEGIN
#define NVWA_NAMESPACE_END
#define NVWA
#endif // NVWA_USE_NAMESPACE

#ifndef NVWA_APPLE
#if defined(__APPLE__) && defined(__MACH__)
#define NVWA_APPLE 1
#else
#define NVWA_APPLE 0
#endif
#endif // NVWA_APPLE

#ifndef NVWA_CYGWIN
#if defined(__CYGWIN__)
#define NVWA_CYGWIN 1
#else
#define NVWA_CYGWIN 0
#endif
#endif // NVWA_CYGWIN

#ifndef NVWA_LINUX
#if defined(__linux__) || defined(__linux)
#define NVWA_LINUX 1
#else
#define NVWA_LINUX 0
#endif
#endif // NVWA_LINUX

#ifndef NVWA_UNIX
#if defined(__unix__) || defined(__unix) || NVWA_APPLE
#define NVWA_UNIX 1
#else
#define NVWA_UNIX 0
#endif
#endif // NVWA_UNIX

#ifndef NVWA_WIN32
#if defined(_WIN32)
#define NVWA_WIN32 1
#else
#define NVWA_WIN32 0
#endif
#endif // NVWA_WIN32

#ifndef NVWA_WINDOWS
#if NVWA_CYGWIN || NVWA_WIN32
#define NVWA_WINDOWS 1
#else
#define NVWA_WINDOWS 0
#endif
#endif // NVWA_WINDOWS

#endif // NVWA_NVWA_H

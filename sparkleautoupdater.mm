/*
 * Copyright (C) 2008  Remko Troncon
 * Copyright (C) 2015  Ivan Romanov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sparkleautoupdater.h"

// MacOSX10.7 SDK has missed NS_ENUM

#ifndef NS_ENUM
# if (__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && __has_feature(objc_fixed_enum))
#  define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#  if (__cplusplus)
#   define NS_OPTIONS(_type, _name) _type _name; enum : _type
#  else
#   define NS_OPTIONS(_type, _name) enum _name : _type _name; enum _name : _type
#  endif
# else
#  define NS_ENUM(_type, _name) _type _name; enum
#  define NS_OPTIONS(_type, _name) _type _name; enum
# endif
#endif

#include <Cocoa/Cocoa.h>
#include <Sparkle/Sparkle.h>

class SparkleAutoUpdater::Private
{
public:
    SUUpdater* updater;
};

SparkleAutoUpdater::SparkleAutoUpdater()
{
    d = new Private;

    d->updater = [SUUpdater sharedUpdater];
    [d->updater retain];
}

SparkleAutoUpdater::~SparkleAutoUpdater()
{
    [d->updater release];
    delete d;
}

void SparkleAutoUpdater::checkForUpdates()
{
    [d->updater checkForUpdatesInBackground];
}

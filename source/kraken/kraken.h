/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#ifndef KRAKEN_H
#define KRAKEN_H

#define KRAKEN_VERSION_MAJOR 1
#define KRAKEN_VERSION_MINOR 50

/* KRAKEN major and minor version. */
#define KRAKEN_VERSION 150
/* KRAKEN patch version for bugfix releases. */
#define KRAKEN_VERSION_PATCH 0
/** KRAKEN release cycle stage: alpha/beta/rc/release. */
#define KRAKEN_VERSION_CYCLE alpha

/* KRAKEN file format version. */
#define KRAKEN_FILE_VERSION KRAKEN_VERSION
#define KRAKEN_FILE_SUBVERSION 1

#define KRAKEN_FILE_MIN_VERSION 150
#define KRAKEN_FILE_MIN_SUBVERSION 0

#ifdef __cplusplus
#  define KRAKEN_USE_NAMESPACES 1
#else /* __cplusplus */
#  define KRAKEN_USE_NAMESPACES 0
#endif /* __cplusplus */

#if KRAKEN_USE_NAMESPACES

#  define KRAKEN_NS kraken
#  define KRAKEN_INTERNAL_NS krakenInternal_v1_50__krakenReserved__
#  define KRAKEN_NS_GLOBAL ::KRAKEN_NS

namespace KRAKEN_INTERNAL_NS
{}

// The root level namespace for all kraken source.
namespace KRAKEN_NS
{
  using namespace KRAKEN_INTERNAL_NS;
}

#  define KRAKEN_NAMESPACE_BEGIN \
    namespace KRAKEN_INTERNAL_NS \
    {
#  define KRAKEN_NAMESPACE_END }
#  define KRAKEN_NAMESPACE_USING using namespace KRAKEN_NS;

#else

#  define KRAKEN_NS
#  define KRAKEN_NS_GLOBAL
#  define KRAKEN_NAMESPACE_BEGIN
#  define KRAKEN_NAMESPACE_END
#  define KRAKEN_NAMESPACE_USING

#endif /* KRAKEN_USE_NAMESPACES */

#endif  /* KRAKEN_H */

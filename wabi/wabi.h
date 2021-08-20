/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */

#ifndef WABI_H
#define WABI_H

/**
 * @file wabi/wabi.h
 *
 * Pixar's Universal Scene Description | Built for Kraken
 * .              +   .                .   . .     .  .
 *                   .                    .       .     *
 *  .       *                        . . . .  .   .  + .
 *            "You Are Here"            .   .  +  . . .
 *.                 |             .  .   .    .    . .
 *                  |           .     .     . +.    +  .
 *                 \|/            .       .   . .
 *        . .       V          .    * . . .  .  +   .
 *           +      .           .   .      +
 *                            .       . +  .+. .
 *  .                      .     . + .  . .     .      .
 *           .      .    .     . .   . . .        ! /
 *      *             .    . .  +    .  .       - O -
 *          .     .    .  +   . .  *  .       . / |
 *               . + .  .  .  .. +  .
 *.      .  .  .  *   .  *  . +..  .            *
 * .      .   . .   .   .   . .  +   .    .            +
 * A custom build, carefully tailored to the needs of the
 * Kraken software project, where it is used as a foundational
 * component framework in the context of a Digital Content
 * Creation Platform. Changes were made to better establish
 * Universal Scene Description for expanded use as the central
 * primary architecture -- rather than for a supplementary role.
 *
 * Capabilities expanded, yet, ensuring it stays consistent and
 * interoperable with Pixar's current Release branch. Additional
 * focus spent on cross-platform interoperability, where Microsoft
 * Windows support was lacking, and maintaining consistency with
 * Linux. And expanding the graphics capabilities of the graphics
 * & imaging APIs, such as Phoenix -- the real-time render engine,
 * and Hydra Engine/Rendering API improvements.
 *
 * The main focus areas on animation toolkits, skel rigging, 2D
 * animation authored and edited with scene description, and
 * expansive procedural node-based workflows.
 */

#define WABI_VERSION_MAJOR 0
#define WABI_VERSION_MINOR 21
#define WABI_VERSION_PATCH 8

#define WABI_VERSION 2108

#define WABI_USE_NAMESPACES 1

#if WABI_USE_NAMESPACES

#  define WABI_NS wabi
#  define WABI_INTERNAL_NS WABImaelstrom
#  define WABI_NS_GLOBAL ::WABI_NS

namespace WABI_INTERNAL_NS
{
}

// The root level namespace for all source in the USD distribution.
namespace WABI_NS
{
  using namespace WABI_INTERNAL_NS;
}

#  define WABI_NAMESPACE_BEGIN \
    namespace WABI_INTERNAL_NS \
    {
#  define WABI_NAMESPACE_END }
#  define WABI_NAMESPACE_USING using namespace WABI_NS;

#else

#  define WABI_NS
#  define WABI_NS_GLOBAL
#  define WABI_NAMESPACE_BEGIN
#  define WABI_NAMESPACE_END
#  define WABI_NAMESPACE_USING

#endif  // WABI_USE_NAMESPACES

#endif  // WABI_H

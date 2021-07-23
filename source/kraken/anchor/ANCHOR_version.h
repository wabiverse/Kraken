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
 * Copyright 2021, Wabi.
 */

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

/**
 * Version (Integer encoded as XYYZZ for use
 * in #if preprocessor conditionals. Work in
 * progress versions typically starts at XYY99
 * then bounce up to XYY00, XYY01 etc. when
 * release tagging happens) */

#define ANCHOR_VERSION "1.84 WIP"
#define ANCHOR_VERSION_NUM 18302

#define ANCHOR_CHECKVERSION()                                    \
  ANCHOR::DebugCheckVersionAndDataLayout(ANCHOR_VERSION,         \
                                         sizeof(AnchorIO),       \
                                         sizeof(AnchorStyle),    \
                                         sizeof(wabi::GfVec2f),  \
                                         sizeof(wabi::GfVec4f),  \
                                         sizeof(AnchorDrawVert), \
                                         sizeof(AnchorDrawIdx))
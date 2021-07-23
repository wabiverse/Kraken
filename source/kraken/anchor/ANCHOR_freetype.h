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

#include "ANCHOR_api.h"  // ANCHOR_API

// Forward declarations
struct AnchorFontAtlas;
struct AnchorFontBuilderIO;

// Hinting greatly impacts visuals (and glyph sizes).
// - By default, hinting is enabled and the font's native hinter is preferred over the auto-hinter.
// - When disabled, FreeType generates blurrier glyphs, more or less matches the stb_truetype.h
// - The Default hinting mode usually looks good, but may distort glyphs in an unusual way.
// - The Light hinting mode generates fuzzier glyphs but better matches Microsoft's rasterizer.
// You can set those flags globaly in AnchorFontAtlas::FontBuilderFlags
// You can set those flags on a per font basis in AnchorFontConfig::FontBuilderFlags
enum AnchorFreeTypeBuilderFlags
{
  AnchorFreeTypeBuilderFlags_NoHinting =
    1 << 0,                                        // Disable hinting. This generally generates 'blurrier' bitmap glyphs when the glyph
                                                   // are rendered in any of the anti-aliased modes.
  AnchorFreeTypeBuilderFlags_NoAutoHint = 1 << 1,  // Disable auto-hinter.
  AnchorFreeTypeBuilderFlags_ForceAutoHint =
    1 << 2,  // Indicates that the auto-hinter is preferred over the font's native hinter.
  AnchorFreeTypeBuilderFlags_LightHinting =
    1 << 3,  // A lighter hinting algorithm for gray-level modes. Many generated glyphs are
             // fuzzier but better resemble their original shape. This is achieved by snapping
             // glyphs to the pixel grid only vertically (Y-axis), as is done by Microsoft's
             // ClearType and Adobe's proprietary font renderer. This preserves inter-glyph
             // spacing in horizontal text.
  AnchorFreeTypeBuilderFlags_MonoHinting =
    1 << 4,                                     // Strong hinting algorithm that should only be used for monochrome output.
  AnchorFreeTypeBuilderFlags_Bold = 1 << 5,     // Styling: Should we artificially embolden the font?
  AnchorFreeTypeBuilderFlags_Oblique = 1 << 6,  // Styling: Should we slant the font, emulating italic style?
  AnchorFreeTypeBuilderFlags_Monochrome =
    1 << 7,                                       // Disable anti-aliasing. Combine this with MonoHinting for best results!
  AnchorFreeTypeBuilderFlags_LoadColor = 1 << 8,  // Enable FreeType color-layered glyphs
  AnchorFreeTypeBuilderFlags_Bitmap = 1 << 9      // Enable FreeType bitmap glyphs
};

namespace AnchorFreeType
{
  // If you need to dynamically select between multiple builders:
  // - you can manually assign this builder with 'atlas->FontBuilderIO =
  // AnchorFreeType::GetBuilderForFreeType()'
  // - prefer deep-copying this into your own AnchorFontBuilderIO instance if you use hot-reloading
  // that messes up static data.
  ANCHOR_API const AnchorFontBuilderIO *GetBuilderForFreeType();

  // Override allocators. By default AnchorFreeType will use ANCHOR_ALLOC()/ANCHOR_FREE()
  // However, as FreeType does lots of allocations we provide a way for the user to redirect it to a
  // separate memory heap if desired.
  ANCHOR_API void SetAllocatorFunctions(void *(*alloc_func)(size_t sz, void *user_data),
                                        void (*free_func)(void *ptr, void *user_data),
                                        void *user_data = NULL);

#ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  static inline bool BuildFontAtlas(AnchorFontAtlas *atlas, unsigned int flags = 0)
  {
    atlas->FontBuilderIO = GetBuilderForFreeType();
    atlas->FontBuilderFlags = flags;
    return atlas->Build();
  }
#endif
}  // namespace AnchorFreeType

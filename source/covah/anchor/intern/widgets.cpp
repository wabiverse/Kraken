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

/**
 * @file
 * Anchor.
 * Bare Metal.
 */

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include "ANCHOR_api.h"
#ifndef ANCHOR_DISABLE

#  ifndef ANCHOR_DEFINE_MATH_OPERATORS
#    define ANCHOR_DEFINE_MATH_OPERATORS
#  endif
#  include "ANCHOR_internal.h"

// System includes
#  include <ctype.h>                         // toupper
#  if defined(_MSC_VER) && _MSC_VER <= 1500  // MSVC 2008 or earlier
#    include <stddef.h>                      // intptr_t
#  else
#    include <stdint.h>  // intptr_t
#  endif

//-------------------------------------------------------------------------
// Warnings
//-------------------------------------------------------------------------

// Visual Studio warnings
#  ifdef _MSC_VER
#    pragma warning(disable : 4127)            // condition expression is constant
#    pragma warning(disable : 4996)            // 'This function or variable may be unsafe': strcpy, strdup,
                                               // sprintf, vsnprintf, sscanf, fopen
#    if defined(_MSC_VER) && _MSC_VER >= 1922  // MSVC 2019 16.2 or later
#      pragma warning(disable : 5054)  // operator '|': deprecated between enumerations of different types
#    endif
#    pragma warning(disable : 26451)  // [Static Analyzer] Arithmetic overflow : Using operator
                                      // 'xxx' on a 4 byte value and then casting the result to a 8
                                      // byte value. Cast the value to the wider type before
                                      // calling operator 'xxx' to avoid overflow(io.2).
#    pragma warning(disable : 26812)  // [Static Analyzer] The enum type 'xxx' is unscoped. Prefer
                                      // 'enum class' over 'enum' (Enum.3).
#  endif

// Clang/GCC warnings with -Weverything
#  if defined(__clang__)
#    if __has_warning("-Wunknown-warning-option")
#      pragma clang diagnostic ignored \
        "-Wunknown-warning-option"  // warning: unknown warning group 'xxx' // not all warnings
                                    // are known by all Clang versions and they tend to be
                                    // rename-happy.. so ignoring warnings triggers new warnings
                                    // on some configuration. Great!
#    endif
#    pragma clang diagnostic ignored "-Wunknown-pragmas"  // warning: unknown warning group 'xxx'
#    pragma clang diagnostic ignored "-Wold-style-cast"   // warning: use of old-style cast // yes,
                                                          // they are more terse.
#    pragma clang diagnostic ignored \
      "-Wfloat-equal"  // warning: comparing floating point with == or != is unsafe // storing
                       // and comparing against same constants (typically 0.0f) is ok.
#    pragma clang diagnostic ignored \
      "-Wformat-nonliteral"  // warning: format string is not a string literal            //
                             // passing non-literal to vsnformat(). yes, user passing incorrect
                             // format strings can crash the code.
#    pragma clang diagnostic ignored "-Wsign-conversion"  // warning: implicit conversion changes signedness
#    pragma clang diagnostic ignored \
      "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant // some
                                         // standard header variations use #define NULL 0
#    pragma clang diagnostic ignored \
      "-Wdouble-promotion"  // warning: implicit conversion from 'float' to 'double' when passing
                            // argument to function  // using printf() is a misery with this as
                            // C++ va_arg ellipsis changes float to double.
#    pragma clang diagnostic ignored \
      "-Wenum-enum-conversion"  // warning: bitwise operation between different enumeration types
                                // ('XXXFlags_' and 'XXXFlagsPrivate_')
#    pragma clang diagnostic ignored \
      "-Wdeprecated-enum-enum-conversion"  // warning: bitwise operation between different
                                           // enumeration types ('XXXFlags_' and
                                           // 'XXXFlagsPrivate_') is deprecated
#    pragma clang diagnostic ignored \
      "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float'
                                         // may lose precision
#  elif defined(__GNUC__)
#    pragma GCC diagnostic ignored \
      "-Wpragmas"  // warning: unknown option after '#pragma GCC diagnostic' kind
#    pragma GCC diagnostic ignored \
      "-Wformat-nonliteral"  // warning: format not a string literal, format string not checked
#    pragma GCC diagnostic ignored \
      "-Wclass-memaccess"  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object
                           // of type 'xxxx' with no trivial copy-assignment; use assignment or
                           // value-initialization instead
#  endif

WABI_NAMESPACE_USING

//-------------------------------------------------------------------------
// Data
//-------------------------------------------------------------------------

// Widgets
static const float DRAGDROP_HOLD_TO_OPEN_TIMER =
  0.70f;  // Time for drag-hold to activate items accepting the
          // ANCHOR_ButtonFlags_PressedOnDragDropHold button behavior.
static const float DRAG_MOUSE_THRESHOLD_FACTOR =
  0.50f;  // Multiplier for the default value of io.MouseDragThreshold to make DragFloat/DragInt
          // react faster to mouse drags.

// Those MIN/MAX values are not define because we need to point to them
static const signed char IM_S8_MIN = -128;
static const signed char IM_S8_MAX = 127;
static const unsigned char IM_U8_MIN = 0;
static const unsigned char IM_U8_MAX = 0xFF;
static const signed short IM_S16_MIN = -32768;
static const signed short IM_S16_MAX = 32767;
static const unsigned short IM_U16_MIN = 0;
static const unsigned short IM_U16_MAX = 0xFFFF;
static const AnchorS32 IM_S32_MIN = INT_MIN;  // (-2147483647 - 1), (0x80000000);
static const AnchorS32 IM_S32_MAX = INT_MAX;  // (2147483647), (0x7FFFFFFF)
static const AnchorU32 IM_U32_MIN = 0;
static const AnchorU32 IM_U32_MAX = UINT_MAX;  // (0xFFFFFFFF)
#  ifdef LLONG_MIN
static const AnchorS64 IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const AnchorS64 IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#  else
static const AnchorS64 IM_S64_MIN = -9223372036854775807LL - 1;
static const AnchorS64 IM_S64_MAX = 9223372036854775807LL;
#  endif
static const AnchorU64 IM_U64_MIN = 0;
#  ifdef ULLONG_MAX
static const AnchorU64 IM_U64_MAX = ULLONG_MAX;  // (0xFFFFFFFFFFFFFFFFull);
#  else
static const AnchorU64 IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#  endif

//-------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-------------------------------------------------------------------------

// For InputTextEx()
static bool InputTextFilterCharacter(unsigned int *p_char,
                                     ANCHORInputTextFlags flags,
                                     ANCHORInputTextCallback callback,
                                     void *user_data,
                                     ANCHORInputSource input_source);
static int InputTextCalcTextLenAndLineCount(const char *text_begin, const char **out_text_end);
static GfVec2f InputTextCalcTextSizeW(const AnchorWChar *text_begin,
                                      const AnchorWChar *text_end,
                                      const AnchorWChar **remaining = NULL,
                                      GfVec2f *out_offset = NULL,
                                      bool stop_on_new_line = false);

//-------------------------------------------------------------------------
// [SECTION] Widgets: Text, etc.
//-------------------------------------------------------------------------
// - TextEx() [Internal]
// - TextUnformatted()
// - Text()
// - TextV()
// - TextColored()
// - TextColoredV()
// - TextDisabled()
// - TextDisabledV()
// - TextWrapped()
// - TextWrappedV()
// - LabelText()
// - LabelTextV()
// - BulletText()
// - BulletTextV()
//-------------------------------------------------------------------------

void ANCHOR::TextEx(const char *text, const char *text_end, ANCHOR_TextFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  ANCHOR_ASSERT(text != NULL);
  const char *text_begin = text;
  if (text_end == NULL)
    text_end = text + strlen(text);  // FIXME-OPT

  const GfVec2f text_pos(window->DC.CursorPos[0],
                         window->DC.CursorPos[1] + window->DC.CurrLineTextBaseOffset);
  const float wrap_pos_x = window->DC.TextWrapPos;
  const bool wrap_enabled = (wrap_pos_x >= 0.0f);
  if (text_end - text > 2000 && !wrap_enabled) {
    // Long text!
    // Perform manual coarse clipping to optimize for long multi-line text
    // - From this point we will only compute the width of lines that are visible. Optimization
    // only available when word-wrapping is disabled.
    // - We also don't vertically center the text within the line full height, which is unlikely to
    // matter because we are likely the biggest and only item on the line.
    // - We use memchr(), pay attention that well optimized versions of those str/mem functions are
    // much faster than a casually written loop.
    const char *line = text;
    const float line_height = GetTextLineHeight();
    GfVec2f text_size(0, 0);

    // Lines to skip (can't skip when logging text)
    GfVec2f pos = text_pos;
    if (!g.LogEnabled) {
      int lines_skippable = (int)((window->ClipRect.Min[1] - text_pos[1]) / line_height);
      if (lines_skippable > 0) {
        int lines_skipped = 0;
        while (line < text_end && lines_skipped < lines_skippable) {
          const char *line_end = (const char *)memchr(line, '\n', text_end - line);
          if (!line_end)
            line_end = text_end;
          if ((flags & ANCHOR_TextFlags_NoWidthForLargeClippedText) == 0)
            text_size[0] = AnchorMax(text_size[0], CalcTextSize(line, line_end)[0]);
          line = line_end + 1;
          lines_skipped++;
        }
        pos[1] += lines_skipped * line_height;
      }
    }

    // Lines to render
    if (line < text_end) {
      ImRect line_rect(pos, pos + GfVec2f(FLT_MAX, line_height));
      while (line < text_end) {
        if (IsClippedEx(line_rect, 0, false))
          break;

        const char *line_end = (const char *)memchr(line, '\n', text_end - line);
        if (!line_end)
          line_end = text_end;
        text_size[0] = AnchorMax(text_size[0], CalcTextSize(line, line_end)[0]);
        RenderText(pos, line, line_end, false);
        line = line_end + 1;
        line_rect.Min[1] += line_height;
        line_rect.Max[1] += line_height;
        pos[1] += line_height;
      }

      // Count remaining lines
      int lines_skipped = 0;
      while (line < text_end) {
        const char *line_end = (const char *)memchr(line, '\n', text_end - line);
        if (!line_end)
          line_end = text_end;
        if ((flags & ANCHOR_TextFlags_NoWidthForLargeClippedText) == 0)
          text_size[0] = AnchorMax(text_size[0], CalcTextSize(line, line_end)[0]);
        line = line_end + 1;
        lines_skipped++;
      }
      pos[1] += lines_skipped * line_height;
    }
    text_size[1] = (pos - text_pos)[1];

    ImRect bb(text_pos, text_pos + text_size);
    ItemSize(text_size, 0.0f);
    ItemAdd(bb, 0);
  }
  else {
    const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
    const GfVec2f text_size = CalcTextSize(text_begin, text_end, false, wrap_width);

    ImRect bb(text_pos, text_pos + text_size);
    ItemSize(text_size, 0.0f);
    if (!ItemAdd(bb, 0))
      return;

    // Render (we don't hide text after ## in this end-user function)
    RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
  }
}

void ANCHOR::TextUnformatted(const char *text, const char *text_end)
{
  TextEx(text, text_end, ANCHOR_TextFlags_NoWidthForLargeClippedText);
}

void ANCHOR::Text(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  TextV(fmt, args);
  va_end(args);
}

void ANCHOR::TextV(const char *fmt, va_list args)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  const char *text_end = g.TempBuffer +
                         ImFormatStringV(g.TempBuffer, ANCHOR_ARRAYSIZE(g.TempBuffer), fmt, args);
  TextEx(g.TempBuffer, text_end, ANCHOR_TextFlags_NoWidthForLargeClippedText);
}

void ANCHOR::TextColored(const GfVec4f &col, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  TextColoredV(col, fmt, args);
  va_end(args);
}

void ANCHOR::TextColoredV(const GfVec4f &col, const char *fmt, va_list args)
{
  PushStyleColor(ANCHOR_Col_Text, col);
  if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
    TextEx(va_arg(args, const char *),
           NULL,
           ANCHOR_TextFlags_NoWidthForLargeClippedText);  // Skip formatting
  else
    TextV(fmt, args);
  PopStyleColor();
}

void ANCHOR::TextDisabled(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  TextDisabledV(fmt, args);
  va_end(args);
}

void ANCHOR::TextDisabledV(const char *fmt, va_list args)
{
  ANCHOR_Context &g = *G_CTX;
  PushStyleColor(ANCHOR_Col_Text, g.Style.Colors[ANCHOR_Col_TextDisabled]);
  if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
    TextEx(va_arg(args, const char *),
           NULL,
           ANCHOR_TextFlags_NoWidthForLargeClippedText);  // Skip formatting
  else
    TextV(fmt, args);
  PopStyleColor();
}

void ANCHOR::TextWrapped(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  TextWrappedV(fmt, args);
  va_end(args);
}

void ANCHOR::TextWrappedV(const char *fmt, va_list args)
{
  ANCHOR_Context &g = *G_CTX;
  bool need_backup = (g.CurrentWindow->DC.TextWrapPos <
                      0.0f);  // Keep existing wrap position if one is already set
  if (need_backup)
    PushTextWrapPos(0.0f);
  if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
    TextEx(va_arg(args, const char *),
           NULL,
           ANCHOR_TextFlags_NoWidthForLargeClippedText);  // Skip formatting
  else
    TextV(fmt, args);
  if (need_backup)
    PopTextWrapPos();
}

void ANCHOR::LabelText(const char *label, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  LabelTextV(label, fmt, args);
  va_end(args);
}

// Add a label+text combo aligned to other label+value widgets
void ANCHOR::LabelTextV(const char *label, const char *fmt, va_list args)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const float w = CalcItemWidth();

  const char *value_text_begin = &g.TempBuffer[0];
  const char *value_text_end = value_text_begin +
                               ImFormatStringV(g.TempBuffer, ANCHOR_ARRAYSIZE(g.TempBuffer), fmt, args);
  const GfVec2f value_size = CalcTextSize(value_text_begin, value_text_end, false);
  const GfVec2f label_size = CalcTextSize(label, NULL, true);

  const GfVec2f pos = window->DC.CursorPos;
  const ImRect value_bb(pos, pos + GfVec2f(w, value_size[1] + style.FramePadding[1] * 2));
  const ImRect total_bb(
    pos,
    pos + GfVec2f(w + (label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f),
                  AnchorMax(value_size[1], label_size[1]) + style.FramePadding[1] * 2));
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, 0))
    return;

  // Render
  RenderTextClipped(value_bb.Min + style.FramePadding,
                    value_bb.Max,
                    value_text_begin,
                    value_text_end,
                    &value_size,
                    GfVec2f(0.0f, 0.0f));
  if (label_size[0] > 0.0f)
    RenderText(GfVec2f(value_bb.Max[0] + style.ItemInnerSpacing[0], value_bb.Min[1] + style.FramePadding[1]),
               label);
}

void ANCHOR::BulletText(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  BulletTextV(fmt, args);
  va_end(args);
}

// Text with a little bullet aligned to the typical tree node.
void ANCHOR::BulletTextV(const char *fmt, va_list args)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;

  const char *text_begin = g.TempBuffer;
  const char *text_end = text_begin +
                         ImFormatStringV(g.TempBuffer, ANCHOR_ARRAYSIZE(g.TempBuffer), fmt, args);
  const GfVec2f label_size = CalcTextSize(text_begin, text_end, false);
  const GfVec2f total_size = GfVec2f(
    g.FontSize + (label_size[0] > 0.0f ? (label_size[0] + style.FramePadding[0] * 2) : 0.0f),
    label_size[1]);  // Empty text doesn't add padding
  GfVec2f pos = window->DC.CursorPos;
  pos[1] += window->DC.CurrLineTextBaseOffset;
  ItemSize(total_size, 0.0f);
  const ImRect bb(pos, pos + total_size);
  if (!ItemAdd(bb, 0))
    return;

  // Render
  AnchorU32 text_col = GetColorU32(ANCHOR_Col_Text);
  RenderBullet(window->DrawList,
               bb.Min + GfVec2f(style.FramePadding[0] + g.FontSize * 0.5f, g.FontSize * 0.5f),
               text_col);
  RenderText(bb.Min + GfVec2f(g.FontSize + style.FramePadding[0] * 2, 0.0f), text_begin, text_end, false);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Main
//-------------------------------------------------------------------------
// - ButtonBehavior() [Internal]
// - Button()
// - SmallButton()
// - InvisibleButton()
// - ArrowButton()
// - CloseButton() [Internal]
// - CollapseButton() [Internal]
// - GetWindowScrollbarID() [Internal]
// - GetWindowScrollbarRect() [Internal]
// - Scrollbar() [Internal]
// - ScrollbarEx() [Internal]
// - Image()
// - ImageButton()
// - Checkbox()
// - CheckboxFlagsT() [Internal]
// - CheckboxFlags()
// - RadioButton()
// - ProgressBar()
// - Bullet()
//-------------------------------------------------------------------------

// The ButtonBehavior() function is key to many interactions and used by many/most widgets.
// Because we handle so many cases (keyboard/gamepad navigation, drag and drop) and many specific
// behavior (via ANCHOR_ButtonFlags_), this code is a little complex. By far the most common path
// is interacting with the Mouse using the default ANCHOR_ButtonFlags_PressedOnClickRelease button
// behavior. See the series of events below and the corresponding state reported by ANCHOR:
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnClickRelease:             return-value  IsItemHovered()  IsItemActive()
// IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+0 (mouse is outside bb)        -             -                -               - - -
//   Frame N+1 (mouse moves inside bb)      -             true             -               - - -
//   Frame N+2 (mouse button is down)       -             true             true            true -
//   true Frame N+3 (mouse button is down)       -             true             true            -
//   -                    - Frame N+4 (mouse moves outside bb)     -             - true - - - Frame
//   N+5 (mouse moves inside bb)      -             true             true            - - - Frame
//   N+6 (mouse button is released)   true          true             -               - true - Frame
//   N+7 (mouse button is released)   -             true             -               - - - Frame
//   N+8 (mouse moves outside bb)     -             -                -               - - -
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnClick:                    return-value  IsItemHovered()  IsItemActive()
// IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+2 (mouse button is down)       true          true             true            true -
//   true Frame N+3 (mouse button is down)       -             true             true            -
//   -                    - Frame N+6 (mouse button is released)   -             true             -
//   -                  true                 - Frame N+7 (mouse button is released)   - true - - -
//   -
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnRelease:                  return-value  IsItemHovered()  IsItemActive()
// IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+2 (mouse button is down)       -             true             -               - - true
//   Frame N+3 (mouse button is down)       -             true             -               - - -
//   Frame N+6 (mouse button is released)   true          true             -               - - -
//   Frame N+7 (mouse button is released)   -             true             -               - - -
//------------------------------------------------------------------------------------------------------------------------------------------------
// with PressedOnDoubleClick:              return-value  IsItemHovered()  IsItemActive()
// IsItemActivated()  IsItemDeactivated()  IsItemClicked()
//   Frame N+0 (mouse button is down)       -             true             -               - - true
//   Frame N+1 (mouse button is down)       -             true             -               - - -
//   Frame N+2 (mouse button is released)   -             true             -               - - -
//   Frame N+3 (mouse button is released)   -             true             -               - - -
//   Frame N+4 (mouse button is down)       true          true             true            true -
//   true Frame N+5 (mouse button is down)       -             true             true            -
//   -                    - Frame N+6 (mouse button is released)   -             true             -
//   -                  true                 - Frame N+7 (mouse button is released)   - true - - -
//   -
//------------------------------------------------------------------------------------------------------------------------------------------------
// Note that some combinations are supported,
// - PressedOnDragDropHold can generally be associated with any flag.
// - PressedOnDoubleClick can be associated by PressedOnClickRelease/PressedOnRelease, in which
// case the second release event won't be reported.
//------------------------------------------------------------------------------------------------------------------------------------------------
// The behavior of the return-value changes when ANCHOR_ButtonFlags_Repeat is set:
//                                         Repeat+                  Repeat+           Repeat+
//                                         Repeat+ PressedOnClickRelease    PressedOnClick
//                                         PressedOnRelease    PressedOnDoubleClick
//-------------------------------------------------------------------------------------------------------------------------------------------------
//   Frame N+0 (mouse button is down)       -                        true              - true
//   ...                                    -                        -                 - - Frame N
//   + RepeatDelay                  true                     true              - true
//   ...                                    -                        -                 - - Frame N
//   + RepeatDelay + RepeatRate*N   true                     true              - true
//-------------------------------------------------------------------------------------------------------------------------------------------------

bool ANCHOR::ButtonBehavior(const ImRect &bb,
                            ANCHOR_ID id,
                            bool *out_hovered,
                            bool *out_held,
                            ANCHOR_ButtonFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = GetCurrentWindow();

  if (flags & ANCHOR_ButtonFlags_Disabled) {
    if (out_hovered)
      *out_hovered = false;
    if (out_held)
      *out_held = false;
    if (g.ActiveId == id)
      ClearActiveID();
    return false;
  }

  // Default only reacts to left mouse button
  if ((flags & ANCHOR_ButtonFlags_MouseButtonMask_) == 0)
    flags |= ANCHOR_ButtonFlags_MouseButtonDefault_;

  // Default behavior requires click + release inside bounding box
  if ((flags & ANCHOR_ButtonFlags_PressedOnMask_) == 0)
    flags |= ANCHOR_ButtonFlags_PressedOnDefault_;

  ANCHOR_Window *backup_hovered_window = g.HoveredWindow;
  const bool flatten_hovered_children = (flags & ANCHOR_ButtonFlags_FlattenChildren) && g.HoveredWindow &&
                                        g.HoveredWindow->RootWindow == window;
  if (flatten_hovered_children)
    g.HoveredWindow = window;

#  ifdef ANCHOR_ENABLE_TEST_ENGINE
  if (id != 0 && window->DC.LastItemId != id)
    ANCHOR_TEST_ENGINE_ITEM_ADD(bb, id);
#  endif

  bool pressed = false;
  bool hovered = ItemHoverable(bb, id);

  // Drag source doesn't report as hovered
  if (hovered && g.DragDropActive && g.DragDropPayload.SourceId == id &&
      !(g.DragDropSourceFlags & ANCHORDragDropFlags_SourceNoDisableHover))
    hovered = false;

  // Special mode for Drag and Drop where holding button pressed for a long time while dragging
  // another item triggers the button
  if (g.DragDropActive && (flags & ANCHOR_ButtonFlags_PressedOnDragDropHold) &&
      !(g.DragDropSourceFlags & ANCHORDragDropFlags_SourceNoHoldToOpenOthers))
    if (IsItemHovered(ANCHORHoveredFlags_AllowWhenBlockedByActiveItem)) {
      hovered = true;
      SetHoveredID(id);
      if (CalcTypematicRepeatAmount(g.HoveredIdTimer + 0.0001f - g.IO.DeltaTime,
                                    g.HoveredIdTimer + 0.0001f,
                                    DRAGDROP_HOLD_TO_OPEN_TIMER,
                                    0.00f)) {
        pressed = true;
        g.DragDropHoldJustPressedId = id;
        FocusWindow(window);
      }
    }

  if (flatten_hovered_children)
    g.HoveredWindow = backup_hovered_window;

  // AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match. This
  // allows using patterns where a later submitted widget overlaps a previous one.
  if (hovered && (flags & ANCHOR_ButtonFlags_AllowItemOverlap) &&
      (g.HoveredIdPreviousFrame != id && g.HoveredIdPreviousFrame != 0))
    hovered = false;

  // Mouse handling
  if (hovered) {
    if (!(flags & ANCHOR_ButtonFlags_NoKeyModifiers) || (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt)) {
      // Poll buttons
      int mouse_button_clicked = -1;
      int mouse_button_released = -1;
      if ((flags & ANCHOR_ButtonFlags_MouseButtonLeft) && g.IO.MouseClicked[0]) {
        mouse_button_clicked = 0;
      }
      else if ((flags & ANCHOR_ButtonFlags_MouseButtonRight) && g.IO.MouseClicked[1]) {
        mouse_button_clicked = 1;
      }
      else if ((flags & ANCHOR_ButtonFlags_MouseButtonMiddle) && g.IO.MouseClicked[2]) {
        mouse_button_clicked = 2;
      }
      if ((flags & ANCHOR_ButtonFlags_MouseButtonLeft) && g.IO.MouseReleased[0]) {
        mouse_button_released = 0;
      }
      else if ((flags & ANCHOR_ButtonFlags_MouseButtonRight) && g.IO.MouseReleased[1]) {
        mouse_button_released = 1;
      }
      else if ((flags & ANCHOR_ButtonFlags_MouseButtonMiddle) && g.IO.MouseReleased[2]) {
        mouse_button_released = 2;
      }

      if (mouse_button_clicked != -1 && g.ActiveId != id) {
        if (flags &
            (ANCHOR_ButtonFlags_PressedOnClickRelease | ANCHOR_ButtonFlags_PressedOnClickReleaseAnywhere)) {
          SetActiveID(id, window);
          g.ActiveIdMouseButton = mouse_button_clicked;
          if (!(flags & ANCHOR_ButtonFlags_NoNavFocus))
            SetFocusID(id, window);
          FocusWindow(window);
        }
        if ((flags & ANCHOR_ButtonFlags_PressedOnClick) ||
            ((flags & ANCHOR_ButtonFlags_PressedOnDoubleClick) &&
             g.IO.MouseDoubleClicked[mouse_button_clicked])) {
          pressed = true;
          if (flags & ANCHOR_ButtonFlags_NoHoldingActiveId)
            ClearActiveID();
          else
            SetActiveID(id, window);  // Hold on ID
          g.ActiveIdMouseButton = mouse_button_clicked;
          FocusWindow(window);
        }
      }
      if ((flags & ANCHOR_ButtonFlags_PressedOnRelease) && mouse_button_released != -1) {
        // Repeat mode trumps on release behavior
        const bool has_repeated_at_least_once = (flags & ANCHOR_ButtonFlags_Repeat) &&
                                                g.IO.MouseDownDurationPrev[mouse_button_released] >=
                                                  g.IO.KeyRepeatDelay;
        if (!has_repeated_at_least_once)
          pressed = true;
        ClearActiveID();
      }

      // 'Repeat' mode acts when held regardless of _PressedOn flags (see table above).
      // Relies on repeat logic of IsMouseClicked() but we may as well do it ourselves if we end up
      // exposing finer RepeatDelay/RepeatRate settings.
      if (g.ActiveId == id && (flags & ANCHOR_ButtonFlags_Repeat))
        if (g.IO.MouseDownDuration[g.ActiveIdMouseButton] > 0.0f &&
            IsMouseClicked(g.ActiveIdMouseButton, true))
          pressed = true;
    }

    if (pressed)
      g.NavDisableHighlight = true;
  }

  // Gamepad/Keyboard navigation
  // We report navigated item as hovered but we don't set g.HoveredId to not interfere with mouse.
  if (g.NavId == id && !g.NavDisableHighlight && g.NavDisableMouseHover &&
      (g.ActiveId == 0 || g.ActiveId == id || g.ActiveId == window->MoveId))
    if (!(flags & ANCHOR_ButtonFlags_NoHoveredOnFocus))
      hovered = true;
  if (g.NavActivateDownId == id) {
    bool nav_activated_by_code = (g.NavActivateId == id);
    bool nav_activated_by_inputs = IsNavInputTest(
      ANCHOR_NavInput_Activate,
      (flags & ANCHOR_ButtonFlags_Repeat) ? ANCHOR_InputReadMode_Repeat : ANCHOR_InputReadMode_Pressed);
    if (nav_activated_by_code || nav_activated_by_inputs)
      pressed = true;
    if (nav_activated_by_code || nav_activated_by_inputs || g.ActiveId == id) {
      // Set active id so it can be queried by user via IsItemActive(), equivalent of holding the
      // mouse button.
      g.NavActivateId = id;  // This is so SetActiveId assign a Nav source
      SetActiveID(id, window);
      if ((nav_activated_by_code || nav_activated_by_inputs) && !(flags & ANCHOR_ButtonFlags_NoNavFocus))
        SetFocusID(id, window);
    }
  }

  // Process while held
  bool held = false;
  if (g.ActiveId == id) {
    if (g.ActiveIdSource == ANCHORInputSource_Mouse) {
      if (g.ActiveIdIsJustActivated)
        g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;

      const int mouse_button = g.ActiveIdMouseButton;
      ANCHOR_ASSERT(mouse_button >= 0 && mouse_button < ANCHOR_MouseButton_COUNT);
      if (g.IO.MouseDown[mouse_button]) {
        held = true;
      }
      else {
        bool release_in = hovered && (flags & ANCHOR_ButtonFlags_PressedOnClickRelease) != 0;
        bool release_anywhere = (flags & ANCHOR_ButtonFlags_PressedOnClickReleaseAnywhere) != 0;
        if ((release_in || release_anywhere) && !g.DragDropActive) {
          // Report as pressed when releasing the mouse (this is the most common path)
          bool is_double_click_release = (flags & ANCHOR_ButtonFlags_PressedOnDoubleClick) &&
                                         g.IO.MouseDownWasDoubleClick[mouse_button];
          bool is_repeating_already = (flags & ANCHOR_ButtonFlags_Repeat) &&
                                      g.IO.MouseDownDurationPrev[mouse_button] >=
                                        g.IO.KeyRepeatDelay;  // Repeat mode trumps <on release>
          if (!is_double_click_release && !is_repeating_already)
            pressed = true;
        }
        ClearActiveID();
      }
      if (!(flags & ANCHOR_ButtonFlags_NoNavFocus))
        g.NavDisableHighlight = true;
    }
    else if (g.ActiveIdSource == ANCHORInputSource_Nav) {
      // When activated using Nav, we hold on the ActiveID until activation button is released
      if (g.NavActivateDownId != id)
        ClearActiveID();
    }
    if (pressed)
      g.ActiveIdHasBeenPressedBefore = true;
  }

  if (out_hovered)
    *out_hovered = hovered;
  if (out_held)
    *out_held = held;

  return pressed;
}

bool ANCHOR::ButtonEx(const char *label, const GfVec2f &size_arg, ANCHOR_ButtonFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  const GfVec2f label_size = CalcTextSize(label, NULL, true);

  GfVec2f pos = window->DC.CursorPos;
  if ((flags & ANCHOR_ButtonFlags_AlignTextBaseLine) &&
      style.FramePadding[1] <
        window->DC.CurrLineTextBaseOffset)  // Try to vertically align buttons that are
                                            // smaller/have no padding so that text baseline
                                            // matches (bit hacky, since it shouldn't be a flag)
    pos[1] += window->DC.CurrLineTextBaseOffset - style.FramePadding[1];
  GfVec2f size = CalcItemSize(
    size_arg, label_size[0] + style.FramePadding[0] * 2.0f, label_size[1] + style.FramePadding[1] * 2.0f);

  const ImRect bb(pos, pos + size);
  ItemSize(size, style.FramePadding[1]);
  if (!ItemAdd(bb, id))
    return false;

  if (g.CurrentItemFlags & ANCHOR_ItemFlags_ButtonRepeat)
    flags |= ANCHOR_ButtonFlags_Repeat;
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  // Render
  const AnchorU32 col = GetColorU32((held && hovered) ? ANCHOR_Col_ButtonActive :
                                    hovered           ? ANCHOR_Col_ButtonHovered :
                                                        ANCHOR_Col_Button);
  RenderNavHighlight(bb, id);
  RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

  if (g.LogEnabled)
    LogSetNextTextDecoration("[", "]");
  RenderTextClipped(bb.Min + style.FramePadding,
                    bb.Max - style.FramePadding,
                    label,
                    NULL,
                    &label_size,
                    style.ButtonTextAlign,
                    &bb);

  // Automatically close popups
  // if (pressed && !(flags & ANCHOR_ButtonFlags_DontClosePopups) && (window->Flags &
  // ANCHOR_WindowFlags_Popup))
  //    CloseCurrentPopup();

  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  return pressed;
}

bool ANCHOR::Button(const char *label, const GfVec2f &size_arg)
{
  return ButtonEx(label, size_arg, ANCHOR_ButtonFlags_None);
}

// Small buttons fits within text without additional vertical spacing.
bool ANCHOR::SmallButton(const char *label)
{
  ANCHOR_Context &g = *G_CTX;
  float backup_padding_y = g.Style.FramePadding[1];
  g.Style.FramePadding[1] = 0.0f;
  bool pressed = ButtonEx(label, GfVec2f(0, 0), ANCHOR_ButtonFlags_AlignTextBaseLine);
  g.Style.FramePadding[1] = backup_padding_y;
  return pressed;
}

// Tip: use ANCHOR::PushID()/PopID() to push indices or pointers in the ID stack.
// Then you can keep 'str_id' empty or the same for all your buttons (instead of creating a string
// based on a non-string id)
bool ANCHOR::InvisibleButton(const char *str_id, const GfVec2f &size_arg, ANCHOR_ButtonFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  // Cannot use zero-size for InvisibleButton(). Unlike Button() there is not way to fallback using
  // the label size.
  ANCHOR_ASSERT(size_arg[0] != 0.0f && size_arg[1] != 0.0f);

  const ANCHOR_ID id = window->GetID(str_id);
  GfVec2f size = CalcItemSize(size_arg, 0.0f, 0.0f);
  const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  ItemSize(size);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  return pressed;
}

bool ANCHOR::ArrowButtonEx(const char *str_id, ANCHOR_Dir dir, GfVec2f size, ANCHOR_ButtonFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_ID id = window->GetID(str_id);
  const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  const float default_size = GetFrameHeight();
  ItemSize(size, (size[1] >= default_size) ? g.Style.FramePadding[1] : -1.0f);
  if (!ItemAdd(bb, id))
    return false;

  if (g.CurrentItemFlags & ANCHOR_ItemFlags_ButtonRepeat)
    flags |= ANCHOR_ButtonFlags_Repeat;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

  // Render
  const AnchorU32 bg_col = GetColorU32((held && hovered) ? ANCHOR_Col_ButtonActive :
                                       hovered           ? ANCHOR_Col_ButtonHovered :
                                                           ANCHOR_Col_Button);
  const AnchorU32 text_col = GetColorU32(ANCHOR_Col_Text);
  RenderNavHighlight(bb, id);
  RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
  RenderArrow(window->DrawList,
              bb.Min + GfVec2f(AnchorMax(0.0f, (size[0] - g.FontSize) * 0.5f),
                               AnchorMax(0.0f, (size[1] - g.FontSize) * 0.5f)),
              text_col,
              dir);

  return pressed;
}

bool ANCHOR::ArrowButton(const char *str_id, ANCHOR_Dir dir)
{
  float sz = GetFrameHeight();
  return ArrowButtonEx(str_id, dir, GfVec2f(sz, sz), ANCHOR_ButtonFlags_None);
}

// Button to close a window
bool ANCHOR::CloseButton(ANCHOR_ID id, const GfVec2f &pos)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;

  // Tweak 1: Shrink hit-testing area if button covers an abnormally large proportion of the
  // visible region. That's in order to facilitate moving the window away. (#3825) This may better
  // be applied as a general hit-rect reduction mechanism for all widgets to ensure the area to
  // move window is always accessible?
  const ImRect bb(pos, pos + GfVec2f(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
  ImRect bb_interact = bb;
  const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
  if (area_to_visible_ratio < 1.5f)
    bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));

  // Tweak 2: We intentionally allow interaction when clipped so that a mechanical
  // Alt,Right,Activate sequence can always close a window. (this isn't the regular behavior of
  // buttons, but it doesn't affect the user much because navigation tends to keep items visible).
  bool is_clipped = !ItemAdd(bb_interact, id);

  bool hovered, held;
  bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
  if (is_clipped)
    return pressed;

  // Render
  // FIXME: Clarify this mess
  AnchorU32 col = GetColorU32(held ? ANCHOR_Col_ButtonActive : ANCHOR_Col_ButtonHovered);
  GfVec2f center = bb.GetCenter();
  if (hovered)
    window->DrawList->AddCircleFilled(center, AnchorMax(2.0f, g.FontSize * 0.5f + 1.0f), col, 12);

  float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
  AnchorU32 cross_col = GetColorU32(ANCHOR_Col_Text);
  center -= GfVec2f(0.5f, 0.5f);
  window->DrawList->AddLine(center + GfVec2f(+cross_extent, +cross_extent),
                            center + GfVec2f(-cross_extent, -cross_extent),
                            cross_col,
                            1.0f);
  window->DrawList->AddLine(center + GfVec2f(+cross_extent, -cross_extent),
                            center + GfVec2f(-cross_extent, +cross_extent),
                            cross_col,
                            1.0f);

  return pressed;
}

bool ANCHOR::CollapseButton(ANCHOR_ID id, const GfVec2f &pos)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;

  ImRect bb(pos, pos + GfVec2f(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
  ItemAdd(bb, id);
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, ANCHOR_ButtonFlags_None);

  // Render
  AnchorU32 bg_col = GetColorU32((held && hovered) ? ANCHOR_Col_ButtonActive :
                                 hovered           ? ANCHOR_Col_ButtonHovered :
                                                     ANCHOR_Col_Button);
  AnchorU32 text_col = GetColorU32(ANCHOR_Col_Text);
  GfVec2f center = bb.GetCenter();
  if (hovered || held)
    window->DrawList->AddCircleFilled(
      center /*+ GfVec2f(0.0f, -0.5f)*/, g.FontSize * 0.5f + 1.0f, bg_col, 12);
  RenderArrow(window->DrawList,
              bb.Min + g.Style.FramePadding,
              text_col,
              window->Collapsed ? ANCHOR_Dir_Right : ANCHOR_Dir_Down,
              1.0f);

  // Switch to moving the window after mouse is moved beyond the initial drag threshold
  if (IsItemActive() && IsMouseDragging(0))
    StartMouseMovingWindow(window);

  return pressed;
}

ANCHOR_ID ANCHOR::GetWindowScrollbarID(ANCHOR_Window *window, ANCHOR_Axis axis)
{
  return window->GetIDNoKeepAlive(axis == ANCHOR_Axis_X ? "#SCROLLX" : "#SCROLLY");
}

// Return scrollbar rectangle, must only be called for corresponding axis if window->ScrollbarX/Y
// is set.
ImRect ANCHOR::GetWindowScrollbarRect(ANCHOR_Window *window, ANCHOR_Axis axis)
{
  const ImRect outer_rect = window->Rect();
  const ImRect inner_rect = window->InnerRect;
  const float border_size = window->WindowBorderSize;
  const float scrollbar_size =
    window->ScrollbarSizes[axis ^ 1];  // (ScrollbarSizes[0] = width of Y scrollbar;
                                       // ScrollbarSizes[1] = height of X scrollbar)
  ANCHOR_ASSERT(scrollbar_size > 0.0f);
  if (axis == ANCHOR_Axis_X)
    return ImRect(inner_rect.Min[0],
                  AnchorMax(outer_rect.Min[1], outer_rect.Max[1] - border_size - scrollbar_size),
                  inner_rect.Max[0],
                  outer_rect.Max[1]);
  else
    return ImRect(AnchorMax(outer_rect.Min[0], outer_rect.Max[0] - border_size - scrollbar_size),
                  inner_rect.Min[1],
                  outer_rect.Max[0],
                  inner_rect.Max[1]);
}

void ANCHOR::Scrollbar(ANCHOR_Axis axis)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;

  const ANCHOR_ID id = GetWindowScrollbarID(window, axis);
  KeepAliveID(id);

  // Calculate scrollbar bounding box
  ImRect bb = GetWindowScrollbarRect(window, axis);
  ImDrawFlags rounding_corners = ImDrawFlags_RoundCornersNone;
  if (axis == ANCHOR_Axis_X) {
    rounding_corners |= ImDrawFlags_RoundCornersBottomLeft;
    if (!window->ScrollbarY)
      rounding_corners |= ImDrawFlags_RoundCornersBottomRight;
  }
  else {
    if ((window->Flags & ANCHOR_WindowFlags_NoTitleBar) && !(window->Flags & ANCHOR_WindowFlags_MenuBar))
      rounding_corners |= ImDrawFlags_RoundCornersTopRight;
    if (!window->ScrollbarX)
      rounding_corners |= ImDrawFlags_RoundCornersBottomRight;
  }
  float size_avail = window->InnerRect.Max[axis] - window->InnerRect.Min[axis];
  float size_contents = window->ContentSize[axis] + window->WindowPadding[axis] * 2.0f;
  ScrollbarEx(bb, id, axis, &window->Scroll[axis], size_avail, size_contents, rounding_corners);
}

// Vertical/Horizontal scrollbar
// The entire piece of code below is rather confusing because:
// - We handle absolute seeking (when first clicking outside the grab) and relative manipulation
// (afterward or when clicking inside the grab)
// - We store values as normalized ratio and in a form that allows the window content to change
// while we are holding on a scrollbar
// - We handle both horizontal and vertical scrollbars, which makes the terminology not ideal.
// Still, the code should probably be made simpler..
bool ANCHOR::ScrollbarEx(const ImRect &bb_frame,
                         ANCHOR_ID id,
                         ANCHOR_Axis axis,
                         float *p_scroll_v,
                         float size_avail_v,
                         float size_contents_v,
                         ImDrawFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  const float bb_frame_width = bb_frame.GetWidth();
  const float bb_frame_height = bb_frame.GetHeight();
  if (bb_frame_width <= 0.0f || bb_frame_height <= 0.0f)
    return false;

  // When we are too small, start hiding and disabling the grab (this reduce visual noise on very
  // small window and facilitate using the window resize grab)
  float alpha = 1.0f;
  if ((axis == ANCHOR_Axis_Y) && bb_frame_height < g.FontSize + g.Style.FramePadding[1] * 2.0f)
    alpha = ImSaturate((bb_frame_height - g.FontSize) / (g.Style.FramePadding[1] * 2.0f));
  if (alpha <= 0.0f)
    return false;

  const ANCHOR_Style &style = g.Style;
  const bool allow_interaction = (alpha >= 1.0f);

  ImRect bb = bb_frame;
  bb.Expand(GfVec2f(-ImClamp(IM_FLOOR((bb_frame_width - 2.0f) * 0.5f), 0.0f, 3.0f),
                    -ImClamp(IM_FLOOR((bb_frame_height - 2.0f) * 0.5f), 0.0f, 3.0f)));

  // V denote the main, longer axis of the scrollbar (= height for a vertical scrollbar)
  const float scrollbar_size_v = (axis == ANCHOR_Axis_X) ? bb.GetWidth() : bb.GetHeight();

  // Calculate the height of our grabbable box. It generally represent the amount visible (vs the
  // total scrollable amount) But we maintain a minimum size in pixel to allow for the user to
  // still aim inside.
  ANCHOR_ASSERT(AnchorMax(size_contents_v, size_avail_v) >
                0.0f);  // Adding this assert to check if the AnchorMax(XXX,1.0f) is still needed.
                        // PLEASE CONTACT ME if this triggers.
  const float win_size_v = AnchorMax(AnchorMax(size_contents_v, size_avail_v), 1.0f);
  const float grab_h_pixels = ImClamp(
    scrollbar_size_v * (size_avail_v / win_size_v), style.GrabMinSize, scrollbar_size_v);
  const float grab_h_norm = grab_h_pixels / scrollbar_size_v;

  // Handle input right away. None of the code of Begin() is relying on scrolling position before
  // calling Scrollbar().
  bool held = false;
  bool hovered = false;
  ButtonBehavior(bb, id, &hovered, &held, ANCHOR_ButtonFlags_NoNavFocus);

  float scroll_max = AnchorMax(1.0f, size_contents_v - size_avail_v);
  float scroll_ratio = ImSaturate(*p_scroll_v / scroll_max);
  float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) /
                      scrollbar_size_v;  // Grab position in normalized space
  if (held && allow_interaction && grab_h_norm < 1.0f) {
    float scrollbar_pos_v = bb.Min[axis];
    float mouse_pos_v = g.IO.MousePos[axis];

    // Click position in scrollbar normalized space (0.0f->1.0f)
    const float clicked_v_norm = ImSaturate((mouse_pos_v - scrollbar_pos_v) / scrollbar_size_v);
    SetHoveredID(id);

    bool seek_absolute = false;
    if (g.ActiveIdIsJustActivated) {
      // On initial click calculate the distance between mouse and the center of the grab
      seek_absolute = (clicked_v_norm < grab_v_norm || clicked_v_norm > grab_v_norm + grab_h_norm);
      if (seek_absolute)
        g.ScrollbarClickDeltaToGrabCenter = 0.0f;
      else
        g.ScrollbarClickDeltaToGrabCenter = clicked_v_norm - grab_v_norm - grab_h_norm * 0.5f;
    }

    // Apply scroll (p_scroll_v will generally point on one member of window->Scroll)
    // It is ok to modify Scroll here because we are being called in Begin() after the calculation
    // of ContentSize and before setting up our starting position
    const float scroll_v_norm = ImSaturate(
      (clicked_v_norm - g.ScrollbarClickDeltaToGrabCenter - grab_h_norm * 0.5f) / (1.0f - grab_h_norm));
    *p_scroll_v = IM_ROUND(scroll_v_norm * scroll_max);  //(win_size_contents_v - win_size_v));

    // Update values for rendering
    scroll_ratio = ImSaturate(*p_scroll_v / scroll_max);
    grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;

    // Update distance to grab now that we have seeked and saturated
    if (seek_absolute)
      g.ScrollbarClickDeltaToGrabCenter = clicked_v_norm - grab_v_norm - grab_h_norm * 0.5f;
  }

  // Render
  const AnchorU32 bg_col = GetColorU32(ANCHOR_Col_ScrollbarBg);
  const AnchorU32 grab_col = GetColorU32(held    ? ANCHOR_Col_ScrollbarGrabActive :
                                         hovered ? ANCHOR_Col_ScrollbarGrabHovered :
                                                   ANCHOR_Col_ScrollbarGrab,
                                         alpha);
  window->DrawList->AddRectFilled(bb_frame.Min, bb_frame.Max, bg_col, window->WindowRounding, flags);
  ImRect grab_rect;
  if (axis == ANCHOR_Axis_X)
    grab_rect = ImRect(ImLerp(bb.Min[0], bb.Max[0], grab_v_norm),
                       bb.Min[1],
                       ImLerp(bb.Min[0], bb.Max[0], grab_v_norm) + grab_h_pixels,
                       bb.Max[1]);
  else
    grab_rect = ImRect(bb.Min[0],
                       ImLerp(bb.Min[1], bb.Max[1], grab_v_norm),
                       bb.Max[0],
                       ImLerp(bb.Min[1], bb.Max[1], grab_v_norm) + grab_h_pixels);
  window->DrawList->AddRectFilled(grab_rect.Min, grab_rect.Max, grab_col, style.ScrollbarRounding);

  return held;
}

void ANCHOR::Image(AnchorTextureID user_texture_id,
                   const GfVec2f &size,
                   const GfVec2f &uv0,
                   const GfVec2f &uv1,
                   const GfVec4f &tint_col,
                   const GfVec4f &border_col)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  if (border_col[3] > 0.0f)
    bb.Max += GfVec2f(2, 2);
  ItemSize(bb);
  if (!ItemAdd(bb, 0))
    return;

  if (border_col[3] > 0.0f) {
    window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
    window->DrawList->AddImage(
      user_texture_id, bb.Min + GfVec2f(1, 1), bb.Max - GfVec2f(1, 1), uv0, uv1, GetColorU32(tint_col));
  }
  else {
    window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
  }
}

// ImageButton() is flawed as 'id' is always derived from 'texture_id' (see #2464 #1390)
// We provide this internal helper to write your own variant while we figure out how to redesign
// the public ImageButton() API.
bool ANCHOR::ImageButtonEx(ANCHOR_ID id,
                           AnchorTextureID texture_id,
                           const GfVec2f &size,
                           const GfVec2f &uv0,
                           const GfVec2f &uv1,
                           const GfVec2f &padding,
                           const GfVec4f &bg_col,
                           const GfVec4f &tint_col)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
  ItemSize(bb);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);

  // Render
  const AnchorU32 col = GetColorU32((held && hovered) ? ANCHOR_Col_ButtonActive :
                                    hovered           ? ANCHOR_Col_ButtonHovered :
                                                        ANCHOR_Col_Button);
  RenderNavHighlight(bb, id);
  RenderFrame(
    bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding[0], padding[1]), 0.0f, g.Style.FrameRounding));
  if (bg_col[3] > 0.0f)
    window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
  window->DrawList->AddImage(
    texture_id, bb.Min + padding, bb.Max - padding, uv0, uv1, GetColorU32(tint_col));

  return pressed;
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
bool ANCHOR::ImageButton(AnchorTextureID user_texture_id,
                         const GfVec2f &size,
                         const GfVec2f &uv0,
                         const GfVec2f &uv1,
                         int frame_padding,
                         const GfVec4f &bg_col,
                         const GfVec4f &tint_col)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  // Default to using texture ID as ID. User can still push string/integer prefixes.
  PushID((void *)(intptr_t)user_texture_id);
  const ANCHOR_ID id = window->GetID("#image");
  PopID();

  const GfVec2f padding = (frame_padding >= 0) ? GfVec2f((float)frame_padding, (float)frame_padding) :
                                                 g.Style.FramePadding;
  return ImageButtonEx(id, user_texture_id, size, uv0, uv1, padding, bg_col, tint_col);
}

bool ANCHOR::Checkbox(const char *label, bool *v)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  const GfVec2f label_size = CalcTextSize(label, NULL, true);

  const float square_sz = GetFrameHeight();
  const GfVec2f pos = window->DC.CursorPos;
  const ImRect total_bb(
    pos,
    pos + GfVec2f(square_sz + (label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f),
                  label_size[1] + style.FramePadding[1] * 2.0f));
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, id)) {
    ANCHOR_TEST_ENGINE_ITEM_INFO(id,
                                 label,
                                 window->DC.LastItemStatusFlags | ANCHOR_ItemStatusFlags_Checkable |
                                   (*v ? ANCHOR_ItemStatusFlags_Checked : 0));
    return false;
  }

  bool hovered, held;
  bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
  if (pressed) {
    *v = !(*v);
    MarkItemEdited(id);
  }

  const ImRect check_bb(pos, pos + GfVec2f(square_sz, square_sz));
  RenderNavHighlight(total_bb, id);
  RenderFrame(check_bb.Min,
              check_bb.Max,
              GetColorU32((held && hovered) ? ANCHOR_Col_FrameBgActive :
                          hovered           ? ANCHOR_Col_FrameBgHovered :
                                              ANCHOR_Col_FrameBg),
              true,
              style.FrameRounding);
  AnchorU32 check_col = GetColorU32(ANCHOR_Col_CheckMark);
  bool mixed_value = (g.CurrentItemFlags & ANCHOR_ItemFlags_MixedValue) != 0;
  if (mixed_value) {
    // Undocumented tristate/mixed/indeterminate checkbox (#2644)
    // This may seem awkwardly designed because the aim is to make ANCHOR_ItemFlags_MixedValue
    // supported by all widgets (not just checkbox)
    GfVec2f pad(AnchorMax(1.0f, IM_FLOOR(square_sz / 3.6f)), AnchorMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
    window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
  }
  else if (*v) {
    const float pad = AnchorMax(1.0f, IM_FLOOR(square_sz / 6.0f));
    RenderCheckMark(window->DrawList, check_bb.Min + GfVec2f(pad, pad), check_col, square_sz - pad * 2.0f);
  }

  GfVec2f label_pos = GfVec2f(check_bb.Max[0] + style.ItemInnerSpacing[0],
                              check_bb.Min[1] + style.FramePadding[1]);
  if (g.LogEnabled)
    LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
  if (label_size[0] > 0.0f)
    RenderText(label_pos, label);

  ANCHOR_TEST_ENGINE_ITEM_INFO(id,
                               label,
                               window->DC.LastItemStatusFlags | ANCHOR_ItemStatusFlags_Checkable |
                                 (*v ? ANCHOR_ItemStatusFlags_Checked : 0));
  return pressed;
}

template<typename T> bool ANCHOR::CheckboxFlagsT(const char *label, T *flags, T flags_value)
{
  bool all_on = (*flags & flags_value) == flags_value;
  bool any_on = (*flags & flags_value) != 0;
  bool pressed;
  if (!all_on && any_on) {
    ANCHOR_Context &g = *G_CTX;
    ANCHOR_ItemFlags backup_item_flags = g.CurrentItemFlags;
    g.CurrentItemFlags |= ANCHOR_ItemFlags_MixedValue;
    pressed = Checkbox(label, &all_on);
    g.CurrentItemFlags = backup_item_flags;
  }
  else {
    pressed = Checkbox(label, &all_on);
  }
  if (pressed) {
    if (all_on)
      *flags |= flags_value;
    else
      *flags &= ~flags_value;
  }
  return pressed;
}

bool ANCHOR::CheckboxFlags(const char *label, int *flags, int flags_value)
{
  return CheckboxFlagsT(label, flags, flags_value);
}

bool ANCHOR::CheckboxFlags(const char *label, unsigned int *flags, unsigned int flags_value)
{
  return CheckboxFlagsT(label, flags, flags_value);
}

bool ANCHOR::CheckboxFlags(const char *label, AnchorS64 *flags, AnchorS64 flags_value)
{
  return CheckboxFlagsT(label, flags, flags_value);
}

bool ANCHOR::CheckboxFlags(const char *label, AnchorU64 *flags, AnchorU64 flags_value)
{
  return CheckboxFlagsT(label, flags, flags_value);
}

bool ANCHOR::RadioButton(const char *label, bool active)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  const GfVec2f label_size = CalcTextSize(label, NULL, true);

  const float square_sz = GetFrameHeight();
  const GfVec2f pos = window->DC.CursorPos;
  const ImRect check_bb(pos, pos + GfVec2f(square_sz, square_sz));
  const ImRect total_bb(
    pos,
    pos + GfVec2f(square_sz + (label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f),
                  label_size[1] + style.FramePadding[1] * 2.0f));
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, id))
    return false;

  GfVec2f center = check_bb.GetCenter();
  center[0] = IM_ROUND(center[0]);
  center[1] = IM_ROUND(center[1]);
  const float radius = (square_sz - 1.0f) * 0.5f;

  bool hovered, held;
  bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
  if (pressed)
    MarkItemEdited(id);

  RenderNavHighlight(total_bb, id);
  window->DrawList->AddCircleFilled(center,
                                    radius,
                                    GetColorU32((held && hovered) ? ANCHOR_Col_FrameBgActive :
                                                hovered           ? ANCHOR_Col_FrameBgHovered :
                                                                    ANCHOR_Col_FrameBg),
                                    16);
  if (active) {
    const float pad = AnchorMax(1.0f, IM_FLOOR(square_sz / 6.0f));
    window->DrawList->AddCircleFilled(center, radius - pad, GetColorU32(ANCHOR_Col_CheckMark), 16);
  }

  if (style.FrameBorderSize > 0.0f) {
    window->DrawList->AddCircle(
      center + GfVec2f(1, 1), radius, GetColorU32(ANCHOR_Col_BorderShadow), 16, style.FrameBorderSize);
    window->DrawList->AddCircle(center, radius, GetColorU32(ANCHOR_Col_Border), 16, style.FrameBorderSize);
  }

  GfVec2f label_pos = GfVec2f(check_bb.Max[0] + style.ItemInnerSpacing[0],
                              check_bb.Min[1] + style.FramePadding[1]);
  if (g.LogEnabled)
    LogRenderedText(&label_pos, active ? "(x)" : "( )");
  if (label_size[0] > 0.0f)
    RenderText(label_pos, label);

  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  return pressed;
}

// FIXME: This would work nicely if it was a public template, e.g. 'template<T> RadioButton(const
// char* label, T* v, T v_button)', but I'm not sure how we would expose it..
bool ANCHOR::RadioButton(const char *label, int *v, int v_button)
{
  const bool pressed = RadioButton(label, *v == v_button);
  if (pressed)
    *v = v_button;
  return pressed;
}

// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
void ANCHOR::ProgressBar(float fraction, const GfVec2f &size_arg, const char *overlay)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;

  GfVec2f pos = window->DC.CursorPos;
  GfVec2f size = CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding[1] * 2.0f);
  ImRect bb(pos, pos + size);
  ItemSize(size, style.FramePadding[1]);
  if (!ItemAdd(bb, 0))
    return;

  // Render
  fraction = ImSaturate(fraction);
  RenderFrame(bb.Min, bb.Max, GetColorU32(ANCHOR_Col_FrameBg), true, style.FrameRounding);
  bb.Expand(GfVec2f(-style.FrameBorderSize, -style.FrameBorderSize));
  const GfVec2f fill_br = GfVec2f(ImLerp(bb.Min[0], bb.Max[0], fraction), bb.Max[1]);
  RenderRectFilledRangeH(
    window->DrawList, bb, GetColorU32(ANCHOR_Col_PlotHistogram), 0.0f, fraction, style.FrameRounding);

  // Default displaying the fraction as percentage string, but user can override it
  char overlay_buf[32];
  if (!overlay) {
    ImFormatString(overlay_buf, ANCHOR_ARRAYSIZE(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
    overlay = overlay_buf;
  }

  GfVec2f overlay_size = CalcTextSize(overlay, NULL);
  if (overlay_size[0] > 0.0f)
    RenderTextClipped(GfVec2f(ImClamp(fill_br[0] + style.ItemSpacing[0],
                                      bb.Min[0],
                                      bb.Max[0] - overlay_size[0] - style.ItemInnerSpacing[0]),
                              bb.Min[1]),
                      bb.Max,
                      overlay,
                      NULL,
                      &overlay_size,
                      GfVec2f(0.0f, 0.5f),
                      &bb);
}

void ANCHOR::Bullet()
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const float line_height = AnchorMax(
    ImMin(window->DC.CurrLineSize[1], g.FontSize + g.Style.FramePadding[1] * 2), g.FontSize);
  const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + GfVec2f(g.FontSize, line_height));
  ItemSize(bb);
  if (!ItemAdd(bb, 0)) {
    SameLine(0, style.FramePadding[0] * 2);
    return;
  }

  // Render and stay on same line
  AnchorU32 text_col = GetColorU32(ANCHOR_Col_Text);
  RenderBullet(window->DrawList,
               bb.Min + GfVec2f(style.FramePadding[0] + g.FontSize * 0.5f, line_height * 0.5f),
               text_col);
  SameLine(0, style.FramePadding[0] * 2.0f);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Low-level Layout helpers
//-------------------------------------------------------------------------
// - Spacing()
// - Dummy()
// - NewLine()
// - AlignTextToFramePadding()
// - SeparatorEx() [Internal]
// - Separator()
// - SplitterBehavior() [Internal]
// - ShrinkWidths() [Internal]
//-------------------------------------------------------------------------

void ANCHOR::Spacing()
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;
  ItemSize(GfVec2f(0, 0));
}

void ANCHOR::Dummy(const GfVec2f &size)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  ItemSize(size);
  ItemAdd(bb, 0);
}

void ANCHOR::NewLine()
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_LayoutType backup_layout_type = window->DC.LayoutType;
  window->DC.LayoutType = ANCHOR_LayoutType_Vertical;
  if (window->DC.CurrLineSize[1] > 0.0f)  // In the event that we are on a line with items that is smaller
                                          // that FontSize high, we will preserve its height.
    ItemSize(GfVec2f(0, 0));
  else
    ItemSize(GfVec2f(0.0f, g.FontSize));
  window->DC.LayoutType = backup_layout_type;
}

void ANCHOR::AlignTextToFramePadding()
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  window->DC.CurrLineSize[1] = AnchorMax(window->DC.CurrLineSize[1],
                                         g.FontSize + g.Style.FramePadding[1] * 2);
  window->DC.CurrLineTextBaseOffset = AnchorMax(window->DC.CurrLineTextBaseOffset, g.Style.FramePadding[1]);
}

// Horizontal/vertical separating line
void ANCHOR::SeparatorEx(ANCHOR_SeparatorFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  ANCHOR_Context &g = *G_CTX;
  ANCHOR_ASSERT(
    ImIsPowerOfTwo(flags & (ANCHOR_SeparatorFlags_Horizontal |
                            ANCHOR_SeparatorFlags_Vertical)));  // Check that only 1 option is selected

  float thickness_draw = 1.0f;
  float thickness_layout = 0.0f;
  if (flags & ANCHOR_SeparatorFlags_Vertical) {
    // Vertical separator, for menu bars (use current line height). Not exposed because it is
    // misleading and it doesn't have an effect on regular layout.
    float y1 = window->DC.CursorPos[1];
    float y2 = window->DC.CursorPos[1] + window->DC.CurrLineSize[1];
    const ImRect bb(GfVec2f(window->DC.CursorPos[0], y1),
                    GfVec2f(window->DC.CursorPos[0] + thickness_draw, y2));
    ItemSize(GfVec2f(thickness_layout, 0.0f));
    if (!ItemAdd(bb, 0))
      return;

    // Draw
    window->DrawList->AddLine(
      GfVec2f(bb.Min[0], bb.Min[1]), GfVec2f(bb.Min[0], bb.Max[1]), GetColorU32(ANCHOR_Col_Separator));
    if (g.LogEnabled)
      LogText(" |");
  }
  else if (flags & ANCHOR_SeparatorFlags_Horizontal) {
    // Horizontal Separator
    float x1 = window->Pos[0];
    float x2 = window->Pos[0] + window->Size[0];

    // FIXME-WORKRECT: old hack (#205) until we decide of consistent behavior with WorkRect/Indent
    // and Separator
    if (g.GroupStack.Size > 0 && g.GroupStack.back().WindowID == window->ID)
      x1 += window->DC.Indent.x;

    ANCHOR_OldColumns *columns = (flags & ANCHOR_SeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns :
                                                                                  NULL;
    if (columns)
      PushColumnsBackground();

    // We don't provide our width to the layout so that it doesn't get feed back into AutoFit
    const ImRect bb(GfVec2f(x1, window->DC.CursorPos[1]),
                    GfVec2f(x2, window->DC.CursorPos[1] + thickness_draw));
    ItemSize(GfVec2f(0.0f, thickness_layout));
    const bool item_visible = ItemAdd(bb, 0);
    if (item_visible) {
      // Draw
      window->DrawList->AddLine(bb.Min, GfVec2f(bb.Max[0], bb.Min[1]), GetColorU32(ANCHOR_Col_Separator));
      if (g.LogEnabled)
        LogRenderedText(&bb.Min, "--------------------------------\n");
    }
    if (columns) {
      PopColumnsBackground();
      columns->LineMinY = window->DC.CursorPos[1];
    }
  }
}

void ANCHOR::Separator()
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  // Those flags should eventually be overridable by the user
  ANCHOR_SeparatorFlags flags = (window->DC.LayoutType == ANCHOR_LayoutType_Horizontal) ?
                                  ANCHOR_SeparatorFlags_Vertical :
                                  ANCHOR_SeparatorFlags_Horizontal;
  flags |= ANCHOR_SeparatorFlags_SpanAllColumns;
  SeparatorEx(flags);
}

// Using 'hover_visibility_delay' allows us to hide the highlight and mouse cursor for a short
// time, which can be convenient to reduce visual noise.
bool ANCHOR::SplitterBehavior(const ImRect &bb,
                              ANCHOR_ID id,
                              ANCHOR_Axis axis,
                              float *size1,
                              float *size2,
                              float min_size1,
                              float min_size2,
                              float hover_extend,
                              float hover_visibility_delay)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;

  const ANCHOR_ItemFlags item_flags_backup = g.CurrentItemFlags;
  g.CurrentItemFlags |= ANCHOR_ItemFlags_NoNav | ANCHOR_ItemFlags_NoNavDefaultFocus;
  bool item_add = ItemAdd(bb, id);
  g.CurrentItemFlags = item_flags_backup;
  if (!item_add)
    return false;

  bool hovered, held;
  ImRect bb_interact = bb;
  bb_interact.Expand(axis == ANCHOR_Axis_Y ? GfVec2f(0.0f, hover_extend) : GfVec2f(hover_extend, 0.0f));
  ButtonBehavior(bb_interact,
                 id,
                 &hovered,
                 &held,
                 ANCHOR_ButtonFlags_FlattenChildren | ANCHOR_ButtonFlags_AllowItemOverlap);
  if (g.ActiveId != id)
    SetItemAllowOverlap();

  if (held ||
      (g.HoveredId == id && g.HoveredIdPreviousFrame == id && g.HoveredIdTimer >= hover_visibility_delay))
    SetMouseCursor(axis == ANCHOR_Axis_Y ? ANCHOR_MouseCursor_ResizeNS : ANCHOR_MouseCursor_ResizeEW);

  ImRect bb_render = bb;
  if (held) {
    GfVec2f mouse_delta_2d = g.IO.MousePos - g.ActiveIdClickOffset - bb_interact.Min;
    float mouse_delta = (axis == ANCHOR_Axis_Y) ? mouse_delta_2d[1] : mouse_delta_2d[0];

    // Minimum pane size
    float size_1_maximum_delta = AnchorMax(0.0f, *size1 - min_size1);
    float size_2_maximum_delta = AnchorMax(0.0f, *size2 - min_size2);
    if (mouse_delta < -size_1_maximum_delta)
      mouse_delta = -size_1_maximum_delta;
    if (mouse_delta > size_2_maximum_delta)
      mouse_delta = size_2_maximum_delta;

    // Apply resize
    if (mouse_delta != 0.0f) {
      if (mouse_delta < 0.0f)
        ANCHOR_ASSERT(*size1 + mouse_delta >= min_size1);
      if (mouse_delta > 0.0f)
        ANCHOR_ASSERT(*size2 - mouse_delta >= min_size2);
      *size1 += mouse_delta;
      *size2 -= mouse_delta;
      bb_render.Translate((axis == ANCHOR_Axis_X) ? GfVec2f(mouse_delta, 0.0f) : GfVec2f(0.0f, mouse_delta));
      MarkItemEdited(id);
    }
  }

  // Render
  const AnchorU32 col = GetColorU32(held ? ANCHOR_Col_SeparatorActive :
                                    (hovered && g.HoveredIdTimer >= hover_visibility_delay) ?
                                           ANCHOR_Col_SeparatorHovered :
                                           ANCHOR_Col_Separator);
  window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, col, 0.0f);

  return held;
}

static int ANCHOR_CDECL ShrinkWidthItemComparer(const void *lhs, const void *rhs)
{
  const ANCHOR_ShrinkWidthItem *a = (const ANCHOR_ShrinkWidthItem *)lhs;
  const ANCHOR_ShrinkWidthItem *b = (const ANCHOR_ShrinkWidthItem *)rhs;
  if (int d = (int)(b->Width - a->Width))
    return d;
  return (b->Index - a->Index);
}

// Shrink excess width from a set of item, by removing width from the larger items first.
// Set items Width to -1.0f to disable shrinking this item.
void ANCHOR::ShrinkWidths(ANCHOR_ShrinkWidthItem *items, int count, float width_excess)
{
  if (count == 1) {
    if (items[0].Width >= 0.0f)
      items[0].Width = AnchorMax(items[0].Width - width_excess, 1.0f);
    return;
  }
  ImQsort(items, (size_t)count, sizeof(ANCHOR_ShrinkWidthItem), ShrinkWidthItemComparer);
  int count_same_width = 1;
  while (width_excess > 0.0f && count_same_width < count) {
    while (count_same_width < count && items[0].Width <= items[count_same_width].Width)
      count_same_width++;
    float max_width_to_remove_per_item = (count_same_width < count &&
                                          items[count_same_width].Width >= 0.0f) ?
                                           (items[0].Width - items[count_same_width].Width) :
                                           (items[0].Width - 1.0f);
    if (max_width_to_remove_per_item <= 0.0f)
      break;
    float width_to_remove_per_item = ImMin(width_excess / count_same_width, max_width_to_remove_per_item);
    for (int item_n = 0; item_n < count_same_width; item_n++)
      items[item_n].Width -= width_to_remove_per_item;
    width_excess -= width_to_remove_per_item * count_same_width;
  }

  // Round width and redistribute remainder left-to-right (could make it an option of the
  // function?) Ensure that e.g. the right-most tab of a shrunk tab-bar always reaches exactly at
  // the same distance from the right-most edge of the tab bar separator.
  width_excess = 0.0f;
  for (int n = 0; n < count; n++) {
    float width_rounded = ImFloor(items[n].Width);
    width_excess += items[n].Width - width_rounded;
    items[n].Width = width_rounded;
  }
  if (width_excess > 0.0f)
    for (int n = 0; n < count; n++)
      if (items[n].Index < (int)(width_excess + 0.01f))
        items[n].Width += 1.0f;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ComboBox
//-------------------------------------------------------------------------
// - CalcMaxPopupHeightFromItemCount() [Internal]
// - BeginCombo()
// - BeginComboPopup() [Internal]
// - EndCombo()
// - Combo()
//-------------------------------------------------------------------------

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
  ANCHOR_Context &g = *G_CTX;
  if (items_count <= 0)
    return FLT_MAX;
  return (g.FontSize + g.Style.ItemSpacing[1]) * items_count - g.Style.ItemSpacing[1] +
         (g.Style.WindowPadding[1] * 2);
}

bool ANCHOR::BeginCombo(const char *label, const char *preview_value, ANCHORComboFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = GetCurrentWindow();

  ANCHOR_NextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
  g.NextWindowData.ClearFlags();  // We behave like Begin() and need to consume those values
  if (window->SkipItems)
    return false;

  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  ANCHOR_ASSERT(
    (flags & (ANCHORComboFlags_NoArrowButton | ANCHORComboFlags_NoPreview)) !=
    (ANCHORComboFlags_NoArrowButton | ANCHORComboFlags_NoPreview));  // Can't use both flags together

  const float arrow_size = (flags & ANCHORComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
  const GfVec2f label_size = CalcTextSize(label, NULL, true);
  const float w = (flags & ANCHORComboFlags_NoPreview) ? arrow_size : CalcItemWidth();
  const ImRect bb(window->DC.CursorPos,
                  window->DC.CursorPos + GfVec2f(w, label_size[1] + style.FramePadding[1] * 2.0f));
  const ImRect total_bb(
    bb.Min, bb.Max + GfVec2f(label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f, 0.0f));
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, id, &bb))
    return false;

  // Open on click
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);
  const ANCHOR_ID popup_id = ImHashStr("##ComboPopup", 0, id);
  bool popup_open = IsPopupOpen(popup_id, ANCHORPopupFlags_None);
  if ((pressed || g.NavActivateId == id) && !popup_open) {
    OpenPopupEx(popup_id, ANCHORPopupFlags_None);
    popup_open = true;
  }

  // Render shape
  const AnchorU32 frame_col = GetColorU32(hovered ? ANCHOR_Col_FrameBgHovered : ANCHOR_Col_FrameBg);
  const float value_x2 = AnchorMax(bb.Min[0], bb.Max[0] - arrow_size);
  RenderNavHighlight(bb, id);
  if (!(flags & ANCHORComboFlags_NoPreview))
    window->DrawList->AddRectFilled(bb.Min,
                                    GfVec2f(value_x2, bb.Max[1]),
                                    frame_col,
                                    style.FrameRounding,
                                    (flags & ANCHORComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll :
                                                                               ImDrawFlags_RoundCornersLeft);
  if (!(flags & ANCHORComboFlags_NoArrowButton)) {
    AnchorU32 bg_col = GetColorU32((popup_open || hovered) ? ANCHOR_Col_ButtonHovered : ANCHOR_Col_Button);
    AnchorU32 text_col = GetColorU32(ANCHOR_Col_Text);
    window->DrawList->AddRectFilled(GfVec2f(value_x2, bb.Min[1]),
                                    bb.Max,
                                    bg_col,
                                    style.FrameRounding,
                                    (w <= arrow_size) ? ImDrawFlags_RoundCornersAll :
                                                        ImDrawFlags_RoundCornersRight);
    if (value_x2 + arrow_size - style.FramePadding[0] <= bb.Max[0])
      RenderArrow(window->DrawList,
                  GfVec2f(value_x2 + style.FramePadding[1], bb.Min[1] + style.FramePadding[1]),
                  text_col,
                  ANCHOR_Dir_Down,
                  1.0f);
  }
  RenderFrameBorder(bb.Min, bb.Max, style.FrameRounding);

  // Render preview and label
  if (preview_value != NULL && !(flags & ANCHORComboFlags_NoPreview)) {
    if (g.LogEnabled)
      LogSetNextTextDecoration("{", "}");
    RenderTextClipped(bb.Min + style.FramePadding, GfVec2f(value_x2, bb.Max[1]), preview_value, NULL, NULL);
  }
  if (label_size[0] > 0)
    RenderText(GfVec2f(bb.Max[0] + style.ItemInnerSpacing[0], bb.Min[1] + style.FramePadding[1]), label);

  if (!popup_open)
    return false;

  g.NextWindowData.Flags = backup_next_window_data_flags;
  return BeginComboPopup(popup_id, bb, flags);
}

bool ANCHOR::BeginComboPopup(ANCHOR_ID popup_id, const ImRect &bb, ANCHORComboFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  if (!IsPopupOpen(popup_id, ANCHORPopupFlags_None)) {
    g.NextWindowData.ClearFlags();
    return false;
  }

  // Set popup size
  float w = bb.GetWidth();
  if (g.NextWindowData.Flags & ANCHOR_NextWindowDataFlags_HasSizeConstraint) {
    g.NextWindowData.SizeConstraintRect.Min[0] = AnchorMax(g.NextWindowData.SizeConstraintRect.Min[0], w);
  }
  else {
    if ((flags & ANCHORComboFlags_HeightMask_) == 0)
      flags |= ANCHORComboFlags_HeightRegular;
    ANCHOR_ASSERT(ImIsPowerOfTwo(flags & ANCHORComboFlags_HeightMask_));  // Only one
    int popup_max_height_in_items = -1;
    if (flags & ANCHORComboFlags_HeightRegular)
      popup_max_height_in_items = 8;
    else if (flags & ANCHORComboFlags_HeightSmall)
      popup_max_height_in_items = 4;
    else if (flags & ANCHORComboFlags_HeightLarge)
      popup_max_height_in_items = 20;
    SetNextWindowSizeConstraints(
      GfVec2f(w, 0.0f), GfVec2f(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
  }

  // This is essentially a specialized version of BeginPopupEx()
  char name[16];
  ImFormatString(name,
                 ANCHOR_ARRAYSIZE(name),
                 "##Combo_%02d",
                 g.BeginPopupStack.Size);  // Recycle windows based on depth

  // Set position given a custom constraint (peak into expected window size so we can position it)
  // FIXME: This might be easier to express with an hypothetical SetNextWindowPosConstraints()
  // function?
  // FIXME: This might be moved to Begin() or at least around the same spot where Tooltips and
  // other Popups are calling FindBestWindowPosForPopupEx()?
  if (ANCHOR_Window *popup_window = FindWindowByName(name))
    if (popup_window->WasActive) {
      // Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect
      // us.
      GfVec2f size_expected = CalcWindowNextAutoFitSize(popup_window);
      popup_window->AutoPosLastDirection = (flags & ANCHORComboFlags_PopupAlignLeft) ?
                                             ANCHOR_Dir_Left :
                                             ANCHOR_Dir_Down;  // Left = "Below, Toward Left", Down
                                                               // = "Below, Toward Right (default)"
      ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
      GfVec2f pos = FindBestWindowPosForPopupEx(bb.GetBL(),
                                                size_expected,
                                                &popup_window->AutoPosLastDirection,
                                                r_outer,
                                                bb,
                                                ANCHORPopupPositionPolicy_ComboBox);
      SetNextWindowPos(pos);
    }

  // We don't use BeginPopupEx() solely because we have a custom name string, which we could make
  // an argument to BeginPopupEx()
  ANCHOR_WindowFlags window_flags = ANCHOR_WindowFlags_AlwaysAutoResize | ANCHOR_WindowFlags_Popup |
                                    ANCHOR_WindowFlags_NoTitleBar | ANCHOR_WindowFlags_NoResize |
                                    ANCHOR_WindowFlags_NoSavedSettings | ANCHOR_WindowFlags_NoMove;
  PushStyleVar(ANCHOR_StyleVar_WindowPadding,
               GfVec2f(g.Style.FramePadding[0],
                       g.Style.WindowPadding[1]));  // Horizontally align ourselves with the framed text
  bool ret = Begin(name, NULL, window_flags);
  PopStyleVar();
  if (!ret) {
    EndPopup();
    ANCHOR_ASSERT(0);  // This should never happen as we tested for IsPopupOpen() above
    return false;
  }
  return true;
}

void ANCHOR::EndCombo()
{
  EndPopup();
}

// Getter for the old Combo() API: const char*[]
static bool Items_ArrayGetter(void *data, int idx, const char **out_text)
{
  const char *const *items = (const char *const *)data;
  if (out_text)
    *out_text = items[idx];
  return true;
}

// Getter for the old Combo() API: "item1\0item2\0item3\0"
static bool Items_SingleStringGetter(void *data, int idx, const char **out_text)
{
  // FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the
  // waste is limited.
  const char *items_separated_by_zeros = (const char *)data;
  int items_count = 0;
  const char *p = items_separated_by_zeros;
  while (*p) {
    if (idx == items_count)
      break;
    p += strlen(p) + 1;
    items_count++;
  }
  if (!*p)
    return false;
  if (out_text)
    *out_text = p;
  return true;
}

// Old API, prefer using BeginCombo() nowadays if you can.
bool ANCHOR::Combo(const char *label,
                   int *current_item,
                   bool (*items_getter)(void *, int, const char **),
                   void *data,
                   int items_count,
                   int popup_max_height_in_items)
{
  ANCHOR_Context &g = *G_CTX;

  // Call the getter to obtain the preview string which is a parameter to BeginCombo()
  const char *preview_value = NULL;
  if (*current_item >= 0 && *current_item < items_count)
    items_getter(data, *current_item, &preview_value);

  // The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API
  // doesn't have/need it, but we emulate it here.
  if (popup_max_height_in_items != -1 &&
      !(g.NextWindowData.Flags & ANCHOR_NextWindowDataFlags_HasSizeConstraint))
    SetNextWindowSizeConstraints(
      GfVec2f(0, 0), GfVec2f(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

  if (!BeginCombo(label, preview_value, ANCHORComboFlags_None))
    return false;

  // Display items
  // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call
  // to SetItemDefaultFocus() is processed)
  bool value_changed = false;
  for (int i = 0; i < items_count; i++) {
    PushID((void *)(intptr_t)i);
    const bool item_selected = (i == *current_item);
    const char *item_text;
    if (!items_getter(data, i, &item_text))
      item_text = "*Unknown item*";
    if (Selectable(item_text, item_selected)) {
      value_changed = true;
      *current_item = i;
    }
    if (item_selected)
      SetItemDefaultFocus();
    PopID();
  }

  EndCombo();
  if (value_changed)
    MarkItemEdited(g.CurrentWindow->DC.LastItemId);

  return value_changed;
}

// Combo box helper allowing to pass an array of strings.
bool ANCHOR::Combo(const char *label,
                   int *current_item,
                   const char *const items[],
                   int items_count,
                   int height_in_items)
{
  const bool value_changed = Combo(
    label, current_item, Items_ArrayGetter, (void *)items, items_count, height_in_items);
  return value_changed;
}

// Combo box helper allowing to pass all items in a single string literal holding multiple
// zero-terminated items "item1\0item2\0"
bool ANCHOR::Combo(const char *label,
                   int *current_item,
                   const char *items_separated_by_zeros,
                   int height_in_items)
{
  int items_count = 0;
  const char *p = items_separated_by_zeros;  // FIXME-OPT: Avoid computing this, or at least only
                                             // when combo is open
  while (*p) {
    p += strlen(p) + 1;
    items_count++;
  }
  bool value_changed = Combo(label,
                             current_item,
                             Items_SingleStringGetter,
                             (void *)items_separated_by_zeros,
                             items_count,
                             height_in_items);
  return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Data Type and Data Formatting Helpers [Internal]
//-------------------------------------------------------------------------
// - PatchFormatStringFloatToInt()
// - DataTypeGetInfo()
// - DataTypeFormatString()
// - DataTypeApplyOp()
// - DataTypeApplyOpFromText()
// - DataTypeClamp()
// - GetMinimumStepAtDecimalPrecision
// - RoundScalarWithFormat<>()
//-------------------------------------------------------------------------

static const ANCHOR_DataTypeInfo GDataTypeInfo[] = {
  {sizeof(char), "S8", "%d", "%d"},  // ANCHOR_DataType_S8
  {sizeof(unsigned char), "U8", "%u", "%u"},
  {sizeof(short), "S16", "%d", "%d"},  // ANCHOR_DataType_S16
  {sizeof(unsigned short), "U16", "%u", "%u"},
  {sizeof(int), "S32", "%d", "%d"},  // ANCHOR_DataType_S32
  {sizeof(unsigned int), "U32", "%u", "%u"},
#  ifdef _MSC_VER
  {sizeof(AnchorS64), "S64", "%I64d", "%I64d"},  // ANCHOR_DataType_S64
  {sizeof(AnchorU64), "U64", "%I64u", "%I64u"},
#  else
  {sizeof(AnchorS64), "S64", "%lld", "%lld"},  // ANCHOR_DataType_S64
  {sizeof(AnchorU64), "U64", "%llu", "%llu"},
#  endif
  {sizeof(float), "float", "%.3f", "%f"},   // ANCHOR_DataType_Float (float are promoted to double in va_arg)
  {sizeof(double), "double", "%f", "%lf"},  // ANCHOR_DataType_Double
};
IM_STATIC_ASSERT(ANCHOR_ARRAYSIZE(GDataTypeInfo) == ANCHOR_DataType_COUNT);

// FIXME-LEGACY: Prior to 1.61 our DragInt() function internally used floats and because of this
// the compile-time default value for format was "%.0f". Even though we changed the compile-time
// default, we expect users to have carried %f around, which would break the display of DragInt()
// calls. To honor backward compatibility we are rewriting the format string, unless
// ANCHOR_DISABLE_OBSOLETE_FUNCTIONS is enabled. What could possibly go wrong?!
static const char *PatchFormatStringFloatToInt(const char *fmt)
{
  if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' &&
      fmt[4] == 0)  // Fast legacy path for "%.0f" which is expected to be the most common case.
    return "%d";
  const char *fmt_start = ImParseFormatFindStart(fmt);  // Find % (if any, and ignore %%)
  const char *fmt_end = ImParseFormatFindEnd(
    fmt_start);  // Find end of format specifier, which itself is an exercise of
                 // confidence/recklessness (because snprintf is dependent on libc or user).
  if (fmt_end > fmt_start && fmt_end[-1] == 'f') {
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
    if (fmt_start == fmt && fmt_end[0] == 0)
      return "%d";
    ANCHOR_Context &g = *G_CTX;
    ImFormatString(g.TempBuffer,
                   ANCHOR_ARRAYSIZE(g.TempBuffer),
                   "%.*s%%d%s",
                   (int)(fmt_start - fmt),
                   fmt,
                   fmt_end);  // Honor leading and trailing decorations, but lose alignment/precision.
    return g.TempBuffer;
#  else
    ANCHOR_ASSERT(0 && "DragInt(): Invalid format string!");  // Old versions used a default parameter of
                                                              // "%.0f", please replace with e.g. "%d"
#  endif
  }
  return fmt;
}

const ANCHOR_DataTypeInfo *ANCHOR::DataTypeGetInfo(ANCHOR_DataType data_type)
{
  ANCHOR_ASSERT(data_type >= 0 && data_type < ANCHOR_DataType_COUNT);
  return &GDataTypeInfo[data_type];
}

int ANCHOR::DataTypeFormatString(char *buf,
                                 int buf_size,
                                 ANCHOR_DataType data_type,
                                 const void *p_data,
                                 const char *format)
{
  // Signedness doesn't matter when pushing integer arguments
  if (data_type == ANCHOR_DataType_S32 || data_type == ANCHOR_DataType_U32)
    return ImFormatString(buf, buf_size, format, *(const AnchorU32 *)p_data);
  if (data_type == ANCHOR_DataType_S64 || data_type == ANCHOR_DataType_U64)
    return ImFormatString(buf, buf_size, format, *(const AnchorU64 *)p_data);
  if (data_type == ANCHOR_DataType_Float)
    return ImFormatString(buf, buf_size, format, *(const float *)p_data);
  if (data_type == ANCHOR_DataType_Double)
    return ImFormatString(buf, buf_size, format, *(const double *)p_data);
  if (data_type == ANCHOR_DataType_S8)
    return ImFormatString(buf, buf_size, format, *(const AnchorS8 *)p_data);
  if (data_type == ANCHOR_DataType_U8)
    return ImFormatString(buf, buf_size, format, *(const AnchorU8 *)p_data);
  if (data_type == ANCHOR_DataType_S16)
    return ImFormatString(buf, buf_size, format, *(const AnchorS16 *)p_data);
  if (data_type == ANCHOR_DataType_U16)
    return ImFormatString(buf, buf_size, format, *(const AnchorU16 *)p_data);
  ANCHOR_ASSERT(0);
  return 0;
}

void ANCHOR::DataTypeApplyOp(ANCHOR_DataType data_type,
                             int op,
                             void *output,
                             const void *arg1,
                             const void *arg2)
{
  ANCHOR_ASSERT(op == '+' || op == '-');
  switch (data_type) {
    case ANCHOR_DataType_S8:
      if (op == '+') {
        *(AnchorS8 *)output = ImAddClampOverflow(
          *(const AnchorS8 *)arg1, *(const AnchorS8 *)arg2, IM_S8_MIN, IM_S8_MAX);
      }
      if (op == '-') {
        *(AnchorS8 *)output = ImSubClampOverflow(
          *(const AnchorS8 *)arg1, *(const AnchorS8 *)arg2, IM_S8_MIN, IM_S8_MAX);
      }
      return;
    case ANCHOR_DataType_U8:
      if (op == '+') {
        *(AnchorU8 *)output = ImAddClampOverflow(
          *(const AnchorU8 *)arg1, *(const AnchorU8 *)arg2, IM_U8_MIN, IM_U8_MAX);
      }
      if (op == '-') {
        *(AnchorU8 *)output = ImSubClampOverflow(
          *(const AnchorU8 *)arg1, *(const AnchorU8 *)arg2, IM_U8_MIN, IM_U8_MAX);
      }
      return;
    case ANCHOR_DataType_S16:
      if (op == '+') {
        *(AnchorS16 *)output = ImAddClampOverflow(
          *(const AnchorS16 *)arg1, *(const AnchorS16 *)arg2, IM_S16_MIN, IM_S16_MAX);
      }
      if (op == '-') {
        *(AnchorS16 *)output = ImSubClampOverflow(
          *(const AnchorS16 *)arg1, *(const AnchorS16 *)arg2, IM_S16_MIN, IM_S16_MAX);
      }
      return;
    case ANCHOR_DataType_U16:
      if (op == '+') {
        *(AnchorU16 *)output = ImAddClampOverflow(
          *(const AnchorU16 *)arg1, *(const AnchorU16 *)arg2, IM_U16_MIN, IM_U16_MAX);
      }
      if (op == '-') {
        *(AnchorU16 *)output = ImSubClampOverflow(
          *(const AnchorU16 *)arg1, *(const AnchorU16 *)arg2, IM_U16_MIN, IM_U16_MAX);
      }
      return;
    case ANCHOR_DataType_S32:
      if (op == '+') {
        *(AnchorS32 *)output = ImAddClampOverflow(
          *(const AnchorS32 *)arg1, *(const AnchorS32 *)arg2, IM_S32_MIN, IM_S32_MAX);
      }
      if (op == '-') {
        *(AnchorS32 *)output = ImSubClampOverflow(
          *(const AnchorS32 *)arg1, *(const AnchorS32 *)arg2, IM_S32_MIN, IM_S32_MAX);
      }
      return;
    case ANCHOR_DataType_U32:
      if (op == '+') {
        *(AnchorU32 *)output = ImAddClampOverflow(
          *(const AnchorU32 *)arg1, *(const AnchorU32 *)arg2, IM_U32_MIN, IM_U32_MAX);
      }
      if (op == '-') {
        *(AnchorU32 *)output = ImSubClampOverflow(
          *(const AnchorU32 *)arg1, *(const AnchorU32 *)arg2, IM_U32_MIN, IM_U32_MAX);
      }
      return;
    case ANCHOR_DataType_S64:
      if (op == '+') {
        *(AnchorS64 *)output = ImAddClampOverflow(
          *(const AnchorS64 *)arg1, *(const AnchorS64 *)arg2, IM_S64_MIN, IM_S64_MAX);
      }
      if (op == '-') {
        *(AnchorS64 *)output = ImSubClampOverflow(
          *(const AnchorS64 *)arg1, *(const AnchorS64 *)arg2, IM_S64_MIN, IM_S64_MAX);
      }
      return;
    case ANCHOR_DataType_U64:
      if (op == '+') {
        *(AnchorU64 *)output = ImAddClampOverflow(
          *(const AnchorU64 *)arg1, *(const AnchorU64 *)arg2, IM_U64_MIN, IM_U64_MAX);
      }
      if (op == '-') {
        *(AnchorU64 *)output = ImSubClampOverflow(
          *(const AnchorU64 *)arg1, *(const AnchorU64 *)arg2, IM_U64_MIN, IM_U64_MAX);
      }
      return;
    case ANCHOR_DataType_Float:
      if (op == '+') {
        *(float *)output = *(const float *)arg1 + *(const float *)arg2;
      }
      if (op == '-') {
        *(float *)output = *(const float *)arg1 - *(const float *)arg2;
      }
      return;
    case ANCHOR_DataType_Double:
      if (op == '+') {
        *(double *)output = *(const double *)arg1 + *(const double *)arg2;
      }
      if (op == '-') {
        *(double *)output = *(const double *)arg1 - *(const double *)arg2;
      }
      return;
    case ANCHOR_DataType_COUNT:
      break;
  }
  ANCHOR_ASSERT(0);
}

// User can input math operators (e.g. +100) to edit a numerical values.
// NB: This is _not_ a full expression evaluator. We should probably add one and replace this dumb
// mess..
bool ANCHOR::DataTypeApplyOpFromText(const char *buf,
                                     const char *initial_value_buf,
                                     ANCHOR_DataType data_type,
                                     void *p_data,
                                     const char *format)
{
  while (ImCharIsBlankA(*buf))
    buf++;

  // We don't support '-' op because it would conflict with inputing negative value.
  // Instead you can use +-100 to subtract from an existing value
  char op = buf[0];
  if (op == '+' || op == '*' || op == '/') {
    buf++;
    while (ImCharIsBlankA(*buf))
      buf++;
  }
  else {
    op = 0;
  }
  if (!buf[0])
    return false;

  // Copy the value in an opaque buffer so we can compare at the end of the function if it changed
  // at all.
  const ANCHOR_DataTypeInfo *type_info = DataTypeGetInfo(data_type);
  ANCHOR_DataTypeTempStorage data_backup;
  memcpy(&data_backup, p_data, type_info->Size);

  if (format == NULL)
    format = type_info->ScanFmt;

  // FIXME-LEGACY: The aim is to remove those operators and write a proper expression evaluator at
  // some point..
  int arg1i = 0;
  if (data_type == ANCHOR_DataType_S32) {
    int *v = (int *)p_data;
    int arg0i = *v;
    float arg1f = 0.0f;
    if (op && sscanf(initial_value_buf, format, &arg0i) < 1)
      return false;
    // Store operand in a float so we can use fractional value for multipliers (*1.1), but constant
    // always parsed as integer so we can fit big integers (e.g. 2000000003) past float precision
    if (op == '+') {
      if (sscanf(buf, "%d", &arg1i))
        *v = (int)(arg0i + arg1i);
    }  // Add (use "+-" to subtract)
    else if (op == '*') {
      if (sscanf(buf, "%f", &arg1f))
        *v = (int)(arg0i * arg1f);
    }  // Multiply
    else if (op == '/') {
      if (sscanf(buf, "%f", &arg1f) && arg1f != 0.0f)
        *v = (int)(arg0i / arg1f);
    }  // Divide
    else {
      if (sscanf(buf, format, &arg1i) == 1)
        *v = arg1i;
    }  // Assign constant
  }
  else if (data_type == ANCHOR_DataType_Float) {
    // For floats we have to ignore format with precision (e.g. "%.2f") because sscanf doesn't take
    // them in
    format = "%f";
    float *v = (float *)p_data;
    float arg0f = *v, arg1f = 0.0f;
    if (op && sscanf(initial_value_buf, format, &arg0f) < 1)
      return false;
    if (sscanf(buf, format, &arg1f) < 1)
      return false;
    if (op == '+') {
      *v = arg0f + arg1f;
    }  // Add (use "+-" to subtract)
    else if (op == '*') {
      *v = arg0f * arg1f;
    }  // Multiply
    else if (op == '/') {
      if (arg1f != 0.0f)
        *v = arg0f / arg1f;
    }  // Divide
    else {
      *v = arg1f;
    }  // Assign constant
  }
  else if (data_type == ANCHOR_DataType_Double) {
    format = "%lf";  // scanf differentiate float/double unlike printf which forces everything to
                     // double because of ellipsis
    double *v = (double *)p_data;
    double arg0f = *v, arg1f = 0.0;
    if (op && sscanf(initial_value_buf, format, &arg0f) < 1)
      return false;
    if (sscanf(buf, format, &arg1f) < 1)
      return false;
    if (op == '+') {
      *v = arg0f + arg1f;
    }  // Add (use "+-" to subtract)
    else if (op == '*') {
      *v = arg0f * arg1f;
    }  // Multiply
    else if (op == '/') {
      if (arg1f != 0.0f)
        *v = arg0f / arg1f;
    }  // Divide
    else {
      *v = arg1f;
    }  // Assign constant
  }
  else if (data_type == ANCHOR_DataType_U32 || data_type == ANCHOR_DataType_S64 ||
           data_type == ANCHOR_DataType_U64) {
    // All other types assign constant
    // We don't bother handling support for legacy operators since they are a little too crappy.
    // Instead we will later implement a proper expression evaluator in the future.
    if (sscanf(buf, format, p_data) < 1)
      return false;
  }
  else {
    // Small types need a 32-bit buffer to receive the result from scanf()
    int v32;
    if (sscanf(buf, format, &v32) < 1)
      return false;
    if (data_type == ANCHOR_DataType_S8)
      *(AnchorS8 *)p_data = (AnchorS8)ImClamp(v32, (int)IM_S8_MIN, (int)IM_S8_MAX);
    else if (data_type == ANCHOR_DataType_U8)
      *(AnchorU8 *)p_data = (AnchorU8)ImClamp(v32, (int)IM_U8_MIN, (int)IM_U8_MAX);
    else if (data_type == ANCHOR_DataType_S16)
      *(AnchorS16 *)p_data = (AnchorS16)ImClamp(v32, (int)IM_S16_MIN, (int)IM_S16_MAX);
    else if (data_type == ANCHOR_DataType_U16)
      *(AnchorU16 *)p_data = (AnchorU16)ImClamp(v32, (int)IM_U16_MIN, (int)IM_U16_MAX);
    else
      ANCHOR_ASSERT(0);
  }

  return memcmp(&data_backup, p_data, type_info->Size) != 0;
}

template<typename T> static int DataTypeCompareT(const T *lhs, const T *rhs)
{
  if (*lhs < *rhs)
    return -1;
  if (*lhs > *rhs)
    return +1;
  return 0;
}

int ANCHOR::DataTypeCompare(ANCHOR_DataType data_type, const void *arg_1, const void *arg_2)
{
  switch (data_type) {
    case ANCHOR_DataType_S8:
      return DataTypeCompareT<AnchorS8>((const AnchorS8 *)arg_1, (const AnchorS8 *)arg_2);
    case ANCHOR_DataType_U8:
      return DataTypeCompareT<AnchorU8>((const AnchorU8 *)arg_1, (const AnchorU8 *)arg_2);
    case ANCHOR_DataType_S16:
      return DataTypeCompareT<AnchorS16>((const AnchorS16 *)arg_1, (const AnchorS16 *)arg_2);
    case ANCHOR_DataType_U16:
      return DataTypeCompareT<AnchorU16>((const AnchorU16 *)arg_1, (const AnchorU16 *)arg_2);
    case ANCHOR_DataType_S32:
      return DataTypeCompareT<AnchorS32>((const AnchorS32 *)arg_1, (const AnchorS32 *)arg_2);
    case ANCHOR_DataType_U32:
      return DataTypeCompareT<AnchorU32>((const AnchorU32 *)arg_1, (const AnchorU32 *)arg_2);
    case ANCHOR_DataType_S64:
      return DataTypeCompareT<AnchorS64>((const AnchorS64 *)arg_1, (const AnchorS64 *)arg_2);
    case ANCHOR_DataType_U64:
      return DataTypeCompareT<AnchorU64>((const AnchorU64 *)arg_1, (const AnchorU64 *)arg_2);
    case ANCHOR_DataType_Float:
      return DataTypeCompareT<float>((const float *)arg_1, (const float *)arg_2);
    case ANCHOR_DataType_Double:
      return DataTypeCompareT<double>((const double *)arg_1, (const double *)arg_2);
    case ANCHOR_DataType_COUNT:
      break;
  }
  ANCHOR_ASSERT(0);
  return 0;
}

template<typename T> static bool DataTypeClampT(T *v, const T *v_min, const T *v_max)
{
  // Clamp, both sides are optional, return true if modified
  if (v_min && *v < *v_min) {
    *v = *v_min;
    return true;
  }
  if (v_max && *v > *v_max) {
    *v = *v_max;
    return true;
  }
  return false;
}

bool ANCHOR::DataTypeClamp(ANCHOR_DataType data_type, void *p_data, const void *p_min, const void *p_max)
{
  switch (data_type) {
    case ANCHOR_DataType_S8:
      return DataTypeClampT<AnchorS8>((AnchorS8 *)p_data, (const AnchorS8 *)p_min, (const AnchorS8 *)p_max);
    case ANCHOR_DataType_U8:
      return DataTypeClampT<AnchorU8>((AnchorU8 *)p_data, (const AnchorU8 *)p_min, (const AnchorU8 *)p_max);
    case ANCHOR_DataType_S16:
      return DataTypeClampT<AnchorS16>(
        (AnchorS16 *)p_data, (const AnchorS16 *)p_min, (const AnchorS16 *)p_max);
    case ANCHOR_DataType_U16:
      return DataTypeClampT<AnchorU16>(
        (AnchorU16 *)p_data, (const AnchorU16 *)p_min, (const AnchorU16 *)p_max);
    case ANCHOR_DataType_S32:
      return DataTypeClampT<AnchorS32>(
        (AnchorS32 *)p_data, (const AnchorS32 *)p_min, (const AnchorS32 *)p_max);
    case ANCHOR_DataType_U32:
      return DataTypeClampT<AnchorU32>(
        (AnchorU32 *)p_data, (const AnchorU32 *)p_min, (const AnchorU32 *)p_max);
    case ANCHOR_DataType_S64:
      return DataTypeClampT<AnchorS64>(
        (AnchorS64 *)p_data, (const AnchorS64 *)p_min, (const AnchorS64 *)p_max);
    case ANCHOR_DataType_U64:
      return DataTypeClampT<AnchorU64>(
        (AnchorU64 *)p_data, (const AnchorU64 *)p_min, (const AnchorU64 *)p_max);
    case ANCHOR_DataType_Float:
      return DataTypeClampT<float>((float *)p_data, (const float *)p_min, (const float *)p_max);
    case ANCHOR_DataType_Double:
      return DataTypeClampT<double>((double *)p_data, (const double *)p_min, (const double *)p_max);
    case ANCHOR_DataType_COUNT:
      break;
  }
  ANCHOR_ASSERT(0);
  return false;
}

static float GetMinimumStepAtDecimalPrecision(int decimal_precision)
{
  static const float min_steps[10] = {
    1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f};
  if (decimal_precision < 0)
    return FLT_MIN;
  return (decimal_precision < ANCHOR_ARRAYSIZE(min_steps)) ? min_steps[decimal_precision] :
                                                             ImPow(10.0f, (float)-decimal_precision);
}

template<typename TYPE> static const char *ImAtoi(const char *src, TYPE *output)
{
  int negative = 0;
  if (*src == '-') {
    negative = 1;
    src++;
  }
  if (*src == '+') {
    src++;
  }
  TYPE v = 0;
  while (*src >= '0' && *src <= '9')
    v = (v * 10) + (*src++ - '0');
  *output = negative ? -v : v;
  return src;
}

// Sanitize format
// - Zero terminate so extra characters after format (e.g. "%f123") don't confuse atof/atoi
// - stb_sprintf.h supports several new modifiers which format numbers in a way that also makes
// them incompatible atof/atoi.
static void SanitizeFormatString(const char *fmt, char *fmt_out, size_t fmt_out_size)
{
  TF_UNUSED(fmt_out_size);
  const char *fmt_end = ImParseFormatFindEnd(fmt);
  ANCHOR_ASSERT((size_t)(fmt_end - fmt + 1) <
                fmt_out_size);  // Format is too long, let us know if this happens to you!
  while (fmt < fmt_end) {
    char c = *(fmt++);
    if (c != '\'' && c != '$' &&
        c != '_')  // Custom flags provided by stb_sprintf.h. POSIX 2008 also supports '.
      *(fmt_out++) = c;
  }
  *fmt_out = 0;  // Zero-terminate
}

template<typename TYPE, typename SIGNEDTYPE>
TYPE ANCHOR::RoundScalarWithFormatT(const char *format, ANCHOR_DataType data_type, TYPE v)
{
  const char *fmt_start = ImParseFormatFindStart(format);
  if (fmt_start[0] != '%' ||
      fmt_start[1] == '%')  // Don't apply if the value is not visible in the format string
    return v;

  // Sanitize format
  char fmt_sanitized[32];
  SanitizeFormatString(fmt_start, fmt_sanitized, ANCHOR_ARRAYSIZE(fmt_sanitized));
  fmt_start = fmt_sanitized;

  // Format value with our rounding, and read back
  char v_str[64];
  ImFormatString(v_str, ANCHOR_ARRAYSIZE(v_str), fmt_start, v);
  const char *p = v_str;
  while (*p == ' ')
    p++;
  if (data_type == ANCHOR_DataType_Float || data_type == ANCHOR_DataType_Double)
    v = (TYPE)ImAtof(p);
  else
    ImAtoi(p, (SIGNEDTYPE *)&v);
  return v;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
//-------------------------------------------------------------------------
// - DragBehaviorT<>() [Internal]
// - DragBehavior() [Internal]
// - DragScalar()
// - DragScalarN()
// - DragFloat()
// - DragFloat2()
// - DragFloat3()
// - DragFloat4()
// - DragFloatRange2()
// - DragInt()
// - DragInt2()
// - DragInt3()
// - DragInt4()
// - DragIntRange2()
//-------------------------------------------------------------------------

// This is called by DragBehavior() when the widget is active (held by mouse or being manipulated
// with Nav controls)
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool ANCHOR::DragBehaviorT(ANCHOR_DataType data_type,
                           TYPE *v,
                           float v_speed,
                           const TYPE v_min,
                           const TYPE v_max,
                           const char *format,
                           ANCHOR_SliderFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Axis axis = (flags & ANCHOR_SliderFlags_Vertical) ? ANCHOR_Axis_Y : ANCHOR_Axis_X;
  const bool is_clamped = (v_min < v_max);
  const bool is_logarithmic = (flags & ANCHOR_SliderFlags_Logarithmic) != 0;
  const bool is_floating_point = (data_type == ANCHOR_DataType_Float) ||
                                 (data_type == ANCHOR_DataType_Double);

  // Default tweak speed
  if (v_speed == 0.0f && is_clamped && (v_max - v_min < FLT_MAX))
    v_speed = (float)((v_max - v_min) * g.DragSpeedDefaultRatio);

  // Inputs accumulates into g.DragCurrentAccum, which is flushed into the current value as soon as
  // it makes a difference with our precision settings
  float adjust_delta = 0.0f;
  if (g.ActiveIdSource == ANCHORInputSource_Mouse && IsMousePosValid() &&
      IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * DRAG_MOUSE_THRESHOLD_FACTOR)) {
    adjust_delta = g.IO.MouseDelta[axis];
    if (g.IO.KeyAlt)
      adjust_delta *= 1.0f / 100.0f;
    if (g.IO.KeyShift)
      adjust_delta *= 10.0f;
  }
  else if (g.ActiveIdSource == ANCHORInputSource_Nav) {
    const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 0;
    adjust_delta = GetNavInputAmount2d(ANCHOR_NavDirSourceFlags_Keyboard | ANCHOR_NavDirSourceFlags_PadDPad,
                                       ANCHOR_InputReadMode_RepeatFast,
                                       1.0f / 10.0f,
                                       10.0f)[axis];
    v_speed = AnchorMax(v_speed, GetMinimumStepAtDecimalPrecision(decimal_precision));
  }
  adjust_delta *= v_speed;

  // For vertical drag we currently assume that Up=higher value (like we do with vertical sliders).
  // This may become a parameter.
  if (axis == ANCHOR_Axis_Y)
    adjust_delta = -adjust_delta;

  // For logarithmic use our range is effectively 0..1 so scale the delta into that range
  if (is_logarithmic && (v_max - v_min < FLT_MAX) && ((v_max - v_min) > 0.000001f))  // Epsilon to avoid /0
    adjust_delta /= (float)(v_max - v_min);

  // Clear current value on activation
  // Avoid altering values and clamping when we are _already_ past the limits and heading in the
  // same direction, so e.g. if range is 0..255, current value is 300 and we are pushing to the
  // right side, keep the 300.
  bool is_just_activated = g.ActiveIdIsJustActivated;
  bool is_already_past_limits_and_pushing_outward = is_clamped && ((*v >= v_max && adjust_delta > 0.0f) ||
                                                                   (*v <= v_min && adjust_delta < 0.0f));
  if (is_just_activated || is_already_past_limits_and_pushing_outward) {
    g.DragCurrentAccum = 0.0f;
    g.DragCurrentAccumDirty = false;
  }
  else if (adjust_delta != 0.0f) {
    g.DragCurrentAccum += adjust_delta;
    g.DragCurrentAccumDirty = true;
  }

  if (!g.DragCurrentAccumDirty)
    return false;

  TYPE v_cur = *v;
  FLOATTYPE v_old_ref_for_accum_remainder = (FLOATTYPE)0.0f;

  float logarithmic_zero_epsilon = 0.0f;      // Only valid when is_logarithmic is true
  const float zero_deadzone_halfsize = 0.0f;  // Drag widgets have no deadzone (as it doesn't make sense)
  if (is_logarithmic) {
    // When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of
    // clamp value greatly affects slider precision. We attempt to use the specified precision to
    // estimate a good lower bound.
    const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 1;
    logarithmic_zero_epsilon = ImPow(0.1f, (float)decimal_precision);

    // Convert to parametric space, apply delta, convert back
    float v_old_parametric = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
      data_type, v_cur, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
    float v_new_parametric = v_old_parametric + g.DragCurrentAccum;
    v_cur = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type,
                                                              v_new_parametric,
                                                              v_min,
                                                              v_max,
                                                              is_logarithmic,
                                                              logarithmic_zero_epsilon,
                                                              zero_deadzone_halfsize);
    v_old_ref_for_accum_remainder = v_old_parametric;
  }
  else {
    v_cur += (SIGNEDTYPE)g.DragCurrentAccum;
  }

  // Round to user desired precision based on format string
  if (!(flags & ANCHOR_SliderFlags_NoRoundToFormat))
    v_cur = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_cur);

  // Preserve remainder after rounding has been applied. This also allow slow tweaking of values.
  g.DragCurrentAccumDirty = false;
  if (is_logarithmic) {
    // Convert to parametric space, apply delta, convert back
    float v_new_parametric = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
      data_type, v_cur, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
    g.DragCurrentAccum -= (float)(v_new_parametric - v_old_ref_for_accum_remainder);
  }
  else {
    g.DragCurrentAccum -= (float)((SIGNEDTYPE)v_cur - (SIGNEDTYPE)*v);
  }

  // Lose zero sign for float/double
  if (v_cur == (TYPE)-0)
    v_cur = (TYPE)0;

  // Clamp values (+ handle overflow/wrap-around for integer types)
  if (*v != v_cur && is_clamped) {
    if (v_cur < v_min || (v_cur > *v && adjust_delta < 0.0f && !is_floating_point))
      v_cur = v_min;
    if (v_cur > v_max || (v_cur < *v && adjust_delta > 0.0f && !is_floating_point))
      v_cur = v_max;
  }

  // Apply result
  if (*v == v_cur)
    return false;
  *v = v_cur;
  return true;
}

bool ANCHOR::DragBehavior(ANCHOR_ID id,
                          ANCHOR_DataType data_type,
                          void *p_v,
                          float v_speed,
                          const void *p_min,
                          const void *p_max,
                          const char *format,
                          ANCHOR_SliderFlags flags)
{
  // Read ANCHOR.cpp "API BREAKING CHANGES" section for 1.78 if you hit this assert.
  ANCHOR_ASSERT((flags == 1 || (flags & ANCHOR_SliderFlags_InvalidMask_) == 0) &&
                "Invalid ANCHOR_SliderFlags flags! Has the 'float power' argument been mistakenly "
                "cast to flags? Call function with ANCHOR_SliderFlags_Logarithmic flags instead.");

  ANCHOR_Context &g = *G_CTX;
  if (g.ActiveId == id) {
    if (g.ActiveIdSource == ANCHORInputSource_Mouse && !g.IO.MouseDown[0])
      ClearActiveID();
    else if (g.ActiveIdSource == ANCHORInputSource_Nav && g.NavActivatePressedId == id &&
             !g.ActiveIdIsJustActivated)
      ClearActiveID();
  }
  if (g.ActiveId != id)
    return false;
  if ((g.CurrentItemFlags & ANCHOR_ItemFlags_ReadOnly) || (flags & ANCHOR_SliderFlags_ReadOnly))
    return false;

  switch (data_type) {
    case ANCHOR_DataType_S8: {
      AnchorS32 v32 = (AnchorS32) * (AnchorS8 *)p_v;
      bool r = DragBehaviorT<AnchorS32, AnchorS32, float>(ANCHOR_DataType_S32,
                                                          &v32,
                                                          v_speed,
                                                          p_min ? *(const AnchorS8 *)p_min : IM_S8_MIN,
                                                          p_max ? *(const AnchorS8 *)p_max : IM_S8_MAX,
                                                          format,
                                                          flags);
      if (r)
        *(AnchorS8 *)p_v = (AnchorS8)v32;
      return r;
    }
    case ANCHOR_DataType_U8: {
      AnchorU32 v32 = (AnchorU32) * (AnchorU8 *)p_v;
      bool r = DragBehaviorT<AnchorU32, AnchorS32, float>(ANCHOR_DataType_U32,
                                                          &v32,
                                                          v_speed,
                                                          p_min ? *(const AnchorU8 *)p_min : IM_U8_MIN,
                                                          p_max ? *(const AnchorU8 *)p_max : IM_U8_MAX,
                                                          format,
                                                          flags);
      if (r)
        *(AnchorU8 *)p_v = (AnchorU8)v32;
      return r;
    }
    case ANCHOR_DataType_S16: {
      AnchorS32 v32 = (AnchorS32) * (AnchorS16 *)p_v;
      bool r = DragBehaviorT<AnchorS32, AnchorS32, float>(ANCHOR_DataType_S32,
                                                          &v32,
                                                          v_speed,
                                                          p_min ? *(const AnchorS16 *)p_min : IM_S16_MIN,
                                                          p_max ? *(const AnchorS16 *)p_max : IM_S16_MAX,
                                                          format,
                                                          flags);
      if (r)
        *(AnchorS16 *)p_v = (AnchorS16)v32;
      return r;
    }
    case ANCHOR_DataType_U16: {
      AnchorU32 v32 = (AnchorU32) * (AnchorU16 *)p_v;
      bool r = DragBehaviorT<AnchorU32, AnchorS32, float>(ANCHOR_DataType_U32,
                                                          &v32,
                                                          v_speed,
                                                          p_min ? *(const AnchorU16 *)p_min : IM_U16_MIN,
                                                          p_max ? *(const AnchorU16 *)p_max : IM_U16_MAX,
                                                          format,
                                                          flags);
      if (r)
        *(AnchorU16 *)p_v = (AnchorU16)v32;
      return r;
    }
    case ANCHOR_DataType_S32:
      return DragBehaviorT<AnchorS32, AnchorS32, float>(data_type,
                                                        (AnchorS32 *)p_v,
                                                        v_speed,
                                                        p_min ? *(const AnchorS32 *)p_min : IM_S32_MIN,
                                                        p_max ? *(const AnchorS32 *)p_max : IM_S32_MAX,
                                                        format,
                                                        flags);
    case ANCHOR_DataType_U32:
      return DragBehaviorT<AnchorU32, AnchorS32, float>(data_type,
                                                        (AnchorU32 *)p_v,
                                                        v_speed,
                                                        p_min ? *(const AnchorU32 *)p_min : IM_U32_MIN,
                                                        p_max ? *(const AnchorU32 *)p_max : IM_U32_MAX,
                                                        format,
                                                        flags);
    case ANCHOR_DataType_S64:
      return DragBehaviorT<AnchorS64, AnchorS64, double>(data_type,
                                                         (AnchorS64 *)p_v,
                                                         v_speed,
                                                         p_min ? *(const AnchorS64 *)p_min : IM_S64_MIN,
                                                         p_max ? *(const AnchorS64 *)p_max : IM_S64_MAX,
                                                         format,
                                                         flags);
    case ANCHOR_DataType_U64:
      return DragBehaviorT<AnchorU64, AnchorS64, double>(data_type,
                                                         (AnchorU64 *)p_v,
                                                         v_speed,
                                                         p_min ? *(const AnchorU64 *)p_min : IM_U64_MIN,
                                                         p_max ? *(const AnchorU64 *)p_max : IM_U64_MAX,
                                                         format,
                                                         flags);
    case ANCHOR_DataType_Float:
      return DragBehaviorT<float, float, float>(data_type,
                                                (float *)p_v,
                                                v_speed,
                                                p_min ? *(const float *)p_min : -FLT_MAX,
                                                p_max ? *(const float *)p_max : FLT_MAX,
                                                format,
                                                flags);
    case ANCHOR_DataType_Double:
      return DragBehaviorT<double, double, double>(data_type,
                                                   (double *)p_v,
                                                   v_speed,
                                                   p_min ? *(const double *)p_min : -DBL_MAX,
                                                   p_max ? *(const double *)p_max : DBL_MAX,
                                                   format,
                                                   flags);
    case ANCHOR_DataType_COUNT:
      break;
  }
  ANCHOR_ASSERT(0);
  return false;
}

// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the data. For a Drag
// widget, p_min and p_max are optional. Read code of e.g. DragFloat(), DragInt() etc. or examples
// in 'Demo->Widgets->Data Types' to understand how to use this function directly.
bool ANCHOR::DragScalar(const char *label,
                        ANCHOR_DataType data_type,
                        void *p_data,
                        float v_speed,
                        const void *p_min,
                        const void *p_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  const float w = CalcItemWidth();

  const GfVec2f label_size = CalcTextSize(label, NULL, true);
  const ImRect frame_bb(window->DC.CursorPos,
                        window->DC.CursorPos + GfVec2f(w, label_size[1] + style.FramePadding[1] * 2.0f));
  const ImRect total_bb(
    frame_bb.Min,
    frame_bb.Max + GfVec2f(label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f, 0.0f));

  const bool temp_input_allowed = (flags & ANCHOR_SliderFlags_NoInput) == 0;
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ANCHOR_ItemAddFlags_Focusable : 0))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;
  else if (data_type == ANCHOR_DataType_S32 &&
           strcmp(format, "%d") != 0)  // (FIXME-LEGACY: Patch old "%.0f" format string to use
                                       // "%d", read function more details.)
    format = PatchFormatStringFloatToInt(format);

  // Tabbing or CTRL-clicking on Drag turns it into an InputText
  const bool hovered = ItemHoverable(frame_bb, id);
  bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
  if (!temp_input_is_active) {
    const bool focus_requested = temp_input_allowed &&
                                 (window->DC.LastItemStatusFlags & ANCHOR_ItemStatusFlags_Focused) != 0;
    const bool clicked = (hovered && g.IO.MouseClicked[0]);
    const bool double_clicked = (hovered && g.IO.MouseDoubleClicked[0]);
    if (focus_requested || clicked || double_clicked || g.NavActivateId == id || g.NavInputId == id) {
      SetActiveID(id, window);
      SetFocusID(id, window);
      FocusWindow(window);
      g.ActiveIdUsingNavDirMask = (1 << ANCHOR_Dir_Left) | (1 << ANCHOR_Dir_Right);
      if (temp_input_allowed &&
          (focus_requested || (clicked && g.IO.KeyCtrl) || double_clicked || g.NavInputId == id))
        temp_input_is_active = true;
    }
    // Experimental: simple click (without moving) turns Drag into an InputText
    // FIXME: Currently polling ANCHORConfigFlags_IsTouchScreen, may either poll an hypothetical
    // ANCHORBackendFlags_HasKeyboard and/or an explicit drag settings.
    if (g.IO.ConfigDragClickToInputText && temp_input_allowed && !temp_input_is_active)
      if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] &&
          !IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * DRAG_MOUSE_THRESHOLD_FACTOR)) {
        g.NavInputId = id;
        temp_input_is_active = true;
      }
  }

  if (temp_input_is_active) {
    // Only clamp CTRL+Click input when ANCHOR_SliderFlags_AlwaysClamp is set
    const bool is_clamp_input = (flags & ANCHOR_SliderFlags_AlwaysClamp) != 0 &&
                                (p_min == NULL || p_max == NULL ||
                                 DataTypeCompare(data_type, p_min, p_max) < 0);
    return TempInputScalar(frame_bb,
                           id,
                           label,
                           data_type,
                           p_data,
                           format,
                           is_clamp_input ? p_min : NULL,
                           is_clamp_input ? p_max : NULL);
  }

  // Draw frame
  const AnchorU32 frame_col = GetColorU32(g.ActiveId == id  ? ANCHOR_Col_FrameBgActive :
                                          g.HoveredId == id ? ANCHOR_Col_FrameBgHovered :
                                                              ANCHOR_Col_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

  // Drag behavior
  const bool value_changed = DragBehavior(id, data_type, p_data, v_speed, p_min, p_max, format, flags);
  if (value_changed)
    MarkItemEdited(id);

  // Display value using user-provided display format so user can add prefix/suffix/decorations to
  // the value.
  char value_buf[64];
  const char *value_buf_end = value_buf +
                              DataTypeFormatString(
                                value_buf, ANCHOR_ARRAYSIZE(value_buf), data_type, p_data, format);
  if (g.LogEnabled)
    LogSetNextTextDecoration("{", "}");
  RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, GfVec2f(0.5f, 0.5f));

  if (label_size[0] > 0.0f)
    RenderText(GfVec2f(frame_bb.Max[0] + style.ItemInnerSpacing[0], frame_bb.Min[1] + style.FramePadding[1]),
               label);

  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  return value_changed;
}

bool ANCHOR::DragScalarN(const char *label,
                         ANCHOR_DataType data_type,
                         void *p_data,
                         int components,
                         float v_speed,
                         const void *p_min,
                         const void *p_max,
                         const char *format,
                         ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  bool value_changed = false;
  BeginGroup();
  PushID(label);
  PushMultiItemsWidths(components, CalcItemWidth());
  size_t type_size = GDataTypeInfo[data_type].Size;
  for (int i = 0; i < components; i++) {
    PushID(i);
    if (i > 0)
      SameLine(0, g.Style.ItemInnerSpacing[0]);
    value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
    PopID();
    PopItemWidth();
    p_data = (void *)((char *)p_data + type_size);
  }
  PopID();

  const char *label_end = FindRenderedTextEnd(label);
  if (label != label_end) {
    SameLine(0, g.Style.ItemInnerSpacing[0]);
    TextEx(label, label_end);
  }

  EndGroup();
  return value_changed;
}

bool ANCHOR::DragFloat(const char *label,
                       float *v,
                       float v_speed,
                       float v_min,
                       float v_max,
                       const char *format,
                       ANCHOR_SliderFlags flags)
{
  return DragScalar(label, ANCHOR_DataType_Float, v, v_speed, &v_min, &v_max, format, flags);
}

bool ANCHOR::DragFloat2(const char *label,
                        float v[2],
                        float v_speed,
                        float v_min,
                        float v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return DragScalarN(label, ANCHOR_DataType_Float, v, 2, v_speed, &v_min, &v_max, format, flags);
}

bool ANCHOR::DragFloat3(const char *label,
                        float v[3],
                        float v_speed,
                        float v_min,
                        float v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return DragScalarN(label, ANCHOR_DataType_Float, v, 3, v_speed, &v_min, &v_max, format, flags);
}

bool ANCHOR::DragFloat4(const char *label,
                        float v[4],
                        float v_speed,
                        float v_min,
                        float v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return DragScalarN(label, ANCHOR_DataType_Float, v, 4, v_speed, &v_min, &v_max, format, flags);
}

// NB: You likely want to specify the ANCHOR_SliderFlags_AlwaysClamp when using this.
bool ANCHOR::DragFloatRange2(const char *label,
                             float *v_current_min,
                             float *v_current_max,
                             float v_speed,
                             float v_min,
                             float v_max,
                             const char *format,
                             const char *format_max,
                             ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  PushID(label);
  BeginGroup();
  PushMultiItemsWidths(2, CalcItemWidth());

  float min_min = (v_min >= v_max) ? -FLT_MAX : v_min;
  float min_max = (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max);
  ANCHOR_SliderFlags min_flags = flags | ((min_min == min_max) ? ANCHOR_SliderFlags_ReadOnly : 0);
  bool value_changed = DragScalar(
    "##min", ANCHOR_DataType_Float, v_current_min, v_speed, &min_min, &min_max, format, min_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing[0]);

  float max_min = (v_min >= v_max) ? *v_current_min : AnchorMax(v_min, *v_current_min);
  float max_max = (v_min >= v_max) ? FLT_MAX : v_max;
  ANCHOR_SliderFlags max_flags = flags | ((max_min == max_max) ? ANCHOR_SliderFlags_ReadOnly : 0);
  value_changed |= DragScalar("##max",
                              ANCHOR_DataType_Float,
                              v_current_max,
                              v_speed,
                              &max_min,
                              &max_max,
                              format_max ? format_max : format,
                              max_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing[0]);

  TextEx(label, FindRenderedTextEnd(label));
  EndGroup();
  PopID();
  return value_changed;
}

// NB: v_speed is float to allow adjusting the drag speed with more precision
bool ANCHOR::DragInt(const char *label,
                     int *v,
                     float v_speed,
                     int v_min,
                     int v_max,
                     const char *format,
                     ANCHOR_SliderFlags flags)
{
  return DragScalar(label, ANCHOR_DataType_S32, v, v_speed, &v_min, &v_max, format, flags);
}

bool ANCHOR::DragInt2(const char *label,
                      int v[2],
                      float v_speed,
                      int v_min,
                      int v_max,
                      const char *format,
                      ANCHOR_SliderFlags flags)
{
  return DragScalarN(label, ANCHOR_DataType_S32, v, 2, v_speed, &v_min, &v_max, format, flags);
}

bool ANCHOR::DragInt3(const char *label,
                      int v[3],
                      float v_speed,
                      int v_min,
                      int v_max,
                      const char *format,
                      ANCHOR_SliderFlags flags)
{
  return DragScalarN(label, ANCHOR_DataType_S32, v, 3, v_speed, &v_min, &v_max, format, flags);
}

bool ANCHOR::DragInt4(const char *label,
                      int v[4],
                      float v_speed,
                      int v_min,
                      int v_max,
                      const char *format,
                      ANCHOR_SliderFlags flags)
{
  return DragScalarN(label, ANCHOR_DataType_S32, v, 4, v_speed, &v_min, &v_max, format, flags);
}

// NB: You likely want to specify the ANCHOR_SliderFlags_AlwaysClamp when using this.
bool ANCHOR::DragIntRange2(const char *label,
                           int *v_current_min,
                           int *v_current_max,
                           float v_speed,
                           int v_min,
                           int v_max,
                           const char *format,
                           const char *format_max,
                           ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  PushID(label);
  BeginGroup();
  PushMultiItemsWidths(2, CalcItemWidth());

  int min_min = (v_min >= v_max) ? INT_MIN : v_min;
  int min_max = (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max);
  ANCHOR_SliderFlags min_flags = flags | ((min_min == min_max) ? ANCHOR_SliderFlags_ReadOnly : 0);
  bool value_changed = DragInt("##min", v_current_min, v_speed, min_min, min_max, format, min_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing[0]);

  int max_min = (v_min >= v_max) ? *v_current_min : AnchorMax(v_min, *v_current_min);
  int max_max = (v_min >= v_max) ? INT_MAX : v_max;
  ANCHOR_SliderFlags max_flags = flags | ((max_min == max_max) ? ANCHOR_SliderFlags_ReadOnly : 0);
  value_changed |= DragInt(
    "##max", v_current_max, v_speed, max_min, max_max, format_max ? format_max : format, max_flags);
  PopItemWidth();
  SameLine(0, g.Style.ItemInnerSpacing[0]);

  TextEx(label, FindRenderedTextEnd(label));
  EndGroup();
  PopID();

  return value_changed;
}

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS

// Obsolete versions with power parameter. See https://github.com/ocornut/ANCHOR/issues/3361 for
// details.
bool ANCHOR::DragScalar(const char *label,
                        ANCHOR_DataType data_type,
                        void *p_data,
                        float v_speed,
                        const void *p_min,
                        const void *p_max,
                        const char *format,
                        float power)
{
  ANCHOR_SliderFlags drag_flags = ANCHOR_SliderFlags_None;
  if (power != 1.0f) {
    ANCHOR_ASSERT(power == 1.0f &&
                  "Call function with ANCHOR_SliderFlags_Logarithmic flags instead of using the "
                  "old 'float power' function!");
    ANCHOR_ASSERT(p_min != NULL &&
                  p_max != NULL);  // When using a power curve the drag needs to have known bounds
    drag_flags |= ANCHOR_SliderFlags_Logarithmic;  // Fallback for non-asserting paths
  }
  return DragScalar(label, data_type, p_data, v_speed, p_min, p_max, format, drag_flags);
}

bool ANCHOR::DragScalarN(const char *label,
                         ANCHOR_DataType data_type,
                         void *p_data,
                         int components,
                         float v_speed,
                         const void *p_min,
                         const void *p_max,
                         const char *format,
                         float power)
{
  ANCHOR_SliderFlags drag_flags = ANCHOR_SliderFlags_None;
  if (power != 1.0f) {
    ANCHOR_ASSERT(power == 1.0f &&
                  "Call function with ANCHOR_SliderFlags_Logarithmic flags instead of using the "
                  "old 'float power' function!");
    ANCHOR_ASSERT(p_min != NULL &&
                  p_max != NULL);  // When using a power curve the drag needs to have known bounds
    drag_flags |= ANCHOR_SliderFlags_Logarithmic;  // Fallback for non-asserting paths
  }
  return DragScalarN(label, data_type, p_data, components, v_speed, p_min, p_max, format, drag_flags);
}

#  endif  // ANCHOR_DISABLE_OBSOLETE_FUNCTIONS

//-------------------------------------------------------------------------
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
//-------------------------------------------------------------------------
// - ScaleRatioFromValueT<> [Internal]
// - ScaleValueFromRatioT<> [Internal]
// - SliderBehaviorT<>() [Internal]
// - SliderBehavior() [Internal]
// - SliderScalar()
// - SliderScalarN()
// - SliderFloat()
// - SliderFloat2()
// - SliderFloat3()
// - SliderFloat4()
// - SliderAngle()
// - SliderInt()
// - SliderInt2()
// - SliderInt3()
// - SliderInt4()
// - VSliderScalar()
// - VSliderFloat()
// - VSliderInt()
//-------------------------------------------------------------------------

// Convert a value v in the output space of a slider into a parametric position on the slider
// itself (the logical opposite of ScaleValueFromRatioT)
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
float ANCHOR::ScaleRatioFromValueT(ANCHOR_DataType data_type,
                                   TYPE v,
                                   TYPE v_min,
                                   TYPE v_max,
                                   bool is_logarithmic,
                                   float logarithmic_zero_epsilon,
                                   float zero_deadzone_halfsize)
{
  if (v_min == v_max)
    return 0.0f;
  TF_UNUSED(data_type);

  const TYPE v_clamped = (v_min < v_max) ? ImClamp(v, v_min, v_max) : ImClamp(v, v_max, v_min);
  if (is_logarithmic) {
    bool flipped = v_max < v_min;

    if (flipped)  // Handle the case where the range is backwards
      ImSwap(v_min, v_max);

    // Fudge min/max to avoid getting close to log(0)
    FLOATTYPE v_min_fudged = (ImAbs((FLOATTYPE)v_min) < logarithmic_zero_epsilon) ?
                               ((v_min < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) :
                               (FLOATTYPE)v_min;
    FLOATTYPE v_max_fudged = (ImAbs((FLOATTYPE)v_max) < logarithmic_zero_epsilon) ?
                               ((v_max < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) :
                               (FLOATTYPE)v_max;

    // Awkward special cases - we need ranges of the form (-100 .. 0) to convert to (-100 ..
    // -epsilon), not (-100 .. epsilon)
    if ((v_min == 0.0f) && (v_max < 0.0f))
      v_min_fudged = -logarithmic_zero_epsilon;
    else if ((v_max == 0.0f) && (v_min < 0.0f))
      v_max_fudged = -logarithmic_zero_epsilon;

    float result;

    if (v_clamped <= v_min_fudged)
      result = 0.0f;  // Workaround for values that are in-range but below our fudge
    else if (v_clamped >= v_max_fudged)
      result = 1.0f;                  // Workaround for values that are in-range but above our fudge
    else if ((v_min * v_max) < 0.0f)  // Range crosses zero, so split into two portions
    {
      float zero_point_center = (-(float)v_min) /
                                ((float)v_max -
                                 (float)v_min);  // The zero point in parametric space.  There's an argument
                                                 // we should take the logarithmic nature into account when
                                                 // calculating this, but for now this should do (and the
                                                 // most common case of a symmetrical range works fine)
      float zero_point_snap_L = zero_point_center - zero_deadzone_halfsize;
      float zero_point_snap_R = zero_point_center + zero_deadzone_halfsize;
      if (v == 0.0f)
        result = zero_point_center;  // Special case for exactly zero
      else if (v < 0.0f)
        result = (1.0f - (float)(ImLog(-(FLOATTYPE)v_clamped / logarithmic_zero_epsilon) /
                                 ImLog(-v_min_fudged / logarithmic_zero_epsilon))) *
                 zero_point_snap_L;
      else
        result = zero_point_snap_R + ((float)(ImLog((FLOATTYPE)v_clamped / logarithmic_zero_epsilon) /
                                              ImLog(v_max_fudged / logarithmic_zero_epsilon)) *
                                      (1.0f - zero_point_snap_R));
    }
    else if ((v_min < 0.0f) || (v_max < 0.0f))  // Entirely negative slider
      result = 1.0f -
               (float)(ImLog(-(FLOATTYPE)v_clamped / -v_max_fudged) / ImLog(-v_min_fudged / -v_max_fudged));
    else
      result = (float)(ImLog((FLOATTYPE)v_clamped / v_min_fudged) / ImLog(v_max_fudged / v_min_fudged));

    return flipped ? (1.0f - result) : result;
  }

  // Linear slider
  return (float)((FLOATTYPE)(SIGNEDTYPE)(v_clamped - v_min) / (FLOATTYPE)(SIGNEDTYPE)(v_max - v_min));
}

// Convert a parametric position on a slider into a value v in the output space (the logical
// opposite of ScaleRatioFromValueT)
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
TYPE ANCHOR::ScaleValueFromRatioT(ANCHOR_DataType data_type,
                                  float t,
                                  TYPE v_min,
                                  TYPE v_max,
                                  bool is_logarithmic,
                                  float logarithmic_zero_epsilon,
                                  float zero_deadzone_halfsize)
{
  if (v_min == v_max)
    return v_min;
  const bool is_floating_point = (data_type == ANCHOR_DataType_Float) ||
                                 (data_type == ANCHOR_DataType_Double);

  TYPE result;
  if (is_logarithmic) {
    // We special-case the extents because otherwise our fudging can lead to "mathematically
    // correct" but non-intuitive behaviors like a fully-left slider not actually reaching the
    // minimum value
    if (t <= 0.0f)
      result = v_min;
    else if (t >= 1.0f)
      result = v_max;
    else {
      bool flipped = v_max < v_min;  // Check if range is "backwards"

      // Fudge min/max to avoid getting silly results close to zero
      FLOATTYPE v_min_fudged = (ImAbs((FLOATTYPE)v_min) < logarithmic_zero_epsilon) ?
                                 ((v_min < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) :
                                 (FLOATTYPE)v_min;
      FLOATTYPE v_max_fudged = (ImAbs((FLOATTYPE)v_max) < logarithmic_zero_epsilon) ?
                                 ((v_max < 0.0f) ? -logarithmic_zero_epsilon : logarithmic_zero_epsilon) :
                                 (FLOATTYPE)v_max;

      if (flipped)
        ImSwap(v_min_fudged, v_max_fudged);

      // Awkward special case - we need ranges of the form (-100 .. 0) to convert to (-100 ..
      // -epsilon), not (-100 .. epsilon)
      if ((v_max == 0.0f) && (v_min < 0.0f))
        v_max_fudged = -logarithmic_zero_epsilon;

      float t_with_flip = flipped ? (1.0f - t) :
                                    t;  // t, but flipped if necessary to account for us flipping the range

      if ((v_min * v_max) < 0.0f)  // Range crosses zero, so we have to do this in two parts
      {
        float zero_point_center = (-(float)ImMin(v_min, v_max)) /
                                  ImAbs((float)v_max - (float)v_min);  // The zero point in parametric space
        float zero_point_snap_L = zero_point_center - zero_deadzone_halfsize;
        float zero_point_snap_R = zero_point_center + zero_deadzone_halfsize;
        if (t_with_flip >= zero_point_snap_L && t_with_flip <= zero_point_snap_R)
          result = (TYPE)0.0f;  // Special case to make getting exactly zero possible (the epsilon
                                // prevents it otherwise)
        else if (t_with_flip < zero_point_center)
          result = (TYPE) -
                   (logarithmic_zero_epsilon * ImPow(-v_min_fudged / logarithmic_zero_epsilon,
                                                     (FLOATTYPE)(1.0f - (t_with_flip / zero_point_snap_L))));
        else
          result = (TYPE)(logarithmic_zero_epsilon * ImPow(v_max_fudged / logarithmic_zero_epsilon,
                                                           (FLOATTYPE)((t_with_flip - zero_point_snap_R) /
                                                                       (1.0f - zero_point_snap_R))));
      }
      else if ((v_min < 0.0f) || (v_max < 0.0f))  // Entirely negative slider
        result = (TYPE) -
                 (-v_max_fudged * ImPow(-v_min_fudged / -v_max_fudged, (FLOATTYPE)(1.0f - t_with_flip)));
      else
        result = (TYPE)(v_min_fudged * ImPow(v_max_fudged / v_min_fudged, (FLOATTYPE)t_with_flip));
    }
  }
  else {
    // Linear slider
    if (is_floating_point) {
      result = ImLerp(v_min, v_max, t);
    }
    else {
      // - For integer values we want the clicking position to match the grab box so we round above
      //   This code is carefully tuned to work with large values (e.g. high ranges of U64) while
      //   preserving this property..
      // - Not doing a *1.0 multiply at the end of a range as it tends to be lossy. While absolute
      // aiming at a large s64/u64
      //   range is going to be imprecise anyway, with this check we at least make the edge values
      //   matches expected limits.
      if (t < 1.0) {
        FLOATTYPE v_new_off_f = (SIGNEDTYPE)(v_max - v_min) * t;
        result = (TYPE)((SIGNEDTYPE)v_min +
                        (SIGNEDTYPE)(v_new_off_f + (FLOATTYPE)(v_min > v_max ? -0.5 : 0.5)));
      }
      else {
        result = v_max;
      }
    }
  }

  return result;
}

// FIXME: Move more of the code into SliderBehavior()
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool ANCHOR::SliderBehaviorT(const ImRect &bb,
                             ANCHOR_ID id,
                             ANCHOR_DataType data_type,
                             TYPE *v,
                             const TYPE v_min,
                             const TYPE v_max,
                             const char *format,
                             ANCHOR_SliderFlags flags,
                             ImRect *out_grab_bb)
{
  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;

  const ANCHOR_Axis axis = (flags & ANCHOR_SliderFlags_Vertical) ? ANCHOR_Axis_Y : ANCHOR_Axis_X;
  const bool is_logarithmic = (flags & ANCHOR_SliderFlags_Logarithmic) != 0;
  const bool is_floating_point = (data_type == ANCHOR_DataType_Float) ||
                                 (data_type == ANCHOR_DataType_Double);

  const float grab_padding = 2.0f;
  const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
  float grab_sz = style.GrabMinSize;
  SIGNEDTYPE v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);
  if (!is_floating_point && v_range >= 0)  // v_range < 0 may happen on integer overflows
    grab_sz = AnchorMax(
      (float)(slider_sz / (v_range + 1)),
      style.GrabMinSize);  // For integer sliders: if possible have the grab size represent 1 unit
  grab_sz = ImMin(grab_sz, slider_sz);
  const float slider_usable_sz = slider_sz - grab_sz;
  const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
  const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;

  float logarithmic_zero_epsilon = 0.0f;  // Only valid when is_logarithmic is true
  float zero_deadzone_halfsize = 0.0f;    // Only valid when is_logarithmic is true
  if (is_logarithmic) {
    // When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of
    // clamp value greatly affects slider precision. We attempt to use the specified precision to
    // estimate a good lower bound.
    const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 1;
    logarithmic_zero_epsilon = ImPow(0.1f, (float)decimal_precision);
    zero_deadzone_halfsize = (style.LogSliderDeadzone * 0.5f) / AnchorMax(slider_usable_sz, 1.0f);
  }

  // Process interacting with the slider
  bool value_changed = false;
  if (g.ActiveId == id) {
    bool set_new_value = false;
    float clicked_t = 0.0f;
    if (g.ActiveIdSource == ANCHORInputSource_Mouse) {
      if (!g.IO.MouseDown[0]) {
        ClearActiveID();
      }
      else {
        const float mouse_abs_pos = g.IO.MousePos[axis];
        clicked_t = (slider_usable_sz > 0.0f) ?
                      ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) :
                      0.0f;
        if (axis == ANCHOR_Axis_Y)
          clicked_t = 1.0f - clicked_t;
        set_new_value = true;
      }
    }
    else if (g.ActiveIdSource == ANCHORInputSource_Nav) {
      if (g.ActiveIdIsJustActivated) {
        g.SliderCurrentAccum = 0.0f;  // Reset any stored nav delta upon activation
        g.SliderCurrentAccumDirty = false;
      }

      const GfVec2f input_delta2 = GetNavInputAmount2d(ANCHOR_NavDirSourceFlags_Keyboard |
                                                         ANCHOR_NavDirSourceFlags_PadDPad,
                                                       ANCHOR_InputReadMode_RepeatFast,
                                                       0.0f,
                                                       0.0f);
      float input_delta = (axis == ANCHOR_Axis_X) ? input_delta2[0] : -input_delta2[1];
      if (input_delta != 0.0f) {
        const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 0;
        if (decimal_precision > 0) {
          input_delta /= 100.0f;  // Gamepad/keyboard tweak speeds in % of slider bounds
          if (IsNavInputDown(ANCHOR_NavInput_TweakSlow))
            input_delta /= 10.0f;
        }
        else {
          if ((v_range >= -100.0f && v_range <= 100.0f) || IsNavInputDown(ANCHOR_NavInput_TweakSlow))
            input_delta = ((input_delta < 0.0f) ? -1.0f : +1.0f) /
                          (float)v_range;  // Gamepad/keyboard tweak speeds in integer steps
          else
            input_delta /= 100.0f;
        }
        if (IsNavInputDown(ANCHOR_NavInput_TweakFast))
          input_delta *= 10.0f;

        g.SliderCurrentAccum += input_delta;
        g.SliderCurrentAccumDirty = true;
      }

      float delta = g.SliderCurrentAccum;
      if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated) {
        ClearActiveID();
      }
      else if (g.SliderCurrentAccumDirty) {
        clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
          data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

        if ((clicked_t >= 1.0f && delta > 0.0f) ||
            (clicked_t <= 0.0f &&
             delta < 0.0f))  // This is to avoid applying the saturation when already past the limits
        {
          set_new_value = false;
          g.SliderCurrentAccum = 0.0f;  // If pushing up against the limits, don't continue to accumulate
        }
        else {
          set_new_value = true;
          float old_clicked_t = clicked_t;
          clicked_t = ImSaturate(clicked_t + delta);

          // Calculate what our "new" clicked_t will be, and thus how far we actually moved the
          // slider, and subtract this from the accumulator
          TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type,
                                                                         clicked_t,
                                                                         v_min,
                                                                         v_max,
                                                                         is_logarithmic,
                                                                         logarithmic_zero_epsilon,
                                                                         zero_deadzone_halfsize);
          if (!(flags & ANCHOR_SliderFlags_NoRoundToFormat))
            v_new = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_new);
          float new_clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type,
                                                                                  v_new,
                                                                                  v_min,
                                                                                  v_max,
                                                                                  is_logarithmic,
                                                                                  logarithmic_zero_epsilon,
                                                                                  zero_deadzone_halfsize);

          if (delta > 0)
            g.SliderCurrentAccum -= ImMin(new_clicked_t - old_clicked_t, delta);
          else
            g.SliderCurrentAccum -= AnchorMax(new_clicked_t - old_clicked_t, delta);
        }

        g.SliderCurrentAccumDirty = false;
      }
    }

    if (set_new_value) {
      TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type,
                                                                     clicked_t,
                                                                     v_min,
                                                                     v_max,
                                                                     is_logarithmic,
                                                                     logarithmic_zero_epsilon,
                                                                     zero_deadzone_halfsize);

      // Round to user desired precision based on format string
      if (!(flags & ANCHOR_SliderFlags_NoRoundToFormat))
        v_new = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_new);

      // Apply result
      if (*v != v_new) {
        *v = v_new;
        value_changed = true;
      }
    }
  }

  if (slider_sz < 1.0f) {
    *out_grab_bb = ImRect(bb.Min, bb.Min);
  }
  else {
    // Output grab position so it can be displayed by the caller
    float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(
      data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
    if (axis == ANCHOR_Axis_Y)
      grab_t = 1.0f - grab_t;
    const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
    if (axis == ANCHOR_Axis_X)
      *out_grab_bb = ImRect(grab_pos - grab_sz * 0.5f,
                            bb.Min[1] + grab_padding,
                            grab_pos + grab_sz * 0.5f,
                            bb.Max[1] - grab_padding);
    else
      *out_grab_bb = ImRect(bb.Min[0] + grab_padding,
                            grab_pos - grab_sz * 0.5f,
                            bb.Max[0] - grab_padding,
                            grab_pos + grab_sz * 0.5f);
  }

  return value_changed;
}

// For 32-bit and larger types, slider bounds are limited to half the natural type range.
// So e.g. an integer Slider between INT_MAX-10 and INT_MAX will fail, but an integer Slider
// between INT_MAX/2-10 and INT_MAX/2 will be ok. It would be possible to lift that limitation with
// some work but it doesn't seem to be worth it for sliders.
bool ANCHOR::SliderBehavior(const ImRect &bb,
                            ANCHOR_ID id,
                            ANCHOR_DataType data_type,
                            void *p_v,
                            const void *p_min,
                            const void *p_max,
                            const char *format,
                            ANCHOR_SliderFlags flags,
                            ImRect *out_grab_bb)
{
  // Read ANCHOR.cpp "API BREAKING CHANGES" section for 1.78 if you hit this assert.
  ANCHOR_ASSERT((flags == 1 || (flags & ANCHOR_SliderFlags_InvalidMask_) == 0) &&
                "Invalid ANCHOR_SliderFlags flag!  Has the 'float power' argument been mistakenly "
                "cast to flags? Call function with ANCHOR_SliderFlags_Logarithmic flags instead.");

  ANCHOR_Context &g = *G_CTX;
  if ((g.CurrentItemFlags & ANCHOR_ItemFlags_ReadOnly) || (flags & ANCHOR_SliderFlags_ReadOnly))
    return false;

  switch (data_type) {
    case ANCHOR_DataType_S8: {
      AnchorS32 v32 = (AnchorS32) * (AnchorS8 *)p_v;
      bool r = SliderBehaviorT<AnchorS32, AnchorS32, float>(bb,
                                                            id,
                                                            ANCHOR_DataType_S32,
                                                            &v32,
                                                            *(const AnchorS8 *)p_min,
                                                            *(const AnchorS8 *)p_max,
                                                            format,
                                                            flags,
                                                            out_grab_bb);
      if (r)
        *(AnchorS8 *)p_v = (AnchorS8)v32;
      return r;
    }
    case ANCHOR_DataType_U8: {
      AnchorU32 v32 = (AnchorU32) * (AnchorU8 *)p_v;
      bool r = SliderBehaviorT<AnchorU32, AnchorS32, float>(bb,
                                                            id,
                                                            ANCHOR_DataType_U32,
                                                            &v32,
                                                            *(const AnchorU8 *)p_min,
                                                            *(const AnchorU8 *)p_max,
                                                            format,
                                                            flags,
                                                            out_grab_bb);
      if (r)
        *(AnchorU8 *)p_v = (AnchorU8)v32;
      return r;
    }
    case ANCHOR_DataType_S16: {
      AnchorS32 v32 = (AnchorS32) * (AnchorS16 *)p_v;
      bool r = SliderBehaviorT<AnchorS32, AnchorS32, float>(bb,
                                                            id,
                                                            ANCHOR_DataType_S32,
                                                            &v32,
                                                            *(const AnchorS16 *)p_min,
                                                            *(const AnchorS16 *)p_max,
                                                            format,
                                                            flags,
                                                            out_grab_bb);
      if (r)
        *(AnchorS16 *)p_v = (AnchorS16)v32;
      return r;
    }
    case ANCHOR_DataType_U16: {
      AnchorU32 v32 = (AnchorU32) * (AnchorU16 *)p_v;
      bool r = SliderBehaviorT<AnchorU32, AnchorS32, float>(bb,
                                                            id,
                                                            ANCHOR_DataType_U32,
                                                            &v32,
                                                            *(const AnchorU16 *)p_min,
                                                            *(const AnchorU16 *)p_max,
                                                            format,
                                                            flags,
                                                            out_grab_bb);
      if (r)
        *(AnchorU16 *)p_v = (AnchorU16)v32;
      return r;
    }
    case ANCHOR_DataType_S32:
      ANCHOR_ASSERT(*(const AnchorS32 *)p_min >= IM_S32_MIN / 2 &&
                    *(const AnchorS32 *)p_max <= IM_S32_MAX / 2);
      return SliderBehaviorT<AnchorS32, AnchorS32, float>(bb,
                                                          id,
                                                          data_type,
                                                          (AnchorS32 *)p_v,
                                                          *(const AnchorS32 *)p_min,
                                                          *(const AnchorS32 *)p_max,
                                                          format,
                                                          flags,
                                                          out_grab_bb);
    case ANCHOR_DataType_U32:
      ANCHOR_ASSERT(*(const AnchorU32 *)p_max <= IM_U32_MAX / 2);
      return SliderBehaviorT<AnchorU32, AnchorS32, float>(bb,
                                                          id,
                                                          data_type,
                                                          (AnchorU32 *)p_v,
                                                          *(const AnchorU32 *)p_min,
                                                          *(const AnchorU32 *)p_max,
                                                          format,
                                                          flags,
                                                          out_grab_bb);
    case ANCHOR_DataType_S64:
      ANCHOR_ASSERT(*(const AnchorS64 *)p_min >= IM_S64_MIN / 2 &&
                    *(const AnchorS64 *)p_max <= IM_S64_MAX / 2);
      return SliderBehaviorT<AnchorS64, AnchorS64, double>(bb,
                                                           id,
                                                           data_type,
                                                           (AnchorS64 *)p_v,
                                                           *(const AnchorS64 *)p_min,
                                                           *(const AnchorS64 *)p_max,
                                                           format,
                                                           flags,
                                                           out_grab_bb);
    case ANCHOR_DataType_U64:
      ANCHOR_ASSERT(*(const AnchorU64 *)p_max <= IM_U64_MAX / 2);
      return SliderBehaviorT<AnchorU64, AnchorS64, double>(bb,
                                                           id,
                                                           data_type,
                                                           (AnchorU64 *)p_v,
                                                           *(const AnchorU64 *)p_min,
                                                           *(const AnchorU64 *)p_max,
                                                           format,
                                                           flags,
                                                           out_grab_bb);
    case ANCHOR_DataType_Float:
      ANCHOR_ASSERT(*(const float *)p_min >= -FLT_MAX / 2.0f && *(const float *)p_max <= FLT_MAX / 2.0f);
      return SliderBehaviorT<float, float, float>(bb,
                                                  id,
                                                  data_type,
                                                  (float *)p_v,
                                                  *(const float *)p_min,
                                                  *(const float *)p_max,
                                                  format,
                                                  flags,
                                                  out_grab_bb);
    case ANCHOR_DataType_Double:
      ANCHOR_ASSERT(*(const double *)p_min >= -DBL_MAX / 2.0f && *(const double *)p_max <= DBL_MAX / 2.0f);
      return SliderBehaviorT<double, double, double>(bb,
                                                     id,
                                                     data_type,
                                                     (double *)p_v,
                                                     *(const double *)p_min,
                                                     *(const double *)p_max,
                                                     format,
                                                     flags,
                                                     out_grab_bb);
    case ANCHOR_DataType_COUNT:
      break;
  }
  ANCHOR_ASSERT(0);
  return false;
}

// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the data. For a slider,
// they are all required. Read code of e.g. SliderFloat(), SliderInt() etc. or examples in
// 'Demo->Widgets->Data Types' to understand how to use this function directly.
bool ANCHOR::SliderScalar(const char *label,
                          ANCHOR_DataType data_type,
                          void *p_data,
                          const void *p_min,
                          const void *p_max,
                          const char *format,
                          ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  const float w = CalcItemWidth();

  const GfVec2f label_size = CalcTextSize(label, NULL, true);
  const ImRect frame_bb(window->DC.CursorPos,
                        window->DC.CursorPos + GfVec2f(w, label_size[1] + style.FramePadding[1] * 2.0f));
  const ImRect total_bb(
    frame_bb.Min,
    frame_bb.Max + GfVec2f(label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f, 0.0f));

  const bool temp_input_allowed = (flags & ANCHOR_SliderFlags_NoInput) == 0;
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ANCHOR_ItemAddFlags_Focusable : 0))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;
  else if (data_type == ANCHOR_DataType_S32 &&
           strcmp(format, "%d") != 0)  // (FIXME-LEGACY: Patch old "%.0f" format string to use
                                       // "%d", read function more details.)
    format = PatchFormatStringFloatToInt(format);

  // Tabbing or CTRL-clicking on Slider turns it into an input box
  const bool hovered = ItemHoverable(frame_bb, id);
  bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
  if (!temp_input_is_active) {
    const bool focus_requested = temp_input_allowed &&
                                 (window->DC.LastItemStatusFlags & ANCHOR_ItemStatusFlags_Focused) != 0;
    const bool clicked = (hovered && g.IO.MouseClicked[0]);
    if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id) {
      SetActiveID(id, window);
      SetFocusID(id, window);
      FocusWindow(window);
      g.ActiveIdUsingNavDirMask |= (1 << ANCHOR_Dir_Left) | (1 << ANCHOR_Dir_Right);
      if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
        temp_input_is_active = true;
    }
  }

  if (temp_input_is_active) {
    // Only clamp CTRL+Click input when ANCHOR_SliderFlags_AlwaysClamp is set
    const bool is_clamp_input = (flags & ANCHOR_SliderFlags_AlwaysClamp) != 0;
    return TempInputScalar(frame_bb,
                           id,
                           label,
                           data_type,
                           p_data,
                           format,
                           is_clamp_input ? p_min : NULL,
                           is_clamp_input ? p_max : NULL);
  }

  // Draw frame
  const AnchorU32 frame_col = GetColorU32(g.ActiveId == id  ? ANCHOR_Col_FrameBgActive :
                                          g.HoveredId == id ? ANCHOR_Col_FrameBgHovered :
                                                              ANCHOR_Col_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

  // Slider behavior
  ImRect grab_bb;
  const bool value_changed = SliderBehavior(
    frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
  if (value_changed)
    MarkItemEdited(id);

  // Render grab
  if (grab_bb.Max[0] > grab_bb.Min[0])
    window->DrawList->AddRectFilled(
      grab_bb.Min,
      grab_bb.Max,
      GetColorU32(g.ActiveId == id ? ANCHOR_Col_SliderGrabActive : ANCHOR_Col_SliderGrab),
      style.GrabRounding);

  // Display value using user-provided display format so user can add prefix/suffix/decorations to
  // the value.
  char value_buf[64];
  const char *value_buf_end = value_buf +
                              DataTypeFormatString(
                                value_buf, ANCHOR_ARRAYSIZE(value_buf), data_type, p_data, format);
  if (g.LogEnabled)
    LogSetNextTextDecoration("{", "}");
  RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, GfVec2f(0.5f, 0.5f));

  if (label_size[0] > 0.0f)
    RenderText(GfVec2f(frame_bb.Max[0] + style.ItemInnerSpacing[0], frame_bb.Min[1] + style.FramePadding[1]),
               label);

  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  return value_changed;
}

// Add multiple sliders on 1 line for compact edition of multiple components
bool ANCHOR::SliderScalarN(const char *label,
                           ANCHOR_DataType data_type,
                           void *v,
                           int components,
                           const void *v_min,
                           const void *v_max,
                           const char *format,
                           ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  bool value_changed = false;
  BeginGroup();
  PushID(label);
  PushMultiItemsWidths(components, CalcItemWidth());
  size_t type_size = GDataTypeInfo[data_type].Size;
  for (int i = 0; i < components; i++) {
    PushID(i);
    if (i > 0)
      SameLine(0, g.Style.ItemInnerSpacing[0]);
    value_changed |= SliderScalar("", data_type, v, v_min, v_max, format, flags);
    PopID();
    PopItemWidth();
    v = (void *)((char *)v + type_size);
  }
  PopID();

  const char *label_end = FindRenderedTextEnd(label);
  if (label != label_end) {
    SameLine(0, g.Style.ItemInnerSpacing[0]);
    TextEx(label, label_end);
  }

  EndGroup();
  return value_changed;
}

bool ANCHOR::SliderFloat(const char *label,
                         float *v,
                         float v_min,
                         float v_max,
                         const char *format,
                         ANCHOR_SliderFlags flags)
{
  return SliderScalar(label, ANCHOR_DataType_Float, v, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderFloat2(const char *label,
                          float v[2],
                          float v_min,
                          float v_max,
                          const char *format,
                          ANCHOR_SliderFlags flags)
{
  return SliderScalarN(label, ANCHOR_DataType_Float, v, 2, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderFloat3(const char *label,
                          float v[3],
                          float v_min,
                          float v_max,
                          const char *format,
                          ANCHOR_SliderFlags flags)
{
  return SliderScalarN(label, ANCHOR_DataType_Float, v, 3, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderFloat4(const char *label,
                          float v[4],
                          float v_min,
                          float v_max,
                          const char *format,
                          ANCHOR_SliderFlags flags)
{
  return SliderScalarN(label, ANCHOR_DataType_Float, v, 4, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderAngle(const char *label,
                         float *v_rad,
                         float v_degrees_min,
                         float v_degrees_max,
                         const char *format,
                         ANCHOR_SliderFlags flags)
{
  if (format == NULL)
    format = "%.0f deg";
  float v_deg = (*v_rad) * 360.0f / (2 * IM_PI);
  bool value_changed = SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, format, flags);
  *v_rad = v_deg * (2 * IM_PI) / 360.0f;
  return value_changed;
}

bool ANCHOR::SliderInt(const char *label,
                       int *v,
                       int v_min,
                       int v_max,
                       const char *format,
                       ANCHOR_SliderFlags flags)
{
  return SliderScalar(label, ANCHOR_DataType_S32, v, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderInt2(const char *label,
                        int v[2],
                        int v_min,
                        int v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return SliderScalarN(label, ANCHOR_DataType_S32, v, 2, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderInt3(const char *label,
                        int v[3],
                        int v_min,
                        int v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return SliderScalarN(label, ANCHOR_DataType_S32, v, 3, &v_min, &v_max, format, flags);
}

bool ANCHOR::SliderInt4(const char *label,
                        int v[4],
                        int v_min,
                        int v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return SliderScalarN(label, ANCHOR_DataType_S32, v, 4, &v_min, &v_max, format, flags);
}

bool ANCHOR::VSliderScalar(const char *label,
                           const GfVec2f &size,
                           ANCHOR_DataType data_type,
                           void *p_data,
                           const void *p_min,
                           const void *p_max,
                           const char *format,
                           ANCHOR_SliderFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);

  const GfVec2f label_size = CalcTextSize(label, NULL, true);
  const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
  const ImRect bb(frame_bb.Min,
                  frame_bb.Max +
                    GfVec2f(label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f, 0.0f));

  ItemSize(bb, style.FramePadding[1]);
  if (!ItemAdd(frame_bb, id))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;
  else if (data_type == ANCHOR_DataType_S32 &&
           strcmp(format, "%d") != 0)  // (FIXME-LEGACY: Patch old "%.0f" format string to use
                                       // "%d", read function more details.)
    format = PatchFormatStringFloatToInt(format);

  const bool hovered = ItemHoverable(frame_bb, id);
  if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputId == id) {
    SetActiveID(id, window);
    SetFocusID(id, window);
    FocusWindow(window);
    g.ActiveIdUsingNavDirMask |= (1 << ANCHOR_Dir_Up) | (1 << ANCHOR_Dir_Down);
  }

  // Draw frame
  const AnchorU32 frame_col = GetColorU32(g.ActiveId == id  ? ANCHOR_Col_FrameBgActive :
                                          g.HoveredId == id ? ANCHOR_Col_FrameBgHovered :
                                                              ANCHOR_Col_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

  // Slider behavior
  ImRect grab_bb;
  const bool value_changed = SliderBehavior(
    frame_bb, id, data_type, p_data, p_min, p_max, format, flags | ANCHOR_SliderFlags_Vertical, &grab_bb);
  if (value_changed)
    MarkItemEdited(id);

  // Render grab
  if (grab_bb.Max[1] > grab_bb.Min[1])
    window->DrawList->AddRectFilled(
      grab_bb.Min,
      grab_bb.Max,
      GetColorU32(g.ActiveId == id ? ANCHOR_Col_SliderGrabActive : ANCHOR_Col_SliderGrab),
      style.GrabRounding);

  // Display value using user-provided display format so user can add prefix/suffix/decorations to
  // the value. For the vertical slider we allow centered text to overlap the frame padding
  char value_buf[64];
  const char *value_buf_end = value_buf +
                              DataTypeFormatString(
                                value_buf, ANCHOR_ARRAYSIZE(value_buf), data_type, p_data, format);
  RenderTextClipped(GfVec2f(frame_bb.Min[0], frame_bb.Min[1] + style.FramePadding[1]),
                    frame_bb.Max,
                    value_buf,
                    value_buf_end,
                    NULL,
                    GfVec2f(0.5f, 0.0f));
  if (label_size[0] > 0.0f)
    RenderText(GfVec2f(frame_bb.Max[0] + style.ItemInnerSpacing[0], frame_bb.Min[1] + style.FramePadding[1]),
               label);

  return value_changed;
}

bool ANCHOR::VSliderFloat(const char *label,
                          const GfVec2f &size,
                          float *v,
                          float v_min,
                          float v_max,
                          const char *format,
                          ANCHOR_SliderFlags flags)
{
  return VSliderScalar(label, size, ANCHOR_DataType_Float, v, &v_min, &v_max, format, flags);
}

bool ANCHOR::VSliderInt(const char *label,
                        const GfVec2f &size,
                        int *v,
                        int v_min,
                        int v_max,
                        const char *format,
                        ANCHOR_SliderFlags flags)
{
  return VSliderScalar(label, size, ANCHOR_DataType_S32, v, &v_min, &v_max, format, flags);
}

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS

// Obsolete versions with power parameter. See https://github.com/ocornut/ANCHOR/issues/3361 for
// details.
bool ANCHOR::SliderScalar(const char *label,
                          ANCHOR_DataType data_type,
                          void *p_data,
                          const void *p_min,
                          const void *p_max,
                          const char *format,
                          float power)
{
  ANCHOR_SliderFlags slider_flags = ANCHOR_SliderFlags_None;
  if (power != 1.0f) {
    ANCHOR_ASSERT(power == 1.0f &&
                  "Call function with ANCHOR_SliderFlags_Logarithmic flags instead of using the "
                  "old 'float power' function!");
    slider_flags |= ANCHOR_SliderFlags_Logarithmic;  // Fallback for non-asserting paths
  }
  return SliderScalar(label, data_type, p_data, p_min, p_max, format, slider_flags);
}

bool ANCHOR::SliderScalarN(const char *label,
                           ANCHOR_DataType data_type,
                           void *v,
                           int components,
                           const void *v_min,
                           const void *v_max,
                           const char *format,
                           float power)
{
  ANCHOR_SliderFlags slider_flags = ANCHOR_SliderFlags_None;
  if (power != 1.0f) {
    ANCHOR_ASSERT(power == 1.0f &&
                  "Call function with ANCHOR_SliderFlags_Logarithmic flags instead of using the "
                  "old 'float power' function!");
    slider_flags |= ANCHOR_SliderFlags_Logarithmic;  // Fallback for non-asserting paths
  }
  return SliderScalarN(label, data_type, v, components, v_min, v_max, format, slider_flags);
}

#  endif  // ANCHOR_DISABLE_OBSOLETE_FUNCTIONS

//-------------------------------------------------------------------------
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
//-------------------------------------------------------------------------
// - ImParseFormatFindStart() [Internal]
// - ImParseFormatFindEnd() [Internal]
// - ImParseFormatTrimDecorations() [Internal]
// - ImParseFormatPrecision() [Internal]
// - TempInputTextScalar() [Internal]
// - InputScalar()
// - InputScalarN()
// - InputFloat()
// - InputFloat2()
// - InputFloat3()
// - InputFloat4()
// - InputInt()
// - InputInt2()
// - InputInt3()
// - InputInt4()
// - InputDouble()
//-------------------------------------------------------------------------

// We don't use strchr() because our strings are usually very short and often start with '%'
const char *ImParseFormatFindStart(const char *fmt)
{
  while (char c = fmt[0]) {
    if (c == '%' && fmt[1] != '%')
      return fmt;
    else if (c == '%')
      fmt++;
    fmt++;
  }
  return fmt;
}

const char *ImParseFormatFindEnd(const char *fmt)
{
  // Printf/scanf types modifiers: I/L/h/j/l/t/w/z. Other uppercase letters qualify as types aka
  // end of the format.
  if (fmt[0] != '%')
    return fmt;
  const unsigned int ignored_uppercase_mask = (1 << ('I' - 'A')) | (1 << ('L' - 'A'));
  const unsigned int ignored_lowercase_mask = (1 << ('h' - 'a')) | (1 << ('j' - 'a')) | (1 << ('l' - 'a')) |
                                              (1 << ('t' - 'a')) | (1 << ('w' - 'a')) | (1 << ('z' - 'a'));
  for (char c; (c = *fmt) != 0; fmt++) {
    if (c >= 'A' && c <= 'Z' && ((1 << (c - 'A')) & ignored_uppercase_mask) == 0)
      return fmt + 1;
    if (c >= 'a' && c <= 'z' && ((1 << (c - 'a')) & ignored_lowercase_mask) == 0)
      return fmt + 1;
  }
  return fmt;
}

// Extract the format out of a format string with leading or trailing decorations
//  fmt = "blah blah"  -> return fmt
//  fmt = "%.3f"       -> return fmt
//  fmt = "hello %.3f" -> return fmt + 6
//  fmt = "%.3f hello" -> return buf written with "%.3f"
const char *ImParseFormatTrimDecorations(const char *fmt, char *buf, size_t buf_size)
{
  const char *fmt_start = ImParseFormatFindStart(fmt);
  if (fmt_start[0] != '%')
    return fmt;
  const char *fmt_end = ImParseFormatFindEnd(fmt_start);
  if (fmt_end[0] == 0)  // If we only have leading decoration, we don't need to copy the data.
    return fmt_start;
  ImStrncpy(buf, fmt_start, ImMin((size_t)(fmt_end - fmt_start) + 1, buf_size));
  return buf;
}

// Parse display precision back from the display format string
// FIXME: This is still used by some navigation code path to infer a minimum tweak step, but we
// should aim to rework widgets so it isn't needed.
int ImParseFormatPrecision(const char *fmt, int default_precision)
{
  fmt = ImParseFormatFindStart(fmt);
  if (fmt[0] != '%')
    return default_precision;
  fmt++;
  while (*fmt >= '0' && *fmt <= '9')
    fmt++;
  int precision = INT_MAX;
  if (*fmt == '.') {
    fmt = ImAtoi<int>(fmt + 1, &precision);
    if (precision < 0 || precision > 99)
      precision = default_precision;
  }
  if (*fmt == 'e' || *fmt == 'E')  // Maximum precision with scientific notation
    precision = -1;
  if ((*fmt == 'g' || *fmt == 'G') && precision == INT_MAX)
    precision = -1;
  return (precision == INT_MAX) ? default_precision : precision;
}

// Create text input in place of another active widget (e.g. used when doing a CTRL+Click on
// drag/slider widgets)
// FIXME: Facilitate using this in variety of other situations.
bool ANCHOR::TempInputText(const ImRect &bb,
                           ANCHOR_ID id,
                           const char *label,
                           char *buf,
                           int buf_size,
                           ANCHORInputTextFlags flags)
{
  // On the first frame, g.TempInputTextId == 0, then on subsequent frames it becomes == id.
  // We clear ActiveID on the first frame to allow the InputText() taking it back.
  ANCHOR_Context &g = *G_CTX;
  const bool init = (g.TempInputId != id);
  if (init)
    ClearActiveID();

  g.CurrentWindow->DC.CursorPos = bb.Min;
  bool value_changed = InputTextEx(
    label, NULL, buf, buf_size, bb.GetSize(), flags | ANCHORInputTextFlags_MergedItem);
  if (init) {
    // First frame we started displaying the InputText widget, we expect it to take the active id.
    ANCHOR_ASSERT(g.ActiveId == id);
    g.TempInputId = g.ActiveId;
  }
  return value_changed;
}

// Note that Drag/Slider functions are only forwarding the min/max values clamping values if the
// ANCHOR_SliderFlags_AlwaysClamp flag is set! This is intended: this way we allow CTRL+Click
// manual input to set a value out of bounds, for maximum flexibility. However this may not be
// ideal for all uses, as some user code may break on out of bound values.
bool ANCHOR::TempInputScalar(const ImRect &bb,
                             ANCHOR_ID id,
                             const char *label,
                             ANCHOR_DataType data_type,
                             void *p_data,
                             const char *format,
                             const void *p_clamp_min,
                             const void *p_clamp_max)
{
  ANCHOR_Context &g = *G_CTX;

  char fmt_buf[32];
  char data_buf[32];
  format = ImParseFormatTrimDecorations(format, fmt_buf, ANCHOR_ARRAYSIZE(fmt_buf));
  DataTypeFormatString(data_buf, ANCHOR_ARRAYSIZE(data_buf), data_type, p_data, format);
  ImStrTrimBlanks(data_buf);

  ANCHORInputTextFlags flags = ANCHORInputTextFlags_AutoSelectAll | ANCHORInputTextFlags_NoMarkEdited;
  flags |= ((data_type == ANCHOR_DataType_Float || data_type == ANCHOR_DataType_Double) ?
              ANCHORInputTextFlags_CharsScientific :
              ANCHORInputTextFlags_CharsDecimal);
  bool value_changed = false;
  if (TempInputText(bb, id, label, data_buf, ANCHOR_ARRAYSIZE(data_buf), flags)) {
    // Backup old value
    size_t data_type_size = DataTypeGetInfo(data_type)->Size;
    ANCHOR_DataTypeTempStorage data_backup;
    memcpy(&data_backup, p_data, data_type_size);

    // Apply new value (or operations) then clamp
    DataTypeApplyOpFromText(data_buf, g.InputTextState.InitialTextA.Data, data_type, p_data, NULL);
    if (p_clamp_min || p_clamp_max) {
      if (p_clamp_min && p_clamp_max && DataTypeCompare(data_type, p_clamp_min, p_clamp_max) > 0)
        ImSwap(p_clamp_min, p_clamp_max);
      DataTypeClamp(data_type, p_data, p_clamp_min, p_clamp_max);
    }

    // Only mark as edited if new value is different
    value_changed = memcmp(&data_backup, p_data, data_type_size) != 0;
    if (value_changed)
      MarkItemEdited(id);
  }
  return value_changed;
}

// Note: p_data, p_step, p_step_fast are _pointers_ to a memory address holding the data. For an
// Input widget, p_step and p_step_fast are optional. Read code of e.g. InputFloat(), InputInt()
// etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
bool ANCHOR::InputScalar(const char *label,
                         ANCHOR_DataType data_type,
                         void *p_data,
                         const void *p_step,
                         const void *p_step_fast,
                         const char *format,
                         ANCHORInputTextFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Style &style = g.Style;

  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;

  char buf[64];
  DataTypeFormatString(buf, ANCHOR_ARRAYSIZE(buf), data_type, p_data, format);

  bool value_changed = false;
  if ((flags & (ANCHORInputTextFlags_CharsHexadecimal | ANCHORInputTextFlags_CharsScientific)) == 0)
    flags |= ANCHORInputTextFlags_CharsDecimal;
  flags |= ANCHORInputTextFlags_AutoSelectAll;
  flags |= ANCHORInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselves by comparing
                                               // the actual data rather than the string.

  if (p_step != NULL) {
    const float button_size = GetFrameHeight();

    BeginGroup();  // The only purpose of the group here is to allow the caller to query item data
                   // e.g. IsItemActive()
    PushID(label);
    SetNextItemWidth(AnchorMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing[0]) * 2));
    if (InputText("",
                  buf,
                  ANCHOR_ARRAYSIZE(buf),
                  flags))  // PushId(label) + "" gives us the expected ID from outside point of view
      value_changed = DataTypeApplyOpFromText(
        buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);

    // Step buttons
    const GfVec2f backup_frame_padding = style.FramePadding;
    style.FramePadding[0] = style.FramePadding[1];
    ANCHOR_ButtonFlags button_flags = ANCHOR_ButtonFlags_Repeat | ANCHOR_ButtonFlags_DontClosePopups;
    if (flags & ANCHORInputTextFlags_ReadOnly)
      button_flags |= ANCHOR_ButtonFlags_Disabled;
    SameLine(0, style.ItemInnerSpacing[0]);
    if (ButtonEx("-", GfVec2f(button_size, button_size), button_flags)) {
      DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
      value_changed = true;
    }
    SameLine(0, style.ItemInnerSpacing[0]);
    if (ButtonEx("+", GfVec2f(button_size, button_size), button_flags)) {
      DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
      value_changed = true;
    }

    const char *label_end = FindRenderedTextEnd(label);
    if (label != label_end) {
      SameLine(0, style.ItemInnerSpacing[0]);
      TextEx(label, label_end);
    }
    style.FramePadding = backup_frame_padding;

    PopID();
    EndGroup();
  }
  else {
    if (InputText(label, buf, ANCHOR_ARRAYSIZE(buf), flags))
      value_changed = DataTypeApplyOpFromText(
        buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
  }
  if (value_changed)
    MarkItemEdited(window->DC.LastItemId);

  return value_changed;
}

bool ANCHOR::InputScalarN(const char *label,
                          ANCHOR_DataType data_type,
                          void *p_data,
                          int components,
                          const void *p_step,
                          const void *p_step_fast,
                          const char *format,
                          ANCHORInputTextFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  bool value_changed = false;
  BeginGroup();
  PushID(label);
  PushMultiItemsWidths(components, CalcItemWidth());
  size_t type_size = GDataTypeInfo[data_type].Size;
  for (int i = 0; i < components; i++) {
    PushID(i);
    if (i > 0)
      SameLine(0, g.Style.ItemInnerSpacing[0]);
    value_changed |= InputScalar("", data_type, p_data, p_step, p_step_fast, format, flags);
    PopID();
    PopItemWidth();
    p_data = (void *)((char *)p_data + type_size);
  }
  PopID();

  const char *label_end = FindRenderedTextEnd(label);
  if (label != label_end) {
    SameLine(0.0f, g.Style.ItemInnerSpacing[0]);
    TextEx(label, label_end);
  }

  EndGroup();
  return value_changed;
}

bool ANCHOR::InputFloat(const char *label,
                        float *v,
                        float step,
                        float step_fast,
                        const char *format,
                        ANCHORInputTextFlags flags)
{
  flags |= ANCHORInputTextFlags_CharsScientific;
  return InputScalar(label,
                     ANCHOR_DataType_Float,
                     (void *)v,
                     (void *)(step > 0.0f ? &step : NULL),
                     (void *)(step_fast > 0.0f ? &step_fast : NULL),
                     format,
                     flags);
}

bool ANCHOR::InputFloat2(const char *label, float v[2], const char *format, ANCHORInputTextFlags flags)
{
  return InputScalarN(label, ANCHOR_DataType_Float, v, 2, NULL, NULL, format, flags);
}

bool ANCHOR::InputFloat3(const char *label, float v[3], const char *format, ANCHORInputTextFlags flags)
{
  return InputScalarN(label, ANCHOR_DataType_Float, v, 3, NULL, NULL, format, flags);
}

bool ANCHOR::InputFloat4(const char *label, float v[4], const char *format, ANCHORInputTextFlags flags)
{
  return InputScalarN(label, ANCHOR_DataType_Float, v, 4, NULL, NULL, format, flags);
}

bool ANCHOR::InputInt(const char *label, int *v, int step, int step_fast, ANCHORInputTextFlags flags)
{
  // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use
  // InputText() to parse your own data, if you want to handle prefixes.
  const char *format = (flags & ANCHORInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
  return InputScalar(label,
                     ANCHOR_DataType_S32,
                     (void *)v,
                     (void *)(step > 0 ? &step : NULL),
                     (void *)(step_fast > 0 ? &step_fast : NULL),
                     format,
                     flags);
}

bool ANCHOR::InputInt2(const char *label, int v[2], ANCHORInputTextFlags flags)
{
  return InputScalarN(label, ANCHOR_DataType_S32, v, 2, NULL, NULL, "%d", flags);
}

bool ANCHOR::InputInt3(const char *label, int v[3], ANCHORInputTextFlags flags)
{
  return InputScalarN(label, ANCHOR_DataType_S32, v, 3, NULL, NULL, "%d", flags);
}

bool ANCHOR::InputInt4(const char *label, int v[4], ANCHORInputTextFlags flags)
{
  return InputScalarN(label, ANCHOR_DataType_S32, v, 4, NULL, NULL, "%d", flags);
}

bool ANCHOR::InputDouble(const char *label,
                         double *v,
                         double step,
                         double step_fast,
                         const char *format,
                         ANCHORInputTextFlags flags)
{
  flags |= ANCHORInputTextFlags_CharsScientific;
  return InputScalar(label,
                     ANCHOR_DataType_Double,
                     (void *)v,
                     (void *)(step > 0.0 ? &step : NULL),
                     (void *)(step_fast > 0.0 ? &step_fast : NULL),
                     format,
                     flags);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: InputText, InputTextMultiline, InputTextWithHint
//-------------------------------------------------------------------------
// - InputText()
// - InputTextWithHint()
// - InputTextMultiline()
// - InputTextEx() [Internal]
//-------------------------------------------------------------------------

bool ANCHOR::InputText(const char *label,
                       char *buf,
                       size_t buf_size,
                       ANCHORInputTextFlags flags,
                       ANCHORInputTextCallback callback,
                       void *user_data)
{
  ANCHOR_ASSERT(!(flags & ANCHORInputTextFlags_Multiline));  // call InputTextMultiline()
  return InputTextEx(label, NULL, buf, (int)buf_size, GfVec2f(0, 0), flags, callback, user_data);
}

bool ANCHOR::InputTextMultiline(const char *label,
                                char *buf,
                                size_t buf_size,
                                const GfVec2f &size,
                                ANCHORInputTextFlags flags,
                                ANCHORInputTextCallback callback,
                                void *user_data)
{
  return InputTextEx(
    label, NULL, buf, (int)buf_size, size, flags | ANCHORInputTextFlags_Multiline, callback, user_data);
}

bool ANCHOR::InputTextWithHint(const char *label,
                               const char *hint,
                               char *buf,
                               size_t buf_size,
                               ANCHORInputTextFlags flags,
                               ANCHORInputTextCallback callback,
                               void *user_data)
{
  ANCHOR_ASSERT(!(flags & ANCHORInputTextFlags_Multiline));  // call InputTextMultiline()
  return InputTextEx(label, hint, buf, (int)buf_size, GfVec2f(0, 0), flags, callback, user_data);
}

static int InputTextCalcTextLenAndLineCount(const char *text_begin, const char **out_text_end)
{
  int line_count = 0;
  const char *s = text_begin;
  while (char c = *s++)  // We are only matching for \n so we can ignore UTF-8 decoding
    if (c == '\n')
      line_count++;
  s--;
  if (s[0] != '\n' && s[0] != '\r')
    line_count++;
  *out_text_end = s;
  return line_count;
}

static GfVec2f InputTextCalcTextSizeW(const AnchorWChar *text_begin,
                                      const AnchorWChar *text_end,
                                      const AnchorWChar **remaining,
                                      GfVec2f *out_offset,
                                      bool stop_on_new_line)
{
  ANCHOR_Context &g = *G_CTX;
  AnchorFont *font = g.Font;
  const float line_height = g.FontSize;
  const float scale = line_height / font->FontSize;

  GfVec2f text_size = GfVec2f(0, 0);
  float line_width = 0.0f;

  const AnchorWChar *s = text_begin;
  while (s < text_end) {
    unsigned int c = (unsigned int)(*s++);
    if (c == '\n') {
      text_size[0] = AnchorMax(text_size[0], line_width);
      text_size[1] += line_height;
      line_width = 0.0f;
      if (stop_on_new_line)
        break;
      continue;
    }
    if (c == '\r')
      continue;

    const float char_width = font->GetCharAdvance((AnchorWChar)c) * scale;
    line_width += char_width;
  }

  if (text_size[0] < line_width)
    text_size[0] = line_width;

  if (out_offset)
    *out_offset = GfVec2f(line_width,
                          text_size[1] +
                            line_height);  // offset allow for the possibility of sitting after a trailing \n

  if (line_width > 0 || text_size[1] == 0.0f)  // whereas size[1] will ignore the trailing \n
    text_size[1] += line_height;

  if (remaining)
    *remaining = s;

  return text_size;
}

// Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer,
// single-line, wchar characters. InputText converts between UTF-8 and wchar)
namespace ImStb {

static int STB_TEXTEDIT_STRINGLEN(const ANCHOR_InputTextState *obj)
{
  return obj->CurLenW;
}
static AnchorWChar STB_TEXTEDIT_GETCHAR(const ANCHOR_InputTextState *obj, int idx)
{
  return obj->TextW[idx];
}
static float STB_TEXTEDIT_GETWIDTH(ANCHOR_InputTextState *obj, int line_start_idx, int char_idx)
{
  AnchorWChar c = obj->TextW[line_start_idx + char_idx];
  if (c == '\n')
    return STB_TEXTEDIT_GETWIDTH_NEWLINE;
  ANCHOR_Context &g = *G_CTX;
  return g.Font->GetCharAdvance(c) * (g.FontSize / g.Font->FontSize);
}
static int STB_TEXTEDIT_KEYTOTEXT(int key)
{
  return key >= 0x200000 ? 0 : key;
}
static AnchorWChar STB_TEXTEDIT_NEWLINE = '\n';
static void STB_TEXTEDIT_LAYOUTROW(StbTexteditRow *r, ANCHOR_InputTextState *obj, int line_start_idx)
{
  const AnchorWChar *text = obj->TextW.Data;
  const AnchorWChar *text_remaining = NULL;
  const GfVec2f size = InputTextCalcTextSizeW(
    text + line_start_idx, text + obj->CurLenW, &text_remaining, NULL, true);
  r->x0 = 0.0f;
  r->x1 = size[0];
  r->baseline_y_delta = size[1];
  r->ymin = 0.0f;
  r->ymax = size[1];
  r->num_chars = (int)(text_remaining - (text + line_start_idx));
}

// When ANCHORInputTextFlags_Password is set, we don't want actions such as CTRL+Arrow to leak the
// fact that underlying data are blanks or separators.
static bool is_separator(unsigned int c)
{
  return ImCharIsBlankW(c) || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' ||
         c == '[' || c == ']' || c == '|';
}
static int is_word_boundary_from_right(ANCHOR_InputTextState *obj, int idx)
{
  if (obj->Flags & ANCHORInputTextFlags_Password)
    return 0;
  return idx > 0 ? (is_separator(obj->TextW[idx - 1]) && !is_separator(obj->TextW[idx])) : 1;
}
static int STB_TEXTEDIT_MOVEWORDLEFT_IMPL(ANCHOR_InputTextState *obj, int idx)
{
  idx--;
  while (idx >= 0 && !is_word_boundary_from_right(obj, idx))
    idx--;
  return idx < 0 ? 0 : idx;
}
#  ifdef __APPLE__  // FIXME: Move setting to IO structure
static int is_word_boundary_from_left(ANCHOR_InputTextState *obj, int idx)
{
  if (obj->Flags & ANCHORInputTextFlags_Password)
    return 0;
  return idx > 0 ? (!is_separator(obj->TextW[idx - 1]) && is_separator(obj->TextW[idx])) : 1;
}
static int STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(ANCHOR_InputTextState *obj, int idx)
{
  idx++;
  int len = obj->CurLenW;
  while (idx < len && !is_word_boundary_from_left(obj, idx))
    idx++;
  return idx > len ? len : idx;
}
#  else
static int STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(ANCHOR_InputTextState *obj, int idx)
{
  idx++;
  int len = obj->CurLenW;
  while (idx < len && !is_word_boundary_from_right(obj, idx))
    idx++;
  return idx > len ? len : idx;
}
#  endif
#  define STB_TEXTEDIT_MOVEWORDLEFT \
    STB_TEXTEDIT_MOVEWORDLEFT_IMPL  // They need to be #define for stb_textedit.h
#  define STB_TEXTEDIT_MOVEWORDRIGHT STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

static void STB_TEXTEDIT_DELETECHARS(ANCHOR_InputTextState *obj, int pos, int n)
{
  AnchorWChar *dst = obj->TextW.Data + pos;

  // We maintain our buffer length in both UTF-8 and wchar formats
  obj->Edited = true;
  obj->CurLenA -= ImTextCountUtf8BytesFromStr(dst, dst + n);
  obj->CurLenW -= n;

  // Offset remaining text (FIXME-OPT: Use memmove)
  const AnchorWChar *src = obj->TextW.Data + pos + n;
  while (AnchorWChar c = *src++)
    *dst++ = c;
  *dst = '\0';
}

static bool STB_TEXTEDIT_INSERTCHARS(ANCHOR_InputTextState *obj,
                                     int pos,
                                     const AnchorWChar *new_text,
                                     int new_text_len)
{
  const bool is_resizable = (obj->Flags & ANCHORInputTextFlags_CallbackResize) != 0;
  const int text_len = obj->CurLenW;
  ANCHOR_ASSERT(pos <= text_len);

  const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
  if (!is_resizable && (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
    return false;

  // Grow internal buffer if needed
  if (new_text_len + text_len + 1 > obj->TextW.Size) {
    if (!is_resizable)
      return false;
    ANCHOR_ASSERT(text_len < obj->TextW.Size);
    obj->TextW.resize(text_len + ImClamp(new_text_len * 4, 32, AnchorMax(256, new_text_len)) + 1);
  }

  AnchorWChar *text = obj->TextW.Data;
  if (pos != text_len)
    memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(AnchorWChar));
  memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(AnchorWChar));

  obj->Edited = true;
  obj->CurLenW += new_text_len;
  obj->CurLenA += new_text_len_utf8;
  obj->TextW[obj->CurLenW] = '\0';

  return true;
}

// We don't use an enum so we can build even with conflicting symbols (if another user of
// stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
#  define STB_TEXTEDIT_K_LEFT 0x200000       // keyboard input to move cursor left
#  define STB_TEXTEDIT_K_RIGHT 0x200001      // keyboard input to move cursor right
#  define STB_TEXTEDIT_K_UP 0x200002         // keyboard input to move cursor up
#  define STB_TEXTEDIT_K_DOWN 0x200003       // keyboard input to move cursor down
#  define STB_TEXTEDIT_K_LINESTART 0x200004  // keyboard input to move cursor to start of line
#  define STB_TEXTEDIT_K_LINEEND 0x200005    // keyboard input to move cursor to end of line
#  define STB_TEXTEDIT_K_TEXTSTART 0x200006  // keyboard input to move cursor to start of text
#  define STB_TEXTEDIT_K_TEXTEND 0x200007    // keyboard input to move cursor to end of text
#  define STB_TEXTEDIT_K_DELETE 0x200008     // keyboard input to delete selection or character under cursor
#  define STB_TEXTEDIT_K_BACKSPACE \
    0x200009                                // keyboard input to delete selection or character left of cursor
#  define STB_TEXTEDIT_K_UNDO 0x20000A      // keyboard input to perform undo
#  define STB_TEXTEDIT_K_REDO 0x20000B      // keyboard input to perform redo
#  define STB_TEXTEDIT_K_WORDLEFT 0x20000C  // keyboard input to move cursor left one word
#  define STB_TEXTEDIT_K_WORDRIGHT 0x20000D  // keyboard input to move cursor right one word
#  define STB_TEXTEDIT_K_PGUP 0x20000E       // keyboard input to move cursor up a page
#  define STB_TEXTEDIT_K_PGDOWN 0x20000F     // keyboard input to move cursor down a page
#  define STB_TEXTEDIT_K_SHIFT 0x400000

#  define STB_TEXTEDIT_IMPLEMENTATION
#  include "ANCHOR_textedit.h"

// stb_textedit internally allows for a single undo record to do addition and deletion, but
// somehow, calling the stb_textedit_paste() function creates two separate records, so we perform
// it manually. (FIXME: Report to nothings/stb?)
static void stb_textedit_replace(ANCHOR_InputTextState *str,
                                 STB_TexteditState *state,
                                 const STB_TEXTEDIT_CHARTYPE *text,
                                 int text_len)
{
  stb_text_makeundo_replace(str, state, 0, str->CurLenW, text_len);
  ImStb::STB_TEXTEDIT_DELETECHARS(str, 0, str->CurLenW);
  if (text_len <= 0)
    return;
  if (ImStb::STB_TEXTEDIT_INSERTCHARS(str, 0, text, text_len)) {
    state->cursor = text_len;
    state->has_preferred_x = 0;
    return;
  }
  ANCHOR_ASSERT(0);  // Failed to insert character, normally shouldn't happen because of how we
                     // currently use stb_textedit_replace()
}

}  // namespace ImStb

void ANCHOR_InputTextState::OnKeyPressed(int key)
{
  stb_textedit_key(this, &Stb, key);
  CursorFollow = true;
  CursorAnimReset();
}

ANCHORInputTextCallbackData::ANCHORInputTextCallbackData()
{
  memset(this, 0, sizeof(*this));
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the STB_TEXTEDIT_* functions which are manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a nuisance.
void ANCHORInputTextCallbackData::DeleteChars(int pos, int bytes_count)
{
  ANCHOR_ASSERT(pos + bytes_count <= BufTextLen);
  char *dst = Buf + pos;
  const char *src = Buf + pos + bytes_count;
  while (char c = *src++)
    *dst++ = c;
  *dst = '\0';

  if (CursorPos >= pos + bytes_count)
    CursorPos -= bytes_count;
  else if (CursorPos >= pos)
    CursorPos = pos;
  SelectionStart = SelectionEnd = CursorPos;
  BufDirty = true;
  BufTextLen -= bytes_count;
}

void ANCHORInputTextCallbackData::InsertChars(int pos, const char *new_text, const char *new_text_end)
{
  const bool is_resizable = (Flags & ANCHORInputTextFlags_CallbackResize) != 0;
  const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
  if (new_text_len + BufTextLen >= BufSize) {
    if (!is_resizable)
      return;

    // Contrary to STB_TEXTEDIT_INSERTCHARS() this is working in the UTF8 buffer, hence the mildly
    // similar code (until we remove the U16 buffer altogether!)
    ANCHOR_Context &g = *G_CTX;
    ANCHOR_InputTextState *edit_state = &g.InputTextState;
    ANCHOR_ASSERT(edit_state->ID != 0 && g.ActiveId == edit_state->ID);
    ANCHOR_ASSERT(Buf == edit_state->TextA.Data);
    int new_buf_size = BufTextLen + ImClamp(new_text_len * 4, 32, AnchorMax(256, new_text_len)) + 1;
    edit_state->TextA.reserve(new_buf_size + 1);
    Buf = edit_state->TextA.Data;
    BufSize = edit_state->BufCapacityA = new_buf_size;
  }

  if (BufTextLen != pos)
    memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
  memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
  Buf[BufTextLen + new_text_len] = '\0';

  if (CursorPos >= pos)
    CursorPos += new_text_len;
  SelectionStart = SelectionEnd = CursorPos;
  BufDirty = true;
  BufTextLen += new_text_len;
}

// Return false to discard a character.
static bool InputTextFilterCharacter(unsigned int *p_char,
                                     ANCHORInputTextFlags flags,
                                     ANCHORInputTextCallback callback,
                                     void *user_data,
                                     ANCHORInputSource input_source)
{
  ANCHOR_ASSERT(input_source == ANCHORInputSource_Keyboard || input_source == ANCHORInputSource_Clipboard);
  unsigned int c = *p_char;

  // Filter non-printable (NB: isprint is unreliable! see #2467)
  if (c < 0x20) {
    bool pass = false;
    pass |= (c == '\n' && (flags & ANCHORInputTextFlags_Multiline));
    pass |= (c == '\t' && (flags & ANCHORInputTextFlags_AllowTabInput));
    if (!pass)
      return false;
  }

  if (input_source != ANCHORInputSource_Clipboard) {
    // We ignore Ascii representation of delete (emitted from Backspace on OSX, see #2578, #2817)
    if (c == 127)
      return false;

    // Filter private Unicode range. GLFW on OSX seems to send private characters for special keys
    // like arrow keys (FIXME)
    if (c >= 0xE000 && c <= 0xF8FF)
      return false;
  }

  // Filter Unicode ranges we are not handling in this build
  if (c > IM_UNICODE_CODEPOINT_MAX)
    return false;

  // Generic named filters
  if (flags & (ANCHORInputTextFlags_CharsDecimal | ANCHORInputTextFlags_CharsHexadecimal |
               ANCHORInputTextFlags_CharsUppercase | ANCHORInputTextFlags_CharsNoBlank |
               ANCHORInputTextFlags_CharsScientific)) {
    // The libc allows overriding locale, with e.g. 'setlocale(LC_NUMERIC, "de_DE.UTF-8");' which
    // affect the output/input of printf/scanf. The standard mandate that programs starts in the
    // "C" locale where the decimal point is '.'. We don't really intend to provide widespread
    // support for it, but out of empathy for people stuck with using odd API, we support the bare
    // minimum aka overriding the decimal point. Change the default decimal_point with:
    //   ANCHOR::GetCurrentContext()->PlatformLocaleDecimalPoint = *localeconv()->decimal_point;
    ANCHOR_Context &g = *G_CTX;
    const unsigned c_decimal_point = (unsigned int)g.PlatformLocaleDecimalPoint;

    // Allow 0-9 . - + * /
    if (flags & ANCHORInputTextFlags_CharsDecimal)
      if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') &&
          (c != '/'))
        return false;

    // Allow 0-9 . - + * / e E
    if (flags & ANCHORInputTextFlags_CharsScientific)
      if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') &&
          (c != '/') && (c != 'e') && (c != 'E'))
        return false;

    // Allow 0-9 a-F A-F
    if (flags & ANCHORInputTextFlags_CharsHexadecimal)
      if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
        return false;

    // Turn a-z into A-Z
    if (flags & ANCHORInputTextFlags_CharsUppercase)
      if (c >= 'a' && c <= 'z')
        *p_char = (c += (unsigned int)('A' - 'a'));

    if (flags & ANCHORInputTextFlags_CharsNoBlank)
      if (ImCharIsBlankW(c))
        return false;
  }

  // Custom callback filter
  if (flags & ANCHORInputTextFlags_CallbackCharFilter) {
    ANCHORInputTextCallbackData callback_data;
    memset(&callback_data, 0, sizeof(ANCHORInputTextCallbackData));
    callback_data.EventFlag = ANCHORInputTextFlags_CallbackCharFilter;
    callback_data.EventChar = (AnchorWChar)c;
    callback_data.Flags = flags;
    callback_data.UserData = user_data;
    if (callback(&callback_data) != 0)
      return false;
    *p_char = callback_data.EventChar;
    if (!callback_data.EventChar)
      return false;
  }

  return true;
}

// Edit a string of text
// - buf_size account for the zero-terminator, so a buf_size of 6 can hold "Hello" but not
// "Hello!".
//   This is so we can easily call InputText() on static arrays using ARRAYSIZE() and to match
//   Note that in std::string world, capacity() would omit 1 byte used by the zero-terminator.
// - When active, hold on a privately held copy of the text (and apply back to 'buf'). So changing
// 'buf' while the InputText is active has no effect.
// - If you want to use ANCHOR::InputText() with std::string, see misc/cpp/ANCHOR_stdlib.h
// (FIXME: Rather confusing and messy function, among the worse part of our codebase, expecting to
// rewrite a V2 at some point.. Partly because we are
//  doing UTF8 > U16 > UTF8 conversions on the go to easily interface with stb_textedit. Ideally
//  should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188)
bool ANCHOR::InputTextEx(const char *label,
                         const char *hint,
                         char *buf,
                         int buf_size,
                         const GfVec2f &size_arg,
                         ANCHORInputTextFlags flags,
                         ANCHORInputTextCallback callback,
                         void *callback_user_data)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_ASSERT(buf != NULL && buf_size >= 0);
  ANCHOR_ASSERT(
    !((flags & ANCHORInputTextFlags_CallbackHistory) &&
      (flags & ANCHORInputTextFlags_Multiline)));  // Can't use both together (they both use up/down keys)
  ANCHOR_ASSERT(
    !((flags & ANCHORInputTextFlags_CallbackCompletion) &&
      (flags & ANCHORInputTextFlags_AllowTabInput)));  // Can't use both together (they both use tab key)

  ANCHOR_Context &g = *G_CTX;
  ANCHOR_IO &io = g.IO;
  const ANCHOR_Style &style = g.Style;

  const bool RENDER_SELECTION_WHEN_INACTIVE = false;
  const bool is_multiline = (flags & ANCHORInputTextFlags_Multiline) != 0;
  const bool is_readonly = (flags & ANCHORInputTextFlags_ReadOnly) != 0;
  const bool is_password = (flags & ANCHORInputTextFlags_Password) != 0;
  const bool is_undoable = (flags & ANCHORInputTextFlags_NoUndoRedo) == 0;
  const bool is_resizable = (flags & ANCHORInputTextFlags_CallbackResize) != 0;
  if (is_resizable)
    ANCHOR_ASSERT(callback !=
                  NULL);  // Must provide a callback if you set the ANCHORInputTextFlags_CallbackResize flag!

  if (is_multiline)  // Open group before calling GetID() because groups tracks id created within
                     // their scope,
    BeginGroup();
  const ANCHOR_ID id = window->GetID(label);
  const GfVec2f label_size = CalcTextSize(label, NULL, true);
  const GfVec2f frame_size = CalcItemSize(size_arg,
                                          CalcItemWidth(),
                                          (is_multiline ? g.FontSize * 8.0f : label_size[1]) +
                                            style.FramePadding[1] *
                                              2.0f);  // Arbitrary default of 8 lines high for multi-line
  const GfVec2f total_size = GfVec2f(
    frame_size[0] + (label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f),
    frame_size[1]);

  const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
  const ImRect total_bb(frame_bb.Min, frame_bb.Min + total_size);

  ANCHOR_Window *draw_window = window;
  GfVec2f inner_size = frame_size;
  if (is_multiline) {
    if (!ItemAdd(total_bb, id, &frame_bb, ANCHOR_ItemAddFlags_Focusable)) {
      ItemSize(total_bb, style.FramePadding[1]);
      EndGroup();
      return false;
    }

    // We reproduce the contents of BeginChildFrame() in order to provide 'label' so our window
    // internal data are easier to read/debug.
    PushStyleColor(ANCHOR_Col_ChildBg, style.Colors[ANCHOR_Col_FrameBg]);
    PushStyleVar(ANCHOR_StyleVar_ChildRounding, style.FrameRounding);
    PushStyleVar(ANCHOR_StyleVar_ChildBorderSize, style.FrameBorderSize);
    bool child_visible = BeginChildEx(label, id, frame_bb.GetSize(), true, ANCHOR_WindowFlags_NoMove);
    PopStyleVar(2);
    PopStyleColor();
    if (!child_visible) {
      EndChild();
      EndGroup();
      return false;
    }
    draw_window = g.CurrentWindow;  // Child window
    draw_window->DC.NavLayersActiveMaskNext |=
      (1 << draw_window->DC.NavLayerCurrent);  // This is to ensure that EndChild() will display a
                                               // navigation highlight so we can "enter" into it.
    draw_window->DC.CursorPos += style.FramePadding;
    inner_size[0] -= draw_window->ScrollbarSizes[0];
  }
  else {
    // Support for internal ANCHORInputTextFlags_MergedItem flag, which could be redesigned as an
    // ItemFlags if needed (with test performed in ItemAdd)
    ItemSize(total_bb, style.FramePadding[1]);
    if (!(flags & ANCHORInputTextFlags_MergedItem))
      if (!ItemAdd(total_bb, id, &frame_bb, ANCHOR_ItemAddFlags_Focusable))
        return false;
  }
  const bool hovered = ItemHoverable(frame_bb, id);
  if (hovered)
    g.MouseCursor = ANCHOR_MouseCursor_TextInput;

  // We are only allowed to access the state if we are already the active widget.
  ANCHOR_InputTextState *state = GetInputTextState(id);

  const bool focus_requested_by_code = (window->DC.LastItemStatusFlags &
                                        ANCHOR_ItemStatusFlags_FocusedByCode) != 0;
  const bool focus_requested_by_tabbing = (window->DC.LastItemStatusFlags &
                                           ANCHOR_ItemStatusFlags_FocusedByTabbing) != 0;

  const bool user_clicked = hovered && io.MouseClicked[0];
  const bool user_nav_input_start = (g.ActiveId != id) && ((g.NavInputId == id) ||
                                                           (g.NavActivateId == id &&
                                                            g.NavInputSource == ANCHORInputSource_Keyboard));
  const bool user_scroll_finish = is_multiline && state != NULL && g.ActiveId == 0 &&
                                  g.ActiveIdPreviousFrame ==
                                    GetWindowScrollbarID(draw_window, ANCHOR_Axis_Y);
  const bool user_scroll_active = is_multiline && state != NULL &&
                                  g.ActiveId == GetWindowScrollbarID(draw_window, ANCHOR_Axis_Y);

  bool clear_active_id = false;
  bool select_all = (g.ActiveId != id) &&
                    ((flags & ANCHORInputTextFlags_AutoSelectAll) != 0 || user_nav_input_start) &&
                    (!is_multiline);

  float scroll_y = is_multiline ? draw_window->Scroll[1] : FLT_MAX;

  const bool init_changed_specs = (state != NULL && state->Stb.single_line != !is_multiline);
  const bool init_make_active = (user_clicked || user_scroll_finish || user_nav_input_start ||
                                 focus_requested_by_code || focus_requested_by_tabbing);
  const bool init_state = (init_make_active || user_scroll_active);
  if ((init_state && g.ActiveId != id) || init_changed_specs) {
    // Access state even if we don't own it yet.
    state = &g.InputTextState;
    state->CursorAnimReset();

    // Take a copy of the initial buffer value (both in original UTF-8 format and converted to
    // wchar) From the moment we focused we are ignoring the content of 'buf' (unless we are in
    // read-only mode)
    const int buf_len = (int)strlen(buf);
    state->InitialTextA.resize(buf_len + 1);  // UTF-8. we use +1 to make sure that .Data is always
                                              // pointing to at least an empty string.
    memcpy(state->InitialTextA.Data, buf, buf_len + 1);

    // Start edition
    const char *buf_end = NULL;
    state->TextW.resize(buf_size + 1);  // wchar count <= UTF-8 count. we use +1 to make sure that
                                        // .Data is always pointing to at least an empty string.
    state->TextA.resize(0);
    state->TextAIsValid = false;  // TextA is not valid yet (we will display buf until then)
    state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, buf_size, buf, NULL, &buf_end);
    state->CurLenA = (int)(buf_end - buf);  // We can't get the result from ImStrncpy() above because it is
                                            // not UTF-8 aware. Here we'll cut off malformed UTF-8.

    // Preserve cursor position and undo/redo stack if we come back to same widget
    // FIXME: For non-readonly widgets we might be able to require that TextAIsValid && TextA ==
    // buf ? (untested) and discard undo stack if user buffer has changed.
    const bool recycle_state = (state->ID == id && !init_changed_specs);
    if (recycle_state) {
      // Recycle existing cursor/selection/undo stack but clamp position
      // Note a single mouse click will override the cursor/position immediately by calling
      // stb_textedit_click handler.
      state->CursorClamp();
    }
    else {
      state->ID = id;
      state->ScrollX = 0.0f;
      stb_textedit_initialize_state(&state->Stb, !is_multiline);
      if (!is_multiline && focus_requested_by_code)
        select_all = true;
    }
    if (flags & ANCHORInputTextFlags_AlwaysOverwrite)
      state->Stb.insert_mode = 1;  // stb field name is indeed incorrect (see #2863)
    if (!is_multiline && (focus_requested_by_tabbing || (user_clicked && io.KeyCtrl)))
      select_all = true;
  }

  if (g.ActiveId != id && init_make_active) {
    ANCHOR_ASSERT(state && state->ID == id);
    SetActiveID(id, window);
    SetFocusID(id, window);
    FocusWindow(window);

    // Declare our inputs
    ANCHOR_ASSERT(ANCHOR_NavInput_COUNT < 32);
    g.ActiveIdUsingNavDirMask |= (1 << ANCHOR_Dir_Left) | (1 << ANCHOR_Dir_Right);
    if (is_multiline || (flags & ANCHORInputTextFlags_CallbackHistory))
      g.ActiveIdUsingNavDirMask |= (1 << ANCHOR_Dir_Up) | (1 << ANCHOR_Dir_Down);
    g.ActiveIdUsingNavInputMask |= (1 << ANCHOR_NavInput_Cancel);
    g.ActiveIdUsingKeyInputMask |= ((AnchorU64)1 << ANCHOR_Key_Home) | ((AnchorU64)1 << ANCHOR_Key_End);
    if (is_multiline)
      g.ActiveIdUsingKeyInputMask |= ((AnchorU64)1 << ANCHOR_Key_PageUp) |
                                     ((AnchorU64)1 << ANCHOR_Key_PageDown);
    if (flags & (ANCHORInputTextFlags_CallbackCompletion |
                 ANCHORInputTextFlags_AllowTabInput))  // Disable keyboard tabbing out as we will
                                                       // use the \t character.
      g.ActiveIdUsingKeyInputMask |= ((AnchorU64)1 << ANCHOR_Key_Tab);
  }

  // We have an edge case if ActiveId was set through another widget (e.g. widget being swapped),
  // clear id immediately (don't wait until the end of the function)
  if (g.ActiveId == id && state == NULL)
    ClearActiveID();

  // Release focus when we click outside
  if (g.ActiveId == id && io.MouseClicked[0] && !init_state && !init_make_active)  //-V560
    clear_active_id = true;

  // Lock the decision of whether we are going to take the path displaying the cursor or selection
  const bool render_cursor = (g.ActiveId == id) || (state && user_scroll_active);
  bool render_selection = state && state->HasSelection() &&
                          (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
  bool value_changed = false;
  bool enter_pressed = false;

  // When read-only we always use the live data passed to the function
  // FIXME-OPT: Because our selection/cursor code currently needs the wide text we need to convert
  // it when active, which is not ideal :(
  if (is_readonly && state != NULL && (render_cursor || render_selection)) {
    const char *buf_end = NULL;
    state->TextW.resize(buf_size + 1);
    state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, buf, NULL, &buf_end);
    state->CurLenA = (int)(buf_end - buf);
    state->CursorClamp();
    render_selection &= state->HasSelection();
  }

  // Select the buffer to render.
  const bool buf_display_from_state = (render_cursor || render_selection || g.ActiveId == id) &&
                                      !is_readonly && state && state->TextAIsValid;
  const bool is_displaying_hint = (hint != NULL &&
                                   (buf_display_from_state ? state->TextA.Data : buf)[0] == 0);

  // Password pushes a temporary font with only a fallback glyph
  if (is_password && !is_displaying_hint) {
    const AnchorFontGlyph *glyph = g.Font->FindGlyph('*');
    AnchorFont *password_font = &g.InputTextPasswordFont;
    password_font->FontSize = g.Font->FontSize;
    password_font->Scale = g.Font->Scale;
    password_font->Ascent = g.Font->Ascent;
    password_font->Descent = g.Font->Descent;
    password_font->ContainerAtlas = g.Font->ContainerAtlas;
    password_font->FallbackGlyph = glyph;
    password_font->FallbackAdvanceX = glyph->AdvanceX;
    ANCHOR_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() &&
                  password_font->IndexLookup.empty());
    PushFont(password_font);
  }

  // Process mouse inputs and character inputs
  int backup_current_text_length = 0;
  if (g.ActiveId == id) {
    ANCHOR_ASSERT(state != NULL);
    backup_current_text_length = state->CurLenA;
    state->Edited = false;
    state->BufCapacityA = buf_size;
    state->Flags = flags;
    state->UserCallback = callback;
    state->UserCallbackData = callback_user_data;

    // Although we are active we don't prevent mouse from hovering other elements unless we are
    // interacting right now with the widget. Down the line we should have a cleaner library-wide
    // concept of Selected vs Active.
    g.ActiveIdAllowOverlap = !io.MouseDown[0];
    g.WantTextInputNextFrame = 1;

    // Edit in progress
    const float mouse_x = (io.MousePos[0] - frame_bb.Min[0] - style.FramePadding[0]) + state->ScrollX;
    const float mouse_y = (is_multiline ? (io.MousePos[1] - draw_window->DC.CursorPos[1]) :
                                          (g.FontSize * 0.5f));

    const bool is_osx = io.ConfigMacOSXBehaviors;
    if (select_all || (hovered && !is_osx && io.MouseDoubleClicked[0])) {
      state->SelectAll();
      state->SelectedAllMouseLock = true;
    }
    else if (hovered && is_osx && io.MouseDoubleClicked[0]) {
      // Double-click select a word only, OS X style (by simulating keystrokes)
      state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
      state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
    }
    else if (io.MouseClicked[0] && !state->SelectedAllMouseLock) {
      if (hovered) {
        stb_textedit_click(state, &state->Stb, mouse_x, mouse_y);
        state->CursorAnimReset();
      }
    }
    else if (io.MouseDown[0] && !state->SelectedAllMouseLock &&
             (io.MouseDelta[0] != 0.0f || io.MouseDelta[1] != 0.0f)) {
      stb_textedit_drag(state, &state->Stb, mouse_x, mouse_y);
      state->CursorAnimReset();
      state->CursorFollow = true;
    }
    if (state->SelectedAllMouseLock && !io.MouseDown[0])
      state->SelectedAllMouseLock = false;

    // It is ill-defined whether the backend needs to send a \t character when pressing the TAB
    // keys. Win32 and GLFW naturally do it but not SDL.
    const bool ignore_char_inputs = (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeySuper);
    if ((flags & ANCHORInputTextFlags_AllowTabInput) && IsKeyPressedMap(ANCHOR_Key_Tab) &&
        !ignore_char_inputs && !io.KeyShift && !is_readonly)
      if (!io.InputQueueCharacters.contains('\t')) {
        unsigned int c = '\t';  // Insert TAB
        if (InputTextFilterCharacter(&c, flags, callback, callback_user_data, ANCHORInputSource_Keyboard))
          state->OnKeyPressed((int)c);
      }

    // Process regular text input (before we check for Return because using some IME will
    // effectively send a Return?) We ignore CTRL inputs, but need to allow ALT+CTRL as some
    // keyboards (e.g. German) use AltGR (which _is_ Alt+Ctrl) to input certain characters.
    if (io.InputQueueCharacters.Size > 0) {
      if (!ignore_char_inputs && !is_readonly && !user_nav_input_start)
        for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
          // Insert character if they pass filtering
          unsigned int c = (unsigned int)io.InputQueueCharacters[n];
          if (c == '\t' && io.KeyShift)
            continue;
          if (InputTextFilterCharacter(&c, flags, callback, callback_user_data, ANCHORInputSource_Keyboard))
            state->OnKeyPressed((int)c);
        }

      // Consume characters
      io.InputQueueCharacters.resize(0);
    }
  }

  // Process other shortcuts/key-presses
  bool cancel_edit = false;
  if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id) {
    ANCHOR_ASSERT(state != NULL);
    ANCHOR_ASSERT(io.KeyMods == GetMergedKeyModFlags() &&
                  "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");  // We rarely do
                                                                                              // this check,
                                                                                              // but if
                                                                                              // anything
                                                                                              // let's do it
                                                                                              // here.

    const int row_count_per_page = AnchorMax((int)((inner_size[1] - style.FramePadding[1]) / g.FontSize), 1);
    state->Stb.row_count_per_page = row_count_per_page;

    const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
    const bool is_osx = io.ConfigMacOSXBehaviors;
    const bool is_osx_shift_shortcut = is_osx &&
                                       (io.KeyMods == (ANCHOR_KeyModFlags_Super | ANCHOR_KeyModFlags_Shift));
    const bool is_wordmove_key_down =
      is_osx ? io.KeyAlt : io.KeyCtrl;  // OS X style: Text editing cursor movement using Alt instead of Ctrl
    const bool is_startend_key_down =
      is_osx && io.KeySuper && !io.KeyCtrl &&
      !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End
    const bool is_ctrl_key_only = (io.KeyMods == ANCHOR_KeyModFlags_Ctrl);
    const bool is_shift_key_only = (io.KeyMods == ANCHOR_KeyModFlags_Shift);
    const bool is_shortcut_key = g.IO.ConfigMacOSXBehaviors ? (io.KeyMods == ANCHOR_KeyModFlags_Super) :
                                                              (io.KeyMods == ANCHOR_KeyModFlags_Ctrl);

    const bool is_cut = ((is_shortcut_key && IsKeyPressedMap(ANCHOR_Key_X)) ||
                         (is_shift_key_only && IsKeyPressedMap(ANCHOR_Key_Delete))) &&
                        !is_readonly && !is_password && (!is_multiline || state->HasSelection());
    const bool is_copy = ((is_shortcut_key && IsKeyPressedMap(ANCHOR_Key_C)) ||
                          (is_ctrl_key_only && IsKeyPressedMap(ANCHOR_Key_Insert))) &&
                         !is_password && (!is_multiline || state->HasSelection());
    const bool is_paste = ((is_shortcut_key && IsKeyPressedMap(ANCHOR_Key_V)) ||
                           (is_shift_key_only && IsKeyPressedMap(ANCHOR_Key_Insert))) &&
                          !is_readonly;
    const bool is_undo = ((is_shortcut_key && IsKeyPressedMap(ANCHOR_Key_Z)) && !is_readonly && is_undoable);
    const bool is_redo = ((is_shortcut_key && IsKeyPressedMap(ANCHOR_Key_Y)) ||
                          (is_osx_shift_shortcut && IsKeyPressedMap(ANCHOR_Key_Z))) &&
                         !is_readonly && is_undoable;

    if (IsKeyPressedMap(ANCHOR_Key_LeftArrow)) {
      state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART :
                           is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT :
                                                  STB_TEXTEDIT_K_LEFT) |
                          k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_RightArrow)) {
      state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND :
                           is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT :
                                                  STB_TEXTEDIT_K_RIGHT) |
                          k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_UpArrow) && is_multiline) {
      if (io.KeyCtrl)
        SetScrollY(draw_window, AnchorMax(draw_window->Scroll[1] - g.FontSize, 0.0f));
      else
        state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_DownArrow) && is_multiline) {
      if (io.KeyCtrl)
        SetScrollY(draw_window, ImMin(draw_window->Scroll[1] + g.FontSize, GetScrollMaxY()));
      else
        state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_PageUp) && is_multiline) {
      state->OnKeyPressed(STB_TEXTEDIT_K_PGUP | k_mask);
      scroll_y -= row_count_per_page * g.FontSize;
    }
    else if (IsKeyPressedMap(ANCHOR_Key_PageDown) && is_multiline) {
      state->OnKeyPressed(STB_TEXTEDIT_K_PGDOWN | k_mask);
      scroll_y += row_count_per_page * g.FontSize;
    }
    else if (IsKeyPressedMap(ANCHOR_Key_Home)) {
      state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask :
                                       STB_TEXTEDIT_K_LINESTART | k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_End)) {
      state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_Delete) && !is_readonly) {
      state->OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_Backspace) && !is_readonly) {
      if (!state->HasSelection()) {
        if (is_wordmove_key_down)
          state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
        else if (is_osx && io.KeySuper && !io.KeyAlt && !io.KeyCtrl)
          state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_SHIFT);
      }
      state->OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
    }
    else if (IsKeyPressedMap(ANCHOR_Key_Enter) || IsKeyPressedMap(ANCHOR_Key_KeyPadEnter)) {
      bool ctrl_enter_for_new_line = (flags & ANCHORInputTextFlags_CtrlEnterForNewLine) != 0;
      if (!is_multiline || (ctrl_enter_for_new_line && !io.KeyCtrl) ||
          (!ctrl_enter_for_new_line && io.KeyCtrl)) {
        enter_pressed = clear_active_id = true;
      }
      else if (!is_readonly) {
        unsigned int c = '\n';  // Insert new line
        if (InputTextFilterCharacter(&c, flags, callback, callback_user_data, ANCHORInputSource_Keyboard))
          state->OnKeyPressed((int)c);
      }
    }
    else if (IsKeyPressedMap(ANCHOR_Key_Escape)) {
      clear_active_id = cancel_edit = true;
    }
    else if (is_undo || is_redo) {
      state->OnKeyPressed(is_undo ? STB_TEXTEDIT_K_UNDO : STB_TEXTEDIT_K_REDO);
      state->ClearSelection();
    }
    else if (is_shortcut_key && IsKeyPressedMap(ANCHOR_Key_A)) {
      state->SelectAll();
      state->CursorFollow = true;
    }
    else if (is_cut || is_copy) {
      // Cut, Copy
      if (io.SetClipboardTextFn) {
        const int ib = state->HasSelection() ? ImMin(state->Stb.select_start, state->Stb.select_end) : 0;
        const int ie = state->HasSelection() ? AnchorMax(state->Stb.select_start, state->Stb.select_end) :
                                               state->CurLenW;
        const int clipboard_data_len = ImTextCountUtf8BytesFromStr(state->TextW.Data + ib,
                                                                   state->TextW.Data + ie) +
                                       1;
        char *clipboard_data = (char *)IM_ALLOC(clipboard_data_len * sizeof(char));
        ImTextStrToUtf8(clipboard_data, clipboard_data_len, state->TextW.Data + ib, state->TextW.Data + ie);
        SetClipboardText(clipboard_data);
        MemFree(clipboard_data);
      }
      if (is_cut) {
        if (!state->HasSelection())
          state->SelectAll();
        state->CursorFollow = true;
        stb_textedit_cut(state, &state->Stb);
      }
    }
    else if (is_paste) {
      if (const char *clipboard = GetClipboardText()) {
        // Filter pasted buffer
        const int clipboard_len = (int)strlen(clipboard);
        AnchorWChar *clipboard_filtered = (AnchorWChar *)IM_ALLOC((clipboard_len + 1) * sizeof(AnchorWChar));
        int clipboard_filtered_len = 0;
        for (const char *s = clipboard; *s;) {
          unsigned int c;
          s += ImTextCharFromUtf8(&c, s, NULL);
          if (c == 0)
            break;
          if (!InputTextFilterCharacter(
                &c, flags, callback, callback_user_data, ANCHORInputSource_Clipboard))
            continue;
          clipboard_filtered[clipboard_filtered_len++] = (AnchorWChar)c;
        }
        clipboard_filtered[clipboard_filtered_len] = 0;
        if (clipboard_filtered_len > 0)  // If everything was filtered, ignore the pasting operation
        {
          stb_textedit_paste(state, &state->Stb, clipboard_filtered, clipboard_filtered_len);
          state->CursorFollow = true;
        }
        MemFree(clipboard_filtered);
      }
    }

    // Update render selection flag after events have been handled, so selection highlight can be
    // displayed during the same frame.
    render_selection |= state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
  }

  // Process callbacks and apply result back to user's buffer.
  if (g.ActiveId == id) {
    ANCHOR_ASSERT(state != NULL);
    const char *apply_new_text = NULL;
    int apply_new_text_length = 0;
    if (cancel_edit) {
      // Restore initial value. Only return true if restoring to the initial value changes the
      // current buffer contents.
      if (!is_readonly && strcmp(buf, state->InitialTextA.Data) != 0) {
        // Push records into the undo stack so we can CTRL+Z the revert operation itself
        apply_new_text = state->InitialTextA.Data;
        apply_new_text_length = state->InitialTextA.Size - 1;
        AnchorVector<AnchorWChar> w_text;
        if (apply_new_text_length > 0) {
          w_text.resize(ImTextCountCharsFromUtf8(apply_new_text, apply_new_text + apply_new_text_length) +
                        1);
          ImTextStrFromUtf8(
            w_text.Data, w_text.Size, apply_new_text, apply_new_text + apply_new_text_length);
        }
        stb_textedit_replace(
          state, &state->Stb, w_text.Data, (apply_new_text_length > 0) ? (w_text.Size - 1) : 0);
      }
    }

    // When using 'ANCHORInputTextFlags_EnterReturnsTrue' as a special case we reapply the live
    // buffer back to the input buffer before clearing ActiveId, even though strictly speaking it
    // wasn't modified on this frame. If we didn't do that, code like InputInt() with
    // ANCHORInputTextFlags_EnterReturnsTrue would fail. This also allows the user to use
    // InputText() with ANCHORInputTextFlags_EnterReturnsTrue without maintaining any user-side
    // storage (please note that if you use this property along ANCHORInputTextFlags_CallbackResize
    // you can end up with your temporary string object unnecessarily allocating once a frame,
    // either store your string data, either if you don't then don't use
    // ANCHORInputTextFlags_CallbackResize).
    bool apply_edit_back_to_user_buffer = !cancel_edit ||
                                          (enter_pressed &&
                                           (flags & ANCHORInputTextFlags_EnterReturnsTrue) != 0);
    if (apply_edit_back_to_user_buffer) {
      // Apply new value immediately - copy modified buffer back
      // Note that as soon as the input box is active, the in-widget value gets priority over any
      // underlying modification of the input buffer
      // FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment
      // above incorrect.
      // FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state
      // from the stb_textedit callbacks.
      if (!is_readonly) {
        state->TextAIsValid = true;
        state->TextA.resize(state->TextW.Size * 4 + 1);
        ImTextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data, NULL);
      }

      // User callback
      if ((flags & (ANCHORInputTextFlags_CallbackCompletion | ANCHORInputTextFlags_CallbackHistory |
                    ANCHORInputTextFlags_CallbackEdit | ANCHORInputTextFlags_CallbackAlways)) != 0) {
        ANCHOR_ASSERT(callback != NULL);

        // The reason we specify the usage semantic (Completion/History) is that Completion needs
        // to disable keyboard TABBING at the moment.
        ANCHORInputTextFlags event_flag = 0;
        ANCHOR_Key event_key = ANCHOR_Key_COUNT;
        if ((flags & ANCHORInputTextFlags_CallbackCompletion) != 0 && IsKeyPressedMap(ANCHOR_Key_Tab)) {
          event_flag = ANCHORInputTextFlags_CallbackCompletion;
          event_key = ANCHOR_Key_Tab;
        }
        else if ((flags & ANCHORInputTextFlags_CallbackHistory) != 0 &&
                 IsKeyPressedMap(ANCHOR_Key_UpArrow)) {
          event_flag = ANCHORInputTextFlags_CallbackHistory;
          event_key = ANCHOR_Key_UpArrow;
        }
        else if ((flags & ANCHORInputTextFlags_CallbackHistory) != 0 &&
                 IsKeyPressedMap(ANCHOR_Key_DownArrow)) {
          event_flag = ANCHORInputTextFlags_CallbackHistory;
          event_key = ANCHOR_Key_DownArrow;
        }
        else if ((flags & ANCHORInputTextFlags_CallbackEdit) && state->Edited) {
          event_flag = ANCHORInputTextFlags_CallbackEdit;
        }
        else if (flags & ANCHORInputTextFlags_CallbackAlways) {
          event_flag = ANCHORInputTextFlags_CallbackAlways;
        }

        if (event_flag) {
          ANCHORInputTextCallbackData callback_data;
          memset(&callback_data, 0, sizeof(ANCHORInputTextCallbackData));
          callback_data.EventFlag = event_flag;
          callback_data.Flags = flags;
          callback_data.UserData = callback_user_data;

          callback_data.EventKey = event_key;
          callback_data.Buf = state->TextA.Data;
          callback_data.BufTextLen = state->CurLenA;
          callback_data.BufSize = state->BufCapacityA;
          callback_data.BufDirty = false;

          // We have to convert from wchar-positions to UTF-8-positions, which can be pretty slow
          // (an incentive to ditch the AnchorWChar buffer, see
          // https://github.com/nothings/stb/issues/188)
          AnchorWChar *text = state->TextW.Data;
          const int utf8_cursor_pos = callback_data.CursorPos = ImTextCountUtf8BytesFromStr(
            text, text + state->Stb.cursor);
          const int utf8_selection_start = callback_data.SelectionStart = ImTextCountUtf8BytesFromStr(
            text, text + state->Stb.select_start);
          const int utf8_selection_end = callback_data.SelectionEnd = ImTextCountUtf8BytesFromStr(
            text, text + state->Stb.select_end);

          // Call user code
          callback(&callback_data);

          // Read back what user may have modified
          ANCHOR_ASSERT(callback_data.Buf == state->TextA.Data);  // Invalid to modify those fields
          ANCHOR_ASSERT(callback_data.BufSize == state->BufCapacityA);
          ANCHOR_ASSERT(callback_data.Flags == flags);
          const bool buf_dirty = callback_data.BufDirty;
          if (callback_data.CursorPos != utf8_cursor_pos || buf_dirty) {
            state->Stb.cursor = ImTextCountCharsFromUtf8(callback_data.Buf,
                                                         callback_data.Buf + callback_data.CursorPos);
            state->CursorFollow = true;
          }
          if (callback_data.SelectionStart != utf8_selection_start || buf_dirty) {
            state->Stb.select_start = (callback_data.SelectionStart == callback_data.CursorPos) ?
                                        state->Stb.cursor :
                                        ImTextCountCharsFromUtf8(callback_data.Buf,
                                                                 callback_data.Buf +
                                                                   callback_data.SelectionStart);
          }
          if (callback_data.SelectionEnd != utf8_selection_end || buf_dirty) {
            state->Stb.select_end = (callback_data.SelectionEnd == callback_data.SelectionStart) ?
                                      state->Stb.select_start :
                                      ImTextCountCharsFromUtf8(
                                        callback_data.Buf, callback_data.Buf + callback_data.SelectionEnd);
          }
          if (buf_dirty) {
            ANCHOR_ASSERT(
              callback_data.BufTextLen ==
              (int)strlen(callback_data.Buf));  // You need to maintain BufTextLen if you change the text!
            if (callback_data.BufTextLen > backup_current_text_length && is_resizable)
              state->TextW.resize(state->TextW.Size +
                                  (callback_data.BufTextLen - backup_current_text_length));
            state->CurLenW = ImTextStrFromUtf8(
              state->TextW.Data, state->TextW.Size, callback_data.Buf, NULL);
            state->CurLenA = callback_data.BufTextLen;  // Assume correct length and valid UTF-8
                                                        // from user, saves us an extra strlen()
            state->CursorAnimReset();
          }
        }
      }

      // Will copy result string if modified
      if (!is_readonly && strcmp(state->TextA.Data, buf) != 0) {
        apply_new_text = state->TextA.Data;
        apply_new_text_length = state->CurLenA;
      }
    }

    // Copy result to user buffer
    if (apply_new_text) {
      // We cannot test for 'backup_current_text_length != apply_new_text_length' here because we
      // have no guarantee that the size of our owned buffer matches the size of the string object
      // held by the user, and by design we allow InputText() to be used without any storage on
      // user's side.
      ANCHOR_ASSERT(apply_new_text_length >= 0);
      if (is_resizable) {
        ANCHORInputTextCallbackData callback_data;
        callback_data.EventFlag = ANCHORInputTextFlags_CallbackResize;
        callback_data.Flags = flags;
        callback_data.Buf = buf;
        callback_data.BufTextLen = apply_new_text_length;
        callback_data.BufSize = AnchorMax(buf_size, apply_new_text_length + 1);
        callback_data.UserData = callback_user_data;
        callback(&callback_data);
        buf = callback_data.Buf;
        buf_size = callback_data.BufSize;
        apply_new_text_length = ImMin(callback_data.BufTextLen, buf_size - 1);
        ANCHOR_ASSERT(apply_new_text_length <= buf_size);
      }
      // ANCHOR_DEBUG_LOG("InputText(\"%s\"): apply_new_text length %d\n", label,
      // apply_new_text_length);

      // If the underlying buffer resize was denied or not carried to the next frame,
      // apply_new_text_length+1 may be >= buf_size.
      ImStrncpy(buf, apply_new_text, ImMin(apply_new_text_length + 1, buf_size));
      value_changed = true;
    }

    // Clear temporary user storage
    state->Flags = ANCHORInputTextFlags_None;
    state->UserCallback = NULL;
    state->UserCallbackData = NULL;
  }

  // Release active ID at the end of the function (so e.g. pressing Return still does a final
  // application of the value)
  if (clear_active_id && g.ActiveId == id)
    ClearActiveID();

  // Render frame
  if (!is_multiline) {
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ANCHOR_Col_FrameBg), true, style.FrameRounding);
  }

  const GfVec4f clip_rect(frame_bb.Min[0],
                          frame_bb.Min[1],
                          frame_bb.Min[0] + inner_size[0],
                          frame_bb.Min[1] +
                            inner_size[1]);  // Not using frame_bb.Max because we have adjusted size
  GfVec2f draw_pos = is_multiline ? draw_window->DC.CursorPos : frame_bb.Min + style.FramePadding;
  GfVec2f text_size(0.0f, 0.0f);

  // Set upper limit of single-line InputTextEx() at 2 million characters strings. The current
  // pathological worst case is a long line without any carriage return, which would makes
  // AnchorFont::RenderText() reserve too many vertices and probably crash. Avoid it altogether.
  // Note that we only use this limit on single-line InputText(), so a pathologically large line on
  // a InputTextMultiline() would still crash.
  const int buf_display_max_length = 2 * 1024 * 1024;
  const char *buf_display = buf_display_from_state ? state->TextA.Data : buf;  //-V595
  const char *buf_display_end = NULL;  // We have specialized paths below for setting the length
  if (is_displaying_hint) {
    buf_display = hint;
    buf_display_end = hint + strlen(hint);
  }

  // Render text. We currently only render selection when the widget is active or while scrolling.
  // FIXME: We could remove the '&& render_cursor' to keep rendering selection when inactive.
  if (render_cursor || render_selection) {
    ANCHOR_ASSERT(state != NULL);
    if (!is_displaying_hint)
      buf_display_end = buf_display + state->CurLenA;

    // Render text (with cursor and selection)
    // This is going to be messy. We need to:
    // - Display the text (this alone can be more easily clipped)
    // - Handle scrolling, highlight selection, display cursor (those all requires some form of
    // 1d->2d cursor position calculation)
    // - Measure text height (for scrollbar)
    // We are attempting to do most of that in **one main pass** to minimize the computation cost
    // (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge
    // them by an extra refactoring effort)
    // FIXME: This should occur on buf_display but we'd need to maintain
    // cursor/select_start/select_end for UTF-8.
    const AnchorWChar *text_begin = state->TextW.Data;
    GfVec2f cursor_offset, select_start_offset;

    {
      // Find lines numbers straddling 'cursor' (slot 0) and 'select_start' (slot 1) positions.
      const AnchorWChar *searches_input_ptr[2] = {NULL, NULL};
      int searches_result_line_no[2] = {-1000, -1000};
      int searches_remaining = 0;
      if (render_cursor) {
        searches_input_ptr[0] = text_begin + state->Stb.cursor;
        searches_result_line_no[0] = -1;
        searches_remaining++;
      }
      if (render_selection) {
        searches_input_ptr[1] = text_begin + ImMin(state->Stb.select_start, state->Stb.select_end);
        searches_result_line_no[1] = -1;
        searches_remaining++;
      }

      // Iterate all lines to find our line numbers
      // In multi-line mode, we never exit the loop until all lines are counted, so add one extra
      // to the searches_remaining counter.
      searches_remaining += is_multiline ? 1 : 0;
      int line_count = 0;
      // for (const AnchorWChar* s = text_begin; (s = (const AnchorWChar*)wcschr((const wchar_t*)s,
      // (wchar_t)'\n')) != NULL; s++)  // FIXME-OPT: Could use this when wchar_t are 16-bit
      for (const AnchorWChar *s = text_begin; *s != 0; s++)
        if (*s == '\n') {
          line_count++;
          if (searches_result_line_no[0] == -1 && s >= searches_input_ptr[0]) {
            searches_result_line_no[0] = line_count;
            if (--searches_remaining <= 0)
              break;
          }
          if (searches_result_line_no[1] == -1 && s >= searches_input_ptr[1]) {
            searches_result_line_no[1] = line_count;
            if (--searches_remaining <= 0)
              break;
          }
        }
      line_count++;
      if (searches_result_line_no[0] == -1)
        searches_result_line_no[0] = line_count;
      if (searches_result_line_no[1] == -1)
        searches_result_line_no[1] = line_count;

      // Calculate 2d position by finding the beginning of the line and measuring distance
      cursor_offset[0] = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[0], text_begin),
                                                searches_input_ptr[0])[0];
      cursor_offset[1] = searches_result_line_no[0] * g.FontSize;
      if (searches_result_line_no[1] >= 0) {
        select_start_offset[0] = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[1], text_begin),
                                                        searches_input_ptr[1])[0];
        select_start_offset[1] = searches_result_line_no[1] * g.FontSize;
      }

      // Store text height (note that we haven't calculated text width at all, see GitHub issues
      // #383, #1224)
      if (is_multiline)
        text_size = GfVec2f(inner_size[0], line_count * g.FontSize);
    }

    // Scroll
    if (render_cursor && state->CursorFollow) {
      // Horizontal scroll in chunks of quarter width
      if (!(flags & ANCHORInputTextFlags_NoHorizontalScroll)) {
        const float scroll_increment_x = inner_size[0] * 0.25f;
        const float visible_width = inner_size[0] - style.FramePadding[0];
        if (cursor_offset[0] < state->ScrollX)
          state->ScrollX = IM_FLOOR(AnchorMax(0.0f, cursor_offset[0] - scroll_increment_x));
        else if (cursor_offset[0] - visible_width >= state->ScrollX)
          state->ScrollX = IM_FLOOR(cursor_offset[0] - visible_width + scroll_increment_x);
      }
      else {
        state->ScrollX = 0.0f;
      }

      // Vertical scroll
      if (is_multiline) {
        // Test if cursor is vertically visible
        if (cursor_offset[1] - g.FontSize < scroll_y)
          scroll_y = AnchorMax(0.0f, cursor_offset[1] - g.FontSize);
        else if (cursor_offset[1] - inner_size[1] >= scroll_y)
          scroll_y = cursor_offset[1] - inner_size[1] + style.FramePadding[1] * 2.0f;
        const float scroll_max_y = AnchorMax((text_size[1] + style.FramePadding[1] * 2.0f) - inner_size[1],
                                             0.0f);
        scroll_y = ImClamp(scroll_y, 0.0f, scroll_max_y);
        draw_pos[1] += (draw_window->Scroll[1] -
                        scroll_y);  // Manipulate cursor pos immediately avoid a frame of lag
        draw_window->Scroll[1] = scroll_y;
      }

      state->CursorFollow = false;
    }

    // Draw selection
    const GfVec2f draw_scroll = GfVec2f(state->ScrollX, 0.0f);
    if (render_selection) {
      const AnchorWChar *text_selected_begin = text_begin +
                                               ImMin(state->Stb.select_start, state->Stb.select_end);
      const AnchorWChar *text_selected_end = text_begin +
                                             AnchorMax(state->Stb.select_start, state->Stb.select_end);

      AnchorU32 bg_color = GetColorU32(
        ANCHOR_Col_TextSelectedBg,
        render_cursor ? 1.0f : 0.6f);  // FIXME: current code flow mandate that render_cursor is always
                                       // true here, we are leaving the transparent one for tests.
      float bg_offy_up = is_multiline ? 0.0f : -1.0f;  // FIXME: those offsets should be part of the style?
                                                       // they don't play so well with multi-line selection.
      float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
      GfVec2f rect_pos = draw_pos + select_start_offset - draw_scroll;
      for (const AnchorWChar *p = text_selected_begin; p < text_selected_end;) {
        if (rect_pos[1] > clip_rect[3] + g.FontSize)
          break;
        if (rect_pos[1] < clip_rect[1]) {
          // p = (const AnchorWChar*)wmemchr((const wchar_t*)p, '\n', text_selected_end - p);  //
          // FIXME-OPT: Could use this when wchar_t are 16-bit p = p ? p + 1 : text_selected_end;
          while (p < text_selected_end)
            if (*p++ == '\n')
              break;
        }
        else {
          GfVec2f rect_size = InputTextCalcTextSizeW(p, text_selected_end, &p, NULL, true);
          if (rect_size[0] <= 0.0f)
            rect_size[0] = IM_FLOOR(g.Font->GetCharAdvance((AnchorWChar)' ') *
                                    0.50f);  // So we can see selected empty lines
          ImRect rect(rect_pos + GfVec2f(0.0f, bg_offy_up - g.FontSize),
                      rect_pos + GfVec2f(rect_size[0], bg_offy_dn));
          rect.ClipWith(clip_rect);
          if (rect.Overlaps(clip_rect))
            draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
        }
        rect_pos[0] = draw_pos[0] - draw_scroll[0];
        rect_pos[1] += g.FontSize;
      }
    }

    // We test for 'buf_display_max_length' as a way to avoid some pathological cases (e.g.
    // single-line 1 MB string) which would make ImDrawList crash.
    if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length) {
      AnchorU32 col = GetColorU32(is_displaying_hint ? ANCHOR_Col_TextDisabled : ANCHOR_Col_Text);
      draw_window->DrawList->AddText(g.Font,
                                     g.FontSize,
                                     draw_pos - draw_scroll,
                                     col,
                                     buf_display,
                                     buf_display_end,
                                     0.0f,
                                     is_multiline ? NULL : &clip_rect);
    }

    // Draw blinking cursor
    if (render_cursor) {
      state->CursorAnim += io.DeltaTime;
      bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) || (state->CursorAnim <= 0.0f) ||
                               ImFmod(state->CursorAnim, 1.20f) <= 0.80f;
      GfVec2f cursor_screen_pos = ImFloor(draw_pos + cursor_offset - draw_scroll);
      ImRect cursor_screen_rect(cursor_screen_pos[0],
                                cursor_screen_pos[1] - g.FontSize + 0.5f,
                                cursor_screen_pos[0] + 1.0f,
                                cursor_screen_pos[1] - 1.5f);
      if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
        draw_window->DrawList->AddLine(
          cursor_screen_rect.Min, cursor_screen_rect.GetBL(), GetColorU32(ANCHOR_Col_Text));

      // Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can
      // cover our cursor. Bit of an extra nicety.)
      if (!is_readonly)
        g.PlatformImePos = GfVec2f(cursor_screen_pos[0] - 1.0f, cursor_screen_pos[1] - g.FontSize);
    }
  }
  else {
    // Render text only (no selection, no cursor)
    if (is_multiline)
      text_size = GfVec2f(inner_size[0],
                          InputTextCalcTextLenAndLineCount(buf_display, &buf_display_end) *
                            g.FontSize);  // We don't need width
    else if (!is_displaying_hint && g.ActiveId == id)
      buf_display_end = buf_display + state->CurLenA;
    else if (!is_displaying_hint)
      buf_display_end = buf_display + strlen(buf_display);

    if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length) {
      AnchorU32 col = GetColorU32(is_displaying_hint ? ANCHOR_Col_TextDisabled : ANCHOR_Col_Text);
      draw_window->DrawList->AddText(g.Font,
                                     g.FontSize,
                                     draw_pos,
                                     col,
                                     buf_display,
                                     buf_display_end,
                                     0.0f,
                                     is_multiline ? NULL : &clip_rect);
    }
  }

  if (is_password && !is_displaying_hint)
    PopFont();

  if (is_multiline) {
    Dummy(GfVec2f(text_size[0], text_size[1] + style.FramePadding[1]));
    EndChild();
    EndGroup();
  }

  // Log as text
  if (g.LogEnabled && (!is_password || is_displaying_hint)) {
    LogSetNextTextDecoration("{", "}");
    LogRenderedText(&draw_pos, buf_display, buf_display_end);
  }

  if (label_size[0] > 0)
    RenderText(GfVec2f(frame_bb.Max[0] + style.ItemInnerSpacing[0], frame_bb.Min[1] + style.FramePadding[1]),
               label);

  if (value_changed && !(flags & ANCHORInputTextFlags_NoMarkEdited))
    MarkItemEdited(id);

  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  if ((flags & ANCHORInputTextFlags_EnterReturnsTrue) != 0)
    return enter_pressed;
  else
    return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
//-------------------------------------------------------------------------
// - ColorEdit3()
// - ColorEdit4()
// - ColorPicker3()
// - RenderColorRectWithAlphaCheckerboard() [Internal]
// - ColorPicker4()
// - ColorButton()
// - SetColorEditOptions()
// - ColorTooltip() [Internal]
// - ColorEditOptionsPopup() [Internal]
// - ColorPickerOptionsPopup() [Internal]
//-------------------------------------------------------------------------

bool ANCHOR::ColorEdit3(const char *label, float col[3], ANCHOR_ColorEditFlags flags)
{
  return ColorEdit4(label, col, flags | ANCHOR_ColorEditFlags_NoAlpha);
}

// Edit colors components (each component in 0.0f..1.0f range).
// See enum ANCHOR_ColorEditFlags_ for available options. e.g. Only access 3 floats if
// ANCHOR_ColorEditFlags_NoAlpha flag is set. With typical options: Left-click on color square to
// open color picker. Right-click to open option menu. CTRL-Click over input fields to edit them
// and TAB to go to next item.
bool ANCHOR::ColorEdit4(const char *label, float col[4], ANCHOR_ColorEditFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const float square_sz = GetFrameHeight();
  const float w_full = CalcItemWidth();
  const float w_button = (flags & ANCHOR_ColorEditFlags_NoSmallPreview) ?
                           0.0f :
                           (square_sz + style.ItemInnerSpacing[0]);
  const float w_inputs = w_full - w_button;
  const char *label_display_end = FindRenderedTextEnd(label);
  g.NextItemData.ClearFlags();

  BeginGroup();
  PushID(label);

  // If we're not showing any slider there's no point in doing any HSV conversions
  const ANCHOR_ColorEditFlags flags_untouched = flags;
  if (flags & ANCHOR_ColorEditFlags_NoInputs)
    flags = (flags & (~ANCHOR_ColorEditFlags__DisplayMask)) | ANCHOR_ColorEditFlags_DisplayRGB |
            ANCHOR_ColorEditFlags_NoOptions;

  // Context menu: display and modify options (before defaults are applied)
  if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
    ColorEditOptionsPopup(col, flags);

  // Read stored options
  if (!(flags & ANCHOR_ColorEditFlags__DisplayMask))
    flags |= (g.ColorEditOptions & ANCHOR_ColorEditFlags__DisplayMask);
  if (!(flags & ANCHOR_ColorEditFlags__DataTypeMask))
    flags |= (g.ColorEditOptions & ANCHOR_ColorEditFlags__DataTypeMask);
  if (!(flags & ANCHOR_ColorEditFlags__PickerMask))
    flags |= (g.ColorEditOptions & ANCHOR_ColorEditFlags__PickerMask);
  if (!(flags & ANCHOR_ColorEditFlags__InputMask))
    flags |= (g.ColorEditOptions & ANCHOR_ColorEditFlags__InputMask);
  flags |= (g.ColorEditOptions & ~(ANCHOR_ColorEditFlags__DisplayMask | ANCHOR_ColorEditFlags__DataTypeMask |
                                   ANCHOR_ColorEditFlags__PickerMask | ANCHOR_ColorEditFlags__InputMask));
  ANCHOR_ASSERT(
    ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__DisplayMask));            // Check that only 1 is selected
  ANCHOR_ASSERT(ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__InputMask));  // Check that only 1 is selected

  const bool alpha = (flags & ANCHOR_ColorEditFlags_NoAlpha) == 0;
  const bool hdr = (flags & ANCHOR_ColorEditFlags_HDR) != 0;
  const int components = alpha ? 4 : 3;

  // Convert to the formats we need
  float f[4] = {col[0], col[1], col[2], alpha ? col[3] : 1.0f};
  if ((flags & ANCHOR_ColorEditFlags_InputHSV) && (flags & ANCHOR_ColorEditFlags_DisplayRGB))
    ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
  else if ((flags & ANCHOR_ColorEditFlags_InputRGB) && (flags & ANCHOR_ColorEditFlags_DisplayHSV)) {
    // Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
    ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
    if (memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0) {
      if (f[1] == 0)
        f[0] = g.ColorEditLastHue;
      if (f[2] == 0)
        f[1] = g.ColorEditLastSat;
    }
  }
  int i[4] = {IM_F32_TO_INT8_UNBOUND(f[0]),
              IM_F32_TO_INT8_UNBOUND(f[1]),
              IM_F32_TO_INT8_UNBOUND(f[2]),
              IM_F32_TO_INT8_UNBOUND(f[3])};

  bool value_changed = false;
  bool value_changed_as_float = false;

  const GfVec2f pos = window->DC.CursorPos;
  const float inputs_offset_x = (style.ColorButtonPosition == ANCHOR_Dir_Left) ? w_button : 0.0f;
  window->DC.CursorPos[0] = pos[0] + inputs_offset_x;

  if ((flags & (ANCHOR_ColorEditFlags_DisplayRGB | ANCHOR_ColorEditFlags_DisplayHSV)) != 0 &&
      (flags & ANCHOR_ColorEditFlags_NoInputs) == 0) {
    // RGB/HSV 0..255 Sliders
    const float w_item_one = AnchorMax(
      1.0f, IM_FLOOR((w_inputs - (style.ItemInnerSpacing[0]) * (components - 1)) / (float)components));
    const float w_item_last = AnchorMax(
      1.0f, IM_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing[0]) * (components - 1)));

    const bool hide_prefix = (w_item_one <=
                              CalcTextSize((flags & ANCHOR_ColorEditFlags_Float) ? "M:0.000" : "M:000")[0]);
    static const char *ids[4] = {"##X", "##Y", "##Z", "##W"};
    static const char *fmt_table_int[3][4] = {
      {"%3d", "%3d", "%3d", "%3d"},          // Short display
      {"R:%3d", "G:%3d", "B:%3d", "A:%3d"},  // Long display for RGBA
      {"H:%3d", "S:%3d", "V:%3d", "A:%3d"}   // Long display for HSVA
    };
    static const char *fmt_table_float[3][4] = {
      {"%0.3f", "%0.3f", "%0.3f", "%0.3f"},          // Short display
      {"R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f"},  // Long display for RGBA
      {"H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f"}   // Long display for HSVA
    };
    const int fmt_idx = hide_prefix ? 0 : (flags & ANCHOR_ColorEditFlags_DisplayHSV) ? 2 : 1;

    for (int n = 0; n < components; n++) {
      if (n > 0)
        SameLine(0, style.ItemInnerSpacing[0]);
      SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

      // FIXME: When ANCHOR_ColorEditFlags_HDR flag is passed HS values snap in weird ways when SV
      // values go below 0.
      if (flags & ANCHOR_ColorEditFlags_Float) {
        value_changed |= DragFloat(
          ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
        value_changed_as_float |= value_changed;
      }
      else {
        value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
      }
      if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
        OpenPopupOnItemClick("context");
    }
  }
  else if ((flags & ANCHOR_ColorEditFlags_DisplayHex) != 0 &&
           (flags & ANCHOR_ColorEditFlags_NoInputs) == 0) {
    // RGB Hexadecimal Input
    char buf[64];
    if (alpha)
      ImFormatString(buf,
                     ANCHOR_ARRAYSIZE(buf),
                     "#%02X%02X%02X%02X",
                     ImClamp(i[0], 0, 255),
                     ImClamp(i[1], 0, 255),
                     ImClamp(i[2], 0, 255),
                     ImClamp(i[3], 0, 255));
    else
      ImFormatString(buf,
                     ANCHOR_ARRAYSIZE(buf),
                     "#%02X%02X%02X",
                     ImClamp(i[0], 0, 255),
                     ImClamp(i[1], 0, 255),
                     ImClamp(i[2], 0, 255));
    SetNextItemWidth(w_inputs);
    if (InputText("##Text",
                  buf,
                  ANCHOR_ARRAYSIZE(buf),
                  ANCHORInputTextFlags_CharsHexadecimal | ANCHORInputTextFlags_CharsUppercase)) {
      value_changed = true;
      char *p = buf;
      while (*p == '#' || ImCharIsBlankA(*p))
        p++;
      i[0] = i[1] = i[2] = 0;
      i[3] = 0xFF;  // alpha default to 255 is not parsed by scanf (e.g. inputting #FFFFFF omitting
                    // alpha)
      int r;
      if (alpha)
        r = sscanf(p,
                   "%02X%02X%02X%02X",
                   (unsigned int *)&i[0],
                   (unsigned int *)&i[1],
                   (unsigned int *)&i[2],
                   (unsigned int *)&i[3]);  // Treat at unsigned (%X is unsigned)
      else
        r = sscanf(p, "%02X%02X%02X", (unsigned int *)&i[0], (unsigned int *)&i[1], (unsigned int *)&i[2]);
      TF_UNUSED(r);  // Fixes C6031: Return value ignored: 'sscanf'.
    }
    if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context");
  }

  ANCHOR_Window *picker_active_window = NULL;
  if (!(flags & ANCHOR_ColorEditFlags_NoSmallPreview)) {
    const float button_offset_x = ((flags & ANCHOR_ColorEditFlags_NoInputs) ||
                                   (style.ColorButtonPosition == ANCHOR_Dir_Left)) ?
                                    0.0f :
                                    w_inputs + style.ItemInnerSpacing[0];
    window->DC.CursorPos = GfVec2f(pos[0] + button_offset_x, pos[1]);

    const GfVec4f col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
    if (ColorButton("##ColorButton", col_v4, flags)) {
      if (!(flags & ANCHOR_ColorEditFlags_NoPicker)) {
        // Store current color and open a picker
        g.ColorPickerRef = col_v4;
        OpenPopup("picker");
        SetNextWindowPos(window->DC.LastItemRect.GetBL() + GfVec2f(-1, style.ItemSpacing[1]));
      }
    }
    if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context");

    if (BeginPopup("picker")) {
      picker_active_window = g.CurrentWindow;
      if (label != label_display_end) {
        TextEx(label, label_display_end);
        Spacing();
      }
      ANCHOR_ColorEditFlags picker_flags_to_forward = ANCHOR_ColorEditFlags__DataTypeMask |
                                                      ANCHOR_ColorEditFlags__PickerMask |
                                                      ANCHOR_ColorEditFlags__InputMask |
                                                      ANCHOR_ColorEditFlags_HDR |
                                                      ANCHOR_ColorEditFlags_NoAlpha |
                                                      ANCHOR_ColorEditFlags_AlphaBar;
      ANCHOR_ColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) |
                                           ANCHOR_ColorEditFlags__DisplayMask |
                                           ANCHOR_ColorEditFlags_NoLabel |
                                           ANCHOR_ColorEditFlags_AlphaPreviewHalf;
      SetNextItemWidth(square_sz * 12.0f);  // Use 256 + bar sizes?
      value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef[0]);
      EndPopup();
    }
  }

  if (label != label_display_end && !(flags & ANCHOR_ColorEditFlags_NoLabel)) {
    const float text_offset_x = (flags & ANCHOR_ColorEditFlags_NoInputs) ?
                                  w_button :
                                  w_full + style.ItemInnerSpacing[0];
    window->DC.CursorPos = GfVec2f(pos[0] + text_offset_x, pos[1] + style.FramePadding[1]);
    TextEx(label, label_display_end);
  }

  // Convert back
  if (value_changed && picker_active_window == NULL) {
    if (!value_changed_as_float)
      for (int n = 0; n < 4; n++)
        f[n] = i[n] / 255.0f;
    if ((flags & ANCHOR_ColorEditFlags_DisplayHSV) && (flags & ANCHOR_ColorEditFlags_InputRGB)) {
      g.ColorEditLastHue = f[0];
      g.ColorEditLastSat = f[1];
      ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
      memcpy(g.ColorEditLastColor, f, sizeof(float) * 3);
    }
    if ((flags & ANCHOR_ColorEditFlags_DisplayRGB) && (flags & ANCHOR_ColorEditFlags_InputHSV))
      ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

    col[0] = f[0];
    col[1] = f[1];
    col[2] = f[2];
    if (alpha)
      col[3] = f[3];
  }

  PopID();
  EndGroup();

  // Drag and Drop Target
  // NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the
  // same test.
  if ((window->DC.LastItemStatusFlags & ANCHOR_ItemStatusFlags_HoveredRect) &&
      !(flags & ANCHOR_ColorEditFlags_NoDragDrop) && BeginDragDropTarget()) {
    bool accepted_drag_drop = false;
    if (const ANCHORPayload *payload = AcceptDragDropPayload(ANCHOR_PAYLOAD_TYPE_COLOR_3F)) {
      memcpy((float *)col, payload->Data, sizeof(float) * 3);  // Preserve alpha if any //-V512
      value_changed = accepted_drag_drop = true;
    }
    if (const ANCHORPayload *payload = AcceptDragDropPayload(ANCHOR_PAYLOAD_TYPE_COLOR_4F)) {
      memcpy((float *)col, payload->Data, sizeof(float) * components);
      value_changed = accepted_drag_drop = true;
    }

    // Drag-drop payloads are always RGB
    if (accepted_drag_drop && (flags & ANCHOR_ColorEditFlags_InputHSV))
      ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
    EndDragDropTarget();
  }

  // When picker is being actively used, use its active id so IsItemActive() will function on
  // ColorEdit4().
  if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
    window->DC.LastItemId = g.ActiveId;

  if (value_changed)
    MarkItemEdited(window->DC.LastItemId);

  return value_changed;
}

bool ANCHOR::ColorPicker3(const char *label, float col[3], ANCHOR_ColorEditFlags flags)
{
  float col4[4] = {col[0], col[1], col[2], 1.0f};
  if (!ColorPicker4(label, col4, flags | ANCHOR_ColorEditFlags_NoAlpha))
    return false;
  col[0] = col4[0];
  col[1] = col4[1];
  col[2] = col4[2];
  return true;
}

// Helper for ColorPicker4()
static void RenderArrowsForVerticalBar(ImDrawList *draw_list,
                                       GfVec2f pos,
                                       GfVec2f half_sz,
                                       float bar_w,
                                       float alpha)
{
  AnchorU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
  ANCHOR::RenderArrowPointingAt(draw_list,
                                GfVec2f(pos[0] + half_sz[0] + 1, pos[1]),
                                GfVec2f(half_sz[0] + 2, half_sz[1] + 1),
                                ANCHOR_Dir_Right,
                                ANCHOR_COL32(0, 0, 0, alpha8));
  ANCHOR::RenderArrowPointingAt(draw_list,
                                GfVec2f(pos[0] + half_sz[0], pos[1]),
                                half_sz,
                                ANCHOR_Dir_Right,
                                ANCHOR_COL32(255, 255, 255, alpha8));
  ANCHOR::RenderArrowPointingAt(draw_list,
                                GfVec2f(pos[0] + bar_w - half_sz[0] - 1, pos[1]),
                                GfVec2f(half_sz[0] + 2, half_sz[1] + 1),
                                ANCHOR_Dir_Left,
                                ANCHOR_COL32(0, 0, 0, alpha8));
  ANCHOR::RenderArrowPointingAt(draw_list,
                                GfVec2f(pos[0] + bar_w - half_sz[0], pos[1]),
                                half_sz,
                                ANCHOR_Dir_Left,
                                ANCHOR_COL32(255, 255, 255, alpha8));
}

// Note: ColorPicker4() only accesses 3 floats if ANCHOR_ColorEditFlags_NoAlpha flag is set.
// (In C++ the 'float col[4]' notation for a function argument is equivalent to 'float* col', we
// only specify a size to facilitate understanding of the code.)
// FIXME: we adjust the big color square height based on item width, which may cause a flickering
// feedback loop (if automatic height makes a vertical scrollbar appears, affecting automatic
// width..)
// FIXME: this is trying to be aware of style.Alpha but not fully correct. Also, the color wheel
// will have overlapping glitches with (style.Alpha < 1.0)
bool ANCHOR::ColorPicker4(const char *label, float col[4], ANCHOR_ColorEditFlags flags, const float *ref_col)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImDrawList *draw_list = window->DrawList;
  ANCHOR_Style &style = g.Style;
  ANCHOR_IO &io = g.IO;

  const float width = CalcItemWidth();
  g.NextItemData.ClearFlags();

  PushID(label);
  BeginGroup();

  if (!(flags & ANCHOR_ColorEditFlags_NoSidePreview))
    flags |= ANCHOR_ColorEditFlags_NoSmallPreview;

  // Context menu: display and store options.
  if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
    ColorPickerOptionsPopup(col, flags);

  // Read stored options
  if (!(flags & ANCHOR_ColorEditFlags__PickerMask))
    flags |= ((g.ColorEditOptions & ANCHOR_ColorEditFlags__PickerMask) ?
                g.ColorEditOptions :
                ANCHOR_ColorEditFlags__OptionsDefault) &
             ANCHOR_ColorEditFlags__PickerMask;
  if (!(flags & ANCHOR_ColorEditFlags__InputMask))
    flags |= ((g.ColorEditOptions & ANCHOR_ColorEditFlags__InputMask) ?
                g.ColorEditOptions :
                ANCHOR_ColorEditFlags__OptionsDefault) &
             ANCHOR_ColorEditFlags__InputMask;
  ANCHOR_ASSERT(ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__PickerMask));  // Check that only 1 is selected
  ANCHOR_ASSERT(ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__InputMask));   // Check that only 1 is selected
  if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
    flags |= (g.ColorEditOptions & ANCHOR_ColorEditFlags_AlphaBar);

  // Setup
  int components = (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 3 : 4;
  bool alpha_bar = (flags & ANCHOR_ColorEditFlags_AlphaBar) && !(flags & ANCHOR_ColorEditFlags_NoAlpha);
  GfVec2f picker_pos = window->DC.CursorPos;
  float square_sz = GetFrameHeight();
  float bars_width = square_sz;  // Arbitrary smallish width of Hue/Alpha picking bars
  float sv_picker_size = AnchorMax(
    bars_width * 1,
    width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing[0]));  // Saturation/Value picking box
  float bar0_pos_x = picker_pos[0] + sv_picker_size + style.ItemInnerSpacing[0];
  float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing[0];
  float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);

  float backup_initial_col[4];
  memcpy(backup_initial_col, col, components * sizeof(float));

  float wheel_thickness = sv_picker_size * 0.08f;
  float wheel_r_outer = sv_picker_size * 0.50f;
  float wheel_r_inner = wheel_r_outer - wheel_thickness;
  GfVec2f wheel_center(picker_pos[0] + (sv_picker_size + bars_width) * 0.5f,
                       picker_pos[1] + sv_picker_size * 0.5f);

  // Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates
  // stays unrotated for logic.
  float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
  GfVec2f triangle_pa = GfVec2f(triangle_r, 0.0f);                             // Hue point.
  GfVec2f triangle_pb = GfVec2f(triangle_r * -0.5f, triangle_r * -0.866025f);  // Black point.
  GfVec2f triangle_pc = GfVec2f(triangle_r * -0.5f, triangle_r * +0.866025f);  // White point.

  float H = col[0], S = col[1], V = col[2];
  float R = col[0], G = col[1], B = col[2];
  if (flags & ANCHOR_ColorEditFlags_InputRGB) {
    // Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
    ColorConvertRGBtoHSV(R, G, B, H, S, V);
    if (memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0) {
      if (S == 0)
        H = g.ColorEditLastHue;
      if (V == 0)
        S = g.ColorEditLastSat;
    }
  }
  else if (flags & ANCHOR_ColorEditFlags_InputHSV) {
    ColorConvertHSVtoRGB(H, S, V, R, G, B);
  }

  bool value_changed = false, value_changed_h = false, value_changed_sv = false;

  PushItemFlag(ANCHOR_ItemFlags_NoNav, true);
  if (flags & ANCHOR_ColorEditFlags_PickerHueWheel) {
    // Hue wheel + SV triangle logic
    InvisibleButton("hsv", GfVec2f(sv_picker_size + style.ItemInnerSpacing[0] + bars_width, sv_picker_size));
    if (IsItemActive()) {
      GfVec2f initial_off = g.IO.MouseClickedPos[0] - wheel_center;
      GfVec2f current_off = g.IO.MousePos - wheel_center;
      float initial_dist2 = ImLengthSqr(initial_off);
      if (initial_dist2 >= (wheel_r_inner - 1) * (wheel_r_inner - 1) &&
          initial_dist2 <= (wheel_r_outer + 1) * (wheel_r_outer + 1)) {
        // Interactive with Hue wheel
        H = ImAtan2(current_off[1], current_off[0]) / IM_PI * 0.5f;
        if (H < 0.0f)
          H += 1.0f;
        value_changed = value_changed_h = true;
      }
      float cos_hue_angle = ImCos(-H * 2.0f * IM_PI);
      float sin_hue_angle = ImSin(-H * 2.0f * IM_PI);
      if (ImTriangleContainsPoint(
            triangle_pa, triangle_pb, triangle_pc, ImRotate(initial_off, cos_hue_angle, sin_hue_angle))) {
        // Interacting with SV triangle
        GfVec2f current_off_unrotated = ImRotate(current_off, cos_hue_angle, sin_hue_angle);
        if (!ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated))
          current_off_unrotated = ImTriangleClosestPoint(
            triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
        float uu, vv, ww;
        ImTriangleBarycentricCoords(
          triangle_pa, triangle_pb, triangle_pc, current_off_unrotated, uu, vv, ww);
        V = ImClamp(1.0f - vv, 0.0001f, 1.0f);
        S = ImClamp(uu / V, 0.0001f, 1.0f);
        value_changed = value_changed_sv = true;
      }
    }
    if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context");
  }
  else if (flags & ANCHOR_ColorEditFlags_PickerHueBar) {
    // SV rectangle logic
    InvisibleButton("sv", GfVec2f(sv_picker_size, sv_picker_size));
    if (IsItemActive()) {
      S = ImSaturate((io.MousePos[0] - picker_pos[0]) / (sv_picker_size - 1));
      V = 1.0f - ImSaturate((io.MousePos[1] - picker_pos[1]) / (sv_picker_size - 1));
      value_changed = value_changed_sv = true;
    }
    if (!(flags & ANCHOR_ColorEditFlags_NoOptions))
      OpenPopupOnItemClick("context");

    // Hue bar logic
    SetCursorScreenPos(GfVec2f(bar0_pos_x, picker_pos[1]));
    InvisibleButton("hue", GfVec2f(bars_width, sv_picker_size));
    if (IsItemActive()) {
      H = ImSaturate((io.MousePos[1] - picker_pos[1]) / (sv_picker_size - 1));
      value_changed = value_changed_h = true;
    }
  }

  // Alpha bar logic
  if (alpha_bar) {
    SetCursorScreenPos(GfVec2f(bar1_pos_x, picker_pos[1]));
    InvisibleButton("alpha", GfVec2f(bars_width, sv_picker_size));
    if (IsItemActive()) {
      col[3] = 1.0f - ImSaturate((io.MousePos[1] - picker_pos[1]) / (sv_picker_size - 1));
      value_changed = true;
    }
  }
  PopItemFlag();  // ANCHOR_ItemFlags_NoNav

  if (!(flags & ANCHOR_ColorEditFlags_NoSidePreview)) {
    SameLine(0, style.ItemInnerSpacing[0]);
    BeginGroup();
  }

  if (!(flags & ANCHOR_ColorEditFlags_NoLabel)) {
    const char *label_display_end = FindRenderedTextEnd(label);
    if (label != label_display_end) {
      if ((flags & ANCHOR_ColorEditFlags_NoSidePreview))
        SameLine(0, style.ItemInnerSpacing[0]);
      TextEx(label, label_display_end);
    }
  }

  if (!(flags & ANCHOR_ColorEditFlags_NoSidePreview)) {
    PushItemFlag(ANCHOR_ItemFlags_NoNavDefaultFocus, true);
    GfVec4f col_v4(col[0], col[1], col[2], (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 1.0f : col[3]);
    if ((flags & ANCHOR_ColorEditFlags_NoLabel))
      Text("Current");

    ANCHOR_ColorEditFlags sub_flags_to_forward = ANCHOR_ColorEditFlags__InputMask |
                                                 ANCHOR_ColorEditFlags_HDR |
                                                 ANCHOR_ColorEditFlags_AlphaPreview |
                                                 ANCHOR_ColorEditFlags_AlphaPreviewHalf |
                                                 ANCHOR_ColorEditFlags_NoTooltip;
    ColorButton("##current", col_v4, (flags & sub_flags_to_forward), GfVec2f(square_sz * 3, square_sz * 2));
    if (ref_col != NULL) {
      Text("Original");
      GfVec4f ref_col_v4(
        ref_col[0], ref_col[1], ref_col[2], (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
      if (ColorButton("##original",
                      ref_col_v4,
                      (flags & sub_flags_to_forward),
                      GfVec2f(square_sz * 3, square_sz * 2))) {
        memcpy(col, ref_col, components * sizeof(float));
        value_changed = true;
      }
    }
    PopItemFlag();
    EndGroup();
  }

  // Convert back color to RGB
  if (value_changed_h || value_changed_sv) {
    if (flags & ANCHOR_ColorEditFlags_InputRGB) {
      ColorConvertHSVtoRGB(H >= 1.0f ? H - 10 * 1e-6f : H,
                           S > 0.0f ? S : 10 * 1e-6f,
                           V > 0.0f ? V : 1e-6f,
                           col[0],
                           col[1],
                           col[2]);
      g.ColorEditLastHue = H;
      g.ColorEditLastSat = S;
      memcpy(g.ColorEditLastColor, col, sizeof(float) * 3);
    }
    else if (flags & ANCHOR_ColorEditFlags_InputHSV) {
      col[0] = H;
      col[1] = S;
      col[2] = V;
    }
  }

  // R,G,B and H,S,V slider color editor
  bool value_changed_fix_hue_wrap = false;
  if ((flags & ANCHOR_ColorEditFlags_NoInputs) == 0) {
    PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos[0]);
    ANCHOR_ColorEditFlags sub_flags_to_forward = ANCHOR_ColorEditFlags__DataTypeMask |
                                                 ANCHOR_ColorEditFlags__InputMask |
                                                 ANCHOR_ColorEditFlags_HDR | ANCHOR_ColorEditFlags_NoAlpha |
                                                 ANCHOR_ColorEditFlags_NoOptions |
                                                 ANCHOR_ColorEditFlags_NoSmallPreview |
                                                 ANCHOR_ColorEditFlags_AlphaPreview |
                                                 ANCHOR_ColorEditFlags_AlphaPreviewHalf;
    ANCHOR_ColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ANCHOR_ColorEditFlags_NoPicker;
    if (flags & ANCHOR_ColorEditFlags_DisplayRGB || (flags & ANCHOR_ColorEditFlags__DisplayMask) == 0)
      if (ColorEdit4("##rgb", col, sub_flags | ANCHOR_ColorEditFlags_DisplayRGB)) {
        // FIXME: Hackily differentiating using the DragInt (ActiveId != 0 &&
        // !ActiveIdAllowOverlap) vs. using the InputText or DropTarget. For the later we don't
        // want to run the hue-wrap canceling code. If you are well versed in HSV picker please
        // provide your input! (See #2050)
        value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
        value_changed = true;
      }
    if (flags & ANCHOR_ColorEditFlags_DisplayHSV || (flags & ANCHOR_ColorEditFlags__DisplayMask) == 0)
      value_changed |= ColorEdit4("##hsv", col, sub_flags | ANCHOR_ColorEditFlags_DisplayHSV);
    if (flags & ANCHOR_ColorEditFlags_DisplayHex || (flags & ANCHOR_ColorEditFlags__DisplayMask) == 0)
      value_changed |= ColorEdit4("##hex", col, sub_flags | ANCHOR_ColorEditFlags_DisplayHex);
    PopItemWidth();
  }

  // Try to cancel hue wrap (after ColorEdit4 call), if any
  if (value_changed_fix_hue_wrap && (flags & ANCHOR_ColorEditFlags_InputRGB)) {
    float new_H, new_S, new_V;
    ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
    if (new_H <= 0 && H > 0) {
      if (new_V <= 0 && V != new_V)
        ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
      else if (new_S <= 0)
        ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
    }
  }

  if (value_changed) {
    if (flags & ANCHOR_ColorEditFlags_InputRGB) {
      R = col[0];
      G = col[1];
      B = col[2];
      ColorConvertRGBtoHSV(R, G, B, H, S, V);
      if (memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) ==
          0)  // Fix local Hue as display below will use it immediately.
      {
        if (S == 0)
          H = g.ColorEditLastHue;
        if (V == 0)
          S = g.ColorEditLastSat;
      }
    }
    else if (flags & ANCHOR_ColorEditFlags_InputHSV) {
      H = col[0];
      S = col[1];
      V = col[2];
      ColorConvertHSVtoRGB(H, S, V, R, G, B);
    }
  }

  const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
  const AnchorU32 col_black = ANCHOR_COL32(0, 0, 0, style_alpha8);
  const AnchorU32 col_white = ANCHOR_COL32(255, 255, 255, style_alpha8);
  const AnchorU32 col_midgrey = ANCHOR_COL32(128, 128, 128, style_alpha8);
  const AnchorU32 col_hues[6 + 1] = {ANCHOR_COL32(255, 0, 0, style_alpha8),
                                     ANCHOR_COL32(255, 255, 0, style_alpha8),
                                     ANCHOR_COL32(0, 255, 0, style_alpha8),
                                     ANCHOR_COL32(0, 255, 255, style_alpha8),
                                     ANCHOR_COL32(0, 0, 255, style_alpha8),
                                     ANCHOR_COL32(255, 0, 255, style_alpha8),
                                     ANCHOR_COL32(255, 0, 0, style_alpha8)};

  GfVec4f hue_color_f(1, 1, 1, style.Alpha);
  ColorConvertHSVtoRGB(H, 1, 1, hue_color_f[0], hue_color_f[1], hue_color_f[2]);
  AnchorU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
  AnchorU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(
    GfVec4f(R,
            G,
            B,
            style.Alpha));  // Important: this is still including the main rendering/style alpha!!

  GfVec2f sv_cursor_pos;

  if (flags & ANCHOR_ColorEditFlags_PickerHueWheel) {
    // Render Hue Wheel
    const float aeps = 0.5f / wheel_r_outer;  // Half a pixel arc length in radians (2pi cancels out).
    const int segment_per_arc = AnchorMax(4, (int)wheel_r_outer / 12);
    for (int n = 0; n < 6; n++) {
      const float a0 = (n) / 6.0f * 2.0f * IM_PI - aeps;
      const float a1 = (n + 1.0f) / 6.0f * 2.0f * IM_PI + aeps;
      const int vert_start_idx = draw_list->VtxBuffer.Size;
      draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer) * 0.5f, a0, a1, segment_per_arc);
      draw_list->PathStroke(col_white, 0, wheel_thickness);
      const int vert_end_idx = draw_list->VtxBuffer.Size;

      // Paint colors over existing vertices
      GfVec2f gradient_p0(wheel_center[0] + ImCos(a0) * wheel_r_inner,
                          wheel_center[1] + ImSin(a0) * wheel_r_inner);
      GfVec2f gradient_p1(wheel_center[0] + ImCos(a1) * wheel_r_inner,
                          wheel_center[1] + ImSin(a1) * wheel_r_inner);
      ShadeVertsLinearColorGradientKeepAlpha(
        draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
    }

    // Render Cursor + preview on Hue Wheel
    float cos_hue_angle = ImCos(H * 2.0f * IM_PI);
    float sin_hue_angle = ImSin(H * 2.0f * IM_PI);
    GfVec2f hue_cursor_pos(wheel_center[0] + cos_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f,
                           wheel_center[1] + sin_hue_angle * (wheel_r_inner + wheel_r_outer) * 0.5f);
    float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
    int hue_cursor_segments = ImClamp((int)(hue_cursor_rad / 1.4f), 9, 32);
    draw_list->AddCircleFilled(hue_cursor_pos, hue_cursor_rad, hue_color32, hue_cursor_segments);
    draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad + 1, col_midgrey, hue_cursor_segments);
    draw_list->AddCircle(hue_cursor_pos, hue_cursor_rad, col_white, hue_cursor_segments);

    // Render SV triangle (rotated according to hue)
    GfVec2f tra = wheel_center + ImRotate(triangle_pa, cos_hue_angle, sin_hue_angle);
    GfVec2f trb = wheel_center + ImRotate(triangle_pb, cos_hue_angle, sin_hue_angle);
    GfVec2f trc = wheel_center + ImRotate(triangle_pc, cos_hue_angle, sin_hue_angle);
    GfVec2f uv_white = GetFontTexUvWhitePixel();
    draw_list->PrimReserve(6, 6);
    draw_list->PrimVtx(tra, uv_white, hue_color32);
    draw_list->PrimVtx(trb, uv_white, hue_color32);
    draw_list->PrimVtx(trc, uv_white, col_white);
    draw_list->PrimVtx(tra, uv_white, 0);
    draw_list->PrimVtx(trb, uv_white, col_black);
    draw_list->PrimVtx(trc, uv_white, 0);
    draw_list->AddTriangle(tra, trb, trc, col_midgrey, 1.5f);
    sv_cursor_pos = ImLerp(ImLerp(trc, tra, ImSaturate(S)), trb, ImSaturate(1 - V));
  }
  else if (flags & ANCHOR_ColorEditFlags_PickerHueBar) {
    // Render SV Square
    draw_list->AddRectFilledMultiColor(picker_pos,
                                       picker_pos + GfVec2f(sv_picker_size, sv_picker_size),
                                       col_white,
                                       hue_color32,
                                       hue_color32,
                                       col_white);
    draw_list->AddRectFilledMultiColor(
      picker_pos, picker_pos + GfVec2f(sv_picker_size, sv_picker_size), 0, 0, col_black, col_black);
    RenderFrameBorder(picker_pos, picker_pos + GfVec2f(sv_picker_size, sv_picker_size), 0.0f);
    sv_cursor_pos[0] = ImClamp(IM_ROUND(picker_pos[0] + ImSaturate(S) * sv_picker_size),
                               picker_pos[0] + 2,
                               picker_pos[0] + sv_picker_size -
                                 2);  // Sneakily prevent the circle to stick out too much
    sv_cursor_pos[1] = ImClamp(IM_ROUND(picker_pos[1] + ImSaturate(1 - V) * sv_picker_size),
                               picker_pos[1] + 2,
                               picker_pos[1] + sv_picker_size - 2);

    // Render Hue Bar
    for (int i = 0; i < 6; ++i)
      draw_list->AddRectFilledMultiColor(
        GfVec2f(bar0_pos_x, picker_pos[1] + i * (sv_picker_size / 6)),
        GfVec2f(bar0_pos_x + bars_width, picker_pos[1] + (i + 1) * (sv_picker_size / 6)),
        col_hues[i],
        col_hues[i],
        col_hues[i + 1],
        col_hues[i + 1]);
    float bar0_line_y = IM_ROUND(picker_pos[1] + H * sv_picker_size);
    RenderFrameBorder(GfVec2f(bar0_pos_x, picker_pos[1]),
                      GfVec2f(bar0_pos_x + bars_width, picker_pos[1] + sv_picker_size),
                      0.0f);
    RenderArrowsForVerticalBar(draw_list,
                               GfVec2f(bar0_pos_x - 1, bar0_line_y),
                               GfVec2f(bars_triangles_half_sz + 1, bars_triangles_half_sz),
                               bars_width + 2.0f,
                               style.Alpha);
  }

  // Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may
  // lead HSV values to be out of range)
  float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
  draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, user_col32_striped_of_alpha, 12);
  draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1, col_midgrey, 12);
  draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad, col_white, 12);

  // Render alpha bar
  if (alpha_bar) {
    float alpha = ImSaturate(col[3]);
    ImRect bar1_bb(bar1_pos_x, picker_pos[1], bar1_pos_x + bars_width, picker_pos[1] + sv_picker_size);
    RenderColorRectWithAlphaCheckerboard(
      draw_list, bar1_bb.Min, bar1_bb.Max, 0, bar1_bb.GetWidth() / 2.0f, GfVec2f(0.0f, 0.0f));
    draw_list->AddRectFilledMultiColor(bar1_bb.Min,
                                       bar1_bb.Max,
                                       user_col32_striped_of_alpha,
                                       user_col32_striped_of_alpha,
                                       user_col32_striped_of_alpha & ~ANCHOR_COL32_A_MASK,
                                       user_col32_striped_of_alpha & ~ANCHOR_COL32_A_MASK);
    float bar1_line_y = IM_ROUND(picker_pos[1] + (1.0f - alpha) * sv_picker_size);
    RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
    RenderArrowsForVerticalBar(draw_list,
                               GfVec2f(bar1_pos_x - 1, bar1_line_y),
                               GfVec2f(bars_triangles_half_sz + 1, bars_triangles_half_sz),
                               bars_width + 2.0f,
                               style.Alpha);
  }

  EndGroup();

  if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
    value_changed = false;
  if (value_changed)
    MarkItemEdited(window->DC.LastItemId);

  PopID();

  return value_changed;
}

// A little color square. Return true when clicked.
// FIXME: May want to display/ignore the alpha component in the color display? Yet show it in the
// tooltip. 'desc_id' is not called 'label' because we don't display it next to the button, but
// only in the tooltip. Note that 'col' may be encoded in HSV if ANCHOR_ColorEditFlags_InputHSV is
// set.
bool ANCHOR::ColorButton(const char *desc_id, const GfVec4f &col, ANCHOR_ColorEditFlags flags, GfVec2f size)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_ID id = window->GetID(desc_id);
  float default_size = GetFrameHeight();
  if (size[0] == 0.0f)
    size[0] = default_size;
  if (size[1] == 0.0f)
    size[1] = default_size;
  const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
  ItemSize(bb, (size[1] >= default_size) ? g.Style.FramePadding[1] : 0.0f);
  if (!ItemAdd(bb, id))
    return false;

  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held);

  if (flags & ANCHOR_ColorEditFlags_NoAlpha)
    flags &= ~(ANCHOR_ColorEditFlags_AlphaPreview | ANCHOR_ColorEditFlags_AlphaPreviewHalf);

  GfVec4f col_rgb = col;
  if (flags & ANCHOR_ColorEditFlags_InputHSV)
    ColorConvertHSVtoRGB(col_rgb[0], col_rgb[1], col_rgb[2], col_rgb[0], col_rgb[1], col_rgb[2]);

  GfVec4f col_rgb_without_alpha(col_rgb[0], col_rgb[1], col_rgb[2], 1.0f);
  float grid_step = ImMin(size[0], size[1]) / 2.99f;
  float rounding = ImMin(g.Style.FrameRounding, grid_step * 0.5f);
  ImRect bb_inner = bb;
  float off = 0.0f;
  if ((flags & ANCHOR_ColorEditFlags_NoBorder) == 0) {
    off = -0.75f;  // The border (using Col_FrameBg) tends to look off when color is near-opaque
                   // and rounding is enabled. This offset seemed like a good middle ground to
                   // reduce those artifacts.
    bb_inner.Expand(off);
  }
  if ((flags & ANCHOR_ColorEditFlags_AlphaPreviewHalf) && col_rgb[3] < 1.0f) {
    float mid_x = IM_ROUND((bb_inner.Min[0] + bb_inner.Max[0]) * 0.5f);
    RenderColorRectWithAlphaCheckerboard(window->DrawList,
                                         GfVec2f(bb_inner.Min[0] + grid_step, bb_inner.Min[1]),
                                         bb_inner.Max,
                                         GetColorU32(col_rgb),
                                         grid_step,
                                         GfVec2f(-grid_step + off, off),
                                         rounding,
                                         ImDrawFlags_RoundCornersRight);
    window->DrawList->AddRectFilled(bb_inner.Min,
                                    GfVec2f(mid_x, bb_inner.Max[1]),
                                    GetColorU32(col_rgb_without_alpha),
                                    rounding,
                                    ImDrawFlags_RoundCornersLeft);
  }
  else {
    // Because GetColorU32() multiplies by the global style Alpha and we don't want to display a
    // checkerboard if the source code had no alpha
    GfVec4f col_source = (flags & ANCHOR_ColorEditFlags_AlphaPreview) ? col_rgb : col_rgb_without_alpha;
    if (col_source[3] < 1.0f)
      RenderColorRectWithAlphaCheckerboard(window->DrawList,
                                           bb_inner.Min,
                                           bb_inner.Max,
                                           GetColorU32(col_source),
                                           grid_step,
                                           GfVec2f(off, off),
                                           rounding);
    else
      window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), rounding);
  }
  RenderNavHighlight(bb, id);
  if ((flags & ANCHOR_ColorEditFlags_NoBorder) == 0) {
    if (g.Style.FrameBorderSize > 0.0f)
      RenderFrameBorder(bb.Min, bb.Max, rounding);
    else
      window->DrawList->AddRect(bb.Min,
                                bb.Max,
                                GetColorU32(ANCHOR_Col_FrameBg),
                                rounding);  // Color button are often in need of some sort of border
  }

  // Drag and Drop Source
  // NB: The ActiveId test is merely an optional micro-optimization, BeginDragDropSource() does the
  // same test.
  if (g.ActiveId == id && !(flags & ANCHOR_ColorEditFlags_NoDragDrop) && BeginDragDropSource()) {
    if (flags & ANCHOR_ColorEditFlags_NoAlpha)
      SetDragDropPayload(ANCHOR_PAYLOAD_TYPE_COLOR_3F, &col_rgb, sizeof(float) * 3, ANCHOR_Cond_Once);
    else
      SetDragDropPayload(ANCHOR_PAYLOAD_TYPE_COLOR_4F, &col_rgb, sizeof(float) * 4, ANCHOR_Cond_Once);
    ColorButton(desc_id, col, flags);
    SameLine();
    TextEx("Color");
    EndDragDropSource();
  }

  // Tooltip
  if (!(flags & ANCHOR_ColorEditFlags_NoTooltip) && hovered)
    ColorTooltip(desc_id,
                 &col[0],
                 flags & (ANCHOR_ColorEditFlags__InputMask | ANCHOR_ColorEditFlags_NoAlpha |
                          ANCHOR_ColorEditFlags_AlphaPreview | ANCHOR_ColorEditFlags_AlphaPreviewHalf));

  return pressed;
}

// Initialize/override default color options
void ANCHOR::SetColorEditOptions(ANCHOR_ColorEditFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  if ((flags & ANCHOR_ColorEditFlags__DisplayMask) == 0)
    flags |= ANCHOR_ColorEditFlags__OptionsDefault & ANCHOR_ColorEditFlags__DisplayMask;
  if ((flags & ANCHOR_ColorEditFlags__DataTypeMask) == 0)
    flags |= ANCHOR_ColorEditFlags__OptionsDefault & ANCHOR_ColorEditFlags__DataTypeMask;
  if ((flags & ANCHOR_ColorEditFlags__PickerMask) == 0)
    flags |= ANCHOR_ColorEditFlags__OptionsDefault & ANCHOR_ColorEditFlags__PickerMask;
  if ((flags & ANCHOR_ColorEditFlags__InputMask) == 0)
    flags |= ANCHOR_ColorEditFlags__OptionsDefault & ANCHOR_ColorEditFlags__InputMask;
  ANCHOR_ASSERT(
    ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__DisplayMask));  // Check only 1 option is selected
  ANCHOR_ASSERT(
    ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__DataTypeMask));  // Check only 1 option is selected
  ANCHOR_ASSERT(
    ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__PickerMask));  // Check only 1 option is selected
  ANCHOR_ASSERT(
    ImIsPowerOfTwo(flags & ANCHOR_ColorEditFlags__InputMask));  // Check only 1 option is selected
  g.ColorEditOptions = flags;
}

// Note: only access 3 floats if ANCHOR_ColorEditFlags_NoAlpha flag is set.
void ANCHOR::ColorTooltip(const char *text, const float *col, ANCHOR_ColorEditFlags flags)
{
  ANCHOR_Context &g = *G_CTX;

  BeginTooltipEx(0, ANCHOR_TooltipFlags_OverridePreviousTooltip);
  const char *text_end = text ? FindRenderedTextEnd(text, NULL) : text;
  if (text_end > text) {
    TextEx(text, text_end);
    Separator();
  }

  GfVec2f sz(g.FontSize * 3 + g.Style.FramePadding[1] * 2, g.FontSize * 3 + g.Style.FramePadding[1] * 2);
  GfVec4f cf(col[0], col[1], col[2], (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 1.0f : col[3]);
  int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]),
      ca = (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
  ColorButton("##preview",
              cf,
              (flags & (ANCHOR_ColorEditFlags__InputMask | ANCHOR_ColorEditFlags_NoAlpha |
                        ANCHOR_ColorEditFlags_AlphaPreview | ANCHOR_ColorEditFlags_AlphaPreviewHalf)) |
                ANCHOR_ColorEditFlags_NoTooltip,
              sz);
  SameLine();
  if ((flags & ANCHOR_ColorEditFlags_InputRGB) || !(flags & ANCHOR_ColorEditFlags__InputMask)) {
    if (flags & ANCHOR_ColorEditFlags_NoAlpha)
      Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)",
           cr,
           cg,
           cb,
           cr,
           cg,
           cb,
           col[0],
           col[1],
           col[2]);
    else
      Text("#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)",
           cr,
           cg,
           cb,
           ca,
           cr,
           cg,
           cb,
           ca,
           col[0],
           col[1],
           col[2],
           col[3]);
  }
  else if (flags & ANCHOR_ColorEditFlags_InputHSV) {
    if (flags & ANCHOR_ColorEditFlags_NoAlpha)
      Text("H: %.3f, S: %.3f, V: %.3f", col[0], col[1], col[2]);
    else
      Text("H: %.3f, S: %.3f, V: %.3f, A: %.3f", col[0], col[1], col[2], col[3]);
  }
  EndTooltip();
}

void ANCHOR::ColorEditOptionsPopup(const float *col, ANCHOR_ColorEditFlags flags)
{
  bool allow_opt_inputs = !(flags & ANCHOR_ColorEditFlags__DisplayMask);
  bool allow_opt_datatype = !(flags & ANCHOR_ColorEditFlags__DataTypeMask);
  if ((!allow_opt_inputs && !allow_opt_datatype) || !BeginPopup("context"))
    return;
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_ColorEditFlags opts = g.ColorEditOptions;
  if (allow_opt_inputs) {
    if (RadioButton("RGB", (opts & ANCHOR_ColorEditFlags_DisplayRGB) != 0))
      opts = (opts & ~ANCHOR_ColorEditFlags__DisplayMask) | ANCHOR_ColorEditFlags_DisplayRGB;
    if (RadioButton("HSV", (opts & ANCHOR_ColorEditFlags_DisplayHSV) != 0))
      opts = (opts & ~ANCHOR_ColorEditFlags__DisplayMask) | ANCHOR_ColorEditFlags_DisplayHSV;
    if (RadioButton("Hex", (opts & ANCHOR_ColorEditFlags_DisplayHex) != 0))
      opts = (opts & ~ANCHOR_ColorEditFlags__DisplayMask) | ANCHOR_ColorEditFlags_DisplayHex;
  }
  if (allow_opt_datatype) {
    if (allow_opt_inputs)
      Separator();
    if (RadioButton("0..255", (opts & ANCHOR_ColorEditFlags_Uint8) != 0))
      opts = (opts & ~ANCHOR_ColorEditFlags__DataTypeMask) | ANCHOR_ColorEditFlags_Uint8;
    if (RadioButton("0.00..1.00", (opts & ANCHOR_ColorEditFlags_Float) != 0))
      opts = (opts & ~ANCHOR_ColorEditFlags__DataTypeMask) | ANCHOR_ColorEditFlags_Float;
  }

  if (allow_opt_inputs || allow_opt_datatype)
    Separator();
  if (Button("Copy as..", GfVec2f(-1, 0)))
    OpenPopup("Copy");
  if (BeginPopup("Copy")) {
    int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]),
        ca = (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
    char buf[64];
    ImFormatString(buf,
                   ANCHOR_ARRAYSIZE(buf),
                   "(%.3ff, %.3ff, %.3ff, %.3ff)",
                   col[0],
                   col[1],
                   col[2],
                   (flags & ANCHOR_ColorEditFlags_NoAlpha) ? 1.0f : col[3]);
    if (Selectable(buf))
      SetClipboardText(buf);
    ImFormatString(buf, ANCHOR_ARRAYSIZE(buf), "(%d,%d,%d,%d)", cr, cg, cb, ca);
    if (Selectable(buf))
      SetClipboardText(buf);
    ImFormatString(buf, ANCHOR_ARRAYSIZE(buf), "#%02X%02X%02X", cr, cg, cb);
    if (Selectable(buf))
      SetClipboardText(buf);
    if (!(flags & ANCHOR_ColorEditFlags_NoAlpha)) {
      ImFormatString(buf, ANCHOR_ARRAYSIZE(buf), "#%02X%02X%02X%02X", cr, cg, cb, ca);
      if (Selectable(buf))
        SetClipboardText(buf);
    }
    EndPopup();
  }

  g.ColorEditOptions = opts;
  EndPopup();
}

void ANCHOR::ColorPickerOptionsPopup(const float *ref_col, ANCHOR_ColorEditFlags flags)
{
  bool allow_opt_picker = !(flags & ANCHOR_ColorEditFlags__PickerMask);
  bool allow_opt_alpha_bar = !(flags & ANCHOR_ColorEditFlags_NoAlpha) &&
                             !(flags & ANCHOR_ColorEditFlags_AlphaBar);
  if ((!allow_opt_picker && !allow_opt_alpha_bar) || !BeginPopup("context"))
    return;
  ANCHOR_Context &g = *G_CTX;
  if (allow_opt_picker) {
    GfVec2f picker_size(g.FontSize * 8,
                        AnchorMax(g.FontSize * 8 - (GetFrameHeight() + g.Style.ItemInnerSpacing[0]),
                                  1.0f));  // FIXME: Picker size copied from main picker function
    PushItemWidth(picker_size[0]);
    for (int picker_type = 0; picker_type < 2; picker_type++) {
      // Draw small/thumbnail version of each picker type (over an invisible button for selection)
      if (picker_type > 0)
        Separator();
      PushID(picker_type);
      ANCHOR_ColorEditFlags picker_flags = ANCHOR_ColorEditFlags_NoInputs | ANCHOR_ColorEditFlags_NoOptions |
                                           ANCHOR_ColorEditFlags_NoLabel |
                                           ANCHOR_ColorEditFlags_NoSidePreview |
                                           (flags & ANCHOR_ColorEditFlags_NoAlpha);
      if (picker_type == 0)
        picker_flags |= ANCHOR_ColorEditFlags_PickerHueBar;
      if (picker_type == 1)
        picker_flags |= ANCHOR_ColorEditFlags_PickerHueWheel;
      GfVec2f backup_pos = GetCursorScreenPos();
      if (Selectable("##selectable", false, 0, picker_size))  // By default, Selectable() is closing popup
        g.ColorEditOptions = (g.ColorEditOptions & ~ANCHOR_ColorEditFlags__PickerMask) |
                             (picker_flags & ANCHOR_ColorEditFlags__PickerMask);
      SetCursorScreenPos(backup_pos);
      GfVec4f previewing_ref_col;
      memcpy(&previewing_ref_col,
             ref_col,
             sizeof(float) * ((picker_flags & ANCHOR_ColorEditFlags_NoAlpha) ? 3 : 4));
      ColorPicker4("##previewing_picker", &previewing_ref_col[0], picker_flags);
      PopID();
    }
    PopItemWidth();
  }
  if (allow_opt_alpha_bar) {
    if (allow_opt_picker)
      Separator();
    CheckboxFlags("Alpha Bar", &g.ColorEditOptions, ANCHOR_ColorEditFlags_AlphaBar);
  }
  EndPopup();
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
//-------------------------------------------------------------------------
// - TreeNode()
// - TreeNodeV()
// - TreeNodeEx()
// - TreeNodeExV()
// - TreeNodeBehavior() [Internal]
// - TreePush()
// - TreePop()
// - GetTreeNodeToLabelSpacing()
// - SetNextItemOpen()
// - CollapsingHeader()
//-------------------------------------------------------------------------

bool ANCHOR::TreeNode(const char *str_id, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(str_id, 0, fmt, args);
  va_end(args);
  return is_open;
}

bool ANCHOR::TreeNode(const void *ptr_id, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
  va_end(args);
  return is_open;
}

bool ANCHOR::TreeNode(const char *label)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;
  return TreeNodeBehavior(window->GetID(label), 0, label, NULL);
}

bool ANCHOR::TreeNodeV(const char *str_id, const char *fmt, va_list args)
{
  return TreeNodeExV(str_id, 0, fmt, args);
}

bool ANCHOR::TreeNodeV(const void *ptr_id, const char *fmt, va_list args)
{
  return TreeNodeExV(ptr_id, 0, fmt, args);
}

bool ANCHOR::TreeNodeEx(const char *label, ANCHOR_TreeNodeFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  return TreeNodeBehavior(window->GetID(label), flags, label, NULL);
}

bool ANCHOR::TreeNodeEx(const char *str_id, ANCHOR_TreeNodeFlags flags, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(str_id, flags, fmt, args);
  va_end(args);
  return is_open;
}

bool ANCHOR::TreeNodeEx(const void *ptr_id, ANCHOR_TreeNodeFlags flags, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
  va_end(args);
  return is_open;
}

bool ANCHOR::TreeNodeExV(const char *str_id, ANCHOR_TreeNodeFlags flags, const char *fmt, va_list args)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const char *label_end = g.TempBuffer +
                          ImFormatStringV(g.TempBuffer, ANCHOR_ARRAYSIZE(g.TempBuffer), fmt, args);
  return TreeNodeBehavior(window->GetID(str_id), flags, g.TempBuffer, label_end);
}

bool ANCHOR::TreeNodeExV(const void *ptr_id, ANCHOR_TreeNodeFlags flags, const char *fmt, va_list args)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const char *label_end = g.TempBuffer +
                          ImFormatStringV(g.TempBuffer, ANCHOR_ARRAYSIZE(g.TempBuffer), fmt, args);
  return TreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
}

bool ANCHOR::TreeNodeBehaviorIsOpen(ANCHOR_ID id, ANCHOR_TreeNodeFlags flags)
{
  if (flags & ANCHOR_TreeNodeFlags_Leaf)
    return true;

  // We only write to the tree storage if the user clicks (or explicitly use the SetNextItemOpen
  // function)
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  ANCHORStorage *storage = window->DC.StateStorage;

  bool is_open;
  if (g.NextItemData.Flags & ANCHOR_NextItemDataFlags_HasOpen) {
    if (g.NextItemData.OpenCond & ANCHOR_Cond_Always) {
      is_open = g.NextItemData.OpenVal;
      storage->SetInt(id, is_open);
    }
    else {
      // We treat ANCHOR_Cond_Once and ANCHOR_Cond_FirstUseEver the same because tree node state
      // are not saved persistently.
      const int stored_value = storage->GetInt(id, -1);
      if (stored_value == -1) {
        is_open = g.NextItemData.OpenVal;
        storage->SetInt(id, is_open);
      }
      else {
        is_open = stored_value != 0;
      }
    }
  }
  else {
    is_open = storage->GetInt(id, (flags & ANCHOR_TreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
  }

  // When logging is enabled, we automatically expand tree nodes (but *NOT* collapsing headers..
  // seems like sensible behavior). NB- If we are above max depth we still allow manually opened
  // nodes to be logged.
  if (g.LogEnabled && !(flags & ANCHOR_TreeNodeFlags_NoAutoOpenOnLog) &&
      (window->DC.TreeDepth - g.LogDepthRef) < g.LogDepthToExpand)
    is_open = true;

  return is_open;
}

bool ANCHOR::TreeNodeBehavior(ANCHOR_ID id,
                              ANCHOR_TreeNodeFlags flags,
                              const char *label,
                              const char *label_end)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const bool display_frame = (flags & ANCHOR_TreeNodeFlags_Framed) != 0;
  const GfVec2f padding = (display_frame || (flags & ANCHOR_TreeNodeFlags_FramePadding)) ?
                            style.FramePadding :
                            GfVec2f(style.FramePadding[0],
                                    ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding[1]));

  if (!label_end)
    label_end = FindRenderedTextEnd(label);
  const GfVec2f label_size = CalcTextSize(label, label_end, false);

  // We vertically grow up to current line height up the typical widget height.
  const float frame_height = AnchorMax(
    ImMin(window->DC.CurrLineSize[1], g.FontSize + style.FramePadding[1] * 2),
    label_size[1] + padding[1] * 2);
  ImRect frame_bb;
  frame_bb.Min[0] = (flags & ANCHOR_TreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min[0] :
                                                                   window->DC.CursorPos[0];
  frame_bb.Min[1] = window->DC.CursorPos[1];
  frame_bb.Max[0] = window->WorkRect.Max[0];
  frame_bb.Max[1] = window->DC.CursorPos[1] + frame_height;
  if (display_frame) {
    // Framed header expand a little outside the default padding, to the edge of InnerClipRect
    // (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding[0]
    // instead of WindowPadding[0]*0.5f)
    frame_bb.Min[0] -= IM_FLOOR(window->WindowPadding[0] * 0.5f - 1.0f);
    frame_bb.Max[0] += IM_FLOOR(window->WindowPadding[0] * 0.5f);
  }

  const float text_offset_x = g.FontSize + (display_frame ?
                                              padding[0] * 3 :
                                              padding[0] * 2);  // Collapser arrow width + Spacing
  const float text_offset_y = AnchorMax(
    padding[1], window->DC.CurrLineTextBaseOffset);  // Latch before ItemSize changes it
  const float text_width = g.FontSize + (label_size[0] > 0.0f ? label_size[0] + padding[0] * 2 :
                                                                0.0f);  // Include collapser
  GfVec2f text_pos(window->DC.CursorPos[0] + text_offset_x, window->DC.CursorPos[1] + text_offset_y);
  ItemSize(GfVec2f(text_width, frame_height), padding[1]);

  // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
  ImRect interact_bb = frame_bb;
  if (!display_frame &&
      (flags & (ANCHOR_TreeNodeFlags_SpanAvailWidth | ANCHOR_TreeNodeFlags_SpanFullWidth)) == 0)
    interact_bb.Max[0] = frame_bb.Min[0] + text_width + style.ItemSpacing[0] * 2.0f;

  // Store a flag for the current depth to tell if we will allow closing this node when navigating
  // one of its child. For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1
  // between TreeNode() and TreePop(). This is currently only support 32 level deep and we are fine
  // with (1 << Depth) overflowing into a zero.
  const bool is_leaf = (flags & ANCHOR_TreeNodeFlags_Leaf) != 0;
  bool is_open = TreeNodeBehaviorIsOpen(id, flags);
  if (is_open && !g.NavIdIsAlive && (flags & ANCHOR_TreeNodeFlags_NavLeftJumpsBackHere) &&
      !(flags & ANCHOR_TreeNodeFlags_NoTreePushOnOpen))
    window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

  bool item_add = ItemAdd(interact_bb, id);
  window->DC.LastItemStatusFlags |= ANCHOR_ItemStatusFlags_HasDisplayRect;
  window->DC.LastItemDisplayRect = frame_bb;

  if (!item_add) {
    if (is_open && !(flags & ANCHOR_TreeNodeFlags_NoTreePushOnOpen))
      TreePushOverrideID(id);
    ANCHOR_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId,
                                 label,
                                 window->DC.LastItemStatusFlags |
                                   (is_leaf ? 0 : ANCHOR_ItemStatusFlags_Openable) |
                                   (is_open ? ANCHOR_ItemStatusFlags_Opened : 0));
    return is_open;
  }

  ANCHOR_ButtonFlags button_flags = ANCHOR_TreeNodeFlags_None;
  if (flags & ANCHOR_TreeNodeFlags_AllowItemOverlap)
    button_flags |= ANCHOR_ButtonFlags_AllowItemOverlap;
  if (!is_leaf)
    button_flags |= ANCHOR_ButtonFlags_PressedOnDragDropHold;

  // We allow clicking on the arrow section with keyboard modifiers held, in order to easily
  // allow browsing a tree while preserving selection with code implementing multi-selection
  // patterns. When clicking on the rest of the tree node we always disallow keyboard modifiers.
  const float arrow_hit_x1 = (text_pos[0] - text_offset_x) - style.TouchExtraPadding[0];
  const float arrow_hit_x2 = (text_pos[0] - text_offset_x) + (g.FontSize + padding[0] * 2.0f) +
                             style.TouchExtraPadding[0];
  const bool is_mouse_x_over_arrow = (g.IO.MousePos[0] >= arrow_hit_x1 && g.IO.MousePos[0] < arrow_hit_x2);
  if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
    button_flags |= ANCHOR_ButtonFlags_NoKeyModifiers;

  // Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
  // Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to
  // requirements for multi-selection and drag and drop support.
  // - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
  // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
  // - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
  // - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
  // - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and
  // _OpenOnArrow=0) It is rather standard that arrow click react on Down rather than Up. We set
  // ANCHOR_ButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be
  // active on the initial MouseDown in order for drag and drop to work.
  if (is_mouse_x_over_arrow)
    button_flags |= ANCHOR_ButtonFlags_PressedOnClick;
  else if (flags & ANCHOR_TreeNodeFlags_OpenOnDoubleClick)
    button_flags |= ANCHOR_ButtonFlags_PressedOnClickRelease | ANCHOR_ButtonFlags_PressedOnDoubleClick;
  else
    button_flags |= ANCHOR_ButtonFlags_PressedOnClickRelease;

  bool selected = (flags & ANCHOR_TreeNodeFlags_Selected) != 0;
  const bool was_selected = selected;

  bool hovered, held;
  bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
  bool toggled = false;
  if (!is_leaf) {
    if (pressed && g.DragDropHoldJustPressedId != id) {
      if ((flags & (ANCHOR_TreeNodeFlags_OpenOnArrow | ANCHOR_TreeNodeFlags_OpenOnDoubleClick)) == 0 ||
          (g.NavActivateId == id))
        toggled = true;
      if (flags & ANCHOR_TreeNodeFlags_OpenOnArrow)
        toggled |= is_mouse_x_over_arrow &&
                   !g.NavDisableMouseHover;  // Lightweight equivalent of IsMouseHoveringRect()
                                             // since ButtonBehavior() already did the job
      if ((flags & ANCHOR_TreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
        toggled = true;
    }
    else if (pressed && g.DragDropHoldJustPressedId == id) {
      ANCHOR_ASSERT(button_flags & ANCHOR_ButtonFlags_PressedOnDragDropHold);
      if (!is_open)  // When using Drag and Drop "hold to open" we keep the node highlighted after
                     // opening, but never close it again.
        toggled = true;
    }

    if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ANCHOR_Dir_Left && is_open) {
      toggled = true;
      NavMoveRequestCancel();
    }
    if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ANCHOR_Dir_Right &&
        !is_open)  // If there's something upcoming on the line we may want to give it the
                   // priority?
    {
      toggled = true;
      NavMoveRequestCancel();
    }

    if (toggled) {
      is_open = !is_open;
      window->DC.StateStorage->SetInt(id, is_open);
      window->DC.LastItemStatusFlags |= ANCHOR_ItemStatusFlags_ToggledOpen;
    }
  }
  if (flags & ANCHOR_TreeNodeFlags_AllowItemOverlap)
    SetItemAllowOverlap();

  // In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
  if (selected != was_selected)  //-V547
    window->DC.LastItemStatusFlags |= ANCHOR_ItemStatusFlags_ToggledSelection;

  // Render
  const AnchorU32 text_col = GetColorU32(ANCHOR_Col_Text);
  ANCHOR_NavHighlightFlags nav_highlight_flags = ANCHOR_NavHighlightFlags_TypeThin;
  if (display_frame) {
    // Framed type
    const AnchorU32 bg_col = GetColorU32((held && hovered) ? ANCHOR_Col_HeaderActive :
                                         hovered           ? ANCHOR_Col_HeaderHovered :
                                                             ANCHOR_Col_Header);
    RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
    RenderNavHighlight(frame_bb, id, nav_highlight_flags);
    if (flags & ANCHOR_TreeNodeFlags_Bullet)
      RenderBullet(window->DrawList,
                   GfVec2f(text_pos[0] - text_offset_x * 0.60f, text_pos[1] + g.FontSize * 0.5f),
                   text_col);
    else if (!is_leaf)
      RenderArrow(window->DrawList,
                  GfVec2f(text_pos[0] - text_offset_x + padding[0], text_pos[1]),
                  text_col,
                  is_open ? ANCHOR_Dir_Down : ANCHOR_Dir_Right,
                  1.0f);
    else  // Leaf without bullet, left-adjusted text
      text_pos[0] -= text_offset_x;
    if (flags & ANCHOR_TreeNodeFlags_ClipLabelForTrailingButton)
      frame_bb.Max[0] -= g.FontSize + style.FramePadding[0];

    if (g.LogEnabled)
      LogSetNextTextDecoration("###", "###");
    RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
  }
  else {
    // Unframed typed for tree nodes
    if (hovered || selected) {
      const AnchorU32 bg_col = GetColorU32((held && hovered) ? ANCHOR_Col_HeaderActive :
                                           hovered           ? ANCHOR_Col_HeaderHovered :
                                                               ANCHOR_Col_Header);
      RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
      RenderNavHighlight(frame_bb, id, nav_highlight_flags);
    }
    if (flags & ANCHOR_TreeNodeFlags_Bullet)
      RenderBullet(window->DrawList,
                   GfVec2f(text_pos[0] - text_offset_x * 0.5f, text_pos[1] + g.FontSize * 0.5f),
                   text_col);
    else if (!is_leaf)
      RenderArrow(window->DrawList,
                  GfVec2f(text_pos[0] - text_offset_x + padding[0], text_pos[1] + g.FontSize * 0.15f),
                  text_col,
                  is_open ? ANCHOR_Dir_Down : ANCHOR_Dir_Right,
                  0.70f);
    if (g.LogEnabled)
      LogSetNextTextDecoration(">", NULL);
    RenderText(text_pos, label, label_end, false);
  }

  if (is_open && !(flags & ANCHOR_TreeNodeFlags_NoTreePushOnOpen))
    TreePushOverrideID(id);
  ANCHOR_TEST_ENGINE_ITEM_INFO(id,
                               label,
                               window->DC.LastItemStatusFlags |
                                 (is_leaf ? 0 : ANCHOR_ItemStatusFlags_Openable) |
                                 (is_open ? ANCHOR_ItemStatusFlags_Opened : 0));
  return is_open;
}

void ANCHOR::TreePush(const char *str_id)
{
  ANCHOR_Window *window = GetCurrentWindow();
  Indent();
  window->DC.TreeDepth++;
  PushID(str_id ? str_id : "#TreePush");
}

void ANCHOR::TreePush(const void *ptr_id)
{
  ANCHOR_Window *window = GetCurrentWindow();
  Indent();
  window->DC.TreeDepth++;
  PushID(ptr_id ? ptr_id : (const void *)"#TreePush");
}

void ANCHOR::TreePushOverrideID(ANCHOR_ID id)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  Indent();
  window->DC.TreeDepth++;
  window->IDStack.push_back(id);
}

void ANCHOR::TreePop()
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  Unindent();

  window->DC.TreeDepth--;
  AnchorU32 tree_depth_mask = (1 << window->DC.TreeDepth);

  // Handle Left arrow to move to parent tree node (when ANCHOR_TreeNodeFlags_NavLeftJumpsBackHere
  // is enabled)
  if (g.NavMoveDir == ANCHOR_Dir_Left && g.NavWindow == window && NavMoveRequestButNoResultYet())
    if (g.NavIdIsAlive && (window->DC.TreeJumpToParentOnPopMask & tree_depth_mask)) {
      SetNavID(window->IDStack.back(), g.NavLayer, 0, ImRect());
      NavMoveRequestCancel();
    }
  window->DC.TreeJumpToParentOnPopMask &= tree_depth_mask - 1;

  ANCHOR_ASSERT(window->IDStack.Size >
                1);  // There should always be 1 element in the IDStack (pushed during window
                     // creation). If this triggers you called TreePop/PopID too much.
  PopID();
}

// Horizontal distance preceding label when using TreeNode() or Bullet()
float ANCHOR::GetTreeNodeToLabelSpacing()
{
  ANCHOR_Context &g = *G_CTX;
  return g.FontSize + (g.Style.FramePadding[0] * 2.0f);
}

// Set next TreeNode/CollapsingHeader open state.
void ANCHOR::SetNextItemOpen(bool is_open, ANCHOR_Cond cond)
{
  ANCHOR_Context &g = *G_CTX;
  if (g.CurrentWindow->SkipItems)
    return;
  g.NextItemData.Flags |= ANCHOR_NextItemDataFlags_HasOpen;
  g.NextItemData.OpenVal = is_open;
  g.NextItemData.OpenCond = cond ? cond : ANCHOR_Cond_Always;
}

// CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because
// of the ANCHOR_TreeNodeFlags_NoTreePushOnOpen flag). This is basically the same as calling
// TreeNodeEx(label, ANCHOR_TreeNodeFlags_CollapsingHeader). You can remove the _NoTreePushOnOpen
// flag if you want behavior closer to normal TreeNode().
bool ANCHOR::CollapsingHeader(const char *label, ANCHOR_TreeNodeFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  return TreeNodeBehavior(window->GetID(label), flags | ANCHOR_TreeNodeFlags_CollapsingHeader, label);
}

// p_visible == NULL                        : regular collapsing header
// p_visible != NULL && *p_visible == true  : show a small close button on the corner of the
// header, clicking the button will set *p_visible = false p_visible != NULL && *p_visible == false
// : do not show the header at all Do not mistake this with the Open state of the header itself,
// which you can adjust with SetNextItemOpen() or ANCHOR_TreeNodeFlags_DefaultOpen.
bool ANCHOR::CollapsingHeader(const char *label, bool *p_visible, ANCHOR_TreeNodeFlags flags)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  if (p_visible && !*p_visible)
    return false;

  ANCHOR_ID id = window->GetID(label);
  flags |= ANCHOR_TreeNodeFlags_CollapsingHeader;
  if (p_visible)
    flags |= ANCHOR_TreeNodeFlags_AllowItemOverlap | ANCHOR_TreeNodeFlags_ClipLabelForTrailingButton;
  bool is_open = TreeNodeBehavior(id, flags, label);
  if (p_visible != NULL) {
    // Create a small overlapping close button
    // FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars,
    // headers, etc.
    // FIXME: CloseButton can overlap into text, need find a way to clip the text somehow.
    ANCHOR_Context &g = *G_CTX;
    ANCHOR_LastItemDataBackup last_item_backup;
    float button_size = g.FontSize;
    float button_x = AnchorMax(window->DC.LastItemRect.Min[0],
                               window->DC.LastItemRect.Max[0] - g.Style.FramePadding[0] * 2.0f -
                                 button_size);
    float button_y = window->DC.LastItemRect.Min[1];
    ANCHOR_ID close_button_id = GetIDWithSeed("#CLOSE", NULL, id);
    if (CloseButton(close_button_id, GfVec2f(button_x, button_y)))
      *p_visible = false;
    last_item_backup.Restore();
  }

  return is_open;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Selectable
//-------------------------------------------------------------------------
// - Selectable()
//-------------------------------------------------------------------------

// Tip: pass a non-visible label (e.g. "##hello") then you can use the space to draw other text or
// image. But you need to make sure the ID is unique, e.g. enclose calls in PushID/PopID or use
// ##unique_id. With this scheme, ANCHORSelectableFlags_SpanAllColumns and
// ANCHORSelectableFlags_AllowItemOverlap are also frequently used flags.
// FIXME: Selectable() with (size[0] == 0.0f) and (SelectableTextAlign[0] > 0.0f) followed by
// SameLine() is currently not supported.
bool ANCHOR::Selectable(const char *label,
                        bool selected,
                        ANCHORSelectableFlags flags,
                        const GfVec2f &size_arg)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;

  // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning
  // rectangle.
  ANCHOR_ID id = window->GetID(label);
  GfVec2f label_size = CalcTextSize(label, NULL, true);
  GfVec2f size(size_arg[0] != 0.0f ? size_arg[0] : label_size[0],
               size_arg[1] != 0.0f ? size_arg[1] : label_size[1]);
  GfVec2f pos = window->DC.CursorPos;
  pos[1] += window->DC.CurrLineTextBaseOffset;
  ItemSize(size, 0.0f);

  // Fill horizontal space
  // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make
  // explicitly right-aligned sizes not visibly match other widgets.
  const bool span_all_columns = (flags & ANCHORSelectableFlags_SpanAllColumns) != 0;
  const float min_x = span_all_columns ? window->ParentWorkRect.Min[0] : pos[0];
  const float max_x = span_all_columns ? window->ParentWorkRect.Max[0] : window->WorkRect.Max[0];
  if (size_arg[0] == 0.0f || (flags & ANCHORSelectableFlags_SpanAvailWidth))
    size[0] = AnchorMax(label_size[0], max_x - min_x);

  // Text stays at the submission position, but bounding box may be extended on both sides
  const GfVec2f text_min = pos;
  const GfVec2f text_max(min_x + size[0], pos[1] + size[1]);

  // Selectables are meant to be tightly packed together with no click-gap, so we extend their box
  // to cover spacing between selectable.
  ImRect bb(min_x, pos[1], text_max[0], text_max[1]);
  if ((flags & ANCHORSelectableFlags_NoPadWithHalfSpacing) == 0) {
    const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing[0];
    const float spacing_y = style.ItemSpacing[1];
    const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
    const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
    bb.Min[0] -= spacing_L;
    bb.Min[1] -= spacing_U;
    bb.Max[0] += (spacing_x - spacing_L);
    bb.Max[1] += (spacing_y - spacing_U);
  }
  // if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, ANCHOR_COL32(0, 255, 0,
  // 255)); }

  // Modify ClipRect for the ItemAdd(), faster than doing a
  // PushColumnsBackground/PushTableBackground for every Selectable..
  const float backup_clip_rect_min_x = window->ClipRect.Min[0];
  const float backup_clip_rect_max_x = window->ClipRect.Max[0];
  if (span_all_columns) {
    window->ClipRect.Min[0] = window->ParentWorkRect.Min[0];
    window->ClipRect.Max[0] = window->ParentWorkRect.Max[0];
  }

  bool item_add;
  if (flags & ANCHORSelectableFlags_Disabled) {
    ANCHOR_ItemFlags backup_item_flags = g.CurrentItemFlags;
    g.CurrentItemFlags |= ANCHOR_ItemFlags_Disabled | ANCHOR_ItemFlags_NoNavDefaultFocus;
    item_add = ItemAdd(bb, id);
    g.CurrentItemFlags = backup_item_flags;
  }
  else {
    item_add = ItemAdd(bb, id);
  }

  if (span_all_columns) {
    window->ClipRect.Min[0] = backup_clip_rect_min_x;
    window->ClipRect.Max[0] = backup_clip_rect_max_x;
  }

  if (!item_add)
    return false;

  // FIXME: We can standardize the behavior of those two, we could also keep the fast path of
  // override ClipRect + full push on render only, which would be advantageous since most
  // selectable are not selected.
  if (span_all_columns && window->DC.CurrentColumns)
    PushColumnsBackground();
  else if (span_all_columns && g.CurrentTable)
    TablePushBackgroundChannel();

  // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse
  // child entries
  ANCHOR_ButtonFlags button_flags = 0;
  if (flags & ANCHORSelectableFlags_NoHoldingActiveID) {
    button_flags |= ANCHOR_ButtonFlags_NoHoldingActiveId;
  }
  if (flags & ANCHORSelectableFlags_SelectOnClick) {
    button_flags |= ANCHOR_ButtonFlags_PressedOnClick;
  }
  if (flags & ANCHORSelectableFlags_SelectOnRelease) {
    button_flags |= ANCHOR_ButtonFlags_PressedOnRelease;
  }
  if (flags & ANCHORSelectableFlags_Disabled) {
    button_flags |= ANCHOR_ButtonFlags_Disabled;
  }
  if (flags & ANCHORSelectableFlags_AllowDoubleClick) {
    button_flags |= ANCHOR_ButtonFlags_PressedOnClickRelease | ANCHOR_ButtonFlags_PressedOnDoubleClick;
  }
  if (flags & ANCHORSelectableFlags_AllowItemOverlap) {
    button_flags |= ANCHOR_ButtonFlags_AllowItemOverlap;
  }

  if (flags & ANCHORSelectableFlags_Disabled)
    selected = false;

  const bool was_selected = selected;
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

  // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so
  // navigation can be resumed with gamepad/keyboard
  if (pressed || (hovered && (flags & ANCHORSelectableFlags_SetNavIdOnHover))) {
    if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent) {
      SetNavID(id,
               window->DC.NavLayerCurrent,
               window->DC.NavFocusScopeIdCurrent,
               ImRect(bb.Min - window->Pos, bb.Max - window->Pos));
      g.NavDisableHighlight = true;
    }
  }
  if (pressed)
    MarkItemEdited(id);

  if (flags & ANCHORSelectableFlags_AllowItemOverlap)
    SetItemAllowOverlap();

  // In this branch, Selectable() cannot toggle the selection so this will never trigger.
  if (selected != was_selected)  //-V547
    window->DC.LastItemStatusFlags |= ANCHOR_ItemStatusFlags_ToggledSelection;

  // Render
  if (held && (flags & ANCHORSelectableFlags_DrawHoveredWhenHeld))
    hovered = true;
  if (hovered || selected) {
    const AnchorU32 col = GetColorU32((held && hovered) ? ANCHOR_Col_HeaderActive :
                                      hovered           ? ANCHOR_Col_HeaderHovered :
                                                          ANCHOR_Col_Header);
    RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
    RenderNavHighlight(bb, id, ANCHOR_NavHighlightFlags_TypeThin | ANCHOR_NavHighlightFlags_NoRounding);
  }

  if (span_all_columns && window->DC.CurrentColumns)
    PopColumnsBackground();
  else if (span_all_columns && g.CurrentTable)
    TablePopBackgroundChannel();

  if (flags & ANCHORSelectableFlags_Disabled)
    PushStyleColor(ANCHOR_Col_Text, style.Colors[ANCHOR_Col_TextDisabled]);
  RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
  if (flags & ANCHORSelectableFlags_Disabled)
    PopStyleColor();

  // Automatically close popups
  if (pressed && (window->Flags & ANCHOR_WindowFlags_Popup) &&
      !(flags & ANCHORSelectableFlags_DontClosePopups) &&
      !(g.CurrentItemFlags & ANCHOR_ItemFlags_SelectableDontClosePopup))
    CloseCurrentPopup();

  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  return pressed;
}

bool ANCHOR::Selectable(const char *label,
                        bool *p_selected,
                        ANCHORSelectableFlags flags,
                        const GfVec2f &size_arg)
{
  if (Selectable(label, *p_selected, flags, size_arg)) {
    *p_selected = !*p_selected;
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ListBox
//-------------------------------------------------------------------------
// - BeginListBox()
// - EndListBox()
// - ListBox()
//-------------------------------------------------------------------------

// Tip: To have a list filling the entire window width, use size[0] = -FLT_MIN and pass an
// non-visible label e.g. "##empty" Tip: If your vertical size is calculated from an item count
// (e.g. 10 * item_height) consider adding a fractional part to facilitate seeing scrolling
// boundaries (e.g. 10.25 * item_height).
bool ANCHOR::BeginListBox(const char *label, const GfVec2f &size_arg)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = GetID(label);
  const GfVec2f label_size = CalcTextSize(label, NULL, true);

  // Size default to hold ~7.25 items.
  // Fractional number of items helps seeing that we can scroll down/up without looking at
  // scrollbar.
  GfVec2f size = ImFloor(CalcItemSize(
    size_arg, CalcItemWidth(), GetTextLineHeightWithSpacing() * 7.25f + style.FramePadding[1] * 2.0f));
  GfVec2f frame_size = GfVec2f(size[0], AnchorMax(size[1], label_size[1]));
  ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
  ImRect bb(frame_bb.Min,
            frame_bb.Max +
              GfVec2f(label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f, 0.0f));
  g.NextItemData.ClearFlags();

  if (!IsRectVisible(bb.Min, bb.Max)) {
    ItemSize(bb.GetSize(), style.FramePadding[1]);
    ItemAdd(bb, 0, &frame_bb);
    return false;
  }

  // FIXME-OPT: We could omit the BeginGroup() if label_size[0] but would need to omit the
  // EndGroup() as well.
  BeginGroup();
  if (label_size[0] > 0.0f) {
    GfVec2f label_pos = GfVec2f(frame_bb.Max[0] + style.ItemInnerSpacing[0],
                                frame_bb.Min[1] + style.FramePadding[1]);
    RenderText(label_pos, label);
    window->DC.CursorMaxPos = AnchorMax(window->DC.CursorMaxPos, label_pos + label_size);
  }

  BeginChildFrame(id, frame_bb.GetSize());
  return true;
}

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
// OBSOLETED in 1.81 (from February 2021)
bool ANCHOR::ListBoxHeader(const char *label, int items_count, int height_in_items)
{
  // If height_in_items == -1, default height is maximum 7.
  ANCHOR_Context &g = *G_CTX;
  float height_in_items_f = (height_in_items < 0 ? ImMin(items_count, 7) : height_in_items) + 0.25f;
  GfVec2f size;
  size[0] = 0.0f;
  size[1] = GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding[1] * 2.0f;
  return BeginListBox(label, size);
}
#  endif

void ANCHOR::EndListBox()
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  ANCHOR_ASSERT((window->Flags & ANCHOR_WindowFlags_ChildWindow) &&
                "Mismatched BeginListBox/EndListBox calls. Did you test the return value of BeginListBox?");
  TF_UNUSED(window);

  EndChildFrame();
  EndGroup();  // This is only required to be able to do IsItemXXX query on the whole ListBox
               // including label
}

bool ANCHOR::ListBox(const char *label,
                     int *current_item,
                     const char *const items[],
                     int items_count,
                     int height_items)
{
  const bool value_changed = ListBox(
    label, current_item, Items_ArrayGetter, (void *)items, items_count, height_items);
  return value_changed;
}

// This is merely a helper around BeginListBox(), EndListBox().
// Considering using those directly to submit custom data or store selection differently.
bool ANCHOR::ListBox(const char *label,
                     int *current_item,
                     bool (*items_getter)(void *, int, const char **),
                     void *data,
                     int items_count,
                     int height_in_items)
{
  ANCHOR_Context &g = *G_CTX;

  // Calculate size from "height_in_items"
  if (height_in_items < 0)
    height_in_items = ImMin(items_count, 7);
  float height_in_items_f = height_in_items + 0.25f;
  GfVec2f size(0.0f,
               ImFloor(GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding[1] * 2.0f));

  if (!BeginListBox(label, size))
    return false;

  // Assume all items have even height (= 1 line of text). If you need items of different height,
  // you can create a custom version of ListBox() in your code without using the clipper.
  bool value_changed = false;
  ANCHORListClipper clipper;
  clipper.Begin(items_count,
                GetTextLineHeightWithSpacing());  // We know exactly our line height here so we pass it as a
                                                  // minor optimization, but generally you don't need to.
  while (clipper.Step())
    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
      const char *item_text;
      if (!items_getter(data, i, &item_text))
        item_text = "*Unknown item*";

      PushID(i);
      const bool item_selected = (i == *current_item);
      if (Selectable(item_text, item_selected)) {
        *current_item = i;
        value_changed = true;
      }
      if (item_selected)
        SetItemDefaultFocus();
      PopID();
    }
  EndListBox();
  if (value_changed)
    MarkItemEdited(g.CurrentWindow->DC.LastItemId);

  return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: PlotLines, PlotHistogram
//-------------------------------------------------------------------------
// - PlotEx() [Internal]
// - PlotLines()
// - PlotHistogram()
//-------------------------------------------------------------------------
// Plot/Graph widgets are not very good.
// Consider writing your own, or using a third-party one, see:
// - ImPlot https://github.com/epezent/implot
// - others https://github.com/ocornut/ANCHOR/wiki/Useful-Extensions
//-------------------------------------------------------------------------

int ANCHOR::PlotEx(ANCHORPlotType plot_type,
                   const char *label,
                   float (*values_getter)(void *data, int idx),
                   void *data,
                   int values_count,
                   int values_offset,
                   const char *overlay_text,
                   float scale_min,
                   float scale_max,
                   GfVec2f frame_size)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return -1;

  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);

  const GfVec2f label_size = CalcTextSize(label, NULL, true);
  if (frame_size[0] == 0.0f)
    frame_size[0] = CalcItemWidth();
  if (frame_size[1] == 0.0f)
    frame_size[1] = label_size[1] + (style.FramePadding[1] * 2);

  const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
  const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
  const ImRect total_bb(
    frame_bb.Min,
    frame_bb.Max + GfVec2f(label_size[0] > 0.0f ? style.ItemInnerSpacing[0] + label_size[0] : 0.0f, 0));
  ItemSize(total_bb, style.FramePadding[1]);
  if (!ItemAdd(total_bb, 0, &frame_bb))
    return -1;
  const bool hovered = ItemHoverable(frame_bb, id);

  // Determine scale from values if not specified
  if (scale_min == FLT_MAX || scale_max == FLT_MAX) {
    float v_min = FLT_MAX;
    float v_max = -FLT_MAX;
    for (int i = 0; i < values_count; i++) {
      const float v = values_getter(data, i);
      if (v != v)  // Ignore NaN values
        continue;
      v_min = ImMin(v_min, v);
      v_max = AnchorMax(v_max, v);
    }
    if (scale_min == FLT_MAX)
      scale_min = v_min;
    if (scale_max == FLT_MAX)
      scale_max = v_max;
  }

  RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ANCHOR_Col_FrameBg), true, style.FrameRounding);

  const int values_count_min = (plot_type == ANCHORPlotType_Lines) ? 2 : 1;
  int idx_hovered = -1;
  if (values_count >= values_count_min) {
    int res_w = ImMin((int)frame_size[0], values_count) + ((plot_type == ANCHORPlotType_Lines) ? -1 : 0);
    int item_count = values_count + ((plot_type == ANCHORPlotType_Lines) ? -1 : 0);

    // Tooltip on hover
    if (hovered && inner_bb.Contains(g.IO.MousePos)) {
      const float t = ImClamp(
        (g.IO.MousePos[0] - inner_bb.Min[0]) / (inner_bb.Max[0] - inner_bb.Min[0]), 0.0f, 0.9999f);
      const int v_idx = (int)(t * item_count);
      ANCHOR_ASSERT(v_idx >= 0 && v_idx < values_count);

      const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
      const float v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
      if (plot_type == ANCHORPlotType_Lines)
        SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
      else if (plot_type == ANCHORPlotType_Histogram)
        SetTooltip("%d: %8.4g", v_idx, v0);
      idx_hovered = v_idx;
    }

    const float t_step = 1.0f / (float)res_w;
    const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

    float v0 = values_getter(data, (0 + values_offset) % values_count);
    float t0 = 0.0f;
    GfVec2f tp0 = GfVec2f(
      t0,
      1.0f -
        ImSaturate((v0 - scale_min) * inv_scale));  // Point in the normalized space of our target rectangle
    float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ?
                                    (-scale_min * inv_scale) :
                                    (scale_min < 0.0f ? 0.0f : 1.0f);  // Where does the zero line stands

    const AnchorU32 col_base = GetColorU32((plot_type == ANCHORPlotType_Lines) ? ANCHOR_Col_PlotLines :
                                                                                 ANCHOR_Col_PlotHistogram);
    const AnchorU32 col_hovered = GetColorU32(
      (plot_type == ANCHORPlotType_Lines) ? ANCHOR_Col_PlotLinesHovered : ANCHOR_Col_PlotHistogramHovered);

    for (int n = 0; n < res_w; n++) {
      const float t1 = t0 + t_step;
      const int v1_idx = (int)(t0 * item_count + 0.5f);
      ANCHOR_ASSERT(v1_idx >= 0 && v1_idx < values_count);
      const float v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
      const GfVec2f tp1 = GfVec2f(t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale));

      // NB: Draw calls are merged together by the DrawList system. Still, we should render our
      // batch are lower level to save a bit of CPU.
      GfVec2f pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
      GfVec2f pos1 = ImLerp(inner_bb.Min,
                            inner_bb.Max,
                            (plot_type == ANCHORPlotType_Lines) ? tp1 :
                                                                  GfVec2f(tp1[0], histogram_zero_line_t));
      if (plot_type == ANCHORPlotType_Lines) {
        window->DrawList->AddLine(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
      }
      else if (plot_type == ANCHORPlotType_Histogram) {
        if (pos1[0] >= pos0[0] + 2.0f)
          pos1[0] -= 1.0f;
        window->DrawList->AddRectFilled(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
      }

      t0 = t1;
      tp0 = tp1;
    }
  }

  // Text overlay
  if (overlay_text)
    RenderTextClipped(GfVec2f(frame_bb.Min[0], frame_bb.Min[1] + style.FramePadding[1]),
                      frame_bb.Max,
                      overlay_text,
                      NULL,
                      NULL,
                      GfVec2f(0.5f, 0.0f));

  if (label_size[0] > 0.0f)
    RenderText(GfVec2f(frame_bb.Max[0] + style.ItemInnerSpacing[0], inner_bb.Min[1]), label);

  // Return hovered index or -1 if none are hovered.
  // This is currently not exposed in the public API because we need a larger redesign of the whole
  // thing, but in the short-term we are making it available in PlotEx().
  return idx_hovered;
}

struct ANCHORPlotArrayGetterData {
  const float *Values;
  int Stride;

  ANCHORPlotArrayGetterData(const float *values, int stride)
  {
    Values = values;
    Stride = stride;
  }
};

static float Plot_ArrayGetter(void *data, int idx)
{
  ANCHORPlotArrayGetterData *plot_data = (ANCHORPlotArrayGetterData *)data;
  const float v = *(const float *)(const void *)((const unsigned char *)plot_data->Values +
                                                 (size_t)idx * plot_data->Stride);
  return v;
}

void ANCHOR::PlotLines(const char *label,
                       const float *values,
                       int values_count,
                       int values_offset,
                       const char *overlay_text,
                       float scale_min,
                       float scale_max,
                       GfVec2f graph_size,
                       int stride)
{
  ANCHORPlotArrayGetterData data(values, stride);
  PlotEx(ANCHORPlotType_Lines,
         label,
         &Plot_ArrayGetter,
         (void *)&data,
         values_count,
         values_offset,
         overlay_text,
         scale_min,
         scale_max,
         graph_size);
}

void ANCHOR::PlotLines(const char *label,
                       float (*values_getter)(void *data, int idx),
                       void *data,
                       int values_count,
                       int values_offset,
                       const char *overlay_text,
                       float scale_min,
                       float scale_max,
                       GfVec2f graph_size)
{
  PlotEx(ANCHORPlotType_Lines,
         label,
         values_getter,
         data,
         values_count,
         values_offset,
         overlay_text,
         scale_min,
         scale_max,
         graph_size);
}

void ANCHOR::PlotHistogram(const char *label,
                           const float *values,
                           int values_count,
                           int values_offset,
                           const char *overlay_text,
                           float scale_min,
                           float scale_max,
                           GfVec2f graph_size,
                           int stride)
{
  ANCHORPlotArrayGetterData data(values, stride);
  PlotEx(ANCHORPlotType_Histogram,
         label,
         &Plot_ArrayGetter,
         (void *)&data,
         values_count,
         values_offset,
         overlay_text,
         scale_min,
         scale_max,
         graph_size);
}

void ANCHOR::PlotHistogram(const char *label,
                           float (*values_getter)(void *data, int idx),
                           void *data,
                           int values_count,
                           int values_offset,
                           const char *overlay_text,
                           float scale_min,
                           float scale_max,
                           GfVec2f graph_size)
{
  PlotEx(ANCHORPlotType_Histogram,
         label,
         values_getter,
         data,
         values_count,
         values_offset,
         overlay_text,
         scale_min,
         scale_max,
         graph_size);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Value helpers
// Those is not very useful, legacy API.
//-------------------------------------------------------------------------
// - Value()
//-------------------------------------------------------------------------

void ANCHOR::Value(const char *prefix, bool b)
{
  Text("%s: %s", prefix, (b ? "true" : "false"));
}

void ANCHOR::Value(const char *prefix, int v)
{
  Text("%s: %d", prefix, v);
}

void ANCHOR::Value(const char *prefix, unsigned int v)
{
  Text("%s: %d", prefix, v);
}

void ANCHOR::Value(const char *prefix, float v, const char *float_format)
{
  if (float_format) {
    char fmt[64];
    ImFormatString(fmt, ANCHOR_ARRAYSIZE(fmt), "%%s: %s", float_format);
    Text(fmt, prefix, v);
  }
  else {
    Text("%s: %.3f", prefix, v);
  }
}

//-------------------------------------------------------------------------
// [SECTION] MenuItem, BeginMenu, EndMenu, etc.
//-------------------------------------------------------------------------
// - ANCHOR_MenuColumns [Internal]
// - BeginMenuBar()
// - EndMenuBar()
// - BeginMainMenuBar()
// - EndMainMenuBar()
// - BeginMenu()
// - EndMenu()
// - MenuItem()
//-------------------------------------------------------------------------

// Helpers for internal use
void ANCHOR_MenuColumns::Update(int count, float spacing, bool clear)
{
  ANCHOR_ASSERT(count == ANCHOR_ARRAYSIZE(Pos));
  TF_UNUSED(count);
  Width = NextWidth = 0.0f;
  Spacing = spacing;
  if (clear)
    memset(NextWidths, 0, sizeof(NextWidths));
  for (int i = 0; i < ANCHOR_ARRAYSIZE(Pos); i++) {
    if (i > 0 && NextWidths[i] > 0.0f)
      Width += Spacing;
    Pos[i] = IM_FLOOR(Width);
    Width += NextWidths[i];
    NextWidths[i] = 0.0f;
  }
}

float ANCHOR_MenuColumns::DeclColumns(float w0,
                                      float w1,
                                      float w2)  // not using va_arg because they promote float to double
{
  NextWidth = 0.0f;
  NextWidths[0] = AnchorMax(NextWidths[0], w0);
  NextWidths[1] = AnchorMax(NextWidths[1], w1);
  NextWidths[2] = AnchorMax(NextWidths[2], w2);
  for (int i = 0; i < ANCHOR_ARRAYSIZE(Pos); i++)
    NextWidth += NextWidths[i] + ((i > 0 && NextWidths[i] > 0.0f) ? Spacing : 0.0f);
  return AnchorMax(Width, NextWidth);
}

float ANCHOR_MenuColumns::CalcExtraSpace(float avail_w) const
{
  return AnchorMax(0.0f, avail_w - Width);
}

// FIXME: Provided a rectangle perhaps e.g. a BeginMenuBarEx() could be used anywhere..
// Currently the main responsibility of this function being to setup clip-rect + horizontal layout
// + menu navigation layer. Ideally we also want this to be responsible for claiming space out of
// the main window scrolling rectangle, in which case ANCHOR_WindowFlags_MenuBar will become
// unnecessary. Then later the same system could be used for multiple menu-bars, scrollbars,
// side-bars.
bool ANCHOR::BeginMenuBar()
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;
  if (!(window->Flags & ANCHOR_WindowFlags_MenuBar))
    return false;

  ANCHOR_ASSERT(!window->DC.MenuBarAppending);
  BeginGroup();  // Backup position on layer 0 // FIXME: Misleading to use a group for that
                 // backup/restore
  PushID("##menubar");

  // We don't clip with current window clipping rectangle as it is already set to the area below.
  // However we clip with window full rect. We remove 1 worth of rounding to Max[0] to that text in
  // long menus and small windows don't tend to display over the lower-right rounded area, which
  // looks particularly glitchy.
  ImRect bar_rect = window->MenuBarRect();
  ImRect clip_rect(
    IM_ROUND(bar_rect.Min[0] + window->WindowBorderSize),
    IM_ROUND(bar_rect.Min[1] + window->WindowBorderSize),
    IM_ROUND(AnchorMax(bar_rect.Min[0],
                       bar_rect.Max[0] - AnchorMax(window->WindowRounding, window->WindowBorderSize))),
    IM_ROUND(bar_rect.Max[1]));
  clip_rect.ClipWith(window->OuterRectClipped);
  PushClipRect(clip_rect.Min, clip_rect.Max, false);

  // We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem
  // hack in EndMenuBar() would need something analogous here, maybe a BeginGroupEx() with flags).
  window->DC.CursorPos = window->DC.CursorMaxPos = GfVec2f(bar_rect.Min[0] + window->DC.MenuBarOffset[0],
                                                           bar_rect.Min[1] + window->DC.MenuBarOffset[1]);
  window->DC.LayoutType = ANCHOR_LayoutType_Horizontal;
  window->DC.NavLayerCurrent = ANCHORNavLayer_Menu;
  window->DC.MenuBarAppending = true;
  AlignTextToFramePadding();
  return true;
}

void ANCHOR::EndMenuBar()
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return;
  ANCHOR_Context &g = *G_CTX;

  // Nav: When a move request within one of our child menu failed, capture the request to navigate
  // among our siblings.
  if (NavMoveRequestButNoResultYet() &&
      (g.NavMoveDir == ANCHOR_Dir_Left || g.NavMoveDir == ANCHOR_Dir_Right) &&
      (g.NavWindow->Flags & ANCHOR_WindowFlags_ChildMenu)) {
    ANCHOR_Window *nav_earliest_child = g.NavWindow;
    while (nav_earliest_child->ParentWindow &&
           (nav_earliest_child->ParentWindow->Flags & ANCHOR_WindowFlags_ChildMenu))
      nav_earliest_child = nav_earliest_child->ParentWindow;
    if (nav_earliest_child->ParentWindow == window &&
        nav_earliest_child->DC.ParentLayoutType == ANCHOR_LayoutType_Horizontal &&
        g.NavMoveRequestForward == ANCHORNavForward_None) {
      // To do so we claim focus back, restore NavId and then process the movement request for yet
      // another frame. This involve a one-frame delay which isn't very problematic in this
      // situation. We could remove it by scoring in advance for multiple window (probably not
      // worth the hassle/cost)
      const ANCHORNavLayer layer = ANCHORNavLayer_Menu;
      ANCHOR_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer));  // Sanity check
      FocusWindow(window);
      SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
      g.NavDisableHighlight = true;  // Hide highlight for the current frame so we don't see the
                                     // intermediary selection.
      g.NavDisableMouseHover = g.NavMousePosDirty = true;
      g.NavMoveRequestForward = ANCHORNavForward_ForwardQueued;
      NavMoveRequestCancel();
    }
  }

  ANCHOR_MSVC_WARNING_SUPPRESS(6011);  // Static Analysis false positive "warning C6011:
                                       // Dereferencing NULL pointer 'window'"
  ANCHOR_ASSERT(window->Flags & ANCHOR_WindowFlags_MenuBar);
  ANCHOR_ASSERT(window->DC.MenuBarAppending);
  PopClipRect();
  PopID();
  window->DC.MenuBarOffset[0] = window->DC.CursorPos[0] -
                                window->Pos[0];  // Save horizontal position so next append can reuse it.
                                                 // This is kinda equivalent to a per-layer CursorPos.
  g.GroupStack.back().EmitItem = false;
  EndGroup();  // Restore position on layer 0
  window->DC.LayoutType = ANCHOR_LayoutType_Vertical;
  window->DC.NavLayerCurrent = ANCHORNavLayer_Main;
  window->DC.MenuBarAppending = false;
}

// Important: calling order matters!
// FIXME: Somehow overlapping with docking tech.
// FIXME: The "rect-cut" aspect of this could be formalized into a lower-level helper (rect-cut:
// https://halt.software/dead-simple-layouts)
bool ANCHOR::BeginViewportSideBar(const char *name,
                                  ANCHORViewport *viewport_p,
                                  ANCHOR_Dir dir,
                                  float axis_size,
                                  ANCHOR_WindowFlags window_flags)
{
  ANCHOR_ASSERT(dir != ANCHOR_Dir_None);

  ANCHOR_Window *bar_window = FindWindowByName(name);
  if (bar_window == NULL || bar_window->BeginCount == 0) {
    // Calculate and set window size/position
    ANCHORViewportP *viewport = (ANCHORViewportP *)(void *)(viewport_p ? viewport_p : GetMainViewport());
    ImRect avail_rect = viewport->GetBuildWorkRect();
    ANCHOR_Axis axis = (dir == ANCHOR_Dir_Up || dir == ANCHOR_Dir_Down) ? ANCHOR_Axis_Y : ANCHOR_Axis_X;
    GfVec2f pos = avail_rect.Min;
    if (dir == ANCHOR_Dir_Right || dir == ANCHOR_Dir_Down)
      pos[axis] = avail_rect.Max[axis] - axis_size;
    GfVec2f size = avail_rect.GetSize();
    size[axis] = axis_size;
    SetNextWindowPos(pos);
    SetNextWindowSize(size);

    // Report our size into work area (for next frame) using actual window size
    if (dir == ANCHOR_Dir_Up || dir == ANCHOR_Dir_Left)
      viewport->BuildWorkOffsetMin[axis] += axis_size;
    else if (dir == ANCHOR_Dir_Down || dir == ANCHOR_Dir_Right)
      viewport->BuildWorkOffsetMax[axis] -= axis_size;
  }

  window_flags |= ANCHOR_WindowFlags_NoTitleBar | ANCHOR_WindowFlags_NoResize | ANCHOR_WindowFlags_NoMove;
  PushStyleVar(ANCHOR_StyleVar_WindowRounding, 0.0f);
  PushStyleVar(ANCHOR_StyleVar_WindowMinSize, GfVec2f(0, 0));  // Lift normal size constraint
  bool is_open = Begin(name, NULL, window_flags);
  PopStyleVar(2);

  return is_open;
}

bool ANCHOR::BeginMainMenuBar()
{
  ANCHOR_Context &g = *G_CTX;
  ANCHORViewportP *viewport = (ANCHORViewportP *)(void *)GetMainViewport();

  // For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to
  // ensure text can be visible on a TV set.
  // FIXME: This could be generalized as an opt-in way to clamp window->DC.CursorStartPos to avoid
  // SafeArea?
  // FIXME: Consider removing support for safe area down the line... it's messy. Nowadays consoles
  // have support for TV calibration in OS settings.
  g.NextWindowData.MenuBarOffsetMinVal = GfVec2f(
    g.Style.DisplaySafeAreaPadding[0],
    AnchorMax(g.Style.DisplaySafeAreaPadding[1] - g.Style.FramePadding[1], 0.0f));
  ANCHOR_WindowFlags window_flags = ANCHOR_WindowFlags_NoScrollbar | ANCHOR_WindowFlags_NoSavedSettings |
                                    ANCHOR_WindowFlags_MenuBar;
  float height = GetFrameHeight();
  bool is_open = BeginViewportSideBar("##MainMenuBar", viewport, ANCHOR_Dir_Up, height, window_flags);
  g.NextWindowData.MenuBarOffsetMinVal = GfVec2f(0.0f, 0.0f);

  if (is_open)
    BeginMenuBar();
  else
    End();
  return is_open;
}

void ANCHOR::EndMainMenuBar()
{
  EndMenuBar();

  // When the user has left the menu layer (typically: closed menus through activation of an item),
  // we restore focus to the previous window
  // FIXME: With this strategy we won't be able to restore a NULL focus.
  ANCHOR_Context &g = *G_CTX;
  if (g.CurrentWindow == g.NavWindow && g.NavLayer == ANCHORNavLayer_Main && !g.NavAnyRequest)
    FocusTopMostWindowUnderOne(g.NavWindow, NULL);

  End();
}

bool ANCHOR::BeginMenu(const char *label, bool enabled)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = window->GetID(label);
  bool menu_is_open = IsPopupOpen(id, ANCHORPopupFlags_None);

  // Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup
  // menu would steal focus and not allow hovering on parent menu)
  ANCHOR_WindowFlags flags = ANCHOR_WindowFlags_ChildMenu | ANCHOR_WindowFlags_AlwaysAutoResize |
                             ANCHOR_WindowFlags_NoMove | ANCHOR_WindowFlags_NoTitleBar |
                             ANCHOR_WindowFlags_NoSavedSettings | ANCHOR_WindowFlags_NoNavFocus;
  if (window->Flags & (ANCHOR_WindowFlags_Popup | ANCHOR_WindowFlags_ChildMenu))
    flags |= ANCHOR_WindowFlags_ChildWindow;

  // If a menu with same the ID was already submitted, we will append to it, matching the behavior
  // of Begin(). We are relying on a O(N) search - so O(N log N) over the frame - which seems like
  // the most efficient for the expected small amount of BeginMenu() calls per frame. If somehow
  // this is ever becoming a problem we can switch to use e.g. ANCHORStorage mapping key to last
  // frame used.
  if (g.MenusIdSubmittedThisFrame.contains(id)) {
    if (menu_is_open)
      menu_is_open = BeginPopupEx(id, flags);  // menu_is_open can be 'false' when the popup is
                                               // completely clipped (e.g. zero size display)
    else
      g.NextWindowData.ClearFlags();  // we behave like Begin() and need to consume those values
    return menu_is_open;
  }

  // Tag menu as used. Next time BeginMenu() with same ID is called it will append to existing menu
  g.MenusIdSubmittedThisFrame.push_back(id);

  GfVec2f label_size = CalcTextSize(label, NULL, true);
  bool pressed;
  bool menuset_is_open = !(window->Flags & ANCHOR_WindowFlags_Popup) &&
                         (g.OpenPopupStack.Size > g.BeginPopupStack.Size &&
                          g.OpenPopupStack[g.BeginPopupStack.Size].OpenParentId == window->IDStack.back());
  ANCHOR_Window *backed_nav_window = g.NavWindow;
  if (menuset_is_open)
    g.NavWindow = window;  // Odd hack to allow hovering across menus of a same menu-set (otherwise
                           // we wouldn't be able to hover parent)

  // The reference position stored in popup_pos will be used by Begin() to find a suitable position
  // for the child menu, However the final position is going to be different! It is chosen by
  // FindBestWindowPosForPopup(). e.g. Menus tend to overlap each other horizontally to amplify
  // relative Z-ordering.
  GfVec2f popup_pos, pos = window->DC.CursorPos;
  if (window->DC.LayoutType == ANCHOR_LayoutType_Horizontal) {
    // Menu inside an horizontal menu bar
    // Selectable extend their highlight by half ItemSpacing in each direction.
    // For ChildMenu, the popup position will be overwritten by the call to
    // FindBestWindowPosForPopup() in Begin()
    popup_pos = GfVec2f(pos[0] - 1.0f - IM_FLOOR(style.ItemSpacing[0] * 0.5f),
                        pos[1] - style.FramePadding[1] + window->MenuBarHeight());
    window->DC.CursorPos[0] += IM_FLOOR(style.ItemSpacing[0] * 0.5f);
    PushStyleVar(ANCHOR_StyleVar_ItemSpacing, GfVec2f(style.ItemSpacing[0] * 2.0f, style.ItemSpacing[1]));
    float w = label_size[0];
    pressed = Selectable(label,
                         menu_is_open,
                         ANCHORSelectableFlags_NoHoldingActiveID | ANCHORSelectableFlags_SelectOnClick |
                           ANCHORSelectableFlags_DontClosePopups |
                           (!enabled ? ANCHORSelectableFlags_Disabled : 0),
                         GfVec2f(w, 0.0f));
    PopStyleVar();
    window->DC.CursorPos[0] += IM_FLOOR(
      style.ItemSpacing[0] *
      (-1.0f + 0.5f));  // -1 spacing to compensate the spacing added when Selectable() did a SameLine().
                        // It would also work to call SameLine() ourselves after the PopStyleVar().
  }
  else {
    // Menu inside a menu
    // (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will
    // always be 0.0f.
    //  Only when they are other items sticking out we're going to add spacing, yet only register
    //  minimum width into the layout system.
    popup_pos = GfVec2f(pos[0], pos[1] - style.WindowPadding[1]);
    float min_w = window->DC.MenuColumns.DeclColumns(
      label_size[0], 0.0f, IM_FLOOR(g.FontSize * 1.20f));  // Feedback to next frame
    float extra_w = AnchorMax(0.0f, GetContentRegionAvail()[0] - min_w);
    pressed = Selectable(label,
                         menu_is_open,
                         ANCHORSelectableFlags_NoHoldingActiveID | ANCHORSelectableFlags_SelectOnClick |
                           ANCHORSelectableFlags_DontClosePopups | ANCHORSelectableFlags_SpanAvailWidth |
                           (!enabled ? ANCHORSelectableFlags_Disabled : 0),
                         GfVec2f(min_w, 0.0f));
    AnchorU32 text_col = GetColorU32(enabled ? ANCHOR_Col_Text : ANCHOR_Col_TextDisabled);
    RenderArrow(window->DrawList,
                pos + GfVec2f(window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f),
                text_col,
                ANCHOR_Dir_Right);
  }

  const bool hovered = enabled && ItemHoverable(window->DC.LastItemRect, id);
  if (menuset_is_open)
    g.NavWindow = backed_nav_window;

  bool want_open = false;
  bool want_close = false;
  if (window->DC.LayoutType ==
      ANCHOR_LayoutType_Vertical)  // (window->Flags &
                                   // (ANCHOR_WindowFlags_Popup|ANCHOR_WindowFlags_ChildMenu))
  {
    // Close menu when not hovering it anymore unless we are moving roughly in the direction of the
    // menu Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid
    // using timers, so menus feels more reactive.
    bool moving_toward_other_child_menu = false;

    ANCHOR_Window *child_menu_window = (g.BeginPopupStack.Size < g.OpenPopupStack.Size &&
                                        g.OpenPopupStack[g.BeginPopupStack.Size].SourceWindow == window) ?
                                         g.OpenPopupStack[g.BeginPopupStack.Size].Window :
                                         NULL;
    if (g.HoveredWindow == window && child_menu_window != NULL &&
        !(window->Flags & ANCHOR_WindowFlags_MenuBar)) {
      // FIXME-DPI: Values should be derived from a master "scale" factor.
      ImRect next_window_rect = child_menu_window->Rect();
      GfVec2f ta = g.IO.MousePos - g.IO.MouseDelta;
      GfVec2f tb = (window->Pos[0] < child_menu_window->Pos[0]) ? next_window_rect.GetTL() :
                                                                  next_window_rect.GetTR();
      GfVec2f tc = (window->Pos[0] < child_menu_window->Pos[0]) ? next_window_rect.GetBL() :
                                                                  next_window_rect.GetBR();
      float extra = ImClamp(ImFabs(ta[0] - tb[0]) * 0.30f, 5.0f, 30.0f);      // add a bit of extra slack.
      ta[0] += (window->Pos[0] < child_menu_window->Pos[0]) ? -0.5f : +0.5f;  // to avoid numerical issues
      tb[1] = ta[1] + AnchorMax((tb[1] - extra) - ta[1],
                                -100.0f);  // triangle is maximum 200 high to limit the slope and the bias
                                           // toward large sub-menus // FIXME: Multiply by fb_scale?
      tc[1] = ta[1] + ImMin((tc[1] + extra) - ta[1], +100.0f);
      moving_toward_other_child_menu = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
      // GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ?
      // ANCHOR_COL32(0,128,0,128) : ANCHOR_COL32(128,0,0,128)); // [DEBUG]
    }
    if (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 &&
        g.HoveredIdPreviousFrame != id && !moving_toward_other_child_menu)
      want_close = true;

    if (!menu_is_open && hovered && pressed)  // Click to open
      want_open = true;
    else if (!menu_is_open && hovered && !moving_toward_other_child_menu)  // Hover to open
      want_open = true;

    if (g.NavActivateId == id) {
      want_close = menu_is_open;
      want_open = !menu_is_open;
    }
    if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ANCHOR_Dir_Right)  // Nav-Right to open
    {
      want_open = true;
      NavMoveRequestCancel();
    }
  }
  else {
    // Menu bar
    if (menu_is_open && pressed && menuset_is_open)  // Click an open menu again to close it
    {
      want_close = true;
      want_open = menu_is_open = false;
    }
    else if (pressed || (hovered && menuset_is_open &&
                         !menu_is_open))  // First click to open, then hover to open others
    {
      want_open = true;
    }
    else if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ANCHOR_Dir_Down)  // Nav-Down to open
    {
      want_open = true;
      NavMoveRequestCancel();
    }
  }

  if (!enabled)  // explicitly close if an open menu becomes disabled, facilitate users code a lot
                 // in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
    want_close = true;
  if (want_close && IsPopupOpen(id, ANCHORPopupFlags_None))
    ClosePopupToLevel(g.BeginPopupStack.Size, true);

  ANCHOR_TEST_ENGINE_ITEM_INFO(id,
                               label,
                               window->DC.LastItemStatusFlags | ANCHOR_ItemStatusFlags_Openable |
                                 (menu_is_open ? ANCHOR_ItemStatusFlags_Opened : 0));

  if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size) {
    // Don't recycle same menu level in the same frame, first close the other menu and yield for a
    // frame.
    OpenPopup(label);
    return false;
  }

  menu_is_open |= want_open;
  if (want_open)
    OpenPopup(label);

  if (menu_is_open) {
    SetNextWindowPos(popup_pos,
                     ANCHOR_Cond_Always);    // Note: this is super misleading! The value will serve as
                                             // reference for FindBestWindowPosForPopup(), not actual pos.
    menu_is_open = BeginPopupEx(id, flags);  // menu_is_open can be 'false' when the popup is
                                             // completely clipped (e.g. zero size display)
  }
  else {
    g.NextWindowData.ClearFlags();  // We behave like Begin() and need to consume those values
  }

  return menu_is_open;
}

void ANCHOR::EndMenu()
{
  // Nav: When a left move request _within our child menu_ failed, close ourselves (the _parent_
  // menu). A menu doesn't close itself because EndMenuBar() wants the catch the last Left<>Right
  // inputs. However, it means that with the current code, a BeginMenu() from outside another menu
  // or a menu-bar won't be closable with the Left direction.
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (g.NavWindow && g.NavWindow->ParentWindow == window && g.NavMoveDir == ANCHOR_Dir_Left &&
      NavMoveRequestButNoResultYet() && window->DC.LayoutType == ANCHOR_LayoutType_Vertical) {
    ClosePopupToLevel(g.BeginPopupStack.Size, true);
    NavMoveRequestCancel();
  }

  EndPopup();
}

bool ANCHOR::MenuItem(const char *label, const char *shortcut, bool selected, bool enabled)
{
  ANCHOR_Window *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Style &style = g.Style;
  GfVec2f pos = window->DC.CursorPos;
  GfVec2f label_size = CalcTextSize(label, NULL, true);

  // We've been using the equivalent of ANCHORSelectableFlags_SetNavIdOnHover on all Selectable()
  // since early Nav system days (commit 43ee5d73), but I am unsure whether this should be kept at
  // all. For now moved it to be an opt-in feature used by menus only.
  ANCHORSelectableFlags flags = ANCHORSelectableFlags_SelectOnRelease |
                                ANCHORSelectableFlags_SetNavIdOnHover |
                                (enabled ? 0 : ANCHORSelectableFlags_Disabled);
  bool pressed;
  if (window->DC.LayoutType == ANCHOR_LayoutType_Horizontal) {
    // Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside a menu bar, which
    // is a little misleading but may be useful Note that in this situation: we don't render the
    // shortcut, we render a highlight instead of the selected tick mark.
    float w = label_size[0];
    window->DC.CursorPos[0] += IM_FLOOR(style.ItemSpacing[0] * 0.5f);
    PushStyleVar(ANCHOR_StyleVar_ItemSpacing, GfVec2f(style.ItemSpacing[0] * 2.0f, style.ItemSpacing[1]));
    pressed = Selectable(label, selected, flags, GfVec2f(w, 0.0f));
    PopStyleVar();
    window->DC.CursorPos[0] += IM_FLOOR(
      style.ItemSpacing[0] *
      (-1.0f + 0.5f));  // -1 spacing to compensate the spacing added when Selectable() did a SameLine().
                        // It would also work to call SameLine() ourselves after the PopStyleVar().
  }
  else {
    // Menu item inside a vertical menu
    // (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will
    // always be 0.0f.
    //  Only when they are other items sticking out we're going to add spacing, yet only register
    //  minimum width into the layout system.
    float shortcut_w = shortcut ? CalcTextSize(shortcut, NULL)[0] : 0.0f;
    float min_w = window->DC.MenuColumns.DeclColumns(
      label_size[0], shortcut_w, IM_FLOOR(g.FontSize * 1.20f));  // Feedback for next frame
    float extra_w = AnchorMax(0.0f, GetContentRegionAvail()[0] - min_w);
    pressed = Selectable(label, false, flags | ANCHORSelectableFlags_SpanAvailWidth, GfVec2f(min_w, 0.0f));
    if (shortcut_w > 0.0f) {
      PushStyleColor(ANCHOR_Col_Text, g.Style.Colors[ANCHOR_Col_TextDisabled]);
      RenderText(pos + GfVec2f(window->DC.MenuColumns.Pos[1] + extra_w, 0.0f), shortcut, NULL, false);
      PopStyleColor();
    }
    if (selected)
      RenderCheckMark(window->DrawList,
                      pos + GfVec2f(window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.40f,
                                    g.FontSize * 0.134f * 0.5f),
                      GetColorU32(enabled ? ANCHOR_Col_Text : ANCHOR_Col_TextDisabled),
                      g.FontSize * 0.866f);
  }

  ANCHOR_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId,
                               label,
                               window->DC.LastItemStatusFlags | ANCHOR_ItemStatusFlags_Checkable |
                                 (selected ? ANCHOR_ItemStatusFlags_Checked : 0));
  return pressed;
}

bool ANCHOR::MenuItem(const char *label, const char *shortcut, bool *p_selected, bool enabled)
{
  if (MenuItem(label, shortcut, p_selected ? *p_selected : false, enabled)) {
    if (p_selected)
      *p_selected = !*p_selected;
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
//-------------------------------------------------------------------------
// - BeginTabBar()
// - BeginTabBarEx() [Internal]
// - EndTabBar()
// - TabBarLayout() [Internal]
// - TabBarCalcTabID() [Internal]
// - TabBarCalcMaxTabWidth() [Internal]
// - TabBarFindTabById() [Internal]
// - TabBarRemoveTab() [Internal]
// - TabBarCloseTab() [Internal]
// - TabBarScrollClamp() [Internal]
// - TabBarScrollToTab() [Internal]
// - TabBarQueueChangeTabOrder() [Internal]
// - TabBarScrollingButtons() [Internal]
// - TabBarTabListPopupButton() [Internal]
//-------------------------------------------------------------------------

struct ANCHOR_TabBarSection {
  int TabCount;   // Number of tabs in this section.
  float Width;    // Sum of width of tabs in this section (after shrinking down)
  float Spacing;  // Horizontal spacing at the end of the section.

  ANCHOR_TabBarSection()
  {
    memset(this, 0, sizeof(*this));
  }
};

namespace ANCHOR {
static void TabBarLayout(ANCHOR_TabBar *tab_bar);
static AnchorU32 TabBarCalcTabID(ANCHOR_TabBar *tab_bar, const char *label);
static float TabBarCalcMaxTabWidth();
static float TabBarScrollClamp(ANCHOR_TabBar *tab_bar, float scrolling);
static void TabBarScrollToTab(ANCHOR_TabBar *tab_bar, ANCHOR_ID tab_id, ANCHOR_TabBarSection *sections);
static ANCHOR_TabItem *TabBarScrollingButtons(ANCHOR_TabBar *tab_bar);
static ANCHOR_TabItem *TabBarTabListPopupButton(ANCHOR_TabBar *tab_bar);
}  // namespace ANCHOR

ANCHOR_TabBar::ANCHOR_TabBar()
{
  memset(this, 0, sizeof(*this));
  CurrFrameVisible = PrevFrameVisible = -1;
  LastTabItemIdx = -1;
}

static inline int TabItemGetSectionIdx(const ANCHOR_TabItem *tab)
{
  return (tab->Flags & ANCHOR_TabItemFlags_Leading)  ? 0 :
         (tab->Flags & ANCHOR_TabItemFlags_Trailing) ? 2 :
                                                       1;
}

static int ANCHOR_CDECL TabItemComparerBySection(const void *lhs, const void *rhs)
{
  const ANCHOR_TabItem *a = (const ANCHOR_TabItem *)lhs;
  const ANCHOR_TabItem *b = (const ANCHOR_TabItem *)rhs;
  const int a_section = TabItemGetSectionIdx(a);
  const int b_section = TabItemGetSectionIdx(b);
  if (a_section != b_section)
    return a_section - b_section;
  return (int)(a->IndexDuringLayout - b->IndexDuringLayout);
}

static int ANCHOR_CDECL TabItemComparerByBeginOrder(const void *lhs, const void *rhs)
{
  const ANCHOR_TabItem *a = (const ANCHOR_TabItem *)lhs;
  const ANCHOR_TabItem *b = (const ANCHOR_TabItem *)rhs;
  return (int)(a->BeginOrder - b->BeginOrder);
}

static ANCHOR_TabBar *GetTabBarFromTabBarRef(const ANCHOR_PtrOrIndex &ref)
{
  ANCHOR_Context &g = *G_CTX;
  return ref.Ptr ? (ANCHOR_TabBar *)ref.Ptr : g.TabBars.GetByIndex(ref.Index);
}

static ANCHOR_PtrOrIndex GetTabBarRefFromTabBar(ANCHOR_TabBar *tab_bar)
{
  ANCHOR_Context &g = *G_CTX;
  if (g.TabBars.Contains(tab_bar))
    return ANCHOR_PtrOrIndex(g.TabBars.GetIndex(tab_bar));
  return ANCHOR_PtrOrIndex(tab_bar);
}

bool ANCHOR::BeginTabBar(const char *str_id, ANCHOR_TabBarFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  ANCHOR_ID id = window->GetID(str_id);
  ANCHOR_TabBar *tab_bar = g.TabBars.GetOrAddByKey(id);
  ImRect tab_bar_bb = ImRect(window->DC.CursorPos[0],
                             window->DC.CursorPos[1],
                             window->WorkRect.Max[0],
                             window->DC.CursorPos[1] + g.FontSize + g.Style.FramePadding[1] * 2);
  tab_bar->ID = id;
  return BeginTabBarEx(tab_bar, tab_bar_bb, flags | ANCHOR_TabBarFlags_IsFocused);
}

bool ANCHOR::BeginTabBarEx(ANCHOR_TabBar *tab_bar, const ImRect &tab_bar_bb, ANCHOR_TabBarFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  if ((flags & ANCHOR_TabBarFlags_DockNode) == 0)
    PushOverrideID(tab_bar->ID);

  // Add to stack
  g.CurrentTabBarStack.push_back(GetTabBarRefFromTabBar(tab_bar));
  g.CurrentTabBar = tab_bar;

  // Append with multiple BeginTabBar()/EndTabBar() pairs.
  tab_bar->BackupCursorPos = window->DC.CursorPos;
  if (tab_bar->CurrFrameVisible == g.FrameCount) {
    window->DC.CursorPos = GfVec2f(tab_bar->BarRect.Min[0], tab_bar->BarRect.Max[1] + tab_bar->ItemSpacingY);
    tab_bar->BeginCount++;
    return true;
  }

  // Ensure correct ordering when toggling ANCHOR_TabBarFlags_Reorderable flag, or when a new tab
  // was added while being not reorderable
  if ((flags & ANCHOR_TabBarFlags_Reorderable) != (tab_bar->Flags & ANCHOR_TabBarFlags_Reorderable) ||
      (tab_bar->TabsAddedNew && !(flags & ANCHOR_TabBarFlags_Reorderable)))
    if (tab_bar->Tabs.Size > 1)
      ImQsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(ANCHOR_TabItem), TabItemComparerByBeginOrder);
  tab_bar->TabsAddedNew = false;

  // Flags
  if ((flags & ANCHOR_TabBarFlags_FittingPolicyMask_) == 0)
    flags |= ANCHOR_TabBarFlags_FittingPolicyDefault_;

  tab_bar->Flags = flags;
  tab_bar->BarRect = tab_bar_bb;
  tab_bar->WantLayout = true;  // Layout will be done on the first call to ItemTab()
  tab_bar->PrevFrameVisible = tab_bar->CurrFrameVisible;
  tab_bar->CurrFrameVisible = g.FrameCount;
  tab_bar->PrevTabsContentsHeight = tab_bar->CurrTabsContentsHeight;
  tab_bar->CurrTabsContentsHeight = 0.0f;
  tab_bar->ItemSpacingY = g.Style.ItemSpacing[1];
  tab_bar->FramePadding = g.Style.FramePadding;
  tab_bar->TabsActiveCount = 0;
  tab_bar->BeginCount = 1;

  // Set cursor pos in a way which only be used in the off-chance the user erroneously submits item
  // before BeginTabItem(): items will overlap
  window->DC.CursorPos = GfVec2f(tab_bar->BarRect.Min[0], tab_bar->BarRect.Max[1] + tab_bar->ItemSpacingY);

  // Draw separator
  const AnchorU32 col = GetColorU32((flags & ANCHOR_TabBarFlags_IsFocused) ? ANCHOR_Col_TabActive :
                                                                             ANCHOR_Col_TabUnfocusedActive);
  const float y = tab_bar->BarRect.Max[1] - 1.0f;
  {
    const float separator_min_x = tab_bar->BarRect.Min[0] - IM_FLOOR(window->WindowPadding[0] * 0.5f);
    const float separator_max_x = tab_bar->BarRect.Max[0] + IM_FLOOR(window->WindowPadding[0] * 0.5f);
    window->DrawList->AddLine(GfVec2f(separator_min_x, y), GfVec2f(separator_max_x, y), col, 1.0f);
  }
  return true;
}

void ANCHOR::EndTabBar()
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  ANCHOR_TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ANCHOR_ASSERT_USER_ERROR(tab_bar != NULL, "Mismatched BeginTabBar()/EndTabBar()!");
    return;
  }

  // Fallback in case no TabItem have been submitted
  if (tab_bar->WantLayout)
    TabBarLayout(tab_bar);

  // Restore the last visible height if no tab is visible, this reduce vertical flicker/movement
  // when a tabs gets removed without calling SetTabItemClosed().
  const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
  if (tab_bar->VisibleTabWasSubmitted || tab_bar->VisibleTabId == 0 || tab_bar_appearing) {
    tab_bar->CurrTabsContentsHeight = AnchorMax(window->DC.CursorPos[1] - tab_bar->BarRect.Max[1],
                                                tab_bar->CurrTabsContentsHeight);
    window->DC.CursorPos[1] = tab_bar->BarRect.Max[1] + tab_bar->CurrTabsContentsHeight;
  }
  else {
    window->DC.CursorPos[1] = tab_bar->BarRect.Max[1] + tab_bar->PrevTabsContentsHeight;
  }
  if (tab_bar->BeginCount > 1)
    window->DC.CursorPos = tab_bar->BackupCursorPos;

  if ((tab_bar->Flags & ANCHOR_TabBarFlags_DockNode) == 0)
    PopID();

  g.CurrentTabBarStack.pop_back();
  g.CurrentTabBar = g.CurrentTabBarStack.empty() ? NULL :
                                                   GetTabBarFromTabBarRef(g.CurrentTabBarStack.back());
}

// This is called only once a frame before by the first call to ItemTab()
// The reason we're not calling it in BeginTabBar() is to leave a chance to the user to call the
// SetTabItemClosed() functions.
static void ANCHOR::TabBarLayout(ANCHOR_TabBar *tab_bar)
{
  ANCHOR_Context &g = *G_CTX;
  tab_bar->WantLayout = false;

  // Garbage collect by compacting list
  // Detect if we need to sort out tab list (e.g. in rare case where a tab changed section)
  int tab_dst_n = 0;
  bool need_sort_by_section = false;
  ANCHOR_TabBarSection sections[3];  // Layout sections: Leading, Central, Trailing
  for (int tab_src_n = 0; tab_src_n < tab_bar->Tabs.Size; tab_src_n++) {
    ANCHOR_TabItem *tab = &tab_bar->Tabs[tab_src_n];
    if (tab->LastFrameVisible < tab_bar->PrevFrameVisible || tab->WantClose) {
      // Remove tab
      if (tab_bar->VisibleTabId == tab->ID) {
        tab_bar->VisibleTabId = 0;
      }
      if (tab_bar->SelectedTabId == tab->ID) {
        tab_bar->SelectedTabId = 0;
      }
      if (tab_bar->NextSelectedTabId == tab->ID) {
        tab_bar->NextSelectedTabId = 0;
      }
      continue;
    }
    if (tab_dst_n != tab_src_n)
      tab_bar->Tabs[tab_dst_n] = tab_bar->Tabs[tab_src_n];

    tab = &tab_bar->Tabs[tab_dst_n];
    tab->IndexDuringLayout = (AnchorS16)tab_dst_n;

    // We will need sorting if tabs have changed section (e.g. moved from one of
    // Leading/Central/Trailing to another)
    int curr_tab_section_n = TabItemGetSectionIdx(tab);
    if (tab_dst_n > 0) {
      ANCHOR_TabItem *prev_tab = &tab_bar->Tabs[tab_dst_n - 1];
      int prev_tab_section_n = TabItemGetSectionIdx(prev_tab);
      if (curr_tab_section_n == 0 && prev_tab_section_n != 0)
        need_sort_by_section = true;
      if (prev_tab_section_n == 2 && curr_tab_section_n != 2)
        need_sort_by_section = true;
    }

    sections[curr_tab_section_n].TabCount++;
    tab_dst_n++;
  }
  if (tab_bar->Tabs.Size != tab_dst_n)
    tab_bar->Tabs.resize(tab_dst_n);

  if (need_sort_by_section)
    ImQsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(ANCHOR_TabItem), TabItemComparerBySection);

  // Calculate spacing between sections
  sections[0].Spacing = sections[0].TabCount > 0 && (sections[1].TabCount + sections[2].TabCount) > 0 ?
                          g.Style.ItemInnerSpacing[0] :
                          0.0f;
  sections[1].Spacing = sections[1].TabCount > 0 && sections[2].TabCount > 0 ? g.Style.ItemInnerSpacing[0] :
                                                                               0.0f;

  // Setup next selected tab
  ANCHOR_ID scroll_to_tab_id = 0;
  if (tab_bar->NextSelectedTabId) {
    tab_bar->SelectedTabId = tab_bar->NextSelectedTabId;
    tab_bar->NextSelectedTabId = 0;
    scroll_to_tab_id = tab_bar->SelectedTabId;
  }

  // Process order change request (we could probably process it when requested but it's just saner
  // to do it in a single spot).
  if (tab_bar->ReorderRequestTabId != 0) {
    if (TabBarProcessReorder(tab_bar))
      if (tab_bar->ReorderRequestTabId == tab_bar->SelectedTabId)
        scroll_to_tab_id = tab_bar->ReorderRequestTabId;
    tab_bar->ReorderRequestTabId = 0;
  }

  // Tab List Popup (will alter tab_bar->BarRect and therefore the available width!)
  const bool tab_list_popup_button = (tab_bar->Flags & ANCHOR_TabBarFlags_TabListPopupButton) != 0;
  if (tab_list_popup_button)
    if (ANCHOR_TabItem *tab_to_select = TabBarTabListPopupButton(tab_bar))  // NB: Will alter BarRect.Min[0]!
      scroll_to_tab_id = tab_bar->SelectedTabId = tab_to_select->ID;

  // Leading/Trailing tabs will be shrink only if central one aren't visible anymore, so layout the
  // shrink data as: leading, trailing, central (whereas our tabs are stored as: leading, central,
  // trailing)
  int shrink_buffer_indexes[3] = {0, sections[0].TabCount + sections[2].TabCount, sections[0].TabCount};
  g.ShrinkWidthBuffer.resize(tab_bar->Tabs.Size);

  // Compute ideal tabs widths + store them into shrink buffer
  ANCHOR_TabItem *most_recently_selected_tab = NULL;
  int curr_section_n = -1;
  bool found_selected_tab_id = false;
  for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
    ANCHOR_TabItem *tab = &tab_bar->Tabs[tab_n];
    ANCHOR_ASSERT(tab->LastFrameVisible >= tab_bar->PrevFrameVisible);

    if ((most_recently_selected_tab == NULL ||
         most_recently_selected_tab->LastFrameSelected < tab->LastFrameSelected) &&
        !(tab->Flags & ANCHOR_TabItemFlags_Button))
      most_recently_selected_tab = tab;
    if (tab->ID == tab_bar->SelectedTabId)
      found_selected_tab_id = true;
    if (scroll_to_tab_id == 0 && g.NavJustMovedToId == tab->ID)
      scroll_to_tab_id = tab->ID;

    // Refresh tab width immediately, otherwise changes of style e.g. style.FramePadding[0] would
    // noticeably lag in the tab bar. Additionally, when using TabBarAddTab() to manipulate tab bar
    // order we occasionally insert new tabs that don't have a width yet, and we cannot wait for
    // the next BeginTabItem() call. We cannot compute this width within TabBarAddTab() because
    // font size depends on the active window.
    const char *tab_name = tab_bar->GetTabName(tab);
    const bool has_close_button = (tab->Flags & ANCHOR_TabItemFlags_NoCloseButton) ? false : true;
    tab->ContentWidth = TabItemCalcSize(tab_name, has_close_button)[0];

    int section_n = TabItemGetSectionIdx(tab);
    ANCHOR_TabBarSection *section = &sections[section_n];
    section->Width += tab->ContentWidth + (section_n == curr_section_n ? g.Style.ItemInnerSpacing[0] : 0.0f);
    curr_section_n = section_n;

    // Store data so we can build an array sorted by width if we need to shrink tabs down
    ANCHOR_MSVC_WARNING_SUPPRESS(6385);
    int shrink_buffer_index = shrink_buffer_indexes[section_n]++;
    g.ShrinkWidthBuffer[shrink_buffer_index].Index = tab_n;
    g.ShrinkWidthBuffer[shrink_buffer_index].Width = tab->ContentWidth;

    ANCHOR_ASSERT(tab->ContentWidth > 0.0f);
    tab->Width = tab->ContentWidth;
  }

  // Compute total ideal width (used for e.g. auto-resizing a window)
  tab_bar->WidthAllTabsIdeal = 0.0f;
  for (int section_n = 0; section_n < 3; section_n++)
    tab_bar->WidthAllTabsIdeal += sections[section_n].Width + sections[section_n].Spacing;

  // Horizontal scrolling buttons
  // (note that TabBarScrollButtons() will alter BarRect.Max[0])
  if ((tab_bar->WidthAllTabsIdeal > tab_bar->BarRect.GetWidth() && tab_bar->Tabs.Size > 1) &&
      !(tab_bar->Flags & ANCHOR_TabBarFlags_NoTabListScrollingButtons) &&
      (tab_bar->Flags & ANCHOR_TabBarFlags_FittingPolicyScroll))
    if (ANCHOR_TabItem *scroll_and_select_tab = TabBarScrollingButtons(tab_bar)) {
      scroll_to_tab_id = scroll_and_select_tab->ID;
      if ((scroll_and_select_tab->Flags & ANCHOR_TabItemFlags_Button) == 0)
        tab_bar->SelectedTabId = scroll_to_tab_id;
    }

  // Shrink widths if full tabs don't fit in their allocated space
  float section_0_w = sections[0].Width + sections[0].Spacing;
  float section_1_w = sections[1].Width + sections[1].Spacing;
  float section_2_w = sections[2].Width + sections[2].Spacing;
  bool central_section_is_visible = (section_0_w + section_2_w) < tab_bar->BarRect.GetWidth();
  float width_excess;
  if (central_section_is_visible)
    width_excess = AnchorMax(section_1_w - (tab_bar->BarRect.GetWidth() - section_0_w - section_2_w),
                             0.0f);  // Excess used to shrink central section
  else
    width_excess = (section_0_w + section_2_w) -
                   tab_bar->BarRect.GetWidth();  // Excess used to shrink leading/trailing section

  // With ANCHOR_TabBarFlags_FittingPolicyScroll policy, we will only shrink leading/trailing if
  // the central section is not visible anymore
  if (width_excess > 0.0f &&
      ((tab_bar->Flags & ANCHOR_TabBarFlags_FittingPolicyResizeDown) || !central_section_is_visible)) {
    int shrink_data_count = (central_section_is_visible ? sections[1].TabCount :
                                                          sections[0].TabCount + sections[2].TabCount);
    int shrink_data_offset = (central_section_is_visible ? sections[0].TabCount + sections[2].TabCount : 0);
    ShrinkWidths(g.ShrinkWidthBuffer.Data + shrink_data_offset, shrink_data_count, width_excess);

    // Apply shrunk values into tabs and sections
    for (int tab_n = shrink_data_offset; tab_n < shrink_data_offset + shrink_data_count; tab_n++) {
      ANCHOR_TabItem *tab = &tab_bar->Tabs[g.ShrinkWidthBuffer[tab_n].Index];
      float shrinked_width = IM_FLOOR(g.ShrinkWidthBuffer[tab_n].Width);
      if (shrinked_width < 0.0f)
        continue;

      int section_n = TabItemGetSectionIdx(tab);
      sections[section_n].Width -= (tab->Width - shrinked_width);
      tab->Width = shrinked_width;
    }
  }

  // Layout all active tabs
  int section_tab_index = 0;
  float tab_offset = 0.0f;
  tab_bar->WidthAllTabs = 0.0f;
  for (int section_n = 0; section_n < 3; section_n++) {
    ANCHOR_TabBarSection *section = &sections[section_n];
    if (section_n == 2)
      tab_offset = ImMin(AnchorMax(0.0f, tab_bar->BarRect.GetWidth() - section->Width), tab_offset);

    for (int tab_n = 0; tab_n < section->TabCount; tab_n++) {
      ANCHOR_TabItem *tab = &tab_bar->Tabs[section_tab_index + tab_n];
      tab->Offset = tab_offset;
      tab_offset += tab->Width + (tab_n < section->TabCount - 1 ? g.Style.ItemInnerSpacing[0] : 0.0f);
    }
    tab_bar->WidthAllTabs += AnchorMax(section->Width + section->Spacing, 0.0f);
    tab_offset += section->Spacing;
    section_tab_index += section->TabCount;
  }

  // If we have lost the selected tab, select the next most recently active one
  if (found_selected_tab_id == false)
    tab_bar->SelectedTabId = 0;
  if (tab_bar->SelectedTabId == 0 && tab_bar->NextSelectedTabId == 0 && most_recently_selected_tab != NULL)
    scroll_to_tab_id = tab_bar->SelectedTabId = most_recently_selected_tab->ID;

  // Lock in visible tab
  tab_bar->VisibleTabId = tab_bar->SelectedTabId;
  tab_bar->VisibleTabWasSubmitted = false;

  // Update scrolling
  if (scroll_to_tab_id != 0)
    TabBarScrollToTab(tab_bar, scroll_to_tab_id, sections);
  tab_bar->ScrollingAnim = TabBarScrollClamp(tab_bar, tab_bar->ScrollingAnim);
  tab_bar->ScrollingTarget = TabBarScrollClamp(tab_bar, tab_bar->ScrollingTarget);
  if (tab_bar->ScrollingAnim != tab_bar->ScrollingTarget) {
    // Scrolling speed adjust itself so we can always reach our target in 1/3 seconds.
    // Teleport if we are aiming far off the visible line
    tab_bar->ScrollingSpeed = AnchorMax(tab_bar->ScrollingSpeed, 70.0f * g.FontSize);
    tab_bar->ScrollingSpeed = AnchorMax(tab_bar->ScrollingSpeed,
                                        ImFabs(tab_bar->ScrollingTarget - tab_bar->ScrollingAnim) / 0.3f);
    const bool teleport = (tab_bar->PrevFrameVisible + 1 < g.FrameCount) ||
                          (tab_bar->ScrollingTargetDistToVisibility > 10.0f * g.FontSize);
    tab_bar->ScrollingAnim = teleport ? tab_bar->ScrollingTarget :
                                        ImLinearSweep(tab_bar->ScrollingAnim,
                                                      tab_bar->ScrollingTarget,
                                                      g.IO.DeltaTime * tab_bar->ScrollingSpeed);
  }
  else {
    tab_bar->ScrollingSpeed = 0.0f;
  }
  tab_bar->ScrollingRectMinX = tab_bar->BarRect.Min[0] + sections[0].Width + sections[0].Spacing;
  tab_bar->ScrollingRectMaxX = tab_bar->BarRect.Max[0] - sections[2].Width - sections[1].Spacing;

  // Clear name buffers
  if ((tab_bar->Flags & ANCHOR_TabBarFlags_DockNode) == 0)
    tab_bar->TabsNames.Buf.resize(0);

  // Actual layout in host window (we don't do it in BeginTabBar() so as not to waste an extra
  // frame)
  ANCHOR_Window *window = g.CurrentWindow;
  window->DC.CursorPos = tab_bar->BarRect.Min;
  ItemSize(GfVec2f(tab_bar->WidthAllTabs, tab_bar->BarRect.GetHeight()), tab_bar->FramePadding[1]);
  window->DC.IdealMaxPos[0] = AnchorMax(window->DC.IdealMaxPos[0],
                                        tab_bar->BarRect.Min[0] + tab_bar->WidthAllTabsIdeal);
}

// Dockables uses Name/ID in the global namespace. Non-dockable items use the ID stack.
static AnchorU32 ANCHOR::TabBarCalcTabID(ANCHOR_TabBar *tab_bar, const char *label)
{
  if (tab_bar->Flags & ANCHOR_TabBarFlags_DockNode) {
    ANCHOR_ID id = ImHashStr(label);
    KeepAliveID(id);
    return id;
  }
  else {
    ANCHOR_Window *window = G_CTX->CurrentWindow;
    return window->GetID(label);
  }
}

static float ANCHOR::TabBarCalcMaxTabWidth()
{
  ANCHOR_Context &g = *G_CTX;
  return g.FontSize * 20.0f;
}

ANCHOR_TabItem *ANCHOR::TabBarFindTabByID(ANCHOR_TabBar *tab_bar, ANCHOR_ID tab_id)
{
  if (tab_id != 0)
    for (int n = 0; n < tab_bar->Tabs.Size; n++)
      if (tab_bar->Tabs[n].ID == tab_id)
        return &tab_bar->Tabs[n];
  return NULL;
}

// The *TabId fields be already set by the docking system _before_ the actual TabItem was created,
// so we clear them regardless.
void ANCHOR::TabBarRemoveTab(ANCHOR_TabBar *tab_bar, ANCHOR_ID tab_id)
{
  if (ANCHOR_TabItem *tab = TabBarFindTabByID(tab_bar, tab_id))
    tab_bar->Tabs.erase(tab);
  if (tab_bar->VisibleTabId == tab_id) {
    tab_bar->VisibleTabId = 0;
  }
  if (tab_bar->SelectedTabId == tab_id) {
    tab_bar->SelectedTabId = 0;
  }
  if (tab_bar->NextSelectedTabId == tab_id) {
    tab_bar->NextSelectedTabId = 0;
  }
}

// Called on manual closure attempt
void ANCHOR::TabBarCloseTab(ANCHOR_TabBar *tab_bar, ANCHOR_TabItem *tab)
{
  ANCHOR_ASSERT(!(tab->Flags & ANCHOR_TabItemFlags_Button));
  if (!(tab->Flags & ANCHOR_TabItemFlags_UnsavedDocument)) {
    // This will remove a frame of lag for selecting another tab on closure.
    // However we don't run it in the case where the 'Unsaved' flag is set, so user gets a chance
    // to fully undo the closure
    tab->WantClose = true;
    if (tab_bar->VisibleTabId == tab->ID) {
      tab->LastFrameVisible = -1;
      tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = 0;
    }
  }
  else {
    // Actually select before expecting closure attempt (on an UnsavedDocument tab user is expect
    // to e.g. show a popup)
    if (tab_bar->VisibleTabId != tab->ID)
      tab_bar->NextSelectedTabId = tab->ID;
  }
}

static float ANCHOR::TabBarScrollClamp(ANCHOR_TabBar *tab_bar, float scrolling)
{
  scrolling = ImMin(scrolling, tab_bar->WidthAllTabs - tab_bar->BarRect.GetWidth());
  return AnchorMax(scrolling, 0.0f);
}

// Note: we may scroll to tab that are not selected! e.g. using keyboard arrow keys
static void ANCHOR::TabBarScrollToTab(ANCHOR_TabBar *tab_bar,
                                      ANCHOR_ID tab_id,
                                      ANCHOR_TabBarSection *sections)
{
  ANCHOR_TabItem *tab = TabBarFindTabByID(tab_bar, tab_id);
  if (tab == NULL)
    return;
  if (tab->Flags & ANCHOR_TabItemFlags_SectionMask_)
    return;

  ANCHOR_Context &g = *G_CTX;
  float margin = g.FontSize * 1.0f;  // When to scroll to make Tab N+1 visible always make a bit of N visible
                                     // to suggest more scrolling area (since we don't have a scrollbar)
  int order = tab_bar->GetTabOrder(tab);

  // Scrolling happens only in the central section (leading/trailing sections are not scrolling)
  // FIXME: This is all confusing.
  float scrollable_width = tab_bar->BarRect.GetWidth() - sections[0].Width - sections[2].Width -
                           sections[1].Spacing;

  // We make all tabs positions all relative Sections[0].Width to make code simpler
  float tab_x1 = tab->Offset - sections[0].Width + (order > sections[0].TabCount - 1 ? -margin : 0.0f);
  float tab_x2 = tab->Offset - sections[0].Width + tab->Width +
                 (order + 1 < tab_bar->Tabs.Size - sections[2].TabCount ? margin : 1.0f);
  tab_bar->ScrollingTargetDistToVisibility = 0.0f;
  if (tab_bar->ScrollingTarget > tab_x1 || (tab_x2 - tab_x1 >= scrollable_width)) {
    // Scroll to the left
    tab_bar->ScrollingTargetDistToVisibility = AnchorMax(tab_bar->ScrollingAnim - tab_x2, 0.0f);
    tab_bar->ScrollingTarget = tab_x1;
  }
  else if (tab_bar->ScrollingTarget < tab_x2 - scrollable_width) {
    // Scroll to the right
    tab_bar->ScrollingTargetDistToVisibility = AnchorMax(
      (tab_x1 - scrollable_width) - tab_bar->ScrollingAnim, 0.0f);
    tab_bar->ScrollingTarget = tab_x2 - scrollable_width;
  }
}

void ANCHOR::TabBarQueueReorder(ANCHOR_TabBar *tab_bar, const ANCHOR_TabItem *tab, int offset)
{
  ANCHOR_ASSERT(offset != 0);
  ANCHOR_ASSERT(tab_bar->ReorderRequestTabId == 0);
  tab_bar->ReorderRequestTabId = tab->ID;
  tab_bar->ReorderRequestOffset = (AnchorS16)offset;
}

void ANCHOR::TabBarQueueReorderFromMousePos(ANCHOR_TabBar *tab_bar,
                                            const ANCHOR_TabItem *src_tab,
                                            GfVec2f mouse_pos)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_ASSERT(tab_bar->ReorderRequestTabId == 0);
  if ((tab_bar->Flags & ANCHOR_TabBarFlags_Reorderable) == 0)
    return;

  const bool is_central_section = (src_tab->Flags & ANCHOR_TabItemFlags_SectionMask_) == 0;
  const float bar_offset = tab_bar->BarRect.Min[0] - (is_central_section ? tab_bar->ScrollingTarget : 0);

  // Count number of contiguous tabs we are crossing over
  const int dir = (bar_offset + src_tab->Offset) > mouse_pos[0] ? -1 : +1;
  const int src_idx = tab_bar->Tabs.index_from_ptr(src_tab);
  int dst_idx = src_idx;
  for (int i = src_idx; i >= 0 && i < tab_bar->Tabs.Size; i += dir) {
    // Reordered tabs must share the same section
    const ANCHOR_TabItem *dst_tab = &tab_bar->Tabs[i];
    if (dst_tab->Flags & ANCHOR_TabItemFlags_NoReorder)
      break;
    if ((dst_tab->Flags & ANCHOR_TabItemFlags_SectionMask_) !=
        (src_tab->Flags & ANCHOR_TabItemFlags_SectionMask_))
      break;
    dst_idx = i;

    // Include spacing after tab, so when mouse cursor is between tabs we would not continue
    // checking further tabs that are not hovered.
    const float x1 = bar_offset + dst_tab->Offset - g.Style.ItemInnerSpacing[0];
    const float x2 = bar_offset + dst_tab->Offset + dst_tab->Width + g.Style.ItemInnerSpacing[0];
    // GetForegroundDrawList()->AddRect(GfVec2f(x1, tab_bar->BarRect.Min[1]), GfVec2f(x2,
    // tab_bar->BarRect.Max[1]), ANCHOR_COL32(255, 0, 0, 255));
    if ((dir < 0 && mouse_pos[0] > x1) || (dir > 0 && mouse_pos[0] < x2))
      break;
  }

  if (dst_idx != src_idx)
    TabBarQueueReorder(tab_bar, src_tab, dst_idx - src_idx);
}

bool ANCHOR::TabBarProcessReorder(ANCHOR_TabBar *tab_bar)
{
  ANCHOR_TabItem *tab1 = TabBarFindTabByID(tab_bar, tab_bar->ReorderRequestTabId);
  if (tab1 == NULL || (tab1->Flags & ANCHOR_TabItemFlags_NoReorder))
    return false;

  // ANCHOR_ASSERT(tab_bar->Flags & ANCHOR_TabBarFlags_Reorderable); // <- this may happen when
  // using debug tools
  int tab2_order = tab_bar->GetTabOrder(tab1) + tab_bar->ReorderRequestOffset;
  if (tab2_order < 0 || tab2_order >= tab_bar->Tabs.Size)
    return false;

  // Reordered tabs must share the same section
  // (Note: TabBarQueueReorderFromMousePos() also has a similar test but since we allow direct
  // calls to TabBarQueueReorder() we do it here too)
  ANCHOR_TabItem *tab2 = &tab_bar->Tabs[tab2_order];
  if (tab2->Flags & ANCHOR_TabItemFlags_NoReorder)
    return false;
  if ((tab1->Flags & ANCHOR_TabItemFlags_SectionMask_) != (tab2->Flags & ANCHOR_TabItemFlags_SectionMask_))
    return false;

  ANCHOR_TabItem item_tmp = *tab1;
  ANCHOR_TabItem *src_tab = (tab_bar->ReorderRequestOffset > 0) ? tab1 + 1 : tab2;
  ANCHOR_TabItem *dst_tab = (tab_bar->ReorderRequestOffset > 0) ? tab1 : tab2 + 1;
  const int move_count = (tab_bar->ReorderRequestOffset > 0) ? tab_bar->ReorderRequestOffset :
                                                               -tab_bar->ReorderRequestOffset;
  memmove(dst_tab, src_tab, move_count * sizeof(ANCHOR_TabItem));
  *tab2 = item_tmp;

  if (tab_bar->Flags & ANCHOR_TabBarFlags_SaveSettings)
    MarkIniSettingsDirty();
  return true;
}

static ANCHOR_TabItem *ANCHOR::TabBarScrollingButtons(ANCHOR_TabBar *tab_bar)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;

  const GfVec2f arrow_button_size(g.FontSize - 2.0f, g.FontSize + g.Style.FramePadding[1] * 2.0f);
  const float scrolling_buttons_width = arrow_button_size[0] * 2.0f;

  const GfVec2f backup_cursor_pos = window->DC.CursorPos;
  // window->DrawList->AddRect(GfVec2f(tab_bar->BarRect.Max[0] - scrolling_buttons_width,
  // tab_bar->BarRect.Min[1]), GfVec2f(tab_bar->BarRect.Max[0], tab_bar->BarRect.Max[1]),
  // ANCHOR_COL32(255,0,0,255));

  int select_dir = 0;
  GfVec4f arrow_col = g.Style.Colors[ANCHOR_Col_Text];
  arrow_col[3] *= 0.5f;

  PushStyleColor(ANCHOR_Col_Text, arrow_col);
  PushStyleColor(ANCHOR_Col_Button, GfVec4f(0, 0, 0, 0));
  const float backup_repeat_delay = g.IO.KeyRepeatDelay;
  const float backup_repeat_rate = g.IO.KeyRepeatRate;
  g.IO.KeyRepeatDelay = 0.250f;
  g.IO.KeyRepeatRate = 0.200f;
  float x = AnchorMax(tab_bar->BarRect.Min[0], tab_bar->BarRect.Max[0] - scrolling_buttons_width);
  window->DC.CursorPos = GfVec2f(x, tab_bar->BarRect.Min[1]);
  if (ArrowButtonEx("##<",
                    ANCHOR_Dir_Left,
                    arrow_button_size,
                    ANCHOR_ButtonFlags_PressedOnClick | ANCHOR_ButtonFlags_Repeat))
    select_dir = -1;
  window->DC.CursorPos = GfVec2f(x + arrow_button_size[0], tab_bar->BarRect.Min[1]);
  if (ArrowButtonEx("##>",
                    ANCHOR_Dir_Right,
                    arrow_button_size,
                    ANCHOR_ButtonFlags_PressedOnClick | ANCHOR_ButtonFlags_Repeat))
    select_dir = +1;
  PopStyleColor(2);
  g.IO.KeyRepeatRate = backup_repeat_rate;
  g.IO.KeyRepeatDelay = backup_repeat_delay;

  ANCHOR_TabItem *tab_to_scroll_to = NULL;
  if (select_dir != 0)
    if (ANCHOR_TabItem *tab_item = TabBarFindTabByID(tab_bar, tab_bar->SelectedTabId)) {
      int selected_order = tab_bar->GetTabOrder(tab_item);
      int target_order = selected_order + select_dir;

      // Skip tab item buttons until another tab item is found or end is reached
      while (tab_to_scroll_to == NULL) {
        // If we are at the end of the list, still scroll to make our tab visible
        tab_to_scroll_to =
          &tab_bar->Tabs[(target_order >= 0 && target_order < tab_bar->Tabs.Size) ? target_order :
                                                                                    selected_order];

        // Cross through buttons
        // (even if first/last item is a button, return it so we can update the scroll)
        if (tab_to_scroll_to->Flags & ANCHOR_TabItemFlags_Button) {
          target_order += select_dir;
          selected_order += select_dir;
          tab_to_scroll_to = (target_order < 0 || target_order >= tab_bar->Tabs.Size) ? tab_to_scroll_to :
                                                                                        NULL;
        }
      }
    }
  window->DC.CursorPos = backup_cursor_pos;
  tab_bar->BarRect.Max[0] -= scrolling_buttons_width + 1.0f;

  return tab_to_scroll_to;
}

static ANCHOR_TabItem *ANCHOR::TabBarTabListPopupButton(ANCHOR_TabBar *tab_bar)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;

  // We use g.Style.FramePadding[1] to match the square ArrowButton size
  const float tab_list_popup_button_width = g.FontSize + g.Style.FramePadding[1];
  const GfVec2f backup_cursor_pos = window->DC.CursorPos;
  window->DC.CursorPos = GfVec2f(tab_bar->BarRect.Min[0] - g.Style.FramePadding[1], tab_bar->BarRect.Min[1]);
  tab_bar->BarRect.Min[0] += tab_list_popup_button_width;

  GfVec4f arrow_col = g.Style.Colors[ANCHOR_Col_Text];
  arrow_col[3] *= 0.5f;
  PushStyleColor(ANCHOR_Col_Text, arrow_col);
  PushStyleColor(ANCHOR_Col_Button, GfVec4f(0, 0, 0, 0));
  bool open = BeginCombo("##v", NULL, ANCHORComboFlags_NoPreview | ANCHORComboFlags_HeightLargest);
  PopStyleColor(2);

  ANCHOR_TabItem *tab_to_select = NULL;
  if (open) {
    for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++) {
      ANCHOR_TabItem *tab = &tab_bar->Tabs[tab_n];
      if (tab->Flags & ANCHOR_TabItemFlags_Button)
        continue;

      const char *tab_name = tab_bar->GetTabName(tab);
      if (Selectable(tab_name, tab_bar->SelectedTabId == tab->ID))
        tab_to_select = tab;
    }
    EndCombo();
  }

  window->DC.CursorPos = backup_cursor_pos;
  return tab_to_select;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
//-------------------------------------------------------------------------
// - BeginTabItem()
// - EndTabItem()
// - TabItemButton()
// - TabItemEx() [Internal]
// - SetTabItemClosed()
// - TabItemCalcSize() [Internal]
// - TabItemBackground() [Internal]
// - TabItemLabelAndCloseButton() [Internal]
//-------------------------------------------------------------------------

bool ANCHOR::BeginTabItem(const char *label, bool *p_open, ANCHOR_TabItemFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  ANCHOR_TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ANCHOR_ASSERT_USER_ERROR(tab_bar, "Needs to be called between BeginTabBar() and EndTabBar()!");
    return false;
  }
  ANCHOR_ASSERT(!(flags & ANCHOR_TabItemFlags_Button));  // BeginTabItem() Can't be used with button flags,
                                                         // use TabItemButton() instead!

  bool ret = TabItemEx(tab_bar, label, p_open, flags);
  if (ret && !(flags & ANCHOR_TabItemFlags_NoPushId)) {
    ANCHOR_TabItem *tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
    PushOverrideID(tab->ID);  // We already hashed 'label' so push into the ID stack directly
                              // instead of doing another hash through PushID(label)
  }
  return ret;
}

void ANCHOR::EndTabItem()
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  ANCHOR_TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ANCHOR_ASSERT_USER_ERROR(tab_bar != NULL, "Needs to be called between BeginTabBar() and EndTabBar()!");
    return;
  }
  ANCHOR_ASSERT(tab_bar->LastTabItemIdx >= 0);
  ANCHOR_TabItem *tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
  if (!(tab->Flags & ANCHOR_TabItemFlags_NoPushId))
    PopID();
}

bool ANCHOR::TabItemButton(const char *label, ANCHOR_TabItemFlags flags)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  ANCHOR_TabBar *tab_bar = g.CurrentTabBar;
  if (tab_bar == NULL) {
    ANCHOR_ASSERT_USER_ERROR(tab_bar != NULL, "Needs to be called between BeginTabBar() and EndTabBar()!");
    return false;
  }
  return TabItemEx(tab_bar, label, NULL, flags | ANCHOR_TabItemFlags_Button | ANCHOR_TabItemFlags_NoReorder);
}

bool ANCHOR::TabItemEx(ANCHOR_TabBar *tab_bar, const char *label, bool *p_open, ANCHOR_TabItemFlags flags)
{
  // Layout whole tab bar if not already done
  if (tab_bar->WantLayout)
    TabBarLayout(tab_bar);

  ANCHOR_Context &g = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  if (window->SkipItems)
    return false;

  const ANCHOR_Style &style = g.Style;
  const ANCHOR_ID id = TabBarCalcTabID(tab_bar, label);

  // If the user called us with *p_open == false, we early out and don't render.
  // We make a call to ItemAdd() so that attempts to use a contextual popup menu with an implicit
  // ID won't use an older ID.
  ANCHOR_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  if (p_open && !*p_open) {
    PushItemFlag(ANCHOR_ItemFlags_NoNav | ANCHOR_ItemFlags_NoNavDefaultFocus, true);
    ItemAdd(ImRect(), id);
    PopItemFlag();
    return false;
  }

  ANCHOR_ASSERT(!p_open || !(flags & ANCHOR_TabItemFlags_Button));
  ANCHOR_ASSERT(
    (flags & (ANCHOR_TabItemFlags_Leading | ANCHOR_TabItemFlags_Trailing)) !=
    (ANCHOR_TabItemFlags_Leading | ANCHOR_TabItemFlags_Trailing));  // Can't use both Leading and Trailing

  // Store into ANCHOR_TabItemFlags_NoCloseButton, also honor ANCHOR_TabItemFlags_NoCloseButton
  // passed by user (although not documented)
  if (flags & ANCHOR_TabItemFlags_NoCloseButton)
    p_open = NULL;
  else if (p_open == NULL)
    flags |= ANCHOR_TabItemFlags_NoCloseButton;

  // Calculate tab contents size
  GfVec2f size = TabItemCalcSize(label, p_open != NULL);

  // Acquire tab data
  ANCHOR_TabItem *tab = TabBarFindTabByID(tab_bar, id);
  bool tab_is_new = false;
  if (tab == NULL) {
    tab_bar->Tabs.push_back(ANCHOR_TabItem());
    tab = &tab_bar->Tabs.back();
    tab->ID = id;
    tab->Width = size[0];
    tab_bar->TabsAddedNew = true;
    tab_is_new = true;
  }
  tab_bar->LastTabItemIdx = (AnchorS16)tab_bar->Tabs.index_from_ptr(tab);
  tab->ContentWidth = size[0];
  tab->BeginOrder = tab_bar->TabsActiveCount++;

  const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
  const bool tab_bar_focused = (tab_bar->Flags & ANCHOR_TabBarFlags_IsFocused) != 0;
  const bool tab_appearing = (tab->LastFrameVisible + 1 < g.FrameCount);
  const bool is_tab_button = (flags & ANCHOR_TabItemFlags_Button) != 0;
  tab->LastFrameVisible = g.FrameCount;
  tab->Flags = flags;

  // Append name with zero-terminator
  tab->NameOffset = (AnchorS32)tab_bar->TabsNames.size();
  tab_bar->TabsNames.append(label, label + strlen(label) + 1);

  // Update selected tab
  if (tab_appearing && (tab_bar->Flags & ANCHOR_TabBarFlags_AutoSelectNewTabs) &&
      tab_bar->NextSelectedTabId == 0)
    if (!tab_bar_appearing || tab_bar->SelectedTabId == 0)
      if (!is_tab_button)
        tab_bar->NextSelectedTabId = id;  // New tabs gets activated
  if ((flags & ANCHOR_TabItemFlags_SetSelected) &&
      (tab_bar->SelectedTabId != id))  // SetSelected can only be passed on explicit tab bar
    if (!is_tab_button)
      tab_bar->NextSelectedTabId = id;

  // Lock visibility
  // (Note: tab_contents_visible != tab_selected... because CTRL+TAB operations may preview some
  // tabs without selecting them!)
  bool tab_contents_visible = (tab_bar->VisibleTabId == id);
  if (tab_contents_visible)
    tab_bar->VisibleTabWasSubmitted = true;

  // On the very first frame of a tab bar we let first tab contents be visible to minimize
  // appearing glitches
  if (!tab_contents_visible && tab_bar->SelectedTabId == 0 && tab_bar_appearing)
    if (tab_bar->Tabs.Size == 1 && !(tab_bar->Flags & ANCHOR_TabBarFlags_AutoSelectNewTabs))
      tab_contents_visible = true;

  // Note that tab_is_new is not necessarily the same as tab_appearing! When a tab bar stops being
  // submitted and then gets submitted again, the tabs will have 'tab_appearing=true' but
  // 'tab_is_new=false'.
  if (tab_appearing && (!tab_bar_appearing || tab_is_new)) {
    PushItemFlag(ANCHOR_ItemFlags_NoNav | ANCHOR_ItemFlags_NoNavDefaultFocus, true);
    ItemAdd(ImRect(), id);
    PopItemFlag();
    if (is_tab_button)
      return false;
    return tab_contents_visible;
  }

  if (tab_bar->SelectedTabId == id)
    tab->LastFrameSelected = g.FrameCount;

  // Backup current layout position
  const GfVec2f backup_main_cursor_pos = window->DC.CursorPos;

  // Layout
  const bool is_central_section = (tab->Flags & ANCHOR_TabItemFlags_SectionMask_) == 0;
  size[0] = tab->Width;
  if (is_central_section)
    window->DC.CursorPos = tab_bar->BarRect.Min +
                           GfVec2f(IM_FLOOR(tab->Offset - tab_bar->ScrollingAnim), 0.0f);
  else
    window->DC.CursorPos = tab_bar->BarRect.Min + GfVec2f(tab->Offset, 0.0f);
  GfVec2f pos = window->DC.CursorPos;
  ImRect bb(pos, pos + size);

  // We don't have CPU clipping primitives to clip the CloseButton (until it becomes a texture), so
  // need to add an extra draw call (temporary in the case of vertical animation)
  const bool want_clip_rect = is_central_section && (bb.Min[0] < tab_bar->ScrollingRectMinX ||
                                                     bb.Max[0] > tab_bar->ScrollingRectMaxX);
  if (want_clip_rect)
    PushClipRect(GfVec2f(AnchorMax(bb.Min[0], tab_bar->ScrollingRectMinX), bb.Min[1] - 1),
                 GfVec2f(tab_bar->ScrollingRectMaxX, bb.Max[1]),
                 true);

  GfVec2f backup_cursor_max_pos = window->DC.CursorMaxPos;
  ItemSize(bb.GetSize(), style.FramePadding[1]);
  window->DC.CursorMaxPos = backup_cursor_max_pos;

  if (!ItemAdd(bb, id)) {
    if (want_clip_rect)
      PopClipRect();
    window->DC.CursorPos = backup_main_cursor_pos;
    return tab_contents_visible;
  }

  // Click to Select a tab
  ANCHOR_ButtonFlags button_flags = ((is_tab_button ? ANCHOR_ButtonFlags_PressedOnClickRelease :
                                                      ANCHOR_ButtonFlags_PressedOnClick) |
                                     ANCHOR_ButtonFlags_AllowItemOverlap);
  if (g.DragDropActive)
    button_flags |= ANCHOR_ButtonFlags_PressedOnDragDropHold;
  bool hovered, held;
  bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
  if (pressed && !is_tab_button)
    tab_bar->NextSelectedTabId = id;
  hovered |= (g.HoveredId == id);

  // Allow the close button to overlap unless we are dragging (in which case we don't want any
  // overlapping tabs to be hovered)
  if (g.ActiveId != id)
    SetItemAllowOverlap();

  // Drag and drop: re-order tabs
  if (held && !tab_appearing && IsMouseDragging(0)) {
    if (!g.DragDropActive && (tab_bar->Flags & ANCHOR_TabBarFlags_Reorderable)) {
      // While moving a tab it will jump on the other side of the mouse, so we also test for
      // MouseDelta[0]
      if (g.IO.MouseDelta[0] < 0.0f && g.IO.MousePos[0] < bb.Min[0]) {
        TabBarQueueReorderFromMousePos(tab_bar, tab, g.IO.MousePos);
      }
      else if (g.IO.MouseDelta[0] > 0.0f && g.IO.MousePos[0] > bb.Max[0]) {
        TabBarQueueReorderFromMousePos(tab_bar, tab, g.IO.MousePos);
      }
    }
  }

#  if 0
    if (hovered && g.HoveredIdNotActiveTimer > TOOLTIP_DELAY && bb.GetWidth() < tab->ContentWidth)
    {
        // Enlarge tab display when hovering
        bb.Max[0] = bb.Min[0] + IM_FLOOR(ImLerp(bb.GetWidth(), tab->ContentWidth, ImSaturate((g.HoveredIdNotActiveTimer - 0.40f) * 6.0f)));
        display_draw_list = GetForegroundDrawList(window);
        TabItemBackground(display_draw_list, bb, flags, GetColorU32(ANCHOR_Col_TitleBgActive));
    }
#  endif

  // Render tab shape
  ImDrawList *display_draw_list = window->DrawList;
  const AnchorU32 tab_col = GetColorU32(
    (held || hovered)    ? ANCHOR_Col_TabHovered :
    tab_contents_visible ? (tab_bar_focused ? ANCHOR_Col_TabActive : ANCHOR_Col_TabUnfocusedActive) :
                           (tab_bar_focused ? ANCHOR_Col_Tab : ANCHOR_Col_TabUnfocused));
  TabItemBackground(display_draw_list, bb, flags, tab_col);
  RenderNavHighlight(bb, id);

  // Select with right mouse button. This is so the common idiom for context menu automatically
  // highlight the current widget.
  const bool hovered_unblocked = IsItemHovered(ANCHORHoveredFlags_AllowWhenBlockedByPopup);
  if (hovered_unblocked && (IsMouseClicked(1) || IsMouseReleased(1)))
    if (!is_tab_button)
      tab_bar->NextSelectedTabId = id;

  if (tab_bar->Flags & ANCHOR_TabBarFlags_NoCloseWithMiddleMouseButton)
    flags |= ANCHOR_TabItemFlags_NoCloseWithMiddleMouseButton;

  // Render tab label, process close button
  const ANCHOR_ID close_button_id = p_open ? GetIDWithSeed("#CLOSE", NULL, id) : 0;
  bool just_closed;
  bool text_clipped;
  TabItemLabelAndCloseButton(display_draw_list,
                             bb,
                             flags,
                             tab_bar->FramePadding,
                             label,
                             id,
                             close_button_id,
                             tab_contents_visible,
                             &just_closed,
                             &text_clipped);
  if (just_closed && p_open != NULL) {
    *p_open = false;
    TabBarCloseTab(tab_bar, tab);
  }

  // Restore main window position so user can draw there
  if (want_clip_rect)
    PopClipRect();
  window->DC.CursorPos = backup_main_cursor_pos;

  // Tooltip (FIXME: Won't work over the close button because ItemOverlap systems messes up with
  // HoveredIdTimer) We test IsItemHovered() to discard e.g. when another item is active or drag
  // and drop over the tab bar (which g.HoveredId ignores)
  if (text_clipped && g.HoveredId == id && !held && g.HoveredIdNotActiveTimer > g.TooltipSlowDelay &&
      IsItemHovered())
    if (!(tab_bar->Flags & ANCHOR_TabBarFlags_NoTooltip) && !(tab->Flags & ANCHOR_TabItemFlags_NoTooltip))
      SetTooltip("%.*s", (int)(FindRenderedTextEnd(label) - label), label);

  ANCHOR_ASSERT(!is_tab_button || !(tab_bar->SelectedTabId == tab->ID &&
                                    is_tab_button));  // TabItemButton should not be selected
  if (is_tab_button)
    return pressed;
  return tab_contents_visible;
}

// [Public] This is call is 100% optional but it allows to remove some one-frame glitches when a
// tab has been unexpectedly removed. To use it to need to call the function SetTabItemClosed()
// between BeginTabBar() and EndTabBar(). Tabs closed by the close button will automatically be
// flagged to avoid this issue.
void ANCHOR::SetTabItemClosed(const char *label)
{
  ANCHOR_Context &g = *G_CTX;
  bool is_within_manual_tab_bar = g.CurrentTabBar && !(g.CurrentTabBar->Flags & ANCHOR_TabBarFlags_DockNode);
  if (is_within_manual_tab_bar) {
    ANCHOR_TabBar *tab_bar = g.CurrentTabBar;
    ANCHOR_ID tab_id = TabBarCalcTabID(tab_bar, label);
    if (ANCHOR_TabItem *tab = TabBarFindTabByID(tab_bar, tab_id))
      tab->WantClose = true;  // Will be processed by next call to TabBarLayout()
  }
}

GfVec2f ANCHOR::TabItemCalcSize(const char *label, bool has_close_button)
{
  ANCHOR_Context &g = *G_CTX;
  GfVec2f label_size = CalcTextSize(label, NULL, true);
  GfVec2f size = GfVec2f(label_size[0] + g.Style.FramePadding[0],
                         label_size[1] + g.Style.FramePadding[1] * 2.0f);
  if (has_close_button)
    size[0] += g.Style.FramePadding[0] +
               (g.Style.ItemInnerSpacing[0] +
                g.FontSize);  // We use Y intentionally to fit the close button circle.
  else
    size[0] += g.Style.FramePadding[0] + 1.0f;
  return GfVec2f(ImMin(size[0], TabBarCalcMaxTabWidth()), size[1]);
}

void ANCHOR::TabItemBackground(ImDrawList *draw_list,
                               const ImRect &bb,
                               ANCHOR_TabItemFlags flags,
                               AnchorU32 col)
{
  // While rendering tabs, we trim 1 pixel off the top of our bounding box so they can fit within a
  // regular frame height while looking "detached" from it.
  ANCHOR_Context &g = *G_CTX;
  const float width = bb.GetWidth();
  TF_UNUSED(flags);
  ANCHOR_ASSERT(width > 0.0f);
  const float rounding = AnchorMax(
    0.0f,
    ImMin((flags & ANCHOR_TabItemFlags_Button) ? g.Style.FrameRounding : g.Style.TabRounding,
          width * 0.5f - 1.0f));
  const float y1 = bb.Min[1] + 1.0f;
  const float y2 = bb.Max[1] - 1.0f;
  draw_list->PathLineTo(GfVec2f(bb.Min[0], y2));
  draw_list->PathArcToFast(GfVec2f(bb.Min[0] + rounding, y1 + rounding), rounding, 6, 9);
  draw_list->PathArcToFast(GfVec2f(bb.Max[0] - rounding, y1 + rounding), rounding, 9, 12);
  draw_list->PathLineTo(GfVec2f(bb.Max[0], y2));
  draw_list->PathFillConvex(col);
  if (g.Style.TabBorderSize > 0.0f) {
    draw_list->PathLineTo(GfVec2f(bb.Min[0] + 0.5f, y2));
    draw_list->PathArcToFast(GfVec2f(bb.Min[0] + rounding + 0.5f, y1 + rounding + 0.5f), rounding, 6, 9);
    draw_list->PathArcToFast(GfVec2f(bb.Max[0] - rounding - 0.5f, y1 + rounding + 0.5f), rounding, 9, 12);
    draw_list->PathLineTo(GfVec2f(bb.Max[0] - 0.5f, y2));
    draw_list->PathStroke(GetColorU32(ANCHOR_Col_Border), 0, g.Style.TabBorderSize);
  }
}

// Render text label (with custom clipping) + Unsaved Document marker + Close Button logic
// We tend to lock style.FramePadding for a given tab-bar, hence the 'frame_padding' parameter.
void ANCHOR::TabItemLabelAndCloseButton(ImDrawList *draw_list,
                                        const ImRect &bb,
                                        ANCHOR_TabItemFlags flags,
                                        GfVec2f frame_padding,
                                        const char *label,
                                        ANCHOR_ID tab_id,
                                        ANCHOR_ID close_button_id,
                                        bool is_contents_visible,
                                        bool *out_just_closed,
                                        bool *out_text_clipped)
{
  ANCHOR_Context &g = *G_CTX;
  GfVec2f label_size = CalcTextSize(label, NULL, true);

  if (out_just_closed)
    *out_just_closed = false;
  if (out_text_clipped)
    *out_text_clipped = false;

  if (bb.GetWidth() <= 1.0f)
    return;

    // In Style V2 we'll have full override of all colors per state (e.g. focused, selected)
    // But right now if you want to alter text color of tabs this is what you need to do.
#  if 0
    const float backup_alpha = g.Style.Alpha;
    if (!is_contents_visible)
        g.Style.Alpha *= 0.7f;
#  endif

  // Render text label (with clipping + alpha gradient) + unsaved marker
  const char *TAB_UNSAVED_MARKER = "*";
  ImRect text_pixel_clip_bb(
    bb.Min[0] + frame_padding[0], bb.Min[1] + frame_padding[1], bb.Max[0] - frame_padding[0], bb.Max[1]);
  if (flags & ANCHOR_TabItemFlags_UnsavedDocument) {
    text_pixel_clip_bb.Max[0] -= CalcTextSize(TAB_UNSAVED_MARKER, NULL, false)[0];
    GfVec2f unsaved_marker_pos(
      ImMin(bb.Min[0] + frame_padding[0] + label_size[0] + 2, text_pixel_clip_bb.Max[0]),
      bb.Min[1] + frame_padding[1] + IM_FLOOR(-g.FontSize * 0.25f));
    RenderTextClippedEx(
      draw_list, unsaved_marker_pos, bb.Max - frame_padding, TAB_UNSAVED_MARKER, NULL, NULL);
  }
  ImRect text_ellipsis_clip_bb = text_pixel_clip_bb;

  // Return clipped state ignoring the close button
  if (out_text_clipped) {
    *out_text_clipped = (text_ellipsis_clip_bb.Min[0] + label_size[0]) > text_pixel_clip_bb.Max[0];
    // draw_list->AddCircle(text_ellipsis_clip_bb.Min, 3.0f, *out_text_clipped ? ANCHOR_COL32(255,
    // 0, 0, 255) : ANCHOR_COL32(0, 255, 0, 255));
  }

  // Close Button
  // We are relying on a subtle and confusing distinction between 'hovered' and 'g.HoveredId' which
  // happens because we are using ANCHOR_ButtonFlags_AllowOverlapMode + SetItemAllowOverlap()
  //  'hovered' will be true when hovering the Tab but NOT when hovering the close button
  //  'g.HoveredId==id' will be true when hovering the Tab including when hovering the close button
  //  'g.ActiveId==close_button_id' will be true when we are holding on the close button, in which
  //  case both hovered booleans are false
  bool close_button_pressed = false;
  bool close_button_visible = false;
  if (close_button_id != 0)
    if (is_contents_visible || bb.GetWidth() >= g.Style.TabMinWidthForCloseButton)
      if (g.HoveredId == tab_id || g.HoveredId == close_button_id || g.ActiveId == tab_id ||
          g.ActiveId == close_button_id)
        close_button_visible = true;
  if (close_button_visible) {
    ANCHOR_LastItemDataBackup last_item_backup;
    const float close_button_sz = g.FontSize;
    PushStyleVar(ANCHOR_StyleVar_FramePadding, frame_padding);
    if (CloseButton(close_button_id,
                    GfVec2f(bb.Max[0] - frame_padding[0] * 2.0f - close_button_sz, bb.Min[1])))
      close_button_pressed = true;
    PopStyleVar();
    last_item_backup.Restore();

    // Close with middle mouse button
    if (!(flags & ANCHOR_TabItemFlags_NoCloseWithMiddleMouseButton) && IsMouseClicked(2))
      close_button_pressed = true;

    text_pixel_clip_bb.Max[0] -= close_button_sz;
  }

  // FIXME: if FramePadding is noticeably large, ellipsis_max_x will be wrong here (e.g. #3497),
  // maybe for consistency that parameter of RenderTextEllipsis() shouldn't exist..
  float ellipsis_max_x = close_button_visible ? text_pixel_clip_bb.Max[0] : bb.Max[0] - 1.0f;
  RenderTextEllipsis(draw_list,
                     text_ellipsis_clip_bb.Min,
                     text_ellipsis_clip_bb.Max,
                     text_pixel_clip_bb.Max[0],
                     ellipsis_max_x,
                     label,
                     NULL,
                     &label_size);

#  if 0
    if (!is_contents_visible)
        g.Style.Alpha = backup_alpha;
#  endif

  if (out_just_closed)
    *out_just_closed = close_button_pressed;
}

#endif  // #ifndef ANCHOR_DISABLE

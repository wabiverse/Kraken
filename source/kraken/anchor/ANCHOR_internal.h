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

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Enable SSE intrinsics if available */
#if defined __SSE__ || defined __x86_64__ || defined _M_X64
#  define ANCHOR_ENABLE_SSE
#  include <immintrin.h>
#endif

#include <wabi/imaging/hd/driver.h>
#include <wabi/imaging/hgi/hgi.h>
#include <wabi/imaging/hgi/tokens.h>

#include <wabi/usdImaging/usdImagingGL/engine.h>
#include <wabi/usdImaging/usdImaging/primAdapter.h>

/**
 * Visual Studio warnings */
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4251)
#  pragma warning(disable : 26812)
#  pragma warning(disable : 26495)
#  if defined(_MSC_VER) && _MSC_VER >= 1922
#    pragma warning(disable : 5054)
#  endif
#endif

/**
 * Clang/GCC warnings with -Weverything */
#if defined(__clang__)
#  pragma clang diagnostic push
#  if __has_warning("-Wunknown-warning-option")
#    pragma clang diagnostic ignored "-Wunknown-warning-option"
#  endif
#  pragma clang diagnostic ignored "-Wunknown-pragmas"
#  pragma clang diagnostic ignored "-Wfloat-equal"
#  pragma clang diagnostic ignored "-Wunused-function"
#  pragma clang diagnostic ignored "-Wmissing-prototypes"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#  pragma clang diagnostic ignored "-Wdouble-promotion"
#  pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpragmas"
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

/**
 * Store 1-bit per value */
struct AnchorBitVector;
/**
 * An axis-aligned rectangle (2 points) */
struct AnchorBBox;
/**
 * Helper to build a AnchorDrawData instance */
struct AnchorDrawDataBuilder;
/**
 * Data shared between all AnchorDrawList instances */
struct AnchorDrawListSharedData;
/**
 * Stacked color modifier, backup of modified data so we can restore it */
struct AnchorColorMod;
/**
 * Main ANCHOR context */
struct AnchorContext;
/**
 * Hook for extensions like ANCHORTestEngine */
struct AnchorContextHook;
/**
 * Type information associated to a AnchorDataType enum */
struct AnchorDataTypeInfo;
/**
 * Stacked storage data for BeginGroup()/EndGroup() */
struct AnchorGroupData;
/**
 * Internal state of the currently focused/edited text input box */
struct AnchorInputTextState;
/**
 * Backup and restore IsItemHovered() internal data */
struct AnchorLastItemDataBackup;
/**
 * Simple column measurement, currently used for MenuItem() only */
struct AnchorMenuColumns;
/**
 * Result of a gamepad/keyboard directional navigation move query result */
struct AnchorNavItemData;
/**
 * Storage for ShowMetricsWindow() and DebugNodeXXX() functions */
struct AnchorMetricsConfig;
/**
 * Storage for SetNextWindow** functions */
struct AnchorNextWindowData;
/**
 * Storage for SetNextItem** functions */
struct AnchorNextItemData;
/**
 * Storage data for a single column for legacy Columns() api */
struct AnchorOldColumnData;
/**
 * Storage data for a columns set for legacy Columns() api */
struct AnchorOldColumns;
/**
 * Storage for current popup stack */
struct AnchorPopupData;
/**
 * Storage for one type registered in the .ini file */
struct AnchorSettingsHandler;
/**
 * Storage of stack sizes for debugging/asserting */
struct AnchorStackSizes;
/**
 * Stacked style modifier, backup of modified data so we can restore it */
struct AnchorStyleMod;
/**
 * Storage for a tab bar */
struct AnchorTabBar;
/**
 * Storage for a tab item (within a tab bar) */
struct AnchorTabItem;
/**
 * Storage for a table */
struct AnchorTable;
/**
 * Storage for one column of a table */
struct AnchorTableColumn;
/**
 * Temporary storage for one table (one per table in the stack), shared between tables. */
struct AnchorTableTempData;
/**
 * Storage for a table .ini settings */
struct AnchorTableSettings;
/**
 * Storage for a column .ini settings */
struct AnchorTableColumnsSettings;
/**
 * Storage for one window */
struct AnchorWindow;
/**
 * Temporary storage for one window */
struct AnchorWindowTempData;
/**
 * Storage for a window .ini settings */
struct AnchorWindowSettings;

/**
 * Use your programming IDE "Go to definition"
 * facility on the names of the center columns
 * to find the actual flags/enum lists. */
/**
 * -> enum AnchorLayoutType_           Enum: Horizontal or vertical */
typedef int AnchorLayoutType;
/**
 * -> enum AnchorItemFlags_            Flags: for PushItemFlag() */
typedef int AnchorItemFlags;
/**
 * -> enum AnchorItemAddFlags_         Flags: for ItemAdd() */
typedef int AnchorItemAddFlags;
/**
 * -> enum AnchorItemStatusFlags_      Flags: for DC.LastItemStatusFlags */
typedef int AnchorItemStatusFlags;
/**
 * -> enum AnchorOldColumnFlags_       Flags: for BeginColumns() */
typedef int AnchorOldColumnFlags;
/**
 * -> enum AnchorNavHighlightFlags_    Flags: for RenderNavHighlight() */
typedef int AnchorNavHighlightFlags;
/**
 * -> enum AnchorNavDirSourceFlags_    Flags: for GetNavInputAmount2d() */
typedef int AnchorNavDirSourceFlags;
/**
 * -> enum AnchorNavMoveFlags_         Flags: for navigation requests */
typedef int AnchorNavMoveFlags;
/**
 * -> enum AnchorNextItemDataFlags_    Flags: for SetNextItemXXX() functions */
typedef int AnchorNextItemDataFlags;
/**
 * -> enum AnchorNextWindowDataFlags_  Flags: for SetNextWindowXXX() functions */
typedef int AnchorNextWindowDataFlags;
/**
 * -> enum AnchorSeparatorFlags_       Flags: for SeparatorEx() */
typedef int AnchorSeparatorFlags;
/**
 * -> enum AnchorTextFlags_            Flags: for TextEx() */
typedef int AnchorTextFlags;
/**
 * -> enum AnchorTooltipFlags_         Flags: for BeginTooltipEx() */
typedef int AnchorTooltipFlags;

typedef void (*AnchorErrorLogCallback)(void *user_data, const char *fmt, ...);


#ifndef G_CTX
/**
 * Current implicit context pointer. */
extern ANCHOR_API AnchorContext *G_CTX;
#endif

namespace AnchorStb
{

#undef STB_TEXTEDIT_STRING
#undef STB_TEXTEDIT_CHARTYPE
#define STB_TEXTEDIT_STRING AnchorInputTextState
#define STB_TEXTEDIT_CHARTYPE AnchorWChar
#define STB_TEXTEDIT_GETWIDTH_NEWLINE (-1.0f)
#define STB_TEXTEDIT_UNDOSTATECOUNT 99
#define STB_TEXTEDIT_UNDOCHARCOUNT 999
#define STB_TEXTEDIT_POSITIONTYPE int

  typedef struct
  {
    /**
     * private data */
    STB_TEXTEDIT_POSITIONTYPE where;
    STB_TEXTEDIT_POSITIONTYPE insert_length;
    STB_TEXTEDIT_POSITIONTYPE delete_length;
    int char_storage;
  } StbUndoRecord;

  typedef struct
  {
    /**
     * private data */
    StbUndoRecord undo_rec[STB_TEXTEDIT_UNDOSTATECOUNT];
    STB_TEXTEDIT_CHARTYPE undo_char[STB_TEXTEDIT_UNDOCHARCOUNT];
    short undo_point, redo_point;
    int undo_char_point, redo_char_point;
  } StbUndoState;

  typedef struct
  {
    /**
     * position of the text cursor within the string */
    int cursor;

    /**
     * selection start and end point in characters;
     * if equal, no selection. note that start may
     * be less than or greater than end (e.g. when
     * dragging the mouse, start is where the first
     * initial click was, and you can drag in either
     * direction) */
    int select_start;
    int select_end;

    /**
     * each textfield keeps its own insert
     * mode state. to keep an application
     * wide insert mode, copy this value
     * in/out of the app state */
    unsigned char insert_mode;

    /**
     * page size in number of row.
     * this value MUST be set to >0
     * for pageup or pagedown within
     * multilines documents. */
    int row_count_per_page;

    /**
     * private data */
    unsigned char cursor_at_end_of_line;
    unsigned char initialized;
    unsigned char has_preferred_x;
    unsigned char single_line;
    unsigned char padding1, padding2, padding3;
    float preferred_x;
    StbUndoState undostate;
  } STB_TexteditState;

  typedef struct
  {
    /**
     * result of layout query */

    /**
     * starting x location,
     * end x location which
     * allows for align=right,
     * etc) */
    float x0, x1;

    /**
     * position of baseline relative
     * to previous row's baseline */
    float baseline_y_delta;

    /**
     * height of row above
     * and below baseline. */
    float ymin, ymax;

    int num_chars;
  } StbTexteditRow;

} /* namespace AnchorStb */

/**
 * Debug Logging */
#ifndef ANCHOR_DEBUG_LOG
#  define ANCHOR_DEBUG_LOG(_FMT, ...) printf("[%05d] " _FMT, G_CTX->FrameCount, __VA_ARGS__)
#endif

/**
 * Debug Logging for selected systems. Remove the '((void)0) //' to enable.
 * #define ANCHOR_DEBUG_LOG_POPUP    ANCHOR_DEBUG_LOG // Enable log
 * #define ANCHOR_DEBUG_LOG_NAV      ANCHOR_DEBUG_LOG // Enable log */
#define ANCHOR_DEBUG_LOG_POPUP(...) ((void)0)  // Disable log
#define ANCHOR_DEBUG_LOG_NAV(...) ((void)0)    // Disable log

/**
 *  Static Asserts */
#if (__cplusplus >= 201100) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201100)
#  define IM_STATIC_ASSERT(_COND) static_assert(_COND, "")
#else
#  define IM_STATIC_ASSERT(_COND) typedef char static_assertion_##__line__[(_COND) ? 1 : -1]
#endif

/**
 * "Paranoid" Debug Asserts are meant to only be enabled
 * during specific debugging/work, otherwise would slow
 * down the code too much. We currently don't have many
 * of those so the effect is currently negligible, but
 * onward intent to add more aggressive ones in the
 * code. */
#ifdef ANCHOR_DEBUG_PARANOID
#  define ANCHOR_ASSERT_PARANOID(_EXPR) ANCHOR_ASSERT(_EXPR)
#else
#  define ANCHOR_ASSERT_PARANOID(_EXPR)
#endif

/**
 * Error handling
 * Down the line in some frameworks/languages
 * we would like to have a way to redirect
 * those to the programmer and recover from
 * more faults. */
#ifndef ANCHOR_ASSERT_USER_ERROR
#  define ANCHOR_ASSERT_USER_ERROR(_EXP, _MSG) \
    ANCHOR_ASSERT((_EXP) && _MSG)  // Recoverable User Error
#endif

/**
 * Misc Macros */
#define IM_PI 3.14159265358979323846f
#ifdef _WIN32
#  define ANCHOR_NEWLINE "\r\n"
#else
#  define ANCHOR_NEWLINE "\n"
#endif
#define IM_TABSIZE (4)
#define ANCHOR_MEMALIGN(_OFF, _ALIGN) (((_OFF) + (_ALIGN - 1)) & ~(_ALIGN - 1))
#define IM_F32_TO_INT8_UNBOUND(_VAL) ((int)((_VAL)*255.0f + ((_VAL) >= 0 ? 0.5f : -0.5f)))
#define IM_F32_TO_INT8_SAT(_VAL) ((int)(AnchorSaturate(_VAL) * 255.0f + 0.5f))
#define ANCHOR_FLOOR(_VAL) ((float)(int)(_VAL))
#define IM_ROUND(_VAL) ((float)(int)((_VAL) + 0.5f))

/**
 * Enforce cdecl calling convention for functions
 * called by the standard library, in case comp
 * settings changed the default to e.g.
 * __vectorcall */
#ifdef _MSC_VER
#  define ANCHOR_CDECL __cdecl
#else
#  define ANCHOR_CDECL
#endif

/**
 * Warnings */
#if defined(_MSC_VER) && !defined(__clang__)
#  define ANCHOR_MSVC_WARNING_SUPPRESS(XXXX) __pragma(warning(suppress : XXXX))
#else
#  define ANCHOR_MSVC_WARNING_SUPPRESS(XXXX)
#endif

/**
 * Debug Tools
 *
 * - Use 'Metrics->Tools->Item Picker' to
 *   break into the call-stack of a specific
 *   item. */
#ifndef IM_DEBUG_BREAK
#  if defined(__clang__)
#    define IM_DEBUG_BREAK() __builtin_debugtrap()
#  elif defined(_MSC_VER)
#    define IM_DEBUG_BREAK() __debugbreak()
#  else
#    define IM_DEBUG_BREAK() ANCHOR_ASSERT(0)
#  endif
/* IM_DEBUG_BREAK */
#endif

/**
 * Helpers: Hashing */
ANCHOR_API ANCHOR_ID AnchorHashData(const void *data, size_t data_size, AnchorU32 seed = 0);
ANCHOR_API ANCHOR_ID AnchorHashStr(const char *data, size_t data_size = 0, AnchorU32 seed = 0);
#ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
static inline ANCHOR_ID ImHash(const void *data, int size, AnchorU32 seed = 0)
{
  return size ? AnchorHashData(data, (size_t)size, seed) :
                AnchorHashStr((const char *)data, 0, seed);
}
#endif

/**
 * Helpers: Sorting */
#define ImQsort qsort

/**
 * Helpers: Color Blending */
ANCHOR_API AnchorU32 ImAlphaBlendColors(AnchorU32 col_a, AnchorU32 col_b);

/**
 * Helpers: Bit manipulation */
static inline bool ImIsPowerOfTwo(int v)
{
  return v != 0 && (v & (v - 1)) == 0;
}
static inline bool ImIsPowerOfTwo(AnchorU64 v)
{
  return v != 0 && (v & (v - 1)) == 0;
}
static inline int ImUpperPowerOfTwo(int v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

/**
 * Helpers: String, Formatting */
ANCHOR_API int AnchorStricmp(const char *str1, const char *str2);
ANCHOR_API int AnchorStrnicmp(const char *str1, const char *str2, size_t count);
ANCHOR_API void AnchorStrncpy(char *dst, const char *src, size_t count);
ANCHOR_API char *AnchorStrdup(const char *str);
ANCHOR_API char *AnchorStrdupcpy(char *dst, size_t *p_dst_size, const char *str);
ANCHOR_API const char *AnchorStrchrRange(const char *str_begin, const char *str_end, char c);
ANCHOR_API int AnchorStrlenW(const AnchorWChar *str);
ANCHOR_API const char *AnchorStreolRange(const char *str, const char *str_end);
ANCHOR_API const AnchorWChar *AnchorStrbolW(const AnchorWChar *buf_mid_line,
                                            const AnchorWChar *buf_begin);
ANCHOR_API const char *AnchorStristr(const char *haystack,
                                     const char *haystack_end,
                                     const char *needle,
                                     const char *needle_end);
ANCHOR_API void AnchorTrimBlanks(char *str);
ANCHOR_API const char *AnchorStrSkipBlank(const char *str);
ANCHOR_API int AnchorFormatString(char *buf, size_t buf_size, const char *fmt, ...)
  ANCHOR_FMTARGS(3);
ANCHOR_API int AnchorFormatStringV(char *buf, size_t buf_size, const char *fmt, va_list args)
  ANCHOR_FMTLIST(3);
ANCHOR_API const char *AnchorFormatFindStart(const char *format);
ANCHOR_API const char *AnchorParseFormatFindEnd(const char *format);
ANCHOR_API const char *AnchorParseFormatTrimDecorations(const char *format,
                                                        char *buf,
                                                        size_t buf_size);
ANCHOR_API int AnchorParseFormatPrecision(const char *format, int default_value);
static inline bool AnchorCharIsBlankA(char c)
{
  return c == ' ' || c == '\t';
}
static inline bool AnchorCharIsBlankW(unsigned int c)
{
  return c == ' ' || c == '\t' || c == 0x3000;
}

// Helpers: UTF-8 <> wchar conversions
// return out_buf
ANCHOR_API const char *AnchorTextCharToUtf8(char out_buf[5], unsigned int c);

// return output UTF-8 bytes count
ANCHOR_API int AnchorTextStrToUtf8(char *out_buf,
                                   int out_buf_size,
                                   const AnchorWChar *in_text,
                                   const AnchorWChar *in_text_end);

// read one character. return input UTF-8 bytes count
ANCHOR_API int AnchorTextCharFromUtf8(unsigned int *out_char,
                                      const char *in_text,
                                      const char *in_text_end);
// return input UTF-8 bytes count
ANCHOR_API int AnchorTextStrFromUtf8(AnchorWChar *out_buf,
                                     int out_buf_size,
                                     const char *in_text,
                                     const char *in_text_end,
                                     const char **in_remaining = NULL);

/**
 * return number of UTF-8
 * code-points (NOT bytes count) */
ANCHOR_API int AnchorTextCountCharsFromUtf8(const char *in_text, const char *in_text_end);

/**
 * return number of bytes to
 * express one char in UTF-8 */
ANCHOR_API int AnchorTextCountUtf8BytesFromChar(const char *in_text, const char *in_text_end);

/**
 * return number of bytes to
 * express string in UTF-8 */
ANCHOR_API int AnchorTextCountUtf8BytesFromStr(const AnchorWChar *in_text,
                                               const AnchorWChar *in_text_end);

/**
 * Helpers: File System */
#ifdef ANCHOR_DISABLE_FILE_FUNCTIONS
#  define ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void *ImFileHandle;
static inline ImFileHandle ImFileOpen(const char *, const char *)
{
  return NULL;
}
static inline bool ImFileClose(ImFileHandle)
{
  return false;
}
static inline AnchorU64 ImFileGetSize(ImFileHandle)
{
  return (AnchorU64)-1;
}
static inline AnchorU64 ImFileRead(void *, AnchorU64, AnchorU64, ImFileHandle)
{
  return 0;
}
static inline AnchorU64 ImFileWrite(const void *, AnchorU64, AnchorU64, ImFileHandle)
{
  return 0;
}
#endif /* ANCHOR_DISABLE_FILE_FUNCTIONS */
#ifndef ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE *ImFileHandle;
ANCHOR_API ImFileHandle ImFileOpen(const char *filename, const char *mode);
ANCHOR_API bool ImFileClose(ImFileHandle file);
ANCHOR_API AnchorU64 ImFileGetSize(ImFileHandle file);
ANCHOR_API AnchorU64 ImFileRead(void *data, AnchorU64 size, AnchorU64 count, ImFileHandle file);
ANCHOR_API AnchorU64 ImFileWrite(const void *data,
                                 AnchorU64 size,
                                 AnchorU64 count,
                                 ImFileHandle file);
#else /* ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS */
/**
 * Can't use stdout, fflush if
 * we are not using default */
#  define ANCHOR_DISABLE_TTY_FUNCTIONS
#endif /* ANCHOR_DISABLE_TTY_FUNCTIONS */
ANCHOR_API void *ImFileLoadToMemory(const char *filename,
                                    const char *mode,
                                    size_t *out_file_size = NULL,
                                    int padding_bytes = 0);

/**
 * Helpers: Maths
 * - Wrapper for standard libs functions.
 *  (Note that ANCHOR_demo.cpp does _not_
 *  use them to keep the code easy to copy) */
ANCHOR_MSVC_RUNTIME_CHECKS_OFF
#ifndef ANCHOR_DISABLE_DEFAULT_MATH_FUNCTIONS
#  define AnchorFabs(X) fabsf(X)
#  define AnchorSqrt(X) sqrtf(X)
#  define AnchorFmod(X, Y) fmodf((X), (Y))
#  define AnchorCos(X) cosf(X)
#  define AnchorSin(X) sinf(X)
#  define AnchorAcos(X) acosf(X)
#  define AnchorAtan2(Y, X) atan2f((Y), (X))
#  define AnchorAtof(STR) atof(STR)
/**
 * We use our own, see
 * AnchorFloor() and
 * AnchorFloorSigned()
 * #define AnchorFloorStd(X) floorf(X) */
#  define ImCeil(X) ceilf(X)
static inline float AnchorPow(float x, float y)
{
  return powf(x, y);
}
/**
 * DragBehaviorT/SliderBehaviorT uses AnchorPow
 * with either float/double and need the
 * precision */
static inline double AnchorPow(double x, double y)
{
  return pow(x, y);
}
static inline float AnchorLog(float x)
{
  return logf(x);
}
/**
 * DragBehaviorT/SliderBehaviorT uses
 * AnchorLog with either float/double and
 * need the precision */
static inline double AnchorLog(double x)
{
  return log(x);
}
static inline int AnchorAbs(int x)
{
  return x < 0 ? -x : x;
}
static inline float AnchorAbs(float x)
{
  return fabsf(x);
}
static inline double AnchorAbs(double x)
{
  return fabs(x);
}
static inline float AnchorSign(float x)
{
  return (x < 0.0f) ? -1.0f : ((x > 0.0f) ? 1.0f : 0.0f);
}
/**
 * Sign operator - returns -1, 0 or 1
 * based on sign of argument */
static inline double AnchorSign(double x)
{
  return (x < 0.0) ? -1.0 : ((x > 0.0) ? 1.0 : 0.0);
}
#  ifdef ANCHOR_ENABLE_SSE
static inline float AnchorRsqrt(float x)
{
  return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
}
#  else  /* !ANCHOR_ENABLE_SSE */
static inline float AnchorRsqrt(float x)
{
  return 1.0f / sqrtf(x);
}
#  endif /* !ANCHOR_ENABLE_SSE */
static inline double AnchorRsqrt(double x)
{
  return 1.0 / sqrt(x);
}
#endif /* ANCHOR_DISABLE_DEFAULT_MATH_FUNCTIONS */
/**
 * The following are used by widgets
 * which support variety of types:
 * - AnchorMin    - AnchorMax
 * - AnchorClamp  - AnchorLerp
 * - AnchorSwap signed/unsigned
 * int/long long float/double
 * (Exceptionally using templates
 * here but we could also redefine
 * them for those types) */
template<typename T> static inline T AnchorMin(T lhs, T rhs)
{
  return lhs < rhs ? lhs : rhs;
}
template<typename T> static inline T AnchorMax(T lhs, T rhs)
{
  return lhs >= rhs ? lhs : rhs;
}
template<typename T> static inline T AnchorClamp(T v, T mn, T mx)
{
  return (v < mn) ? mn : (v > mx) ? mx : v;
}
template<typename T> static inline T AnchorLerp(T a, T b, float t)
{
  return (T)(a + (b - a) * t);
}
template<typename T> static inline void AnchorSwap(T &a, T &b)
{
  T tmp = a;
  a = b;
  b = tmp;
}
template<typename T> static inline T AnchorAddClampOverflow(T a, T b, T mn, T mx)
{
  if (b < 0 && (a < mn - b))
    return mn;
  if (b > 0 && (a > mx - b))
    return mx;
  return a + b;
}
template<typename T> static inline T AnchorSubClampOverflow(T a, T b, T mn, T mx)
{
  if (b > 0 && (a < mn + b))
    return mn;
  if (b < 0 && (a > mx + b))
    return mx;
  return a - b;
}
/**
 * - Misc maths helpers */
static inline wabi::GfVec2f AnchorMin(const wabi::GfVec2f &lhs, const wabi::GfVec2f &rhs)
{
  return wabi::GfVec2f(lhs[0] < rhs[0] ? lhs[0] : rhs[0], lhs[1] < rhs[1] ? lhs[1] : rhs[1]);
}
static inline wabi::GfVec2f AnchorMax(const wabi::GfVec2f &lhs, const wabi::GfVec2f &rhs)
{
  return wabi::GfVec2f(lhs[0] >= rhs[0] ? lhs[0] : rhs[0], lhs[1] >= rhs[1] ? lhs[1] : rhs[1]);
}
static inline wabi::GfVec2f AnchorClamp(const wabi::GfVec2f &v,
                                        const wabi::GfVec2f &mn,
                                        wabi::GfVec2f mx)
{
  return wabi::GfVec2f((v[0] < mn[0]) ? mn[0] :
                       (v[0] > mx[0]) ? mx[0] :
                                        v[0],
                       (v[1] < mn[1]) ? mn[1] :
                       (v[1] > mx[1]) ? mx[1] :
                                        v[1]);
}
static inline wabi::GfVec2f AnchorLerp(const wabi::GfVec2f &a, const wabi::GfVec2f &b, float t)
{
  return wabi::GfVec2f(a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t);
}
static inline wabi::GfVec2f AnchorLerp(const wabi::GfVec2f &a,
                                       const wabi::GfVec2f &b,
                                       const wabi::GfVec2f &t)
{
  return wabi::GfVec2f(a[0] + (b[0] - a[0]) * t[0], a[1] + (b[1] - a[1]) * t[1]);
}
static inline wabi::GfVec4f AnchorLerp(const wabi::GfVec4f &a, const wabi::GfVec4f &b, float t)
{
  return wabi::GfVec4f(a[0] + (b[0] - a[0]) * t,
                       a[1] + (b[1] - a[1]) * t,
                       a[2] + (b[2] - a[2]) * t,
                       a[3] + (b[3] - a[3]) * t);
}
static inline float AnchorSaturate(float f)
{
  return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f;
}
static inline float AnchorLengthSqr(const wabi::GfVec2f &lhs)
{
  return (lhs[0] * lhs[0]) + (lhs[1] * lhs[1]);
}
static inline float AnchorLengthSqr(const wabi::GfVec4f &lhs)
{
  return (lhs[0] * lhs[0]) + (lhs[1] * lhs[1]) + (lhs[2] * lhs[2]) + (lhs[3] * lhs[3]);
}
static inline float AnchorInvLength(const wabi::GfVec2f &lhs, float fail_value)
{
  float d = (lhs[0] * lhs[0]) + (lhs[1] * lhs[1]);
  if (d > 0.0f)
    return AnchorRsqrt(d);
  return fail_value;
}
static inline float AnchorFloor(float f)
{
  return (float)(int)(f);
}
static inline float AnchorFloorSigned(float f)
{
  return (float)((f >= 0 || (int)f == f) ? (int)f : (int)f - 1);
}  // Decent replacement for floorf()
static inline wabi::GfVec2f AnchorFloor(const wabi::GfVec2f &v)
{
  return wabi::GfVec2f((float)(int)(v[0]), (float)(int)(v[1]));
}
static inline int AnchorModPositive(int a, int b)
{
  return (a + b) % b;
}
static inline float AnchorDot(const wabi::GfVec2f &a, const wabi::GfVec2f &b)
{
  return a[0] * b[0] + a[1] * b[1];
}
static inline wabi::GfVec2f AnchorRotate(const wabi::GfVec2f &v, float cos_a, float sin_a)
{
  return wabi::GfVec2f(v[0] * cos_a - v[1] * sin_a, v[0] * sin_a + v[1] * cos_a);
}
static inline float AnchorLinearSweep(float current, float target, float speed)
{
  if (current < target)
    return AnchorMin(current + speed, target);
  if (current > target)
    return AnchorMax(current - speed, target);
  return current;
}
static inline wabi::GfVec2f AnchorMul(const wabi::GfVec2f &lhs, const wabi::GfVec2f &rhs)
{
  return wabi::GfVec2f(lhs[0] * rhs[0], lhs[1] * rhs[1]);
}
ANCHOR_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Geometry
ANCHOR_API wabi::GfVec2f AnchorBezierCubicCalc(const wabi::GfVec2f &p1,
                                               const wabi::GfVec2f &p2,
                                               const wabi::GfVec2f &p3,
                                               const wabi::GfVec2f &p4,
                                               float t);
ANCHOR_API wabi::GfVec2f AnchorBezierCubicClosestPoint(
  const wabi::GfVec2f &p1,
  const wabi::GfVec2f &p2,
  const wabi::GfVec2f &p3,
  const wabi::GfVec2f &p4,
  const wabi::GfVec2f &p,
  int num_segments);  // For curves with explicit number of segments
ANCHOR_API wabi::GfVec2f AnchorBezierCubicClosestPointCasteljau(
  const wabi::GfVec2f &p1,
  const wabi::GfVec2f &p2,
  const wabi::GfVec2f &p3,
  const wabi::GfVec2f &p4,
  const wabi::GfVec2f &p,
  float tess_tol);  // For auto-tessellated curves you can use
                    // tess_tol = style.CurveTessellationTol
ANCHOR_API wabi::GfVec2f AnchorBezierQuadraticCalc(const wabi::GfVec2f &p1,
                                                   const wabi::GfVec2f &p2,
                                                   const wabi::GfVec2f &p3,
                                                   float t);
ANCHOR_API wabi::GfVec2f AnchorLineClosestPoint(const wabi::GfVec2f &a,
                                                const wabi::GfVec2f &b,
                                                const wabi::GfVec2f &p);
ANCHOR_API bool AnchorTriangleContainsPoint(const wabi::GfVec2f &a,
                                            const wabi::GfVec2f &b,
                                            const wabi::GfVec2f &c,
                                            const wabi::GfVec2f &p);
ANCHOR_API wabi::GfVec2f AnchorTriangleClosestPoint(const wabi::GfVec2f &a,
                                                    const wabi::GfVec2f &b,
                                                    const wabi::GfVec2f &c,
                                                    const wabi::GfVec2f &p);
ANCHOR_API void AnchorTriangleBarycentricCoords(const wabi::GfVec2f &a,
                                                const wabi::GfVec2f &b,
                                                const wabi::GfVec2f &c,
                                                const wabi::GfVec2f &p,
                                                float &out_u,
                                                float &out_v,
                                                float &out_w);
inline float AnchorTriangleArea(const wabi::GfVec2f &a,
                                const wabi::GfVec2f &b,
                                const wabi::GfVec2f &c)
{
  return AnchorFabs((a[0] * (b[1] - c[1])) + (b[0] * (c[1] - a[1])) + (c[0] * (a[1] - b[1]))) *
         0.5f;
}
ANCHOR_API AnchorDir AnchorGetDirQuadrantFromDelta(float dx, float dy);

/**
 * Helper: GfVec1 (1D vector)
 * (this odd construct is used
 * to facilitate the transition
 * between 1D and 2D, and also
 * the maintenance of some of
 * the branches / patches) */
ANCHOR_MSVC_RUNTIME_CHECKS_OFF
struct GfVec1
{
  float x;
  GfVec1()
  {
    x = 0.0f;
  }
  GfVec1(float _x)
  {
    x = _x;
  }
};

/**
 * Helper: AnchorBBox (2D axis aligned bounding-box)
 * NB: we can't rely on wabi::GfVec2f math operators
 * being available here! */
struct ANCHOR_API AnchorBBox
{
  /**
   * Upper-left */
  wabi::GfVec2f Min;
  /**
   * Lower-right */
  wabi::GfVec2f Max;

  AnchorBBox() : Min(0.0f, 0.0f), Max(0.0f, 0.0f) {}
  AnchorBBox(const wabi::GfVec2f &min, const wabi::GfVec2f &max) : Min(min), Max(max) {}
  AnchorBBox(const wabi::GfVec4f &v) : Min(v[0], v[1]), Max(v[2], v[3]) {}
  AnchorBBox(float x1, float y1, float x2, float y2) : Min(x1, y1), Max(x2, y2) {}

  wabi::GfVec2f GetCenter() const
  {
    return wabi::GfVec2f((Min[0] + Max[0]) * 0.5f, (Min[1] + Max[1]) * 0.5f);
  }
  wabi::GfVec2f GetSize() const
  {
    return wabi::GfVec2f(Max[0] - Min[0], Max[1] - Min[1]);
  }
  float GetWidth() const
  {
    return Max[0] - Min[0];
  }
  float GetHeight() const
  {
    return Max[1] - Min[1];
  }
  float GetArea() const
  {
    return (Max[0] - Min[0]) * (Max[1] - Min[1]);
  }
  wabi::GfVec2f GetTL() const
  {
    return Min;
  }
  /**
   * Top-left */
  wabi::GfVec2f GetTR() const
  {
    return wabi::GfVec2f(Max[0], Min[1]);
  }
  /**
   * Top-right */
  wabi::GfVec2f GetBL() const
  {
    return wabi::GfVec2f(Min[0], Max[1]);
  }
  /**
   * Bottom-left */
  wabi::GfVec2f GetBR() const
  {
    return Max;
  }
  /**
   * Bottom-right */
  bool Contains(const wabi::GfVec2f &p) const
  {
    return p[0] >= Min[0] && p[1] >= Min[1] && p[0] < Max[0] && p[1] < Max[1];
  }
  bool Contains(const AnchorBBox &r) const
  {
    return r.Min[0] >= Min[0] && r.Min[1] >= Min[1] && r.Max[0] <= Max[0] && r.Max[1] <= Max[1];
  }
  bool Overlaps(const AnchorBBox &r) const
  {
    return r.Min[1] < Max[1] && r.Max[1] > Min[1] && r.Min[0] < Max[0] && r.Max[0] > Min[0];
  }
  void Add(const wabi::GfVec2f &p)
  {
    if (Min[0] > p[0])
      Min[0] = p[0];
    if (Min[1] > p[1])
      Min[1] = p[1];
    if (Max[0] < p[0])
      Max[0] = p[0];
    if (Max[1] < p[1])
      Max[1] = p[1];
  }
  void Add(const AnchorBBox &r)
  {
    if (Min[0] > r.Min[0])
      Min[0] = r.Min[0];
    if (Min[1] > r.Min[1])
      Min[1] = r.Min[1];
    if (Max[0] < r.Max[0])
      Max[0] = r.Max[0];
    if (Max[1] < r.Max[1])
      Max[1] = r.Max[1];
  }
  void Expand(const float amount)
  {
    Min[0] -= amount;
    Min[1] -= amount;
    Max[0] += amount;
    Max[1] += amount;
  }
  void Expand(const wabi::GfVec2f &amount)
  {
    Min[0] -= amount[0];
    Min[1] -= amount[1];
    Max[0] += amount[0];
    Max[1] += amount[1];
  }
  void Translate(const wabi::GfVec2f &d)
  {
    Min[0] += d[0];
    Min[1] += d[1];
    Max[0] += d[0];
    Max[1] += d[1];
  }
  void TranslateX(float dx)
  {
    Min[0] += dx;
    Max[0] += dx;
  }
  void TranslateY(float dy)
  {
    Min[1] += dy;
    Max[1] += dy;
  }
  void ClipWith(const AnchorBBox &r)
  {
    Min = AnchorMax(Min, r.Min);
    Max = AnchorMin(Max, r.Max);
  }
  /**
   * Simple version, may lead to an
   * inverted rectangle, which is fine
   * for Contains/Overlaps test but not
   * for display. */
  void ClipWithFull(const AnchorBBox &r)
  {
    Min = AnchorClamp(Min, r.Min, r.Max);
    Max = AnchorClamp(Max, r.Min, r.Max);
  }
  /**
   * Full version, ensure both
   * points are fully clipped */
  void Floor()
  {
    Min[0] = ANCHOR_FLOOR(Min[0]);
    Min[1] = ANCHOR_FLOOR(Min[1]);
    Max[0] = ANCHOR_FLOOR(Max[0]);
    Max[1] = ANCHOR_FLOOR(Max[1]);
  }
  bool IsInverted() const
  {
    return Min[0] > Max[0] || Min[1] > Max[1];
  }
  wabi::GfVec4f ToVec4() const
  {
    return wabi::GfVec4f(Min[0], Min[1], Max[0], Max[1]);
  }
};
ANCHOR_MSVC_RUNTIME_CHECKS_RESTORE

/**
 * Helper: AnchorBitArray */
inline bool AnchorBitArrayTestBit(const AnchorU32 *arr, int n)
{
  AnchorU32 mask = (AnchorU32)1 << (n & 31);
  return (arr[n >> 5] & mask) != 0;
}
inline void AnchorBitArrayClearBit(AnchorU32 *arr, int n)
{
  AnchorU32 mask = (AnchorU32)1 << (n & 31);
  arr[n >> 5] &= ~mask;
}
inline void AnchorBitArraySetBit(AnchorU32 *arr, int n)
{
  AnchorU32 mask = (AnchorU32)1 << (n & 31);
  arr[n >> 5] |= mask;
}
inline void AnchorBitArraySetBitRange(AnchorU32 *arr, int n, int n2)  // Works on range [n..n2)
{
  n2--;
  while (n <= n2) {
    int a_mod = (n & 31);
    int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
    AnchorU32 mask = (AnchorU32)(((AnchorU64)1 << b_mod) - 1) &
                     ~(AnchorU32)(((AnchorU64)1 << a_mod) - 1);
    arr[n >> 5] |= mask;
    n = (n + 32) & ~31;
  }
}

/**
 * Helper: AnchorBitArray class
 * (wrapper over AnchorBitArray
 * functions) Store 1-bit per
 * value. */
template<int BITCOUNT> struct ANCHOR_API AnchorBitArray
{
  AnchorU32 Storage[(BITCOUNT + 31) >> 5];
  AnchorBitArray()
  {
    ClearAllBits();
  }
  void ClearAllBits()
  {
    memset(Storage, 0, sizeof(Storage));
  }
  void SetAllBits()
  {
    memset(Storage, 255, sizeof(Storage));
  }
  bool TestBit(int n) const
  {
    ANCHOR_ASSERT(n < BITCOUNT);
    return AnchorBitArrayTestBit(Storage, n);
  }
  void SetBit(int n)
  {
    ANCHOR_ASSERT(n < BITCOUNT);
    AnchorBitArraySetBit(Storage, n);
  }
  void ClearBit(int n)
  {
    ANCHOR_ASSERT(n < BITCOUNT);
    AnchorBitArrayClearBit(Storage, n);
  }
  /**
   * Works on range [n..n2) */
  void SetBitRange(int n, int n2)
  {
    AnchorBitArraySetBitRange(Storage, n, n2);
  }
};

/**
 * Helper: AnchorBitVector
 * Store 1-bit per value. */
struct ANCHOR_API AnchorBitVector
{
  AnchorVector<AnchorU32> Storage;
  void Create(int sz)
  {
    Storage.resize((sz + 31) >> 5);
    memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0]));
  }
  void Clear()
  {
    Storage.clear();
  }
  bool TestBit(int n) const
  {
    ANCHOR_ASSERT(n < (Storage.Size << 5));
    return AnchorBitArrayTestBit(Storage.Data, n);
  }
  void SetBit(int n)
  {
    ANCHOR_ASSERT(n < (Storage.Size << 5));
    AnchorBitArraySetBit(Storage.Data, n);
  }
  void ClearBit(int n)
  {
    ANCHOR_ASSERT(n < (Storage.Size << 5));
    AnchorBitArrayClearBit(Storage.Data, n);
  }
};

/**
 * Helper: AnchorSpan<>
 * Pointing to a span
 * of data we don't
 * own. */
template<typename T> struct AnchorSpan
{
  T *Data;
  T *DataEnd;

  /**
   * Constructors, destructor */
  inline AnchorSpan()
  {
    Data = DataEnd = NULL;
  }
  inline AnchorSpan(T *data, int size)
  {
    Data = data;
    DataEnd = data + size;
  }
  inline AnchorSpan(T *data, T *data_end)
  {
    Data = data;
    DataEnd = data_end;
  }

  inline void set(T *data, int size)
  {
    Data = data;
    DataEnd = data + size;
  }
  inline void set(T *data, T *data_end)
  {
    Data = data;
    DataEnd = data_end;
  }
  inline int size() const
  {
    return (int)(ptrdiff_t)(DataEnd - Data);
  }
  inline int size_in_bytes() const
  {
    return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T);
  }
  inline T &operator[](int i)
  {
    T *p = Data + i;
    ANCHOR_ASSERT(p >= Data && p < DataEnd);
    return *p;
  }
  inline const T &operator[](int i) const
  {
    const T *p = Data + i;
    ANCHOR_ASSERT(p >= Data && p < DataEnd);
    return *p;
  }

  inline T *begin()
  {
    return Data;
  }
  inline const T *begin() const
  {
    return Data;
  }
  inline T *end()
  {
    return DataEnd;
  }
  inline const T *end() const
  {
    return DataEnd;
  }

  /**
   * Utilities */
  inline int index_from_ptr(const T *it) const
  {
    ANCHOR_ASSERT(it >= Data && it < DataEnd);
    const ptrdiff_t off = it - Data;
    return (int)off;
  }
};

/**
 * Helper: AnchorSpanAllocator<>
 *
 * Facilitate storing multiple chunks into a single large block (the "arena")
 * - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass it to
 * SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned ranges. */
template<int CHUNKS> struct AnchorSpanAllocator
{
  char *BasePtr;
  int CurrOff;
  int CurrIdx;
  int Offsets[CHUNKS];
  int Sizes[CHUNKS];

  AnchorSpanAllocator()
  {
    memset(this, 0, sizeof(*this));
  }
  inline void Reserve(int n, size_t sz, int a = 4)
  {
    ANCHOR_ASSERT(n == CurrIdx && n < CHUNKS);
    CurrOff = ANCHOR_MEMALIGN(CurrOff, a);
    Offsets[n] = CurrOff;
    Sizes[n] = (int)sz;
    CurrIdx++;
    CurrOff += (int)sz;
  }
  inline int GetArenaSizeInBytes()
  {
    return CurrOff;
  }
  inline void SetArenaBasePtr(void *base_ptr)
  {
    BasePtr = (char *)base_ptr;
  }
  inline void *GetSpanPtrBegin(int n)
  {
    ANCHOR_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS);
    return (void *)(BasePtr + Offsets[n]);
  }
  inline void *GetSpanPtrEnd(int n)
  {
    ANCHOR_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS);
    return (void *)(BasePtr + Offsets[n] + Sizes[n]);
  }
  template<typename T> inline void GetSpan(int n, AnchorSpan<T> *span)
  {
    span->set((T *)GetSpanPtrBegin(n), (T *)GetSpanPtrEnd(n));
  }
};

/**
 * Helper: AnchorPool<>
 *
 * Basic keyed storage for contiguous instances,
 * slow/amortized insertion, O(1) indexable, O(Log N)
 * queries by ID over a dense/hot buffer, Honor ctor,
 * dtors. Add/remove invalidate all pointers. Indexes
 * have the same lifetime as the associated object. */
typedef int AnchorPoolIdx;
template<typename T> struct ANCHOR_API AnchorPool
{
  /* Contiguous data */
  AnchorVector<T> Buf;
  /* ID->Index */
  AnchorStorage Map;
  /* Next free idx to use */
  AnchorPoolIdx FreeIdx;

  AnchorPool()
  {
    FreeIdx = 0;
  }
  ~AnchorPool()
  {
    Clear();
  }
  T *GetByKey(ANCHOR_ID key)
  {
    int idx = Map.GetInt(key, -1);
    return (idx != -1) ? &Buf[idx] : NULL;
  }
  T *GetByIndex(AnchorPoolIdx n)
  {
    return &Buf[n];
  }
  AnchorPoolIdx GetIndex(const T *p) const
  {
    ANCHOR_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size);
    return (AnchorPoolIdx)(p - Buf.Data);
  }
  T *GetOrAddByKey(ANCHOR_ID key)
  {
    int *p_idx = Map.GetIntRef(key, -1);
    if (*p_idx != -1)
      return &Buf[*p_idx];
    *p_idx = FreeIdx;
    return Add();
  }
  bool Contains(const T *p) const
  {
    return (p >= Buf.Data && p < Buf.Data + Buf.Size);
  }
  void Clear()
  {
    for (int n = 0; n < Map.Data.Size; n++) {
      int idx = Map.Data[n].val_i;
      if (idx != -1)
        Buf[idx].~T();
    }
    Map.Clear();
    Buf.clear();
    FreeIdx = 0;
  }
  T *Add()
  {
    int idx = FreeIdx;
    if (idx == Buf.Size) {
      Buf.resize(Buf.Size + 1);
      FreeIdx++;
    } else {
      FreeIdx = *(int *)&Buf[idx];
    }
    ANCHOR_PLACEMENT_NEW(&Buf[idx])
    T();
    return &Buf[idx];
  }
  void Remove(ANCHOR_ID key, const T *p)
  {
    Remove(key, GetIndex(p));
  }
  void Remove(ANCHOR_ID key, AnchorPoolIdx idx)
  {
    Buf[idx].~T();
    *(int *)&Buf[idx] = FreeIdx;
    FreeIdx = idx;
    Map.SetInt(key, -1);
  }
  void Reserve(int capacity)
  {
    Buf.reserve(capacity);
    Map.Data.reserve(capacity);
  }
  int GetSize() const
  {
    return Buf.Size;
  }
};

/**
 * Helper: AnchorChunkStream<>
 *
 * Build and iterate a contiguous stream of variable-sized structures.
 * This is used by Settings to store persistent data while reducing
 * allocation count. We store the chunk size first, and align the
 * final size on 4 bytes boundaries. The tedious/zealous amount of
 * casting is to avoid -Wcast-align warnings. */
template<typename T> struct ANCHOR_API AnchorChunkStream
{
  AnchorVector<char> Buf;

  void clear()
  {
    Buf.clear();
  }
  bool empty() const
  {
    return Buf.Size == 0;
  }
  int size() const
  {
    return Buf.Size;
  }
  T *alloc_chunk(size_t sz)
  {
    size_t HDR_SZ = 4;
    sz = ANCHOR_MEMALIGN(HDR_SZ + sz, 4u);
    int off = Buf.Size;
    Buf.resize(off + (int)sz);
    ((int *)(void *)(Buf.Data + off))[0] = (int)sz;
    return (T *)(void *)(Buf.Data + off + (int)HDR_SZ);
  }
  T *begin()
  {
    size_t HDR_SZ = 4;
    if (!Buf.Data)
      return NULL;
    return (T *)(void *)(Buf.Data + HDR_SZ);
  }
  T *next_chunk(T *p)
  {
    size_t HDR_SZ = 4;
    ANCHOR_ASSERT(p >= begin() && p < end());
    p = (T *)(void *)((char *)(void *)p + chunk_size(p));
    if (p == (T *)(void *)((char *)end() + HDR_SZ))
      return (T *)0;
    ANCHOR_ASSERT(p < end());
    return p;
  }
  int chunk_size(const T *p)
  {
    return ((const int *)p)[-1];
  }
  T *end()
  {
    return (T *)(void *)(Buf.Data + Buf.Size);
  }
  int offset_from_ptr(const T *p)
  {
    ANCHOR_ASSERT(p >= begin() && p < end());
    const ptrdiff_t off = (const char *)p - Buf.Data;
    return (int)off;
  }
  T *ptr_from_offset(int off)
  {
    ANCHOR_ASSERT(off >= 4 && off < Buf.Size);
    return (T *)(void *)(Buf.Data + off);
  }
  void swap(AnchorChunkStream<T> &rhs)
  {
    rhs.Buf.swap(Buf);
  }
};

/**
 * AnchorDrawList: Helper function to calculate a circle's\
 * segment count given its radius and a "maximum error" value.
 *
 * Estimation of number of circle segment based on error is then
 * derived using method described in the following URL posted on:
 * https://stackoverflow.com/a/2244088/15194693 Number of segments
 * (N) is calculated using the following equation, given the form
 * N = ceil ( pi / acos(1 - error / r) ) where r > 0, error <= r
 *
 * Our equation is significantly simpler that one in the post thanks
 * for choosing segment that is perpendicular to X axis. Follow steps
 * in the article from this starting condition and you will will get
 * this same result.
 *
 * Rendering circles with an odd number of segments, while mathematically
 * correct will produce asymmetrical results on the raster grid. Therefore
 * we're rounding N to next even number (7->8, 8->8, 9->10 etc.) */
#define ANCHOR_ROUNDUP_TO_EVEN(_V) ((((_V) + 1) / 2) * 2)
#define ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN 4
#define ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX 512
#define ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD, _MAXERROR)                              \
  AnchorClamp(ANCHOR_ROUNDUP_TO_EVEN(                                                          \
                (int)ImCeil(IM_PI / AnchorAcos(1 - AnchorMin((_MAXERROR), (_RAD)) / (_RAD)))), \
              ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN,                                         \
              ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

/**
 * Raw equation from ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC
 * rewritten for 'r' and 'error'. */
#define ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N, _MAXERROR) \
  ((_MAXERROR) / (1 - AnchorCos(IM_PI / AnchorMax((float)(_N), IM_PI))))
#define ANCHOR_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N, _RAD) \
  ((1 - AnchorCos(IM_PI / AnchorMax((float)(_N), IM_PI))) / (_RAD))

/**
 * AnchorDrawList: Lookup table size for
 * adaptive arc drawing, cover full circle. */
#ifndef ANCHOR_DRAWLIST_ARCFAST_TABLE_SIZE
/**
 * Number of samples in lookup table. */
#  define ANCHOR_DRAWLIST_ARCFAST_TABLE_SIZE 48
#endif
/**
 * Sample index _PathArcToFastEx() for 360 angle. */
#define ANCHOR_DRAWLIST_ARCFAST_SAMPLE_MAX ANCHOR_DRAWLIST_ARCFAST_TABLE_SIZE

/**
 * Data shared between all AnchorDrawList instances
 * You may want to create your own instance of this
 * if you want to use AnchorDrawList completely w/o
 * ANCHOR. In that case, watch out for changes to
 * this structure in the future. */
struct ANCHOR_API AnchorDrawListSharedData
{
  /**
   * UV of white pixel in the atlas */
  wabi::GfVec2f TexUvWhitePixel;
  /**
   * Current/default font (optional,
   * for simplified AddText overload) */
  AnchorFont *Font;
  /**
   * Current/default font size (optional,
   * for simplified  AddText  overload)  */
  float FontSize;
  /**
   * Tessellation tolerance when
   * using PathBezierCurveTo()  */
  float CurveTessellationTol;
  /**
   * Number of circle segments
   * to use per pixel of radius
   * for AddCircle() etc. */
  float CircleSegmentMaxError;

  /**
   * Value for PushClipRectFullscreen() */
  wabi::GfVec4f ClipRectFullscreen;
  /**
   * Initial flags at the beginning
   * of the frame (it is possible to
   * alter flags on a per-drawlist
   * basis afterwards) */
  AnchorDrawListFlags InitialFlags;


  /**
   * [Internal] Lookup tables
   *
   * Sample points on the quarter of the circle.
   * Cutoff radius after which arc drawing will
   * fallback to slower PathArcTo() precomputed
   * segment count for given radius before  we
   * calculate it dynamically (to avoid recalc
   * overhead) UV of anti-aliased lines in the
   * atlas. */
  wabi::GfVec2f ArcFastVtx[ANCHOR_DRAWLIST_ARCFAST_TABLE_SIZE];
  float ArcFastRadiusCutoff;

  AnchorU8 CircleSegmentCounts[64];

  const wabi::GfVec4f *TexUvLines;

  AnchorDrawListSharedData();
  void SetCircleTessellationMaxError(float max_error);
};

struct AnchorDrawDataBuilder
{
  AnchorVector<AnchorDrawList *> Layers[2];  // Global layers for: regular, tooltip

  void Clear()
  {
    for (int n = 0; n < ANCHOR_ARRAYSIZE(Layers); n++)
      Layers[n].resize(0);
  }
  void ClearFreeMemory()
  {
    for (int n = 0; n < ANCHOR_ARRAYSIZE(Layers); n++)
      Layers[n].clear();
  }
  int GetDrawListCount() const
  {
    int count = 0;
    for (int n = 0; n < ANCHOR_ARRAYSIZE(Layers); n++)
      count += Layers[n].Size;
    return count;
  }
  ANCHOR_API void FlattenIntoSingleLayer();
};

/**
 * Transient per-window flags, reset at
 * the beginning of the frame. For child
 * window, inherited from parent on first
 * Begin().  This is going to be  exposed
 * in ANCHOR_api.h when stabilized enough. */
enum AnchorItemFlags_
{
  /**
   * false. */
  AnchorItemFlags_None = 0,
  AnchorItemFlags_NoTabStop = 1 << 0,

  /**
   * Button() will return true multiple
   * times based on io.KeyRepeatDelay
   * and io.KeyRepeatRate settings. */
  AnchorItemFlags_ButtonRepeat = 1 << 1,

  /**
   * false [BETA] Disable interactions
   * but doesn't affect visuals yet. */
  AnchorItemFlags_Disabled = 1 << 2,

  /**
   * false */
  AnchorItemFlags_NoNav = 1 << 3,

  /**
   * false */
  AnchorItemFlags_NoNavDefaultFocus = 1 << 4,

  /**
   * false MenuItem/Selectable()
   * automatically closes current
   * Popup window */
  AnchorItemFlags_SelectableDontClosePopup = 1 << 5,

  /**
   * false [BETA] Represent a mixed or
   * indeterminate value, generally duo
   * multi-selection where values differ.
   * Currently only supported Checkbox() */
  AnchorItemFlags_MixedValue = 1 << 6,
  //
  /**
   * false [ALPHA] Allow hovering
   * interactions but (later should
   * support all sorts of widgets)
   * underlying value not changed.*/
  AnchorItemFlags_ReadOnly = 1 << 7
};

/**
 * Flags for ItemAdd()
 *
 * FIXME-NAV: _Focusable is _ALMOST_
 * what you would expect to be called
 * '_TabStop' but SetKeyboardFocusHere()
 * works on items with no TabStop we
 * distinguish Focusable from TabStop. */
enum AnchorItemAddFlags_
{
  /**
   * FIXME-NAV: In current/legacy scheme, Focusable
   * + TabStop support are opt-in by widgets. We will
   * transition it toward being opt-out, so this flag
   * is expected to eventually disappear. */
  AnchorItemAddFlags_None = 0,
  AnchorItemAddFlags_Focusable = 1 << 0
};

enum AnchorItemStatusFlags_
{
  /**
   * Storage for LastItem data */
  AnchorItemStatusFlags_None = 0,
  /**
   * Mouse position is within item rectangle
   * (does NOT mean that the window is in the
   * correct z-order and can be hovered, this
   * is only one part of the most-common of
   * IsItemHovered test)  additionally  the
   * window->DC.LastItemDisplayRect is valid
   * Value exposed by item was edited in the
   * current frame (should match the bool
   * return value of most widgets) */
  AnchorItemStatusFlags_HoveredRect = 1 << 0,


  AnchorItemStatusFlags_HasDisplayRect = 1 << 1,
  AnchorItemStatusFlags_Edited = 1 << 2,
  /**
   * Set when Selectable(), TreeNode() reports toggling a selection. We can't report
   * "Selected", only state changes, in order to easily handle clipping with less
   * issues.  */
  AnchorItemStatusFlags_ToggledSelection = 1 << 3,
  /**
   * Set when TreeNode() report
   * toggling their open state.
   * Set if the widget/group is
   * able to provide data for the
   * StatusFlags_Deactivated flag. */
  AnchorItemStatusFlags_ToggledOpen = 1 << 4,
  AnchorItemStatusFlags_HasDeactivated = 1 << 5,

  /**
   * Only valid if StatusFlags_HasDeactivated
   * has been set. */
  AnchorItemStatusFlags_Deactivated = 1 << 6,
  AnchorItemStatusFlags_HoveredWindow = 1 << 7,

  /**
   *  Override the HoveredWindow test to
   *  allow cross-window hover testing.
   *  Set when the focusable item just
   *  got focused from code.*/
  AnchorItemStatusFlags_FocusedByCode = 1 << 8,
  /**
   * Set when the Focusable item
   * just got focused by Tabbing. */
  AnchorItemStatusFlags_FocusedByTabbing = 1 << 9,
  AnchorItemStatusFlags_Focused = AnchorItemStatusFlags_FocusedByCode |
                                  AnchorItemStatusFlags_FocusedByTabbing

#ifdef ANCHOR_ENABLE_TEST_ENGINE
  ,                                           // [ANCHOR_tests only]
  AnchorItemStatusFlags_Openable = 1 << 20,   //
  AnchorItemStatusFlags_Opened = 1 << 21,     //
  AnchorItemStatusFlags_Checkable = 1 << 22,  //
  AnchorItemStatusFlags_Checked = 1 << 23     //
#endif
};

// Extend AnchorInputTextFlags_
enum AnchorInputTextFlagsPrivate_
{
  // [Internal]
  AnchorInputTextFlags_Multiline = 1 << 26,  // For internal use by InputTextMultiline()
  AnchorInputTextFlags_NoMarkEdited =
    1 << 27,  // For internal use by functions using InputText() before reformatting data
  AnchorInputTextFlags_MergedItem =
    1 << 28  // For internal use by TempInputText(), will skip calling
             // ItemAdd(). Require bounding-box to strictly match.
};

// Extend AnchorButtonFlags_
enum AnchorButtonFlagsPrivate_
{
  AnchorButtonFlags_PressedOnClick = 1 << 4,  // return true on click (mouse down event)
  AnchorButtonFlags_PressedOnClickRelease =
    1 << 5,  // [Default] return true on click + release on same item
             // <-- this is what the majority of Button are using
  AnchorButtonFlags_PressedOnClickReleaseAnywhere =
    1 << 6,  // return true on click + release even if the release event is not done while
             // hovering the item
  AnchorButtonFlags_PressedOnRelease =
    1 << 7,  // return true on release (default requires click+release)
  AnchorButtonFlags_PressedOnDoubleClick =
    1 << 8,  // return true on double-click (default requires click+release)
  AnchorButtonFlags_PressedOnDragDropHold =
    1 << 9,  // return true when held into while we are drag and dropping another item (used by
             // e.g. tree nodes, collapsing headers)
  AnchorButtonFlags_Repeat = 1 << 10,  // hold to repeat
  AnchorButtonFlags_FlattenChildren =
    1 << 11,  // allow interactions even if a child window is overlapping
  AnchorButtonFlags_AllowItemOverlap =
    1 << 12,  // require previous frame HoveredId to either match id or be null before being
              // usable, use along with SetItemAllowOverlap()
  AnchorButtonFlags_DontClosePopups =
    1 << 13,  // disable automatically closing parent popup on press // [UNUSED]
  AnchorButtonFlags_Disabled = 1 << 14,  // disable interactions
  AnchorButtonFlags_AlignTextBaseLine =
    1 << 15,  // vertically align button to match text baseline - ButtonEx() only // FIXME:
              // Should be removed and handled by SmallButton(), not possible currently because
              // of DC.CursorPosPrevLine
  AnchorButtonFlags_NoKeyModifiers = 1
                                     << 16,  // disable mouse interaction if a key modifier is held
  AnchorButtonFlags_NoHoldingActiveId = 1 << 17,  // don't set ActiveId while holding the mouse
                                                  // (AnchorButtonFlags_PressedOnClick only)
  AnchorButtonFlags_NoNavFocus = 1 << 18,         // don't override navigation focus when activated
  AnchorButtonFlags_NoHoveredOnFocus =
    1 << 19,  // don't report as hovered when nav focus is on this item
  AnchorButtonFlags_PressedOnMask_ = AnchorButtonFlags_PressedOnClick |
                                     AnchorButtonFlags_PressedOnClickRelease |
                                     AnchorButtonFlags_PressedOnClickReleaseAnywhere |
                                     AnchorButtonFlags_PressedOnRelease |
                                     AnchorButtonFlags_PressedOnDoubleClick |
                                     AnchorButtonFlags_PressedOnDragDropHold,
  AnchorButtonFlags_PressedOnDefault_ = AnchorButtonFlags_PressedOnClickRelease
};

// Extend AnchorSliderFlags_
enum AnchorSliderFlagsPrivate_
{
  AnchorSliderFlags_Vertical = 1 << 20,  // Should this slider be orientated vertically?
  AnchorSliderFlags_ReadOnly = 1 << 21
};

// Extend AnchorSelectableFlags_
enum AnchorSelectableFlagsPrivate_
{
  // NB: need to be in sync with last value of AnchorSelectableFlags_
  AnchorSelectableFlags_NoHoldingActiveID = 1 << 20,
  AnchorSelectableFlags_SelectOnClick =
    1 << 21,  // Override button behavior to react on Click (default is Click+Release)
  AnchorSelectableFlags_SelectOnRelease =
    1 << 22,  // Override button behavior to react on Release (default is Click+Release)
  AnchorSelectableFlags_SpanAvailWidth =
    1 << 23,  // Span all avail width even if we declared less for layout purpose. FIXME: We may
              // be able to remove this (added in 6251d379, 2bcafc86 for menus)
  AnchorSelectableFlags_DrawHoveredWhenHeld =
    1 << 24,  // Always show active when held, even is not hovered. This concept could probably
              // be renamed/formalized somehow.
  AnchorSelectableFlags_SetNavIdOnHover =
    1 << 25,  // Set Nav/Focus ID on mouse hover (used by MenuItem)
  AnchorSelectableFlags_NoPadWithHalfSpacing =
    1 << 26  // Disable padding each side with ItemSpacing * 0.5f
};

// Extend AnchorTreeNodeFlags_
enum AnchorTreeNodeFlagsPrivate_
{
  AnchorTreeNodeFlags_ClipLabelForTrailingButton = 1 << 20
};

enum AnchorSeparatorFlags_
{
  AnchorSeparatorFlags_None = 0,
  AnchorSeparatorFlags_Horizontal = 1 << 0,  // Axis default to current layout type, so generally
                                             // Horizontal unless e.g. in a menu bar
  AnchorSeparatorFlags_Vertical = 1 << 1,
  AnchorSeparatorFlags_SpanAllColumns = 1 << 2
};

enum AnchorTextFlags_
{
  AnchorTextFlags_None = 0,
  AnchorTextFlags_NoWidthForLargeClippedText = 1 << 0
};

enum AnchorTooltipFlags_
{
  AnchorTooltipFlags_None = 0,
  /**
   * Override will clear/ignore
   * previously submitted tooltip
   * (defaults to append) */
  AnchorTooltipFlags_OverridePreviousTooltip = 1 << 0
};

/**
 * FIXME: this is in development,
 * not exposed/functional as a
 * generic feature yet. Horz /
 * Vert enums are fixed to 0/1
 * so they may be used to index
 * wabi::GfVec2f */
enum AnchorLayoutType_
{
  AnchorLayoutType_Horizontal = 0,
  AnchorLayoutType_Vertical = 1
};

enum ANCHORLogType
{
  ANCHORLogType_None = 0,
  ANCHORLogType_TTY,
  ANCHORLogType_File,
  ANCHORLogType_Buffer,
  ANCHORLogType_Clipboard
};

/**
 * X/Y enums are fixed to 0/1 so
 * they may be used to index
 * wabi::GfVec2f */
enum ANCHOR_Axis
{
  ANCHOR_Axis_None = -1,
  ANCHOR_Axis_X = 0,
  ANCHOR_Axis_Y = 1
};

enum ANCHORPlotType
{
  ANCHORPlotType_Lines,
  ANCHORPlotType_Histogram
};

enum ANCHORInputSource
{
  ANCHORInputSource_None = 0,
  ANCHORInputSource_Mouse,
  ANCHORInputSource_Keyboard,
  /**
   * Stored in g.ActiveIdSource only */
  ANCHORInputSource_Gamepad,
  /**
   * Currently only used by InputText() */
  ANCHORInputSource_Nav,
  ANCHORInputSource_Clipboard,
  ANCHORInputSource_COUNT
};

/**
 * FIXME-NAV: Clarify/expose
 * various repeat delay/rate */
enum ANCHOR_InputReadMode
{
  ANCHOR_InputReadMode_Down,
  ANCHOR_InputReadMode_Pressed,
  ANCHOR_InputReadMode_Released,
  ANCHOR_InputReadMode_Repeat,
  ANCHOR_InputReadMode_RepeatSlow,
  ANCHOR_InputReadMode_RepeatFast
};

enum AnchorNavHighlightFlags_
{
  AnchorNavHighlightFlags_None = 0,
  AnchorNavHighlightFlags_TypeDefault = 1 << 0,
  AnchorNavHighlightFlags_TypeThin = 1 << 1,
  /**
   * Draw rectangular highlight if
   * (g.NavId == id) _even_ when
   * using the mouse. */
  AnchorNavHighlightFlags_AlwaysDraw = 1 << 2,
  AnchorNavHighlightFlags_NoRounding = 1 << 3
};

enum AnchorNavDirSourceFlags_
{
  AnchorNavDirSourceFlags_None = 0,
  AnchorNavDirSourceFlags_Keyboard = 1 << 0,
  AnchorNavDirSourceFlags_PadDPad = 1 << 1,
  AnchorNavDirSourceFlags_PadLStick = 1 << 2
};

enum AnchorNavMoveFlags_
{
  AnchorNavMoveFlags_None = 0,
  AnchorNavMoveFlags_LoopX = 1 << 0,  // On failed request, restart from opposite side
  AnchorNavMoveFlags_LoopY = 1 << 1,
  AnchorNavMoveFlags_WrapX =
    1 << 2,  // On failed request, request from opposite side one line down (when
             // NavDir==right) or one line up (when NavDir==left)
  AnchorNavMoveFlags_WrapY = 1 << 3,  // This is not super useful for provided for completeness
  AnchorNavMoveFlags_AllowCurrentNavId =
    1 << 4,  // Allow scoring and considering the current NavId as a move target candidate. This
             // is used when the move source is offset (e.g. pressing PageDown actually needs to
             // send a Up move request, if we are pressing PageDown from the bottom-most item we
             // need to stay in place)
  AnchorNavMoveFlags_AlsoScoreVisibleSet =
    1 << 5,  // Store alternate result in NavMoveResultLocalVisibleSet that only comprise
             // elements that are already fully visible.
  AnchorNavMoveFlags_ScrollToEdge = 1 << 6
};

enum ANCHORNavForward
{
  ANCHORNavForward_None,
  ANCHORNavForward_ForwardQueued,
  ANCHORNavForward_ForwardActive
};

enum ANCHORNavLayer
{
  ANCHORNavLayer_Main = 0,  // Main scrolling layer
  ANCHORNavLayer_Menu = 1,  // Menu layer (access with Alt/AnchorNavInput_Menu)
  ANCHORNavLayer_COUNT
};

enum ANCHORPopupPositionPolicy
{
  ANCHORPopupPositionPolicy_Default,
  ANCHORPopupPositionPolicy_ComboBox,
  ANCHORPopupPositionPolicy_Tooltip
};

struct AnchorDataTypeTempStorage
{
  AnchorU8 Data[8];  // Can fit any data up to AnchorDataType_COUNT
};

// Type information associated to one AnchorDataType. Retrieve with DataTypeGetInfo().
struct AnchorDataTypeInfo
{
  size_t Size;           // Size in bytes
  const char *Name;      // Short descriptive name for the type, for debugging
  const char *PrintFmt;  // Default printf format for the type
  const char *ScanFmt;   // Default scanf format for the type
};

// Extend AnchorDataType_
enum AnchorDataTypePrivate_
{
  AnchorDataType_String = AnchorDataType_COUNT + 1,
  AnchorDataType_Pointer,
  AnchorDataType_ID
};

// Stacked color modifier, backup of modified data so we can restore it
struct AnchorColorMod
{
  AnchorCol Col;
  wabi::GfVec4f BackupValue;
};

// Stacked style modifier, backup of modified data so we can restore it. Data type inferred from
// the variable.
struct AnchorStyleMod
{
  AnchorStyleVar VarIdx;
  union
  {
    int BackupInt[2];
    float BackupFloat[2];
  };
  AnchorStyleMod(AnchorStyleVar idx, int v)
  {
    VarIdx = idx;
    BackupInt[0] = v;
  }
  AnchorStyleMod(AnchorStyleVar idx, float v)
  {
    VarIdx = idx;
    BackupFloat[0] = v;
  }
  AnchorStyleMod(AnchorStyleVar idx, wabi::GfVec2f v)
  {
    VarIdx = idx;
    BackupFloat[0] = v[0];
    BackupFloat[1] = v[1];
  }
};

// Stacked storage data for BeginGroup()/EndGroup()
struct ANCHOR_API AnchorGroupData
{
  ANCHOR_ID WindowID;
  wabi::GfVec2f BackupCursorPos;
  wabi::GfVec2f BackupCursorMaxPos;
  GfVec1 BackupIndent;
  GfVec1 BackupGroupOffset;
  wabi::GfVec2f BackupCurrLineSize;
  float BackupCurrLineTextBaseOffset;
  ANCHOR_ID BackupActiveIdIsAlive;
  bool BackupActiveIdPreviousFrameIsAlive;
  bool BackupHoveredIdIsAlive;
  bool EmitItem;
};

// Simple column measurement, currently used for MenuItem() only.. This is very
// short-sighted/throw-away code and NOT a generic helper.
struct ANCHOR_API AnchorMenuColumns
{
  float Spacing;
  float Width, NextWidth;
  float Pos[3], NextWidths[3];

  AnchorMenuColumns()
  {
    memset(this, 0, sizeof(*this));
  }
  void Update(int count, float spacing, bool clear);
  float DeclColumns(float w0, float w1, float w2);
  float CalcExtraSpace(float avail_w) const;
};

// Internal state of the currently focused/edited text input box
// For a given item ID, access with ANCHOR::GetInputTextState()
struct ANCHOR_API AnchorInputTextState
{
  ANCHOR_ID ID;          // widget id owning the text state
  int CurLenW, CurLenA;  // we need to maintain our buffer length in both UTF-8 and wchar format.
                         // UTF-8 length is valid even if TextA is not.
  AnchorVector<AnchorWChar>
    TextW;                   // edit buffer, we need to persist but can't guarantee the persistence
                             // of the user-provided buffer. so we copy into own buffer.
  AnchorVector<char> TextA;  // temporary UTF8 buffer for callbacks and other operations. this is
                             // not updated in every code-path! size=capacity.
  AnchorVector<char>
    InitialTextA;     // backup of end-user buffer at the time of focus (in UTF-8, unaltered)
  bool TextAIsValid;  // temporary UTF8 buffer is not initially valid before we make the widget
                      // active (until then we pull the data from user argument)
  int BufCapacityA;   // end-user buffer capacity
  float ScrollX;      // horizontal scrolling/offset
  AnchorStb::STB_TexteditState Stb;  // state for stb_textedit.h
  float CursorAnim;   // timer for cursor blink, reset on every user action so the cursor reappears
                      // immediately
  bool CursorFollow;  // set when we want scrolling to follow the current cursor position (not
                      // always!)
  bool SelectedAllMouseLock;   // after a double-click to select all, we ignore further mouse drags
                               // to update selection
  bool Edited;                 // edited this frame
  AnchorInputTextFlags Flags;  // copy of InputText() flags
  ANCHORInputTextCallback UserCallback;  // "
  void *UserCallbackData;                // "

  AnchorInputTextState()
  {
    memset(this, 0, sizeof(*this));
  }
  void ClearText()
  {
    CurLenW = CurLenA = 0;
    TextW[0] = 0;
    TextA[0] = 0;
    CursorClamp();
  }
  void ClearFreeMemory()
  {
    TextW.clear();
    TextA.clear();
    InitialTextA.clear();
  }
  int GetUndoAvailCount() const
  {
    return Stb.undostate.undo_point;
  }
  int GetRedoAvailCount() const
  {
    return STB_TEXTEDIT_UNDOSTATECOUNT - Stb.undostate.redo_point;
  }
  void OnKeyPressed(
    int key);  // Cannot be inline because we call in code in stb_textedit.h implementation

  // Cursor & Selection
  void CursorAnimReset()
  {
    CursorAnim = -0.30f;
  }  // After a user-input the cursor stays on for a while without blinking
  void CursorClamp()
  {
    Stb.cursor = AnchorMin(Stb.cursor, CurLenW);
    Stb.select_start = AnchorMin(Stb.select_start, CurLenW);
    Stb.select_end = AnchorMin(Stb.select_end, CurLenW);
  }
  bool HasSelection() const
  {
    return Stb.select_start != Stb.select_end;
  }
  void ClearSelection()
  {
    Stb.select_start = Stb.select_end = Stb.cursor;
  }
  void SelectAll()
  {
    Stb.select_start = 0;
    Stb.cursor = Stb.select_end = CurLenW;
    Stb.has_preferred_x = 0;
  }
};

// Storage for current popup stack
struct AnchorPopupData
{
  ANCHOR_ID PopupId;  // Set on OpenPopup()
  AnchorWindow
    *Window;  // Resolved on BeginPopup() - may stay unresolved if user never calls OpenPopup()
  AnchorWindow
    *SourceWindow;         // Set on OpenPopup() copy of NavWindow at the time of opening the popup
  int OpenFrameCount;      // Set on OpenPopup()
  ANCHOR_ID OpenParentId;  // Set on OpenPopup(), we need this to differentiate multiple menu sets
                           // from each others (e.g. inside menu bar vs loose menu items)
  wabi::GfVec2f OpenPopupPos;  // Set on OpenPopup(), preferred popup position (typically ==
                               // OpenMousePos when using mouse)
  wabi::GfVec2f
    OpenMousePos;  // Set on OpenPopup(), copy of mouse position at the time of opening popup

  AnchorPopupData()
  {
    memset(this, 0, sizeof(*this));
    OpenFrameCount = -1;
  }
};

struct AnchorNavItemData
{
  AnchorWindow *Window;    // Init,Move    // Best candidate window
                           // (result->ItemWindow->RootWindowForNav == request->Window)
  ANCHOR_ID ID;            // Init,Move    // Best candidate item ID
  ANCHOR_ID FocusScopeId;  // Init,Move    // Best candidate focus scope ID
  AnchorBBox RectRel;      // Init,Move    // Best candidate bounding box in window relative space
  float DistBox;           //      Move    // Best candidate box distance to current NavId
  float DistCenter;        //      Move    // Best candidate center distance to current NavId
  float DistAxial;         //      Move    // Best candidate axial distance to current NavId

  AnchorNavItemData()
  {
    Clear();
  }
  void Clear()
  {
    Window = NULL;
    ID = FocusScopeId = 0;
    RectRel = AnchorBBox();
    DistBox = DistCenter = DistAxial = FLT_MAX;
  }
};

enum AnchorNextWindowDataFlags_
{
  AnchorNextWindowDataFlags_None = 0,
  AnchorNextWindowDataFlags_HasPos = 1 << 0,
  AnchorNextWindowDataFlags_HasSize = 1 << 1,
  AnchorNextWindowDataFlags_HasContentSize = 1 << 2,
  AnchorNextWindowDataFlags_HasCollapsed = 1 << 3,
  AnchorNextWindowDataFlags_HasSizeConstraint = 1 << 4,
  AnchorNextWindowDataFlags_HasFocus = 1 << 5,
  AnchorNextWindowDataFlags_HasBgAlpha = 1 << 6,
  AnchorNextWindowDataFlags_HasScroll = 1 << 7
};

// Storage for SetNexWindow** functions
struct AnchorNextWindowData
{
  AnchorNextWindowDataFlags Flags;
  AnchorCond PosCond;
  AnchorCond SizeCond;
  AnchorCond CollapsedCond;
  wabi::GfVec2f PosVal;
  wabi::GfVec2f PosPivotVal;
  wabi::GfVec2f SizeVal;
  wabi::GfVec2f ContentSizeVal;
  wabi::GfVec2f ScrollVal;
  bool CollapsedVal;
  AnchorBBox SizeConstraintRect;
  ANCHORSizeCallback SizeCallback;
  void *SizeCallbackUserData;
  float BgAlphaVal;  // Override background alpha
  wabi::GfVec2f
    MenuBarOffsetMinVal;  // *Always on* This is not exposed publicly, so we don't clear it.

  AnchorNextWindowData()
  {
    memset(this, 0, sizeof(*this));
  }
  inline void ClearFlags()
  {
    Flags = AnchorNextWindowDataFlags_None;
  }
};

enum AnchorNextItemDataFlags_
{
  AnchorNextItemDataFlags_None = 0,
  AnchorNextItemDataFlags_HasWidth = 1 << 0,
  AnchorNextItemDataFlags_HasOpen = 1 << 1
};

struct AnchorNextItemData
{
  AnchorNextItemDataFlags Flags;
  float Width;  // Set by SetNextItemWidth()
  ANCHOR_ID
  FocusScopeId;  // Set by SetNextItemMultiSelectData() (!= 0 signify value has been set, so
                 // it's an alternate version of HasSelectionData, we don't use Flags for this
                 // because they are cleared too early. This is mostly used for debugging)
  AnchorCond OpenCond;
  bool OpenVal;  // Set by SetNextItemOpen()

  AnchorNextItemData()
  {
    memset(this, 0, sizeof(*this));
  }
  inline void ClearFlags()
  {
    Flags = AnchorNextItemDataFlags_None;
  }  // Also cleared manually by ItemAdd()!
};

struct ANCHOR_ShrinkWidthItem
{
  int Index;
  float Width;
};

struct ANCHOR_PtrOrIndex
{
  void *Ptr;  // Either field can be set, not both. e.g. Dock node tab bars are loose while
              // BeginTabBar() ones are in a pool.
  int Index;  // Usually index in a main pool.

  ANCHOR_PtrOrIndex(void *ptr)
  {
    Ptr = ptr;
    Index = -1;
  }
  ANCHOR_PtrOrIndex(int index)
  {
    Ptr = NULL;
    Index = index;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Columns support
//-----------------------------------------------------------------------------

// Flags for internal's BeginColumns(). Prefix using BeginTable() nowadays!
enum AnchorOldColumnFlags_
{
  AnchorOldColumnFlags_None = 0,
  AnchorOldColumnFlags_NoBorder = 1 << 0,  // Disable column dividers
  AnchorOldColumnFlags_NoResize = 1
                                  << 1,  // Disable resizing columns when clicking on the dividers
  AnchorOldColumnFlags_NoPreserveWidths =
    1 << 2,  // Disable column width preservation when adjusting columns
  AnchorOldColumnFlags_NoForceWithinWindow = 1
                                             << 3,  // Disable forcing columns to fit within window
  AnchorOldColumnFlags_GrowParentContentsSize =
    1 << 4  // (WIP) Restore pre-1.51 behavior of extending the parent window contents size but
            // _without affecting the columns width at all_. Will eventually remove.

// Obsolete names (will be removed)
#ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  ,
  AnchorColumnsFlags_None = AnchorOldColumnFlags_None,
  AnchorColumnsFlags_NoBorder = AnchorOldColumnFlags_NoBorder,
  AnchorColumnsFlags_NoResize = AnchorOldColumnFlags_NoResize,
  AnchorColumnsFlags_NoPreserveWidths = AnchorOldColumnFlags_NoPreserveWidths,
  AnchorColumnsFlags_NoForceWithinWindow = AnchorOldColumnFlags_NoForceWithinWindow,
  AnchorColumnsFlags_GrowParentContentsSize = AnchorOldColumnFlags_GrowParentContentsSize
#endif
};

struct AnchorOldColumnData
{
  float OffsetNorm;  // Column start offset, normalized 0.0 (far left) -> 1.0 (far right)
  float OffsetNormBeforeResize;
  AnchorOldColumnFlags Flags;  // Not exposed
  AnchorBBox ClipRect;

  AnchorOldColumnData()
  {
    memset(this, 0, sizeof(*this));
  }
};

struct AnchorOldColumns
{
  ANCHOR_ID ID;
  AnchorOldColumnFlags Flags;
  bool IsFirstFrame;
  bool IsBeingResized;
  int Current;
  int Count;
  float OffMinX, OffMaxX;  // Offsets from HostWorkRect.Min[0]
  float LineMinY, LineMaxY;
  float HostCursorPosY;                 // Backup of CursorPos at the time of BeginColumns()
  float HostCursorMaxPosX;              // Backup of CursorMaxPos at the time of BeginColumns()
  AnchorBBox HostInitialClipRect;       // Backup of ClipRect at the time of BeginColumns()
  AnchorBBox HostBackupClipRect;        // Backup of ClipRect during
                                        // PushColumnsBackground()/PopColumnsBackground()
  AnchorBBox HostBackupParentWorkRect;  // Backup of WorkRect at the time of BeginColumns()
  AnchorVector<AnchorOldColumnData> Columns;
  AnchorDrawListSplitter Splitter;

  AnchorOldColumns()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-select support
//-----------------------------------------------------------------------------

#ifdef ANCHOR_HAS_MULTI_SELECT
// <this is filled in 'range_select' branch>
#endif  // #ifdef ANCHOR_HAS_MULTI_SELECT

//-----------------------------------------------------------------------------
// [SECTION] Docking support
//-----------------------------------------------------------------------------

#ifdef ANCHOR_HAS_DOCK
// <this is filled in 'docking' branch>
#endif  // #ifdef ANCHOR_HAS_DOCK

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// AnchorViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of AnchorViewport is in fact a AnchorViewportP.
struct AnchorViewportP : public AnchorViewport
{
  int DrawListsLastFrame[2];  // Last frame number the background (0) and foreground (1) draw lists
                              // were used
  AnchorDrawList *DrawLists[2];  // Convenience background (0) and foreground (1) draw lists. We
                                 // use them to draw software mouser cursor when io.MouseDrawCursor
                                 // is set and to draw most debug overlays.
  AnchorDrawData DrawDataP;
  AnchorDrawDataBuilder DrawDataBuilder;

  wabi::GfVec2f
    WorkOffsetMin;  // Work Area: Offset from Pos to top-left corner of Work Area. Generally
                    // (0,0) or (0,+main_menu_bar_height). Work Area is Full Area but without
                    // menu-bars/status-bars (so WorkArea always fit inside Pos/Size!)
  wabi::GfVec2f WorkOffsetMax;  // Work Area: Offset from Pos+Size to bottom-right corner of Work
                                // Area. Generally (0,0) or (0,-status_bar_height).
  wabi::GfVec2f BuildWorkOffsetMin;  // Work Area: Offset being built during current frame.
                                     // Generally >= 0.0f.
  wabi::GfVec2f BuildWorkOffsetMax;  // Work Area: Offset being built during current frame.
                                     // Generally <= 0.0f.

  AnchorViewportP()
  {
    DrawListsLastFrame[0] = DrawListsLastFrame[1] = -1;
    DrawLists[0] = DrawLists[1] = NULL;
  }
  ~AnchorViewportP()
  {
    if (DrawLists[0])
      ANCHOR_DELETE(DrawLists[0]);
    if (DrawLists[1])
      ANCHOR_DELETE(DrawLists[1]);
  }

  // Calculate work rect pos/size given a set of offset (we have 1 pair of offset for rect locked
  // from last frame data, and 1 pair for currently building rect)
  wabi::GfVec2f CalcWorkRectPos(const wabi::GfVec2f &off_min) const
  {
    return wabi::GfVec2f(Pos[0] + off_min[0], Pos[1] + off_min[1]);
  }
  wabi::GfVec2f CalcWorkRectSize(const wabi::GfVec2f &off_min, const wabi::GfVec2f &off_max) const
  {
    return wabi::GfVec2f(AnchorMax(0.0f, Size[0] - off_min[0] + off_max[0]),
                         AnchorMax(0.0f, Size[1] - off_min[1] + off_max[1]));
  }
  void UpdateWorkRect()
  {
    WorkPos = CalcWorkRectPos(WorkOffsetMin);
    WorkSize = CalcWorkRectSize(WorkOffsetMin, WorkOffsetMax);
  }  // Update public fields

  // Helpers to retrieve AnchorBBox (we don't need to store BuildWorkRect as every access tend to
  // change it, hence the code asymmetry)
  AnchorBBox GetMainRect() const
  {
    return AnchorBBox(Pos[0], Pos[1], Pos[0] + Size[0], Pos[1] + Size[1]);
  }
  AnchorBBox GetWorkRect() const
  {
    return AnchorBBox(WorkPos[0], WorkPos[1], WorkPos[0] + WorkSize[0], WorkPos[1] + WorkSize[1]);
  }
  AnchorBBox GetBuildWorkRect() const
  {
    wabi::GfVec2f pos = CalcWorkRectPos(BuildWorkOffsetMin);
    wabi::GfVec2f size = CalcWorkRectSize(BuildWorkOffsetMin, BuildWorkOffsetMax);
    return AnchorBBox(pos[0], pos[1], pos[0] + size[0], pos[1] + size[1]);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in ANCHOR.ini file
// Because we never destroy or rename AnchorWindowSettings, we can store the names in a separate
// buffer easily. (this is designed to be stored in a AnchorChunkStream buffer, with the
// variable-length Name following our structure)
struct AnchorWindowSettings
{
  ANCHOR_ID ID;
  wabi::GfVec2h Pos;
  wabi::GfVec2h Size;
  bool Collapsed;
  bool WantApply;  // Set when loaded from .ini data (to enable merging/loading .ini data into an
                   // already running context)

  AnchorWindowSettings()
  {
    memset(this, 0, sizeof(*this));
  }
  char *GetName()
  {
    return (char *)(this + 1);
  }
};

struct AnchorSettingsHandler
{
  const char *TypeName;  // Short description stored in .ini file. Disallowed characters: '[' ']'
  ANCHOR_ID TypeHash;    // == AnchorHashStr(TypeName)
  void (*ClearAllFn)(AnchorContext *ctx,
                     AnchorSettingsHandler *handler);  // Clear all settings data
  void (*ReadInitFn)(
    AnchorContext *ctx,
    AnchorSettingsHandler *handler);  // Read: Called before reading (in registration order)
  void *(*ReadOpenFn)(
    AnchorContext *ctx,
    AnchorSettingsHandler *handler,
    const char *name);  // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
  void (*ReadLineFn)(AnchorContext *ctx,
                     AnchorSettingsHandler *handler,
                     void *entry,
                     const char *line);  // Read: Called for every line of text within an ini entry
  void (*ApplyAllFn)(
    AnchorContext *ctx,
    AnchorSettingsHandler *handler);  // Read: Called after reading (in registration order)
  void (*WriteAllFn)(AnchorContext *ctx,
                     AnchorSettingsHandler *handler,
                     AnchorTextBuffer *out_buf);  // Write: Output every entries into 'out_buf'
  void *UserData;

  AnchorSettingsHandler()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug
//-----------------------------------------------------------------------------

struct AnchorMetricsConfig
{
  bool ShowWindowsRects;
  bool ShowWindowsBeginOrder;
  bool ShowTablesRects;
  bool ShowDrawCmdMesh;
  bool ShowDrawCmdBoundingBoxes;
  int ShowWindowsRectsType;
  int ShowTablesRectsType;

  AnchorMetricsConfig()
  {
    ShowWindowsRects = false;
    ShowWindowsBeginOrder = false;
    ShowTablesRects = false;
    ShowDrawCmdMesh = true;
    ShowDrawCmdBoundingBoxes = true;
    ShowWindowsRectsType = -1;
    ShowTablesRectsType = -1;
  }
};

struct ANCHOR_API AnchorStackSizes
{
  short SizeOfIDStack;
  short SizeOfColorStack;
  short SizeOfStyleVarStack;
  short SizeOfFontStack;
  short SizeOfFocusScopeStack;
  short SizeOfGroupStack;
  short SizeOfBeginPopupStack;

  AnchorStackSizes()
  {
    memset(this, 0, sizeof(*this));
  }
  void SetToCurrentState();
  void CompareWithCurrentState();
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*AnchorContextHookCallback)(AnchorContext *ctx, AnchorContextHook *hook);
enum AnchorContextHookType
{
  AnchorContextHookType_NewFramePre,
  AnchorContextHookType_NewFramePost,
  AnchorContextHookType_EndFramePre,
  AnchorContextHookType_EndFramePost,
  AnchorContextHookType_RenderPre,
  AnchorContextHookType_RenderPost,
  AnchorContextHookType_Shutdown,
  AnchorContextHookType_PendingRemoval_
};

struct AnchorContextHook
{
  ANCHOR_ID HookId;  // A unique ID assigned by AddContextHook()
  AnchorContextHookType Type;
  ANCHOR_ID Owner;
  AnchorContextHookCallback Callback;
  void *UserData;

  AnchorContextHook()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] AnchorContext (main ANCHOR context)
//-----------------------------------------------------------------------------

struct AnchorContext
{
  bool Initialized;
  bool FontAtlasOwnedByContext;  // IO.Fonts-> is owned by the AnchorContext and will be
                                 // destructed along with it.

  /**
   * Pixar Hydra Driver.
   * Points to the Rendering Device and is
   * responsible for instancing any number
   * of Hydra Engines -- all using the same
   * underlying & shared Graphics Resources. */
  wabi::HdDriver HydraDriver;
  wabi::UsdImagingGLEngineSharedPtr GLEngine;

  AnchorIO IO;
  AnchorStyle Style;
  AnchorFont *Font;    // (Shortcut) == FontStack.empty() ? IO.Font : FontStack.back()
  float FontSize;      // (Shortcut) == FontBaseSize * g.CurrentWindow->FontWindowScale ==
                       // window->FontSize(). Text height for current window.
  float FontBaseSize;  // (Shortcut) == IO.FontGlobalScale * Font->Scale * Font->FontSize. Base
                       // text height.
  AnchorDrawListSharedData DrawListSharedData;
  double Time;
  int FrameCount;
  int FrameCountEnded;
  int FrameCountRendered;
  bool WithinFrameScope;                    // Set by NewFrame(), cleared by EndFrame()
  bool WithinFrameScopeWithImplicitWindow;  // Set by NewFrame(), cleared by EndFrame() when the
                                            // implicit debug window has been pushed
  bool WithinEndChild;                      // Set within EndChild()
  bool GcCompactAll;                        // Request full GC
  bool TestEngineHookItems;        // Will call test engine hooks: ANCHORTestEngineHook_ItemAdd(),
                                   // ANCHORTestEngineHook_ItemInfo(), ANCHORTestEngineHook_Log()
  ANCHOR_ID TestEngineHookIdInfo;  // Will call test engine hooks: ANCHORTestEngineHook_IdInfo()
                                   // from GetID()
  void *TestEngine;                // Test engine user data

  // Windows state
  AnchorVector<AnchorWindow *> Windows;  // Windows, sorted in display order, back to front
  AnchorVector<AnchorWindow *>
    WindowsFocusOrder;  // Root windows, sorted in focus order, back to front.
  AnchorVector<AnchorWindow *>
    WindowsTempSortBuffer;  // Temporary buffer used in EndFrame() to reorder
                            // windows so parents are kept before their child
  AnchorVector<AnchorWindow *> CurrentWindowStack;
  AnchorStorage WindowsById;          // Map window's ANCHOR_ID to AnchorWindow*
  int WindowsActiveCount;             // Number of unique windows submitted by frame
  wabi::GfVec2f WindowsHoverPadding;  // Padding around resizable windows for which hovering on
                                      // counts as hovering the window ==
                                      // AnchorMax(style.TouchExtraPadding, WINDOWS_HOVER_PADDING)
  AnchorWindow *CurrentWindow;        // Window being drawn into
  AnchorWindow *HoveredWindow;  // Window the mouse is hovering. Will typically catch mouse inputs.
  AnchorWindow *HoveredWindowUnderMovingWindow;  // Hovered window ignoring MovingWindow. Only set
                                                 // if MovingWindow is set.
  AnchorWindow
    *MovingWindow;  // Track the window we clicked on (in order to preserve focus). The actual
                    // window that is moved is generally MovingWindow->RootWindow.
  AnchorWindow
    *WheelingWindow;  // Track the window we started mouse-wheeling on. Until a timer elapse or
                      // mouse has moved, generally keep scrolling the same window even if during
                      // the course of scrolling the mouse ends up hovering a child window.
  wabi::GfVec2f WheelingWindowRefMousePos;
  float WheelingWindowTimer;

  // Item/widgets state and tracking information
  AnchorItemFlags CurrentItemFlags;  // == g.ItemFlagsStack.back()
  ANCHOR_ID HoveredId;               // Hovered widget, filled during the frame
  ANCHOR_ID HoveredIdPreviousFrame;
  bool HoveredIdAllowOverlap;
  bool HoveredIdUsingMouseWheel;  // Hovered widget will use mouse wheel. Blocks scrolling the
                                  // underlying window.
  bool HoveredIdPreviousFrameUsingMouseWheel;
  bool HoveredIdDisabled;  // At least one widget passed the rect test, but has been discarded by
                           // disabled flag or popup inhibit. May be true even if HoveredId == 0.
  float HoveredIdTimer;    // Measure contiguous hovering time
  float HoveredIdNotActiveTimer;  // Measure contiguous hovering time where the item has not been
                                  // active
  ANCHOR_ID ActiveId;             // Active widget
  ANCHOR_ID ActiveIdIsAlive;  // Active widget has been seen this frame (we can't use a bool as the
                              // ActiveId may change within the frame)
  float ActiveIdTimer;
  bool ActiveIdIsJustActivated;  // Set at the time of activation for one frame
  bool ActiveIdAllowOverlap;  // Active widget allows another widget to steal active id (generally
                              // for overlapping widgets, but not always)
  bool ActiveIdNoClearOnFocusLoss;    // Disable losing active id if the active id window gets
                                      // unfocused.
  bool ActiveIdHasBeenPressedBefore;  // Track whether the active id led to a press (this is to
                                      // allow changing between PressOnClick and PressOnRelease
                                      // without pressing twice). Used by range_select branch.
  bool ActiveIdHasBeenEditedBefore;   // Was the value associated to the widget Edited over the
                                      // course of the Active state.
  bool ActiveIdHasBeenEditedThisFrame;
  bool ActiveIdUsingMouseWheel;  // Active widget will want to read mouse wheel. Blocks scrolling
                                 // the underlying window.
  AnchorU32 ActiveIdUsingNavDirMask;    // Active widget will want to read those nav move requests
                                        // (e.g. can activate a button and move away from it)
  AnchorU32 ActiveIdUsingNavInputMask;  // Active widget will want to read those nav inputs.
  AnchorU64 ActiveIdUsingKeyInputMask;  // Active widget will want to read those key inputs. When
                                        // we grow the AnchorKey enum we'll need to either to
                                        // order the enum to make useful keys come first, either
                                        // redesign this into e.g. a small array.
  wabi::GfVec2f ActiveIdClickOffset;    // Clicked offset from upper-left corner, if applicable
                                        // (currently only set by ButtonBehavior)
  AnchorWindow *ActiveIdWindow;
  ANCHORInputSource ActiveIdSource;  // Activating with mouse or nav (gamepad/keyboard)
  int ActiveIdMouseButton;
  ANCHOR_ID ActiveIdPreviousFrame;
  bool ActiveIdPreviousFrameIsAlive;
  bool ActiveIdPreviousFrameHasBeenEditedBefore;
  AnchorWindow *ActiveIdPreviousFrameWindow;
  ANCHOR_ID LastActiveId;   // Store the last non-zero ActiveId, useful for animation.
  float LastActiveIdTimer;  // Store the last non-zero ActiveId timer since the beginning of
                            // activation, useful for animation.

  // Next window/item data
  AnchorNextWindowData NextWindowData;  // Storage for SetNextWindow** functions
  AnchorNextItemData NextItemData;      // Storage for SetNextItem** functions

  // Shared stacks
  AnchorVector<AnchorColorMod>
    ColorStack;  // Stack for PushStyleColor()/PopStyleColor() - inherited by Begin()
  AnchorVector<AnchorStyleMod>
    StyleVarStack;  // Stack for PushStyleVar()/PopStyleVar() - inherited by Begin()
  AnchorVector<AnchorFont *> FontStack;  // Stack for PushFont()/PopFont() - inherited by Begin()
  AnchorVector<ANCHOR_ID> FocusScopeStack;  // Stack for PushFocusScope()/PopFocusScope() - not
                                            // inherited by Begin(), unless child window
  AnchorVector<AnchorItemFlags>
    ItemFlagsStack;  // Stack for PushItemFlag()/PopItemFlag() - inherited by Begin()
  AnchorVector<AnchorGroupData>
    GroupStack;  // Stack for BeginGroup()/EndGroup() - not inherited by Begin()
  AnchorVector<AnchorPopupData> OpenPopupStack;  // Which popups are open (persistent)
  AnchorVector<AnchorPopupData>
    BeginPopupStack;  // Which level of BeginPopup() we are in (reset every frame)

  // Viewports
  AnchorVector<AnchorViewportP *> Viewports;  // Active viewports (Size==1 in 'master' branch).
                                              // Each viewports hold their copy of AnchorDrawData.

  // Gamepad/keyboard Navigation
  AnchorWindow *NavWindow;    // Focused window for navigation. Could be called 'FocusWindow'
  ANCHOR_ID NavId;            // Focused item for navigation
  ANCHOR_ID NavFocusScopeId;  // Identify a selection scope (selection code often wants to "clear
                              // other items" when landing on an item of the selection set)
  ANCHOR_ID NavActivateId;    // ~~ (g.ActiveId == 0) && IsNavInputPressed(AnchorNavInput_Activate)
                              // ? NavId : 0, also set when calling ActivateItem()
  ANCHOR_ID NavActivateDownId;     // ~~ IsNavInputDown(AnchorNavInput_Activate) ? NavId : 0
  ANCHOR_ID NavActivatePressedId;  // ~~ IsNavInputPressed(AnchorNavInput_Activate) ? NavId : 0
  ANCHOR_ID NavInputId;            // ~~ IsNavInputPressed(AnchorNavInput_Input) ? NavId : 0
  ANCHOR_ID NavJustTabbedId;       // Just tabbed to this id.
  ANCHOR_ID NavJustMovedToId;  // Just navigated to this id (result of a successfully MoveRequest).
  ANCHOR_ID NavJustMovedToFocusScopeId;  // Just navigated to this focus scope id (result of a
                                         // successfully MoveRequest).
  AnchorKeyModFlags NavJustMovedToKeyMods;
  ANCHOR_ID NavNextActivateId;       // Set by ActivateItem(), queued until next frame.
  ANCHORInputSource NavInputSource;  // Keyboard or Gamepad mode? THIS WILL ONLY BE None or
                                     // NavGamepad or NavKeyboard.
  AnchorBBox NavScoringRect;         // Rectangle used for scoring, in screen space. Based of
                              // window->NavRectRel[], modified for directional navigation scoring.
  int NavScoringCount;      // Metrics for debugging
  ANCHORNavLayer NavLayer;  // Layer we are navigating on. For now the system is hard-coded for
                            // 0=main contents and 1=menu/title bar, may expose layers later.
  int NavIdTabCounter;      // == NavWindow->DC.FocusIdxTabCounter at time of NavId processing
  bool NavIdIsAlive;        // Nav widget has been seen this frame ~~ NavRectRel is valid
  bool NavMousePosDirty;    // When set we will update mouse position if (io.ConfigFlags &
                            // AnchorConfigFlags_NavEnableSetMousePos) if set (NB: this not enabled
                            // by default)
  bool NavDisableHighlight;  // When user starts using mouse, we hide gamepad/keyboard highlight
                             // (NB: but they are still available, which is why NavDisableHighlight
                             // isn't always != NavDisableMouseHover)
  bool NavDisableMouseHover;  // When user starts using gamepad/keyboard, we hide mouse hovering
                              // highlight until mouse is touched again.
  bool NavAnyRequest;         // ~~ NavMoveRequest || NavInitRequest
  bool NavInitRequest;        // Init request for appearing window to select first item
  bool NavInitRequestFromMove;
  ANCHOR_ID NavInitResultId;  // Init request result (first item of the window, or one for which
                              // SetItemDefaultFocus() was called)
  AnchorBBox NavInitResultRectRel;  // Init request result rectangle (relative to parent window)
  bool NavMoveRequest;              // Move request for this frame
  AnchorNavMoveFlags NavMoveRequestFlags;
  ANCHORNavForward NavMoveRequestForward;  // None / ForwardQueued / ForwardActive (this is used to
                                           // navigate sibling parent menus from a child menu)
  AnchorKeyModFlags NavMoveRequestKeyMods;
  AnchorDir NavMoveDir, NavMoveDirLast;  // Direction of the move request (left/right/up/down),
                                         // direction of the previous move request
  AnchorDir
    NavMoveClipDir;  // FIXME-NAV: Describe the purpose of this better. Might want to rename?
  AnchorNavItemData NavMoveResultLocal;            // Best move request candidate within NavWindow
  AnchorNavItemData NavMoveResultLocalVisibleSet;  // Best move request candidate within NavWindow
                                                   // that are mostly visible (when using
                                                   // AnchorNavMoveFlags_AlsoScoreVisibleSet flag)
  AnchorNavItemData
    NavMoveResultOther;  // Best move request candidate within NavWindow's flattened
                         // hierarchy (when using AnchorWindowFlags_NavFlattened flag)
  AnchorWindow *NavWrapRequestWindow;      // Window which requested trying nav wrap-around.
  AnchorNavMoveFlags NavWrapRequestFlags;  // Wrap-around operation flags.

  // Navigation: Windowing (CTRL+TAB for list, or Menu button + keys or directional pads to
  // move/resize)
  AnchorWindow
    *NavWindowingTarget;  // Target window when doing CTRL+Tab (or Pad Menu + FocusPrev/Next),
                          // this window is temporarily displayed top-most!
  AnchorWindow *NavWindowingTargetAnim;  // Record of last valid NavWindowingTarget until
                                         // DimBgRatio and NavWindowingHighlightAlpha becomes
                                         // 0.0f, so the fade-out can stay on it.
  AnchorWindow *NavWindowingListWindow;  // Internal window actually listing the CTRL+Tab contents
  float NavWindowingTimer;
  float NavWindowingHighlightAlpha;
  bool NavWindowingToggleLayer;

  // Legacy Focus/Tabbing system (older than Nav, active even if Nav is disabled, misnamed.
  // FIXME-NAV: This needs a redesign!)
  AnchorWindow *TabFocusRequestCurrWindow;  //
  AnchorWindow *TabFocusRequestNextWindow;  //
  int TabFocusRequestCurrCounterRegular;  // Any item being requested for focus, stored as an index
                                          // (we on layout to be stable between the frame pressing
                                          // TAB and the next frame, semi-ouch)
  int TabFocusRequestCurrCounterTabStop;  // Tab item being requested for focus, stored as an index
  int TabFocusRequestNextCounterRegular;  // Stored for next frame
  int TabFocusRequestNextCounterTabStop;  // "
  bool TabFocusPressed;                   // Set in NewFrame() when user pressed Tab

  // Render
  float DimBgRatio;  // 0.0..1.0 animation when fading in a dimming background (for modal window
                     // and CTRL+TAB list)
  AnchorMouseCursor MouseCursor;

  // Drag and Drop
  bool DragDropActive;
  bool DragDropWithinSource;  // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag
                              // source.
  bool DragDropWithinTarget;  // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag
                              // target.
  AnchorDragDropFlags DragDropSourceFlags;
  int DragDropSourceFrameCount;
  int DragDropMouseButton;
  AnchorPayload DragDropPayload;
  AnchorBBox DragDropTargetRect;  // Store rectangle of current target candidate (we favor small
                                  // targets when overlapping)
  ANCHOR_ID DragDropTargetId;
  AnchorDragDropFlags DragDropAcceptFlags;
  float DragDropAcceptIdCurrRectSurface;  // Target item surface (we resolve overlapping targets by
                                          // prioritizing the smaller surface)
  ANCHOR_ID DragDropAcceptIdCurr;  // Target item id (set at the time of accepting the payload)
  ANCHOR_ID DragDropAcceptIdPrev;  // Target item id from previous frame (we need to store this to
                                   // allow for overlapping drag and drop targets)
  int DragDropAcceptFrameCount;    // Last time a target expressed a desire to accept the source
  ANCHOR_ID DragDropHoldJustPressedId;  // Set when holding a payload just made ButtonBehavior()
                                        // return a press.
  AnchorVector<unsigned char>
    DragDropPayloadBufHeap;                   // We don't expose the AnchorVector<> directly,
                                              // AnchorPayload only holds pointer+size
  unsigned char DragDropPayloadBufLocal[16];  // Local buffer for small payloads

  // Table
  AnchorTable *CurrentTable;
  int CurrentTableStackIdx;
  AnchorPool<AnchorTable> Tables;
  AnchorVector<AnchorTableTempData> TablesTempDataStack;
  AnchorVector<float>
    TablesLastTimeActive;  // Last used timestamp of each tables (SOA, for efficient GC)
  AnchorVector<AnchorDrawChannel> DrawChannelsTempMergeBuffer;

  // Tab bars
  AnchorTabBar *CurrentTabBar;
  AnchorPool<AnchorTabBar> TabBars;
  AnchorVector<ANCHOR_PtrOrIndex> CurrentTabBarStack;
  AnchorVector<ANCHOR_ShrinkWidthItem> ShrinkWidthBuffer;

  // Widget state
  wabi::GfVec2f LastValidMousePos;
  AnchorInputTextState InputTextState;
  AnchorFont InputTextPasswordFont;
  ANCHOR_ID TempInputId;  // Temporary text input when CTRL+clicking on a slider, etc.
  AnchorColorEditFlags ColorEditOptions;  // Store user options for color edit widgets
  float ColorEditLastHue;  // Backup of last Hue associated to LastColor[3], so we can restore Hue
                           // in lossy RGB<>HSV round trips
  float ColorEditLastSat;  // Backup of last Saturation associated to LastColor[3], so we can
                           // restore Saturation in lossy RGB<>HSV round trips
  float ColorEditLastColor[3];
  wabi::GfVec4f
    ColorPickerRef;          // Initial/reference color at the time of opening the color picker.
  float SliderCurrentAccum;  // Accumulated slider delta when using navigation controls.
  bool SliderCurrentAccumDirty;  // Has the accumulated slider delta changed since last time we
                                 // tried to apply it?
  bool DragCurrentAccumDirty;
  float DragCurrentAccum;  // Accumulator for dragging modification. Always high-precision, not
                           // rounded by end-user precision settings
  float DragSpeedDefaultRatio;  // If speed == 0.0f, uses (max-min) * DragSpeedDefaultRatio
  float ScrollbarClickDeltaToGrabCenter;  // Distance between mouse and center of grab box,
                                          // normalized in parent space. Use storage?
  int TooltipOverrideCount;
  float TooltipSlowDelay;  // Time before slow tooltips appears (FIXME: This is temporary until we
                           // merge in tooltip timer+priority work)
  AnchorVector<char> ClipboardHandlerData;  // If no custom clipboard handler is defined
  AnchorVector<ANCHOR_ID>
    MenusIdSubmittedThisFrame;  // A list of menu IDs that were rendered at least once

  // Platform support
  wabi::GfVec2f
    PlatformImePos;  // Cursor position request & last passed to the OS Input Method Editor
  wabi::GfVec2f PlatformImeLastPos;
  char PlatformLocaleDecimalPoint;  // '.' or *localeconv()->decimal_point

  // Settings
  bool SettingsLoaded;
  float SettingsDirtyTimer;          // Save .ini Settings to memory when time reaches zero
  AnchorTextBuffer SettingsIniData;  // In memory .ini settings
  AnchorVector<AnchorSettingsHandler> SettingsHandlers;     // List of .ini settings handlers
  AnchorChunkStream<AnchorWindowSettings> SettingsWindows;  // AnchorWindow .ini settings entries
  AnchorChunkStream<AnchorTableSettings> SettingsTables;    // AnchorTable .ini settings entries
  AnchorVector<AnchorContextHook> Hooks;  // Hooks for extensions (e.g. test engine)
  ANCHOR_ID HookIdNext;                   // Next available HookId

  // Capture/Logging
  bool LogEnabled;             // Currently capturing
  ANCHORLogType LogType;       // Capture target
  ImFileHandle LogFile;        // If != NULL log to stdout/ file
  AnchorTextBuffer LogBuffer;  // Accumulation buffer when log to clipboard. This is pointer so our
                               // G_CTX static constructor doesn't call heap allocators.
  const char *LogNextPrefix;
  const char *LogNextSuffix;
  float LogLinePosY;
  bool LogLineFirstItem;
  int LogDepthRef;
  int LogDepthToExpand;
  int LogDepthToExpandDefault;  // Default/stored value for LogDepthMaxExpand if not specified in
                                // the LogXXX function call.

  // Debug Tools
  bool DebugItemPickerActive;        // Item picker is active (started with DebugStartItemPicker())
  ANCHOR_ID DebugItemPickerBreakId;  // Will call IM_DEBUG_BREAK() when encountering this id
  AnchorMetricsConfig DebugMetricsConfig;

  // Misc
  float FramerateSecPerFrame[120];  // Calculate estimate of framerate for user over the last 2
                                    // seconds.
  int FramerateSecPerFrameIdx;
  int FramerateSecPerFrameCount;
  float FramerateSecPerFrameAccum;
  int WantCaptureMouseNextFrame;  // Explicit capture via
                                  // CaptureKeyboardFromApp()/CaptureMouseFromApp() sets those
                                  // flags
  int WantCaptureKeyboardNextFrame;
  int WantTextInputNextFrame;
  char TempBuffer[1024 * 3 + 1];  // Temporary text buffer

  AnchorContext(AnchorFontAtlas *shared_font_atlas)
  {
    Initialized = false;
    FontAtlasOwnedByContext = shared_font_atlas ? false : true;

    HydraDriver.name = wabi::HgiTokens->renderDriver;
    HydraDriver.driver = wabi::VtValue();

    Font = NULL;
    FontSize = FontBaseSize = 0.0f;
    IO.Fonts = shared_font_atlas ? shared_font_atlas : ANCHOR_NEW(AnchorFontAtlas)();
    Time = 0.0f;
    FrameCount = 0;
    FrameCountEnded = FrameCountRendered = -1;
    WithinFrameScope = WithinFrameScopeWithImplicitWindow = WithinEndChild = false;
    GcCompactAll = false;
    TestEngineHookItems = false;
    TestEngineHookIdInfo = 0;
    TestEngine = NULL;

    WindowsActiveCount = 0;
    CurrentWindow = NULL;
    HoveredWindow = NULL;
    HoveredWindowUnderMovingWindow = NULL;
    MovingWindow = NULL;
    WheelingWindow = NULL;
    WheelingWindowTimer = 0.0f;

    CurrentItemFlags = AnchorItemFlags_None;
    HoveredId = HoveredIdPreviousFrame = 0;
    HoveredIdAllowOverlap = false;
    HoveredIdUsingMouseWheel = HoveredIdPreviousFrameUsingMouseWheel = false;
    HoveredIdDisabled = false;
    HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
    ActiveId = 0;
    ActiveIdIsAlive = 0;
    ActiveIdTimer = 0.0f;
    ActiveIdIsJustActivated = false;
    ActiveIdAllowOverlap = false;
    ActiveIdNoClearOnFocusLoss = false;
    ActiveIdHasBeenPressedBefore = false;
    ActiveIdHasBeenEditedBefore = false;
    ActiveIdHasBeenEditedThisFrame = false;
    ActiveIdUsingMouseWheel = false;
    ActiveIdUsingNavDirMask = 0x00;
    ActiveIdUsingNavInputMask = 0x00;
    ActiveIdUsingKeyInputMask = 0x00;
    ActiveIdClickOffset = wabi::GfVec2f(-1, -1);
    ActiveIdWindow = NULL;
    ActiveIdSource = ANCHORInputSource_None;
    ActiveIdMouseButton = -1;
    ActiveIdPreviousFrame = 0;
    ActiveIdPreviousFrameIsAlive = false;
    ActiveIdPreviousFrameHasBeenEditedBefore = false;
    ActiveIdPreviousFrameWindow = NULL;
    LastActiveId = 0;
    LastActiveIdTimer = 0.0f;

    NavWindow = NULL;
    NavId = NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId =
      NavInputId = 0;
    NavJustTabbedId = NavJustMovedToId = NavJustMovedToFocusScopeId = NavNextActivateId = 0;
    NavJustMovedToKeyMods = AnchorKeyModFlags_None;
    NavInputSource = ANCHORInputSource_None;
    NavScoringRect = AnchorBBox();
    NavScoringCount = 0;
    NavLayer = ANCHORNavLayer_Main;
    NavIdTabCounter = INT_MAX;
    NavIdIsAlive = false;
    NavMousePosDirty = false;
    NavDisableHighlight = true;
    NavDisableMouseHover = false;
    NavAnyRequest = false;
    NavInitRequest = false;
    NavInitRequestFromMove = false;
    NavInitResultId = 0;
    NavMoveRequest = false;
    NavMoveRequestFlags = AnchorNavMoveFlags_None;
    NavMoveRequestForward = ANCHORNavForward_None;
    NavMoveRequestKeyMods = AnchorKeyModFlags_None;
    NavMoveDir = NavMoveDirLast = NavMoveClipDir = AnchorDir_None;
    NavWrapRequestWindow = NULL;
    NavWrapRequestFlags = AnchorNavMoveFlags_None;

    NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
    NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
    NavWindowingToggleLayer = false;

    TabFocusRequestCurrWindow = TabFocusRequestNextWindow = NULL;
    TabFocusRequestCurrCounterRegular = TabFocusRequestCurrCounterTabStop = INT_MAX;
    TabFocusRequestNextCounterRegular = TabFocusRequestNextCounterTabStop = INT_MAX;
    TabFocusPressed = false;

    DimBgRatio = 0.0f;
    MouseCursor = ANCHOR_StandardCursorDefault;

    DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
    DragDropSourceFlags = AnchorDragDropFlags_None;
    DragDropSourceFrameCount = -1;
    DragDropMouseButton = -1;
    DragDropTargetId = 0;
    DragDropAcceptFlags = AnchorDragDropFlags_None;
    DragDropAcceptIdCurrRectSurface = 0.0f;
    DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
    DragDropAcceptFrameCount = -1;
    DragDropHoldJustPressedId = 0;
    memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

    CurrentTable = NULL;
    CurrentTableStackIdx = -1;
    CurrentTabBar = NULL;

    LastValidMousePos = wabi::GfVec2f(0.0f, 0.0f);
    TempInputId = 0;
    ColorEditOptions = AnchorColorEditFlags__OptionsDefault;
    ColorEditLastHue = ColorEditLastSat = 0.0f;
    ColorEditLastColor[0] = ColorEditLastColor[1] = ColorEditLastColor[2] = FLT_MAX;
    SliderCurrentAccum = 0.0f;
    SliderCurrentAccumDirty = false;
    DragCurrentAccumDirty = false;
    DragCurrentAccum = 0.0f;
    DragSpeedDefaultRatio = 1.0f / 100.0f;
    ScrollbarClickDeltaToGrabCenter = 0.0f;
    TooltipOverrideCount = 0;
    TooltipSlowDelay = 0.50f;

    PlatformImePos = PlatformImeLastPos = wabi::GfVec2f(FLT_MAX, FLT_MAX);
    PlatformLocaleDecimalPoint = '.';

    SettingsLoaded = false;
    SettingsDirtyTimer = 0.0f;
    HookIdNext = 0;

    LogEnabled = false;
    LogType = ANCHORLogType_None;
    LogNextPrefix = LogNextSuffix = NULL;
    LogFile = NULL;
    LogLinePosY = FLT_MAX;
    LogLineFirstItem = false;
    LogDepthRef = 0;
    LogDepthToExpand = LogDepthToExpandDefault = 2;

    DebugItemPickerActive = false;
    DebugItemPickerBreakId = 0;

    memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
    FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
    FramerateSecPerFrameAccum = 0.0f;
    WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
    memset(TempBuffer, 0, sizeof(TempBuffer));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] AnchorWindowTempData, AnchorWindow
//-----------------------------------------------------------------------------

// Transient per-window data, reset at the beginning of the frame. This used to be called
// ANCHORDrawContext, hence the DC variable name in AnchorWindow. (That's theory, in practice the
// delimitation between AnchorWindow and AnchorWindowTempData is quite tenuous and could be
// reconsidered..) (This doesn't need a constructor because we zero-clear it as part of
// AnchorWindow and all frame-temporary data are setup on Begin)
struct ANCHOR_API AnchorWindowTempData
{
  // Layout
  wabi::GfVec2f CursorPos;  // Current emitting position, in absolute coordinates.
  wabi::GfVec2f CursorPosPrevLine;
  wabi::GfVec2f CursorStartPos;  // Initial position after Begin(), generally ~ window position +
                                 // WindowPadding.
  wabi::GfVec2f
    CursorMaxPos;  // Used to implicitly calculate ContentSize at the beginning of next frame,
                   // for scrolling range and auto-resize. Always growing during the frame.
  wabi::GfVec2f IdealMaxPos;  // Used to implicitly calculate ContentSizeIdeal at the beginning of
                              // next frame, for auto-resize only. Always growing during the frame.
  wabi::GfVec2f CurrLineSize;
  wabi::GfVec2f PrevLineSize;
  float CurrLineTextBaseOffset;  // Baseline offset (0.0f by default on a new line, generally ==
                                 // style.FramePadding[1] when a framed item has been added).
  float PrevLineTextBaseOffset;
  GfVec1 Indent;         // Indentation / start position from left of window (increased by
                         // TreePush/TreePop, etc.)
  GfVec1 ColumnsOffset;  // Offset to the current column (if ColumnsCurrent > 0). FIXME: This and
                         // the above should be a stack to allow use cases like Tree->Column->Tree.
                         // Need revamp columns API.
  GfVec1 GroupOffset;

  // Last item status
  ANCHOR_ID LastItemId;  // ID for last item
  AnchorItemStatusFlags
    LastItemStatusFlags;           // Status flags for last item (see AnchorItemStatusFlags_)
  AnchorBBox LastItemRect;         // Interaction rect for last item
  AnchorBBox LastItemDisplayRect;  // End-user display rect for last item (only valid if
                                   // LastItemStatusFlags & AnchorItemStatusFlags_HasDisplayRect)

  // Keyboard/Gamepad navigation
  ANCHORNavLayer NavLayerCurrent;  // Current layer, 0..31 (we currently only use 0..1)
  short NavLayersActiveMask;      // Which layers have been written to (result from previous frame)
  short NavLayersActiveMaskNext;  // Which layers have been written to (accumulator for current
                                  // frame)
  ANCHOR_ID NavFocusScopeIdCurrent;  // Current focus scope ID while appending
  bool NavHideHighlightOneFrame;
  bool NavHasScroll;  // Set when scrolling can be used (ScrollMax > 0.0f)

  // Miscellaneous
  bool MenuBarAppending;          // FIXME: Remove this
  wabi::GfVec2f MenuBarOffset;    // MenuBarOffset[0] is sort of equivalent of a per-layer
                                  // CursorPos[0], saved/restored as we switch to the menu bar. The
                                  // only situation when MenuBarOffset[1] is > 0 if when
                                  // (SafeAreaPadding[1] > FramePadding[1]), often used on TVs.
  AnchorMenuColumns MenuColumns;  // Simplified columns storage for menu items measurement
  int TreeDepth;                  // Current tree depth.
  AnchorU32 TreeJumpToParentOnPopMask;  // Store a copy of !g.NavIdIsAlive for TreeDepth 0..31..
                                        // Could be turned into a AnchorU64 if necessary.
  AnchorVector<AnchorWindow *> ChildWindows;
  AnchorStorage *StateStorage;       // Current persistent per-window storage (store e.g. tree node
                                     // open/close state)
  AnchorOldColumns *CurrentColumns;  // Current columns set
  int CurrentTableIdx;               // Current table index (into g.Tables)
  AnchorLayoutType LayoutType;
  AnchorLayoutType ParentLayoutType;  // Layout type of parent window at the time of Begin()
  int FocusCounterRegular;  // (Legacy Focus/Tabbing system) Sequential counter, start at -1 and
                            // increase as assigned via FocusableItemRegister() (FIXME-NAV: Needs
                            // redesign)
  int FocusCounterTabStop;  // (Legacy Focus/Tabbing system) Same, but only count widgets which you
                            // can Tab through.

  // Local parameters stacks
  // We store the current settings outside of the vectors to increase memory locality (reduce cache
  // misses). The vectors are rarely modified. Also it allows us to not heap allocate for
  // short-lived windows which are not using those settings.
  float ItemWidth;    // Current item width (>0.0: width in pixels, <0.0: align xx pixels to the
                      // right of window).
  float TextWrapPos;  // Current text wrap pos.
  AnchorVector<float>
    ItemWidthStack;  // Store item widths to restore (attention: .back() is not == ItemWidth)
  AnchorVector<float>
    TextWrapPosStack;                  // Store text wrap pos to restore (attention: .back() is not
                                       // == TextWrapPos)
  AnchorStackSizes StackSizesOnBegin;  // Store size of various stacks for asserting
};

// Storage for one window
struct ANCHOR_API AnchorWindow
{
  char *Name;                 // Window name, owned by the window.
  ANCHOR_ID ID;               // == AnchorHashStr(Name)
  AnchorWindowFlags Flags;    // See enum AnchorWindowFlags_
  wabi::GfVec2f Pos;          // Position (always rounded-up to nearest pixel)
  wabi::GfVec2f Size;         // Current size (==SizeFull or collapsed title bar size)
  wabi::GfVec2f SizeFull;     // Size when non collapsed
  wabi::GfVec2f ContentSize;  // Size of contents/scrollable client area (calculated from the
                              // extents reach of the cursor) from previous frame. Does not include
                              // window decoration or window padding.
  wabi::GfVec2f ContentSizeIdeal;
  wabi::GfVec2f ContentSizeExplicit;  // Size of contents/scrollable client area explicitly request
                                      // by the user via SetNextWindowContentSize().
  wabi::GfVec2f WindowPadding;        // Window padding at the time of Begin().
  float WindowRounding;    // Window rounding at the time of Begin(). May be clamped lower to avoid
                           // rendering artifacts with title bar, menu bar etc.
  float WindowBorderSize;  // Window border size at the time of Begin().
  int NameBufLen;          // Size of buffer storing Name. May be larger than strlen(Name)!
  ANCHOR_ID MoveId;        // == window->GetID("#MOVE")
  ANCHOR_ID ChildId;  // ID of corresponding item in parent window (for navigation to return from
                      // child window to parent window)
  wabi::GfVec2f Scroll;
  wabi::GfVec2f ScrollMax;
  wabi::GfVec2f
    ScrollTarget;  // target scroll position. stored as cursor position with scrolling canceled
                   // out, so the highest point is always 0.0f. (FLT_MAX for no change)
  wabi::GfVec2f ScrollTargetCenterRatio;   // 0.0f = scroll so that target position is at top, 0.5f
                                           // = scroll so that target position is centered
  wabi::GfVec2f ScrollTargetEdgeSnapDist;  // 0.0f = no snapping, >0.0f snapping threshold
  wabi::GfVec2f
    ScrollbarSizes;  // Size taken by each scrollbars on their smaller axis. Pay attention!
                     // ScrollbarSizes[0] == width of the vertical scrollbar, ScrollbarSizes[1]
                     // = height of the horizontal scrollbar.
  bool ScrollbarX, ScrollbarY;  // Are scrollbars visible?
  bool Active;                  // Set to true on Begin(), unless Collapsed
  bool WasActive;
  bool WriteAccessed;  // Set to true when any widget access the current window
  bool Collapsed;      // Set when collapsing window to become only title-bar
  bool WantCollapseToggle;
  bool SkipItems;         // Set when items can safely be all clipped (e.g. window not visible or
                          // collapsed)
  bool Appearing;         // Set during the frame where the window is appearing (or re-appearing)
  bool Hidden;            // Do not display (== HiddenFrames*** > 0)
  bool IsFallbackWindow;  // Set on the "Debug##Default" window.
  bool HasCloseButton;    // Set when the window has a close button (p_open != NULL)
  signed char ResizeBorderHeld;  // Current border being held for resize (-1: none, otherwise 0-3)
  short BeginCount;  // Number of Begin() during the current frame (generally 0 or 1, 1+ if
                     // appending via multiple Begin/End pairs)
  short BeginOrderWithinParent;  // Begin() order within immediate parent window, if we are a child
                                 // window. Otherwise 0.
  short BeginOrderWithinContext;  // Begin() order within entire ANCHOR context. This is mostly
                                  // used for debugging submission order related issues.
  short FocusOrder;   // Order within WindowsFocusOrder[], altered when windows are focused.
  ANCHOR_ID PopupId;  // ID in the popup stack when this window is used as a popup/menu (because we
                      // use generic Name/ID for recycling)
  AnchorS8 AutoFitFramesX, AutoFitFramesY;
  AnchorS8 AutoFitChildAxises;
  bool AutoFitOnlyGrows;
  AnchorDir AutoPosLastDirection;
  AnchorS8 HiddenFramesCanSkipItems;     // Hide the window for N frames
  AnchorS8 HiddenFramesCannotSkipItems;  // Hide the window for N frames while allowing items to be
                                         // submitted so we can measure their size
  AnchorS8 HiddenFramesForRenderOnly;    // Hide the window until frame N at Render() time only
  AnchorS8 DisableInputsFrames;          // Disable window interactions for N frames
  AnchorCond
    SetWindowPosAllowFlags : 8;  // store acceptable condition flags for SetNextWindowPos() use.
  AnchorCond SetWindowSizeAllowFlags : 8;       // store acceptable condition flags for
                                                // SetNextWindowSize() use.
  AnchorCond SetWindowCollapsedAllowFlags : 8;  // store acceptable condition flags for
                                                // SetNextWindowCollapsed() use.
  wabi::GfVec2f SetWindowPosVal;    // store window position when using a non-zero Pivot (position
                                    // set needs to be processed when we know the window size)
  wabi::GfVec2f SetWindowPosPivot;  // store window pivot for positioning. wabi::GfVec2f(0, 0) when
                                    // positioning from top-left corner; wabi::GfVec2f(0.5f, 0.5f)
                                    // for centering; wabi::GfVec2f(1, 1) for bottom right.

  AnchorVector<ANCHOR_ID>
    IDStack;                // ID stack. ID are hashes seeded with the value at the top of the
                            // stack. (In theory this should be in the TempData structure)
  AnchorWindowTempData DC;  // Temporary per-window data, reset at the beginning of the frame. This
                            // used to be called ANCHORDrawContext, hence the "DC" variable name.

  // The best way to understand what those rectangles are is to use the 'Metrics->Tools->Show
  // Windows Rectangles' viewer. The main 'OuterRect', omitted as a field, is window->Rect().
  AnchorBBox OuterRectClipped;  // == Window->Rect() just after setup in Begin(). == window->Rect()
                                // for root window.
  AnchorBBox InnerRect;         // Inner rectangle (omit title bar, menu bar, scroll bar)
  AnchorBBox InnerClipRect;     // == InnerRect shrunk by WindowPadding*0.5f on each side, clipped
                                // within viewport or parent clip rect.
  AnchorBBox
    WorkRect;  // Initially covers the whole scrolling region. Reduced by containers e.g
               // columns/tables when active. Shrunk by WindowPadding*1.0f on each side. This
               // is meant to replace ContentRegionRect over time (from 1.71+ onward).
  AnchorBBox ParentWorkRect;  // Backup of WorkRect before entering a container such as
                              // columns/tables. Used by e.g. SpanAllColumns functions to easily
                              // access. Stacked containers are responsible for maintaining this.
                              // // FIXME-WORKRECT: Could be a stack?
  AnchorBBox ClipRect;        // Current clipping/scissoring rectangle, evolve as we are using
                              // PushClipRect(), etc. == DrawList->clip_rect_stack.back().
  AnchorBBox
    ContentRegionRect;  // FIXME: This is currently confusing/misleading. It is essentially
                        // WorkRect but not handling of scrolling. We currently rely on it as
                        // right/bottom aligned sizing operation need some size to rely on.
  wabi::GfVec2h HitTestHoleSize;  // Define an optional rectangular hole where mouse will
                                  // pass-through the window.
  wabi::GfVec2h HitTestHoleOffset;

  int LastFrameActive;   // Last frame number the window was Active.
  float LastTimeActive;  // Last timestamp the window was Active (using float as we don't need high
                         // precision there)
  float ItemWidthDefault;
  AnchorStorage StateStorage;
  AnchorVector<AnchorOldColumns> ColumnsStorage;
  float FontWindowScale;  // User scale multiplier per-window, via SetWindowFontScale()
  int SettingsOffset;  // Offset into SettingsWindows[] (offsets are always valid as we only grow
                       // the array from the back)

  AnchorDrawList *DrawList;  // == &DrawListInst (for backward compatibility reason with code using
                             // ANCHOR_internal.h we keep this a pointer)
  AnchorDrawList DrawListInst;
  AnchorWindow *ParentWindow;  // If we are a child _or_ popup window, this is pointing to our
                               // parent. Otherwise NULL.
  AnchorWindow *RootWindow;    // Point to ourself or first ancestor that is not a child window ==
                               // Top-level window.
  AnchorWindow
    *RootWindowForTitleBarHighlight;  // Point to ourself or first ancestor which will display
                                      // TitleBgActive color when this window is active.
  AnchorWindow *RootWindowForNav;     // Point to ourself or first ancestor which doesn't have the
                                      // NavFlattened flag.

  AnchorWindow
    *NavLastChildNavWindow;  // When going to the menu bar, we remember the child window we came
                             // from. (This could probably be made implicit if we kept g.Windows
                             // sorted by last focused including child window.)
  ANCHOR_ID NavLastIds[ANCHORNavLayer_COUNT];  // Last known NavId for this window, per layer (0/1)
  AnchorBBox NavRectRel[ANCHORNavLayer_COUNT];  // Reference rectangle, in window relative space

  int MemoryDrawListIdxCapacity;  // Backup of last idx/vtx count, so when waking up the window we
                                  // can preallocate and avoid iterative alloc/copy
  int MemoryDrawListVtxCapacity;
  bool MemoryCompacted;  // Set when window extraneous data have been garbage collected

 public:

  AnchorWindow(AnchorContext *context, const char *name);
  ~AnchorWindow();

  ANCHOR_ID GetID(const char *str, const char *str_end = NULL);
  ANCHOR_ID GetID(const void *ptr);
  ANCHOR_ID GetID(int n);
  ANCHOR_ID GetIDNoKeepAlive(const char *str, const char *str_end = NULL);
  ANCHOR_ID GetIDNoKeepAlive(const void *ptr);
  ANCHOR_ID GetIDNoKeepAlive(int n);
  ANCHOR_ID GetIDFromRectangle(const AnchorBBox &r_abs);

  // We don't use g.FontSize because the window may be != g.CurrentWidow.
  AnchorBBox Rect() const
  {
    return AnchorBBox(Pos[0], Pos[1], Pos[0] + Size[0], Pos[1] + Size[1]);
  }
  float CalcFontSize() const
  {
    AnchorContext &g = *G_CTX;
    float scale = g.FontBaseSize * FontWindowScale;
    if (ParentWindow)
      scale *= ParentWindow->FontWindowScale;
    return scale;
  }
  float TitleBarHeight() const
  {
    AnchorContext &g = *G_CTX;
    return (Flags & AnchorWindowFlags_NoTitleBar) ?
             0.0f :
             CalcFontSize() + g.Style.FramePadding[1] * 2.0f;
  }
  AnchorBBox TitleBarRect() const
  {
    return AnchorBBox(Pos, wabi::GfVec2f(Pos[0] + SizeFull[0], Pos[1] + TitleBarHeight()));
  }
  float MenuBarHeight() const
  {
    AnchorContext &g = *G_CTX;
    return (Flags & AnchorWindowFlags_MenuBar) ?
             DC.MenuBarOffset[1] + CalcFontSize() + g.Style.FramePadding[1] * 2.0f :
             0.0f;
  }
  AnchorBBox MenuBarRect() const
  {
    float y1 = Pos[1] + TitleBarHeight();
    return AnchorBBox(Pos[0], y1, Pos[0] + SizeFull[0], y1 + MenuBarHeight());
  }
};

// Backup and restore just enough data to be able to use IsItemHovered() on item A after another B
// in the same window has overwritten the data.
struct AnchorLastItemDataBackup
{
  ANCHOR_ID LastItemId;
  AnchorItemStatusFlags LastItemStatusFlags;
  AnchorBBox LastItemRect;
  AnchorBBox LastItemDisplayRect;

  AnchorLastItemDataBackup()
  {
    Backup();
  }
  void Backup()
  {
    AnchorWindow *window = G_CTX->CurrentWindow;
    LastItemId = window->DC.LastItemId;
    LastItemStatusFlags = window->DC.LastItemStatusFlags;
    LastItemRect = window->DC.LastItemRect;
    LastItemDisplayRect = window->DC.LastItemDisplayRect;
  }
  void Restore() const
  {
    AnchorWindow *window = G_CTX->CurrentWindow;
    window->DC.LastItemId = LastItemId;
    window->DC.LastItemStatusFlags = LastItemStatusFlags;
    window->DC.LastItemRect = LastItemRect;
    window->DC.LastItemDisplayRect = LastItemDisplayRect;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Tab bar, Tab item support
//-----------------------------------------------------------------------------

// Extend AnchorTabBarFlags_
enum AnchorTabBarFlagsPrivate_
{
  AnchorTabBarFlags_DockNode =
    1 << 20,  // Part of a dock node [we don't use this in the master branch but
              // it facilitate branch syncing to keep this around]
  AnchorTabBarFlags_IsFocused = 1 << 21,
  AnchorTabBarFlags_SaveSettings =
    1 << 22  // FIXME: Settings are handled by the docking system, this only
             // request the tab bar to mark settings dirty when reordering tabs
};

// Extend AnchorTabItemFlags_
enum AnchorTabItemFlagsPrivate_
{
  AnchorTabItemFlags_SectionMask_ = AnchorTabItemFlags_Leading | AnchorTabItemFlags_Trailing,
  AnchorTabItemFlags_NoCloseButton =
    1 << 20,  // Track whether p_open was set or not (we'll need this info
              // on the next frame to recompute ContentWidth during layout)
  AnchorTabItemFlags_Button =
    1 << 21  // Used by TabItemButton, change the tab item behavior to mimic a button
};

// Storage for one active tab item (sizeof() 40 bytes)
struct AnchorTabItem
{
  ANCHOR_ID ID;
  AnchorTabItemFlags Flags;
  int LastFrameVisible;
  int LastFrameSelected;  // This allows us to infer an ordered list of the last activated tabs
                          // with little maintenance
  float Offset;           // Position relative to beginning of tab
  float Width;            // Width currently displayed
  float ContentWidth;     // Width of label, stored during BeginTabItem() call
  AnchorS32 NameOffset;  // When Window==NULL, offset to name within parent AnchorTabBar::TabsNames
  AnchorS16 BeginOrder;  // BeginTabItem() order, used to re-order tabs after toggling
                         // AnchorTabBarFlags_Reorderable
  AnchorS16 IndexDuringLayout;  // Index only used during TabBarLayout()
  bool WantClose;               // Marked as closed by SetTabItemClosed()

  AnchorTabItem()
  {
    memset(this, 0, sizeof(*this));
    LastFrameVisible = LastFrameSelected = -1;
    NameOffset = -1;
    BeginOrder = IndexDuringLayout = -1;
  }
};

// Storage for a tab bar (sizeof() 152 bytes)
struct AnchorTabBar
{
  AnchorVector<AnchorTabItem> Tabs;
  AnchorTabBarFlags Flags;
  ANCHOR_ID ID;             // Zero for tab-bars used by docking
  ANCHOR_ID SelectedTabId;  // Selected tab/window
  ANCHOR_ID
  NextSelectedTabId;       // Next selected tab/window. Will also trigger a scrolling animation
  ANCHOR_ID VisibleTabId;  // Can occasionally be != SelectedTabId (e.g. when previewing contents
                           // for CTRL+TAB preview)
  int CurrFrameVisible;
  int PrevFrameVisible;
  AnchorBBox BarRect;
  float CurrTabsContentsHeight;
  float PrevTabsContentsHeight;  // Record the height of contents submitted below the tab bar
  float WidthAllTabs;            // Actual width of all tabs (locked during layout)
  float WidthAllTabsIdeal;       // Ideal width if all tabs were visible and not clipped
  float ScrollingAnim;
  float ScrollingTarget;
  float ScrollingTargetDistToVisibility;
  float ScrollingSpeed;
  float ScrollingRectMinX;
  float ScrollingRectMaxX;
  ANCHOR_ID ReorderRequestTabId;
  AnchorS16 ReorderRequestOffset;
  AnchorS8 BeginCount;
  bool WantLayout;
  bool VisibleTabWasSubmitted;
  bool TabsAddedNew;  // Set to true when a new tab item or button has been added to the tab bar
                      // during last frame
  AnchorS16 TabsActiveCount;  // Number of tabs submitted this frame.
  AnchorS16 LastTabItemIdx;   // Index of last BeginTabItem() tab for use by EndTabItem()
  float ItemSpacingY;
  wabi::GfVec2f FramePadding;  // style.FramePadding locked at the time of BeginTabBar()
  wabi::GfVec2f BackupCursorPos;
  AnchorTextBuffer
    TabsNames;  // For non-docking tab bar we re-append names in a contiguous buffer.

  AnchorTabBar();
  int GetTabOrder(const AnchorTabItem *tab) const
  {
    return Tabs.index_from_ptr(tab);
  }
  const char *GetTabName(const AnchorTabItem *tab) const
  {
    ANCHOR_ASSERT(tab->NameOffset != -1 && tab->NameOffset < TabsNames.Buf.Size);
    return TabsNames.Buf.Data + tab->NameOffset;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Table support
//-----------------------------------------------------------------------------

#define ANCHOR_COL32_DISABLE \
  ANCHOR_COL32(0, 0, 0, 1)  // Special sentinel code which cannot be used as a regular color.
#define ANCHOR_TABLE_MAX_COLUMNS \
  64  // sizeof(AnchorU64) * 8. This is solely because we frequently encode columns set in a \
                                                       // AnchorU64.
#define ANCHOR_TABLE_MAX_DRAW_CHANNELS (4 + 64 * 2)  // See TableSetupDrawChannels()

// Our current column maximum is 64 but we may raise that in the future.
typedef AnchorS8 AnchorTableColumnIdx;
typedef AnchorU8 AnchorTableDrawChannelIdx;

// [Internal] sizeof() ~ 104
// We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
// We use the terminology "Clipped" to refer to a column that is out of sight because of
// scrolling/clipping. This is in contrast with some user-facing api such as IsItemVisible() /
// IsRectVisible() which use "Visible" to mean "not clipped".
struct AnchorTableColumn
{
  AnchorTableColumnFlags Flags;  // Flags after some patching (not directly same as provided by
                                 // user). See AnchorTableColumnFlags_
  float WidthGiven;  // Final/actual width visible == (MaxX - MinX), locked in TableUpdateLayout().
                     // May be > WidthRequest to honor minimum width, may be < WidthRequest to
                     // honor shrinking columns down in tight space.
  float MinX;        // Absolute positions
  float MaxX;
  float WidthRequest;   // Master width absolute value when !(Flags & _WidthStretch). When Stretch
                        // this is derived every frame from StretchWeight in TableUpdateLayout()
  float WidthAuto;      // Automatic width
  float StretchWeight;  // Master width weight when (Flags & _WidthStretch). Often around ~1.0f
                        // initially.
  float InitStretchWeightOrWidth;  // Value passed to TableSetupColumn(). For Width it is a content
                                   // width (_without padding_).
  AnchorBBox ClipRect;             // Clipping rectangle for the column
  ANCHOR_ID UserID;                // Optional, value passed to TableSetupColumn()
  float WorkMinX;   // Contents region min ~(MinX + CellPaddingX + CellSpacingX1) == cursor start
                    // position when entering column
  float WorkMaxX;   // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
  float ItemWidth;  // Current item width for the column, preserved across rows
  float ContentMaxXFrozen;  // Contents maximum position for frozen rows (apart from headers), from
                            // which we can infer content width.
  float ContentMaxXUnfrozen;
  float ContentMaxXHeadersUsed;  // Contents maximum position for headers rows (regardless of
                                 // freezing). TableHeader() automatically softclip itself + report
                                 // ideal desired size, to avoid creating extraneous draw calls
  float ContentMaxXHeadersIdeal;
  AnchorS16 NameOffset;               // Offset into parent ColumnsNames[]
  AnchorTableColumnIdx DisplayOrder;  // Index within Table's IndexToDisplayOrder[] (column may be
                                      // reordered by users)
  AnchorTableColumnIdx
    IndexWithinEnabledSet;  // Index within enabled/visible set (<= IndexToDisplayOrder)
  AnchorTableColumnIdx PrevEnabledColumn;  // Index of prev enabled/visible column within
                                           // Columns[], -1 if first enabled/visible column
  AnchorTableColumnIdx NextEnabledColumn;  // Index of next enabled/visible column within
                                           // Columns[], -1 if last enabled/visible column
  AnchorTableColumnIdx SortOrder;  // Index of this column within sort specs, -1 if not sorting on
                                   // this column, 0 for single-sort, may be >0 on multi-sort
  AnchorTableDrawChannelIdx DrawChannelCurrent;  // Index within DrawSplitter.Channels[]
  AnchorTableDrawChannelIdx DrawChannelFrozen;
  AnchorTableDrawChannelIdx DrawChannelUnfrozen;
  bool IsEnabled;  // Is the column not marked Hidden by the user? (even if off view, e.g. clipped
                   // by scrolling).
  bool IsEnabledNextFrame;
  bool IsVisibleX;  // Is actually in view (e.g. overlapping the host window clipping rectangle,
                    // not scrolled).
  bool IsVisibleY;
  bool IsRequestOutput;  // Return value for TableSetColumnIndex() / TableNextColumn(): whether we
                         // request user to output contents or not.
  bool IsSkipItems;      // Do we want item submissions to this column to be completely ignored (no
                         // layout will happen).
  bool IsPreserveWidthAuto;
  AnchorS8 NavLayerCurrent;  // ANCHORNavLayer in 1 byte
  AnchorU8 AutoFitQueue;     // Queue of 8 values for the next 8 frames to request auto-fit
  AnchorU8
    CannotSkipItemsQueue;  // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
  AnchorU8 SortDirection : 2;  // AnchorSortDirection_Ascending or AnchorSortDirection_Descending
  AnchorU8 SortDirectionsAvailCount : 2;  // Number of available sort directions (0 to 3)
  AnchorU8 SortDirectionsAvailMask : 4;   // Mask of available sort directions (1-bit each)
  AnchorU8 SortDirectionsAvailList;       // Ordered of available sort directions (2-bits each)

  AnchorTableColumn()
  {
    memset(this, 0, sizeof(*this));
    StretchWeight = WidthRequest = -1.0f;
    NameOffset = -1;
    DisplayOrder = IndexWithinEnabledSet = -1;
    PrevEnabledColumn = NextEnabledColumn = -1;
    SortOrder = -1;
    SortDirection = AnchorSortDirection_None;
    DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (AnchorU8)-1;
  }
};

// Transient cell data stored per row.
// sizeof() ~ 6
struct AnchorTableCellData
{
  AnchorU32 BgColor;            // Actual color
  AnchorTableColumnIdx Column;  // Column number
};

// FIXME-TABLE: more transient data could be stored in a per-stacked table structure: DrawSplitter,
// SortSpecs, incoming RowData
struct AnchorTable
{
  ANCHOR_ID ID;
  AnchorTableFlags Flags;
  void *RawData;  // Single allocation to hold Columns[], DisplayOrderToIndex[] and RowCellData[]
  AnchorTableTempData
    *TempData;  // Transient data while table is active. Point within g.CurrentTableStack[]
  AnchorSpan<AnchorTableColumn> Columns;  // Point within RawData[]
  AnchorSpan<AnchorTableColumnIdx>
    DisplayOrderToIndex;  // Point within RawData[]. Store display order of columns (when not
                          // reordered, the values are 0...Count-1)
  AnchorSpan<AnchorTableCellData>
    RowCellData;  // Point within RawData[]. Store cells background requests for current row.
  AnchorU64 EnabledMaskByDisplayOrder;  // Column DisplayOrder -> IsEnabled map
  AnchorU64 EnabledMaskByIndex;  // Column Index -> IsEnabled map (== not hidden by user/api) in a
                                 // format adequate for iterating column without touching cold data
  AnchorU64 VisibleMaskByIndex;  // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by
                                 // user/api && not hidden by scrolling/cliprect)
  AnchorU64 RequestOutputMaskByIndex;    // Column Index -> IsVisible || AutoFit (== expect user to
                                         // submit items)
  AnchorTableFlags SettingsLoadedFlags;  // Which data were loaded from the .ini file (e.g. when
                                         // order is not altered we won't save order)
  int SettingsOffset;                    // Offset in g.SettingsTables
  int LastFrameActive;
  int ColumnsCount;  // Number of columns declared in BeginTable()
  int CurrentRow;
  int CurrentColumn;
  AnchorS16 InstanceCurrent;  // Count of BeginTable() calls with same ID in the same frame
                              // (generally 0). This is a little bit similar to BeginCount for a
                              // window, but multiple table with same ID look are multiple tables,
                              // they are just synched.
  AnchorS16 InstanceInteracted;  // Mark which instance (generally 0) of the same ID is being
                                 // interacted with
  float RowPosY1;
  float RowPosY2;
  float RowMinHeight;  // Height submitted to TableNextRow()
  float RowTextBaseline;
  float RowIndentOffsetX;
  AnchorTableRowFlags RowFlags : 16;  // Current row flags, see AnchorTableRowFlags_
  AnchorTableRowFlags LastRowFlags : 16;
  int RowBgColorCounter;    // Counter for alternating background colors (can be fast-forwarded by
                            // e.g clipper), not same as CurrentRow because header rows typically
                            // don't increase this.
  AnchorU32 RowBgColor[2];  // Background color override for current row.
  AnchorU32 BorderColorStrong;
  AnchorU32 BorderColorLight;
  float BorderX1;
  float BorderX2;
  float HostIndentX;
  float MinColumnWidth;
  float OuterPaddingX;
  float CellPaddingX;  // Padding from each borders
  float CellPaddingY;
  float CellSpacingX1;  // Spacing between non-bordered cells
  float CellSpacingX2;
  float LastOuterHeight;     // Outer height from last frame
  float LastFirstRowHeight;  // Height of first row from last frame
  float InnerWidth;  // User value passed to BeginTable(), see comments at the top of BeginTable()
                     // for details.
  float ColumnsGivenWidth;    // Sum of current column width
  float ColumnsAutoFitWidth;  // Sum of ideal column width in order nothing to be clipped, used for
                              // auto-fitting and content width submission in outer window
  float ResizedColumnNextWidth;
  float ResizeLockMinContentsX2;  // Lock minimum contents width while resizing down in order to
                                  // not create feedback loops. But we allow growing the table.
  float RefScale;        // Reference scale to be able to rescale columns on font/dpi changes.
  AnchorBBox OuterRect;  // Note: for non-scrolling table, OuterRect.Max[1] is often FLT_MAX until
                         // EndTable(), unless a height has been specified in BeginTable().
  AnchorBBox InnerRect;  // InnerRect but without decoration. As with OuterRect, for non-scrolling
                         // tables, InnerRect.Max[1] is
  AnchorBBox WorkRect;
  AnchorBBox InnerClipRect;
  AnchorBBox BgClipRect;  // We use this to cpu-clip cell background color fill
  AnchorBBox
    Bg0ClipRectForDrawCmd;  // Actual AnchorDrawCmd clip rect for BG0/1 channel. This tends to be
                            // == OuterWindow->ClipRect at BeginTable() because output in
                            // BG0/BG1 is cpu-clipped
  AnchorBBox Bg2ClipRectForDrawCmd;  // Actual AnchorDrawCmd clip rect for BG2 channel. This tends
                                     // to be a correct, tight-fit, because output to BG2 are done
                                     // by widgets relying on regular ClipRect.
  AnchorBBox HostClipRect;  // This is used to check if we can eventually merge our columns draw
                            // calls into the current draw call of the current window.
  AnchorBBox HostBackupInnerClipRect;  // Backup of InnerWindow->ClipRect during
                                       // PushTableBackground()/PopTableBackground()
  AnchorWindow *OuterWindow;           // Parent window for the table
  AnchorWindow *InnerWindow;  // Window holding the table data (== OuterWindow or a child window)
  AnchorTextBuffer ColumnsNames;  // Contiguous buffer holding columns names
  AnchorDrawListSplitter
    *DrawSplitter;  // Shortcut to TempData->DrawSplitter while in table. Isolate draw
                    // commands per columns to avoid switching clip rect constantly
  AnchorTableSortSpecs
    SortSpecs;  // Public facing sorts specs, this is what we return in TableGetSortSpecs()
  AnchorTableColumnIdx SortSpecsCount;
  AnchorTableColumnIdx ColumnsEnabledCount;       // Number of enabled columns (<= ColumnsCount)
  AnchorTableColumnIdx ColumnsEnabledFixedCount;  // Number of enabled columns (<= ColumnsCount)
  AnchorTableColumnIdx DeclColumnsCount;          // Count calls to TableSetupColumn()
  AnchorTableColumnIdx
    HoveredColumnBody;  // Index of column whose visible region is being hovered. Important: ==
                        // ColumnsCount when hovering empty region after the right-most column!
  AnchorTableColumnIdx
    HoveredColumnBorder;  // Index of column whose right-border is being hovered (for resizing).
  AnchorTableColumnIdx AutoFitSingleColumn;  // Index of single column requesting auto-fit.
  AnchorTableColumnIdx
    ResizedColumn;  // Index of column being resized. Reset when InstanceCurrent==0.
  AnchorTableColumnIdx LastResizedColumn;  // Index of column being resized from previous frame.
  AnchorTableColumnIdx HeldHeaderColumn;   // Index of column header being held.
  AnchorTableColumnIdx ReorderColumn;      // Index of column being reordered. (not cleared)
  AnchorTableColumnIdx ReorderColumnDir;   // -1 or +1
  AnchorTableColumnIdx LeftMostEnabledColumn;     // Index of left-most non-hidden column.
  AnchorTableColumnIdx RightMostEnabledColumn;    // Index of right-most non-hidden column.
  AnchorTableColumnIdx LeftMostStretchedColumn;   // Index of left-most stretched column.
  AnchorTableColumnIdx RightMostStretchedColumn;  // Index of right-most stretched column.
  AnchorTableColumnIdx ContextPopupColumn;  // Column right-clicked on, of -1 if opening context
                                            // menu from a neutral/empty spot
  AnchorTableColumnIdx FreezeRowsRequest;   // Requested frozen rows count
  AnchorTableColumnIdx FreezeRowsCount;  // Actual frozen row count (== FreezeRowsRequest, or == 0
                                         // when no scrolling offset)
  AnchorTableColumnIdx FreezeColumnsRequest;  // Requested frozen columns count
  AnchorTableColumnIdx
    FreezeColumnsCount;  // Actual frozen columns count (== FreezeColumnsRequest, or == 0
                         // when no scrolling offset)
  AnchorTableColumnIdx RowCellDataCurrent;  // Index of current RowCellData[] entry in current row
  AnchorTableDrawChannelIdx DummyDrawChannel;  // Redirect non-visible columns here.
  AnchorTableDrawChannelIdx
    Bg2DrawChannelCurrent;  // For Selectable() and other widgets drawing across columns after
                            // the freezing line. Index within DrawSplitter.Channels[]
  AnchorTableDrawChannelIdx Bg2DrawChannelUnfrozen;
  bool IsLayoutLocked;  // Set by TableUpdateLayout() which is called when beginning the first row.
  bool IsInsideRow;     // Set when inside TableBeginRow()/TableEndRow().
  bool IsInitializing;
  bool IsSortSpecsDirty;
  bool IsUsingHeaders;      // Set when the first row had the AnchorTableRowFlags_Headers flag.
  bool IsContextPopupOpen;  // Set when default context menu is open (also see: ContextPopupColumn,
                            // InstanceInteracted).
  bool IsSettingsRequestLoad;
  bool IsSettingsDirty;  // Set when table settings have changed and needs to be reported into
                         // AnchorTableSettings data.
  bool IsDefaultDisplayOrder;  // Set when display order is unchanged from default (DisplayOrder
                               // contains 0...Count-1)
  bool IsResetAllRequest;
  bool IsResetDisplayOrderRequest;
  bool IsUnfrozenRows;         // Set when we got past the frozen row.
  bool IsDefaultSizingPolicy;  // Set if user didn't explicitly set a sizing policy in BeginTable()
  bool MemoryCompacted;
  bool HostSkipItems;  // Backup of InnerWindow->SkipItem at the end of BeginTable(), because we
                       // will overwrite InnerWindow->SkipItem on a per-column basis

  ANCHOR_API AnchorTable()
  {
    memset(this, 0, sizeof(*this));
    LastFrameActive = -1;
  }
  ANCHOR_API ~AnchorTable()
  {
    ANCHOR_FREE(RawData);
  }
};

// Transient data that are only needed between BeginTable() and EndTable(), those buffers are
// shared (1 per level of stacked table).
// - Accessing those requires chasing an extra pointer so for very frequently used data we leave
// them in the main table structure.
// - We also leave out of this structure data that tend to be particularly useful for
// debugging/metrics.
// FIXME-TABLE: more transient data could be stored here: DrawSplitter, incoming RowData?
struct AnchorTableTempData
{
  int TableIndex;        // Index in g.Tables.Buf[] pool
  float LastTimeActive;  // Last timestamp this structure was used

  wabi::GfVec2f UserOuterSize;  // outer_size[0] passed to BeginTable()
  AnchorDrawListSplitter DrawSplitter;
  AnchorTableColumnSortSpecs SortSpecsSingle;
  AnchorVector<AnchorTableColumnSortSpecs>
    SortSpecsMulti;  // FIXME-OPT: Using a small-vector pattern would be good.

  AnchorBBox HostBackupWorkRect;  // Backup of InnerWindow->WorkRect at the end of BeginTable()
  AnchorBBox HostBackupParentWorkRect;  // Backup of InnerWindow->ParentWorkRect at the end of
                                        // BeginTable()
  wabi::GfVec2f
    HostBackupPrevLineSize;  // Backup of InnerWindow->DC.PrevLineSize at the end of BeginTable()
  wabi::GfVec2f
    HostBackupCurrLineSize;  // Backup of InnerWindow->DC.CurrLineSize at the end of BeginTable()
  wabi::GfVec2f
    HostBackupCursorMaxPos;  // Backup of InnerWindow->DC.CursorMaxPos at the end of BeginTable()
  GfVec1 HostBackupColumnsOffset;  // Backup of OuterWindow->DC.ColumnsOffset at the end of
                                   // BeginTable()
  float HostBackupItemWidth;  // Backup of OuterWindow->DC.ItemWidth at the end of BeginTable()
  int HostBackupItemWidthStackSize;  // Backup of OuterWindow->DC.ItemWidthStack.Size at the end of
                                     // BeginTable()

  ANCHOR_API AnchorTableTempData()
  {
    memset(this, 0, sizeof(*this));
    LastTimeActive = -1.0f;
  }
};

// sizeof() ~ 12
struct AnchorTableColumnSettings
{
  float WidthOrWeight;
  ANCHOR_ID UserID;
  AnchorTableColumnIdx Index;
  AnchorTableColumnIdx DisplayOrder;
  AnchorTableColumnIdx SortOrder;
  AnchorU8 SortDirection : 2;
  AnchorU8 IsEnabled : 1;  // "Visible" in ini file
  AnchorU8 IsStretch : 1;

  AnchorTableColumnSettings()
  {
    WidthOrWeight = 0.0f;
    UserID = 0;
    Index = -1;
    DisplayOrder = SortOrder = -1;
    SortDirection = AnchorSortDirection_None;
    IsEnabled = 1;
    IsStretch = 0;
  }
};

// This is designed to be stored in a single AnchorChunkStream (1 header followed by N
// AnchorTableColumnSettings, etc.)
struct AnchorTableSettings
{
  ANCHOR_ID ID;  // Set to 0 to invalidate/delete the setting
  AnchorTableFlags
    SaveFlags;  // Indicate data we want to save using the
                // Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
  float RefScale;  // Reference scale to be able to rescale columns on font/dpi changes.
  AnchorTableColumnIdx ColumnsCount;
  AnchorTableColumnIdx
    ColumnsCountMax;  // Maximum number of columns this settings instance can store, we can
                      // recycle a settings instance with lower number of columns but not higher
  bool WantApply;  // Set when loaded from .ini data (to enable merging/loading .ini data into an
                   // already running context)

  AnchorTableSettings()
  {
    memset(this, 0, sizeof(*this));
  }
  AnchorTableColumnSettings *GetColumnSettings()
  {
    return (AnchorTableColumnSettings *)(this + 1);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] ANCHOR internal API
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

namespace ANCHOR
{
  // Windows
  // We should always have a CurrentWindow in the stack (there is an implicit "Debug" window)
  // If this ever crash because g.CurrentWindow is NULL it means that either
  // - ANCHOR::NewFrame() has never been called, which is illegal.
  // - You are calling ANCHOR functions after ANCHOR::EndFrame()/ANCHOR::Render() and before the
  // next ANCHOR::NewFrame(), which is also illegal.
  inline AnchorWindow *GetCurrentWindowRead()
  {
    AnchorContext &g = *G_CTX;
    return g.CurrentWindow;
  }
  inline AnchorWindow *GetCurrentWindow()
  {
    AnchorContext &g = *G_CTX;
    g.CurrentWindow->WriteAccessed = true;
    return g.CurrentWindow;
  }
  ANCHOR_API AnchorWindow *FindWindowByID(ANCHOR_ID id);
  ANCHOR_API AnchorWindow *FindWindowByName(const char *name);
  ANCHOR_API void UpdateWindowParentAndRootLinks(AnchorWindow *window,
                                                 AnchorWindowFlags flags,
                                                 AnchorWindow *parent_window);
  ANCHOR_API wabi::GfVec2f CalcWindowNextAutoFitSize(AnchorWindow *window);
  ANCHOR_API bool IsWindowChildOf(AnchorWindow *window, AnchorWindow *potential_parent);
  ANCHOR_API bool IsWindowAbove(AnchorWindow *potential_above, AnchorWindow *potential_below);
  ANCHOR_API bool IsWindowNavFocusable(AnchorWindow *window);
  ANCHOR_API void SetWindowPos(AnchorWindow *window,
                               const wabi::GfVec2f &pos,
                               AnchorCond cond = 0);
  ANCHOR_API void SetWindowSize(AnchorWindow *window,
                                const wabi::GfVec2f &size,
                                AnchorCond cond = 0);
  ANCHOR_API void SetWindowCollapsed(AnchorWindow *window, bool collapsed, AnchorCond cond = 0);
  ANCHOR_API void SetWindowHitTestHole(AnchorWindow *window,
                                       const wabi::GfVec2f &pos,
                                       const wabi::GfVec2f &size);

  // Windows: Display Order and Focus Order
  ANCHOR_API void FocusWindow(AnchorWindow *window);
  ANCHOR_API void FocusTopMostWindowUnderOne(AnchorWindow *under_this_window,
                                             AnchorWindow *ignore_window);
  ANCHOR_API void BringWindowToFocusFront(AnchorWindow *window);
  ANCHOR_API void BringWindowToDisplayFront(AnchorWindow *window);
  ANCHOR_API void BringWindowToDisplayBack(AnchorWindow *window);

  // Fonts, drawing
  ANCHOR_API void SetCurrentFont(AnchorFont *font);
  inline AnchorFont *GetDefaultFont()
  {
    AnchorContext &g = *G_CTX;
    return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0];
  }
  inline AnchorDrawList *GetForegroundDrawList(AnchorWindow *window)
  {
    TF_UNUSED(window);
    // This seemingly unnecessary wrapper simplifies compatibility between the 'master' and
    // 'docking' branches.
    return GetForegroundDrawList();
  }

  // get background draw list for the given viewport. this draw list
  // will be the first rendering one. Useful to quickly draw
  // shapes/text behind ANCHOR contents.
  ANCHOR_API AnchorDrawList *GetBackgroundDrawList(AnchorViewport *viewport);
  // get foreground draw list for the given viewport. this draw list will be the
  // last rendered one. Useful to quickly draw shapes/text over ANCHOR contents.
  ANCHOR_API AnchorDrawList *GetForegroundDrawList(AnchorViewport *viewport);

  // Init
  ANCHOR_API void Initialize(AnchorContext *context);
  ANCHOR_API void Shutdown(AnchorContext *context);  // Since 1.60 this is a _private_ function.
                                                     // You can call DestroyContext() to destroy
                                                     // the context created by CreateContext().

  // NewFrame
  ANCHOR_API void UpdateHoveredWindowAndCaptureFlags();
  ANCHOR_API void StartMouseMovingWindow(AnchorWindow *window);
  ANCHOR_API void UpdateMouseMovingWindowNewFrame();
  ANCHOR_API void UpdateMouseMovingWindowEndFrame();

  // Generic context hooks
  ANCHOR_API ANCHOR_ID AddContextHook(AnchorContext *context, const AnchorContextHook *hook);
  ANCHOR_API void RemoveContextHook(AnchorContext *context, ANCHOR_ID hook_to_remove);
  ANCHOR_API void CallContextHooks(AnchorContext *context, AnchorContextHookType type);

  // Settings
  ANCHOR_API void MarkIniSettingsDirty();
  ANCHOR_API void MarkIniSettingsDirty(AnchorWindow *window);
  ANCHOR_API void ClearIniSettings();
  ANCHOR_API AnchorWindowSettings *CreateNewWindowSettings(const char *name);
  ANCHOR_API AnchorWindowSettings *FindWindowSettings(ANCHOR_ID id);
  ANCHOR_API AnchorWindowSettings *FindOrCreateWindowSettings(const char *name);
  ANCHOR_API AnchorSettingsHandler *FindSettingsHandler(const char *type_name);

  // Scrolling
  ANCHOR_API void SetNextWindowScroll(
    const wabi::GfVec2f &scroll);  // Use -1.0f on one axis to leave as-is
  ANCHOR_API void SetScrollX(AnchorWindow *window, float scroll_x);
  ANCHOR_API void SetScrollY(AnchorWindow *window, float scroll_y);
  ANCHOR_API void SetScrollFromPosX(AnchorWindow *window, float local_x, float center_x_ratio);
  ANCHOR_API void SetScrollFromPosY(AnchorWindow *window, float local_y, float center_y_ratio);
  ANCHOR_API wabi::GfVec2f ScrollToBringRectIntoView(AnchorWindow *window,
                                                     const AnchorBBox &item_rect);

  // Basic Accessors
  inline ANCHOR_ID GetItemID()
  {
    AnchorContext &g = *G_CTX;
    return g.CurrentWindow->DC.LastItemId;
  }  // Get ID of last item (~~ often same ANCHOR::GetID(label) beforehand)
  inline AnchorItemStatusFlags GetItemStatusFlags()
  {
    AnchorContext &g = *G_CTX;
    return g.CurrentWindow->DC.LastItemStatusFlags;
  }
  inline ANCHOR_ID GetActiveID()
  {
    AnchorContext &g = *G_CTX;
    return g.ActiveId;
  }
  inline ANCHOR_ID GetFocusID()
  {
    AnchorContext &g = *G_CTX;
    return g.NavId;
  }
  inline AnchorItemFlags GetItemFlags()
  {
    AnchorContext &g = *G_CTX;
    return g.CurrentItemFlags;
  }
  ANCHOR_API void SetActiveID(ANCHOR_ID id, AnchorWindow *window);
  ANCHOR_API void SetFocusID(ANCHOR_ID id, AnchorWindow *window);
  ANCHOR_API void ClearActiveID();
  ANCHOR_API ANCHOR_ID GetHoveredID();
  ANCHOR_API void SetHoveredID(ANCHOR_ID id);
  ANCHOR_API void KeepAliveID(ANCHOR_ID id);

  // Mark data associated to given item as "edited",
  // used by IsItemDeactivatedAfterEdit() function.
  ANCHOR_API void MarkItemEdited(ANCHOR_ID id);

  // Push given value as-is at the top of the ID stack
  // (whereas PushID combines old and new hashes)
  ANCHOR_API void PushOverrideID(ANCHOR_ID id);
  ANCHOR_API ANCHOR_ID GetIDWithSeed(const char *str_id_begin,
                                     const char *str_id_end,
                                     ANCHOR_ID seed);

  // Basic Helpers for widget code
  ANCHOR_API void ItemSize(const wabi::GfVec2f &size, float text_baseline_y = -1.0f);
  ANCHOR_API void ItemSize(const AnchorBBox &bb, float text_baseline_y = -1.0f);
  ANCHOR_API bool ItemAdd(const AnchorBBox &bb,
                          ANCHOR_ID id,
                          const AnchorBBox *nav_bb = NULL,
                          AnchorItemAddFlags flags = 0);
  ANCHOR_API bool ItemHoverable(const AnchorBBox &bb, ANCHOR_ID id);
  ANCHOR_API void ItemFocusable(AnchorWindow *window, ANCHOR_ID id);
  ANCHOR_API bool IsClippedEx(const AnchorBBox &bb, ANCHOR_ID id, bool clip_even_when_logged);
  ANCHOR_API void SetLastItemData(AnchorWindow *window,
                                  ANCHOR_ID item_id,
                                  AnchorItemStatusFlags status_flags,
                                  const AnchorBBox &item_rect);
  ANCHOR_API wabi::GfVec2f CalcItemSize(wabi::GfVec2f size, float default_w, float default_h);
  ANCHOR_API float CalcWrapWidthForPos(const wabi::GfVec2f &pos, float wrap_pos_x);
  ANCHOR_API void PushMultiItemsWidths(int components, float width_full);
  ANCHOR_API void PushItemFlag(AnchorItemFlags option, bool enabled);
  ANCHOR_API void PopItemFlag();

  // Was the last item selection toggled? (after
  // Selectable(), TreeNode() etc. We only returns toggle
  // _event_ in order to handle clipping correctly)
  ANCHOR_API bool IsItemToggledSelection();
  ANCHOR_API wabi::GfVec2f GetContentRegionMaxAbs();
  ANCHOR_API void ShrinkWidths(ANCHOR_ShrinkWidthItem *items, int count, float width_excess);

#ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS

  inline bool FocusableItemRegister(AnchorWindow *window, ANCHOR_ID id)
  {
    ANCHOR_ASSERT(0);
    TF_UNUSED(window);
    TF_UNUSED(id);
    return false;
  }  // -> pass AnchorItemAddFlags_Focusable flag to ItemAdd()
  inline void FocusableItemUnregister(AnchorWindow *window)
  {
    ANCHOR_ASSERT(0);
    TF_UNUSED(window);
  }  // -> unnecessary: TempInputText() uses AnchorInputTextFlags_MergedItem
#endif

  // Logging/Capture
  // -> BeginCapture() when we design v2 api, for now
  // stay under the radar by using the old name.
  ANCHOR_API void LogBegin(ANCHORLogType type, int auto_open_depth);
  // Start logging/capturing to internal buffer
  ANCHOR_API void LogToBuffer(int auto_open_depth = -1);
  ANCHOR_API void LogRenderedText(const wabi::GfVec2f *ref_pos,
                                  const char *text,
                                  const char *text_end = NULL);
  ANCHOR_API void LogSetNextTextDecoration(const char *prefix, const char *suffix);

  // Popups, Modals, Tooltips
  ANCHOR_API bool BeginChildEx(const char *name,
                               ANCHOR_ID id,
                               const wabi::GfVec2f &size_arg,
                               bool border,
                               AnchorWindowFlags flags);
  ANCHOR_API void OpenPopupEx(ANCHOR_ID id, AnchorPopupFlags popup_flags = AnchorPopupFlags_None);
  ANCHOR_API void ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
  ANCHOR_API void ClosePopupsOverWindow(AnchorWindow *ref_window,
                                        bool restore_focus_to_window_under_popup);
  ANCHOR_API bool IsPopupOpen(ANCHOR_ID id, AnchorPopupFlags popup_flags);
  ANCHOR_API bool BeginPopupEx(ANCHOR_ID id, AnchorWindowFlags extra_flags);
  ANCHOR_API void BeginTooltipEx(AnchorWindowFlags extra_flags, AnchorTooltipFlags tooltip_flags);
  ANCHOR_API AnchorBBox GetPopupAllowedExtentRect(AnchorWindow *window);
  ANCHOR_API AnchorWindow *GetTopMostPopupModal();
  ANCHOR_API wabi::GfVec2f FindBestWindowPosForPopup(AnchorWindow *window);
  ANCHOR_API wabi::GfVec2f FindBestWindowPosForPopupEx(const wabi::GfVec2f &ref_pos,
                                                       const wabi::GfVec2f &size,
                                                       AnchorDir *last_dir,
                                                       const AnchorBBox &r_outer,
                                                       const AnchorBBox &r_avoid,
                                                       ANCHORPopupPositionPolicy policy);
  ANCHOR_API bool BeginViewportSideBar(const char *name,
                                       AnchorViewport *viewport,
                                       AnchorDir dir,
                                       float size,
                                       AnchorWindowFlags window_flags);

  // Combos
  ANCHOR_API bool BeginComboPopup(ANCHOR_ID popup_id,
                                  const AnchorBBox &bb,
                                  AnchorComboFlags flags);

  // Gamepad/Keyboard Navigation
  ANCHOR_API void NavInitWindow(AnchorWindow *window, bool force_reinit);
  ANCHOR_API bool NavMoveRequestButNoResultYet();
  ANCHOR_API void NavMoveRequestCancel();
  ANCHOR_API void NavMoveRequestForward(AnchorDir move_dir,
                                        AnchorDir clip_dir,
                                        const AnchorBBox &bb_rel,
                                        AnchorNavMoveFlags move_flags);
  ANCHOR_API void NavMoveRequestTryWrapping(AnchorWindow *window, AnchorNavMoveFlags move_flags);
  ANCHOR_API float GetNavInputAmount(AnchorNavInput n, ANCHOR_InputReadMode mode);
  ANCHOR_API wabi::GfVec2f GetNavInputAmount2d(AnchorNavDirSourceFlags dir_sources,
                                               ANCHOR_InputReadMode mode,
                                               float slow_factor = 0.0f,
                                               float fast_factor = 0.0f);
  ANCHOR_API int CalcTypematicRepeatAmount(float t0,
                                           float t1,
                                           float repeat_delay,
                                           float repeat_rate);
  // Remotely activate a button, checkbox, tree node etc. given its unique ID.
  // activation is queued and processed on the next frame when the item is
  // encountered again.
  ANCHOR_API void ActivateItem(ANCHOR_ID id);
  ANCHOR_API void SetNavID(ANCHOR_ID id,
                           ANCHORNavLayer nav_layer,
                           ANCHOR_ID focus_scope_id,
                           const AnchorBBox &rect_rel);

  // Focus Scope (WIP)
  // This is generally used to identify a selection set (multiple of which may be in the same
  // window), as selection patterns generally need to react (e.g. clear selection) when landing on
  // an item of the set.
  ANCHOR_API void PushFocusScope(ANCHOR_ID id);
  ANCHOR_API void PopFocusScope();
  inline ANCHOR_ID GetFocusedFocusScope()
  {
    AnchorContext &g = *G_CTX;
    return g.NavFocusScopeId;
  }  // Focus scope which is actually active
  inline ANCHOR_ID GetFocusScope()
  {
    AnchorContext &g = *G_CTX;
    return g.CurrentWindow->DC.NavFocusScopeIdCurrent;
  }  // Focus scope we are outputting into, set by PushFocusScope()

  // Inputs
  // FIXME: Eventually we should aim to move e.g. IsActiveIdUsingKey() into IsKeyXXX functions.
  ANCHOR_API void SetItemUsingMouseWheel();
  inline bool IsActiveIdUsingNavDir(AnchorDir dir)
  {
    AnchorContext &g = *G_CTX;
    return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0;
  }
  inline bool IsActiveIdUsingNavInput(AnchorNavInput input)
  {
    AnchorContext &g = *G_CTX;
    return (g.ActiveIdUsingNavInputMask & (1 << input)) != 0;
  }
  inline bool IsActiveIdUsingKey(AnchorKey key)
  {
    AnchorContext &g = *G_CTX;
    ANCHOR_ASSERT(key < 64);
    return (g.ActiveIdUsingKeyInputMask & ((AnchorU64)1 << key)) != 0;
  }
  ANCHOR_API bool IsMouseDragPastThreshold(AnchorMouseButton button, float lock_threshold = -1.0f);
  inline bool IsKeyPressedMap(AnchorKey key, bool repeat = true)
  {
    AnchorContext &g = *G_CTX;
    const int key_index = g.IO.KeyMap[key];
    return (key_index >= 0) ? IsKeyPressed(key_index, repeat) : false;
  }
  inline bool IsNavInputDown(AnchorNavInput n)
  {
    AnchorContext &g = *G_CTX;
    return g.IO.NavInputs[n] > 0.0f;
  }
  inline bool IsNavInputTest(AnchorNavInput n, ANCHOR_InputReadMode rm)
  {
    return (GetNavInputAmount(n, rm) > 0.0f);
  }
  ANCHOR_API AnchorKeyModFlags GetMergedKeyModFlags();

  // Drag and Drop
  ANCHOR_API bool BeginDragDropTargetCustom(const AnchorBBox &bb, ANCHOR_ID id);
  ANCHOR_API void ClearDragDrop();
  ANCHOR_API bool IsDragDropPayloadBeingAccepted();

  // Internal Columns API (this is not exposed because we will encourage transitioning to the
  // Tables API)
  ANCHOR_API void SetWindowClipRectBeforeSetChannel(AnchorWindow *window,
                                                    const AnchorBBox &clip_rect);
  ANCHOR_API void BeginColumns(
    const char *str_id,
    int count,
    AnchorOldColumnFlags flags = 0);  // setup number of columns. use an identifier to distinguish
                                      // multiple column sets. close with EndColumns().
  ANCHOR_API void EndColumns();       // close columns
  ANCHOR_API void PushColumnClipRect(int column_index);
  ANCHOR_API void PushColumnsBackground();
  ANCHOR_API void PopColumnsBackground();
  ANCHOR_API ANCHOR_ID GetColumnsID(const char *str_id, int count);
  ANCHOR_API AnchorOldColumns *FindOrCreateColumns(AnchorWindow *window, ANCHOR_ID id);
  ANCHOR_API float GetColumnOffsetFromNorm(const AnchorOldColumns *columns, float offset_norm);
  ANCHOR_API float GetColumnNormFromOffset(const AnchorOldColumns *columns, float offset);

  // Tables: Candidates for public API
  ANCHOR_API void TableOpenContextMenu(int column_n = -1);
  ANCHOR_API void TableSetColumnWidth(int column_n, float width);
  ANCHOR_API void TableSetColumnSortDirection(int column_n,
                                              AnchorSortDirection sort_direction,
                                              bool append_to_sort_specs);
  ANCHOR_API int TableGetHoveredColumn();  // May use (TableGetColumnFlags() &
                                           // AnchorTableColumnFlags_IsHovered) instead. Return
                                           // hovered column. return -1 when table is not hovered.
                                           // return columns_count if the unused space at the right
                                           // of visible columns is hovered.
  ANCHOR_API float TableGetHeaderRowHeight();
  ANCHOR_API void TablePushBackgroundChannel();
  ANCHOR_API void TablePopBackgroundChannel();

  // Tables: Internals
  inline AnchorTable *GetCurrentTable()
  {
    AnchorContext &g = *G_CTX;
    return g.CurrentTable;
  }
  ANCHOR_API AnchorTable *TableFindByID(ANCHOR_ID id);
  ANCHOR_API bool BeginTableEx(const char *name,
                               ANCHOR_ID id,
                               int columns_count,
                               AnchorTableFlags flags = 0,
                               const wabi::GfVec2f &outer_size = wabi::GfVec2f(0, 0),
                               float inner_width = 0.0f);
  ANCHOR_API void TableBeginInitMemory(AnchorTable *table, int columns_count);
  ANCHOR_API void TableBeginApplyRequests(AnchorTable *table);
  ANCHOR_API void TableSetupDrawChannels(AnchorTable *table);
  ANCHOR_API void TableUpdateLayout(AnchorTable *table);
  ANCHOR_API void TableUpdateBorders(AnchorTable *table);
  ANCHOR_API void TableUpdateColumnsWeightFromWidth(AnchorTable *table);
  ANCHOR_API void TableDrawBorders(AnchorTable *table);
  ANCHOR_API void TableDrawContextMenu(AnchorTable *table);
  ANCHOR_API void TableMergeDrawChannels(AnchorTable *table);
  ANCHOR_API void TableSortSpecsSanitize(AnchorTable *table);
  ANCHOR_API void TableSortSpecsBuild(AnchorTable *table);
  ANCHOR_API AnchorSortDirection TableGetColumnNextSortDirection(AnchorTableColumn *column);
  ANCHOR_API void TableFixColumnSortDirection(AnchorTable *table, AnchorTableColumn *column);
  ANCHOR_API float TableGetColumnWidthAuto(AnchorTable *table, AnchorTableColumn *column);
  ANCHOR_API void TableBeginRow(AnchorTable *table);
  ANCHOR_API void TableEndRow(AnchorTable *table);
  ANCHOR_API void TableBeginCell(AnchorTable *table, int column_n);
  ANCHOR_API void TableEndCell(AnchorTable *table);
  ANCHOR_API AnchorBBox TableGetCellBgRect(const AnchorTable *table, int column_n);
  ANCHOR_API const char *TableGetColumnName(const AnchorTable *table, int column_n);
  ANCHOR_API ANCHOR_ID TableGetColumnResizeID(const AnchorTable *table,
                                              int column_n,
                                              int instance_no = 0);
  ANCHOR_API float TableGetMaxColumnWidth(const AnchorTable *table, int column_n);
  ANCHOR_API void TableSetColumnWidthAutoSingle(AnchorTable *table, int column_n);
  ANCHOR_API void TableSetColumnWidthAutoAll(AnchorTable *table);
  ANCHOR_API void TableRemove(AnchorTable *table);
  ANCHOR_API void TableGcCompactTransientBuffers(AnchorTable *table);
  ANCHOR_API void TableGcCompactTransientBuffers(AnchorTableTempData *table);
  ANCHOR_API void TableGcCompactSettings();

  // Tables: Settings
  ANCHOR_API void TableLoadSettings(AnchorTable *table);
  ANCHOR_API void TableSaveSettings(AnchorTable *table);
  ANCHOR_API void TableResetSettings(AnchorTable *table);
  ANCHOR_API AnchorTableSettings *TableGetBoundSettings(AnchorTable *table);
  ANCHOR_API void TableSettingsInstallHandler(AnchorContext *context);
  ANCHOR_API AnchorTableSettings *TableSettingsCreate(ANCHOR_ID id, int columns_count);
  ANCHOR_API AnchorTableSettings *TableSettingsFindByID(ANCHOR_ID id);

  // Tab Bars
  ANCHOR_API bool BeginTabBarEx(AnchorTabBar *tab_bar,
                                const AnchorBBox &bb,
                                AnchorTabBarFlags flags);
  ANCHOR_API AnchorTabItem *TabBarFindTabByID(AnchorTabBar *tab_bar, ANCHOR_ID tab_id);
  ANCHOR_API void TabBarRemoveTab(AnchorTabBar *tab_bar, ANCHOR_ID tab_id);
  ANCHOR_API void TabBarCloseTab(AnchorTabBar *tab_bar, AnchorTabItem *tab);
  ANCHOR_API void TabBarQueueReorder(AnchorTabBar *tab_bar, const AnchorTabItem *tab, int offset);
  ANCHOR_API void TabBarQueueReorderFromMousePos(AnchorTabBar *tab_bar,
                                                 const AnchorTabItem *tab,
                                                 wabi::GfVec2f mouse_pos);
  ANCHOR_API bool TabBarProcessReorder(AnchorTabBar *tab_bar);
  ANCHOR_API bool TabItemEx(AnchorTabBar *tab_bar,
                            const char *label,
                            bool *p_open,
                            AnchorTabItemFlags flags);
  ANCHOR_API wabi::GfVec2f TabItemCalcSize(const char *label, bool has_close_button);
  ANCHOR_API void TabItemBackground(AnchorDrawList *draw_list,
                                    const AnchorBBox &bb,
                                    AnchorTabItemFlags flags,
                                    AnchorU32 col);
  ANCHOR_API void TabItemLabelAndCloseButton(AnchorDrawList *draw_list,
                                             const AnchorBBox &bb,
                                             AnchorTabItemFlags flags,
                                             wabi::GfVec2f frame_padding,
                                             const char *label,
                                             ANCHOR_ID tab_id,
                                             ANCHOR_ID close_button_id,
                                             bool is_contents_visible,
                                             bool *out_just_closed,
                                             bool *out_text_clipped);

  // Render helpers
  // AVOID USING OUTSIDE OF ANCHOR.CPP! NOT FOR PUBLIC CONSUMPTION. THOSE FUNCTIONS ARE A MESS.
  // THEIR SIGNATURE AND BEHAVIOR WILL CHANGE, THEY NEED TO BE REFACTORED INTO SOMETHING DECENT.
  // NB: All position are in absolute pixels coordinates (we are never using window coordinates
  // internally)
  ANCHOR_API void RenderText(wabi::GfVec2f pos,
                             const char *text,
                             const char *text_end = NULL,
                             bool hide_text_after_hash = true);
  ANCHOR_API void RenderTextWrapped(wabi::GfVec2f pos,
                                    const char *text,
                                    const char *text_end,
                                    float wrap_width);
  ANCHOR_API void RenderTextClipped(const wabi::GfVec2f &pos_min,
                                    const wabi::GfVec2f &pos_max,
                                    const char *text,
                                    const char *text_end,
                                    const wabi::GfVec2f *text_size_if_known,
                                    const wabi::GfVec2f &align = wabi::GfVec2f(0, 0),
                                    const AnchorBBox *clip_rect = NULL);
  ANCHOR_API void RenderTextClippedEx(AnchorDrawList *draw_list,
                                      const wabi::GfVec2f &pos_min,
                                      const wabi::GfVec2f &pos_max,
                                      const char *text,
                                      const char *text_end,
                                      const wabi::GfVec2f *text_size_if_known,
                                      const wabi::GfVec2f &align = wabi::GfVec2f(0, 0),
                                      const AnchorBBox *clip_rect = NULL);
  ANCHOR_API void RenderTextEllipsis(AnchorDrawList *draw_list,
                                     const wabi::GfVec2f &pos_min,
                                     const wabi::GfVec2f &pos_max,
                                     float clip_max_x,
                                     float ellipsis_max_x,
                                     const char *text,
                                     const char *text_end,
                                     const wabi::GfVec2f *text_size_if_known);
  ANCHOR_API void RenderFrame(wabi::GfVec2f p_min,
                              wabi::GfVec2f p_max,
                              AnchorU32 fill_col,
                              bool border = true,
                              float rounding = 0.0f);
  ANCHOR_API void RenderFrameBorder(wabi::GfVec2f p_min,
                                    wabi::GfVec2f p_max,
                                    float rounding = 0.0f);
  ANCHOR_API void RenderColorRectWithAlphaCheckerboard(AnchorDrawList *draw_list,
                                                       wabi::GfVec2f p_min,
                                                       wabi::GfVec2f p_max,
                                                       AnchorU32 fill_col,
                                                       float grid_step,
                                                       wabi::GfVec2f grid_off,
                                                       float rounding = 0.0f,
                                                       AnchorDrawFlags flags = 0);
  ANCHOR_API void RenderNavHighlight(
    const AnchorBBox &bb,
    ANCHOR_ID id,
    AnchorNavHighlightFlags flags = AnchorNavHighlightFlags_TypeDefault);  // Navigation highlight
  ANCHOR_API const char *FindRenderedTextEnd(
    const char *text,
    const char *text_end = NULL);  // Find the optional ## from which we stop displaying text.

  // Render helpers (those functions don't access any ANCHOR state!)
  ANCHOR_API void RenderArrow(AnchorDrawList *draw_list,
                              wabi::GfVec2f pos,
                              AnchorU32 col,
                              AnchorDir dir,
                              float scale = 1.0f);
  ANCHOR_API void RenderBullet(AnchorDrawList *draw_list, wabi::GfVec2f pos, AnchorU32 col);
  ANCHOR_API void RenderCheckMark(AnchorDrawList *draw_list,
                                  wabi::GfVec2f pos,
                                  AnchorU32 col,
                                  float sz);
  ANCHOR_API void RenderMouseCursor(AnchorDrawList *draw_list,
                                    wabi::GfVec2f pos,
                                    float scale,
                                    AnchorMouseCursor mouse_cursor,
                                    AnchorU32 col_fill,
                                    AnchorU32 col_border,
                                    AnchorU32 col_shadow);
  ANCHOR_API void RenderArrowPointingAt(AnchorDrawList *draw_list,
                                        wabi::GfVec2f pos,
                                        wabi::GfVec2f half_sz,
                                        AnchorDir direction,
                                        AnchorU32 col);
  ANCHOR_API void RenderRectFilledRangeH(AnchorDrawList *draw_list,
                                         const AnchorBBox &rect,
                                         AnchorU32 col,
                                         float x_start_norm,
                                         float x_end_norm,
                                         float rounding);
  ANCHOR_API void RenderRectFilledWithHole(AnchorDrawList *draw_list,
                                           AnchorBBox outer,
                                           AnchorBBox inner,
                                           AnchorU32 col,
                                           float rounding);

#ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  // [1.71: 2019/06/07: Updating prototypes of some of the internal functions. Leaving those for
  // reference for a short while]
  inline void RenderArrow(wabi::GfVec2f pos, AnchorDir dir, float scale = 1.0f)
  {
    AnchorWindow *window = GetCurrentWindow();
    RenderArrow(window->DrawList, pos, GetColorU32(AnchorCol_Text), dir, scale);
  }
  inline void RenderBullet(wabi::GfVec2f pos)
  {
    AnchorWindow *window = GetCurrentWindow();
    RenderBullet(window->DrawList, pos, GetColorU32(AnchorCol_Text));
  }
#endif

  // Widgets
  ANCHOR_API void TextEx(const char *text, const char *text_end = NULL, AnchorTextFlags flags = 0);
  ANCHOR_API bool ButtonEx(const char *label,
                           const wabi::GfVec2f &size_arg = wabi::GfVec2f(0, 0),
                           AnchorButtonFlags flags = 0);
  ANCHOR_API bool CloseButton(ANCHOR_ID id, const wabi::GfVec2f &pos);
  ANCHOR_API bool CollapseButton(ANCHOR_ID id, const wabi::GfVec2f &pos);
  ANCHOR_API bool ArrowButtonEx(const char *str_id,
                                AnchorDir dir,
                                wabi::GfVec2f size_arg,
                                AnchorButtonFlags flags = 0);
  ANCHOR_API void Scrollbar(ANCHOR_Axis axis);
  ANCHOR_API bool ScrollbarEx(const AnchorBBox &bb,
                              ANCHOR_ID id,
                              ANCHOR_Axis axis,
                              float *p_scroll_v,
                              float avail_v,
                              float contents_v,
                              AnchorDrawFlags flags);
  ANCHOR_API bool ImageButtonEx(ANCHOR_ID id,
                                AnchorTextureID texture_id,
                                const wabi::GfVec2f &size,
                                const wabi::GfVec2f &uv0,
                                const wabi::GfVec2f &uv1,
                                const wabi::GfVec2f &padding,
                                const wabi::GfVec4f &bg_col,
                                const wabi::GfVec4f &tint_col);
  ANCHOR_API AnchorBBox GetWindowScrollbarRect(AnchorWindow *window, ANCHOR_Axis axis);
  ANCHOR_API ANCHOR_ID GetWindowScrollbarID(AnchorWindow *window, ANCHOR_Axis axis);
  ANCHOR_API ANCHOR_ID GetWindowResizeCornerID(AnchorWindow *window, int n);  // 0..3: corners
  ANCHOR_API ANCHOR_ID GetWindowResizeBorderID(AnchorWindow *window, AnchorDir dir);
  ANCHOR_API void SeparatorEx(AnchorSeparatorFlags flags);
  ANCHOR_API bool CheckboxFlags(const char *label, AnchorS64 *flags, AnchorS64 flags_value);
  ANCHOR_API bool CheckboxFlags(const char *label, AnchorU64 *flags, AnchorU64 flags_value);

  // Widgets low-level behaviors
  ANCHOR_API bool ButtonBehavior(const AnchorBBox &bb,
                                 ANCHOR_ID id,
                                 bool *out_hovered,
                                 bool *out_held,
                                 AnchorButtonFlags flags = 0);
  ANCHOR_API bool DragBehavior(ANCHOR_ID id,
                               AnchorDataType data_type,
                               void *p_v,
                               float v_speed,
                               const void *p_min,
                               const void *p_max,
                               const char *format,
                               AnchorSliderFlags flags);
  ANCHOR_API bool SliderBehavior(const AnchorBBox &bb,
                                 ANCHOR_ID id,
                                 AnchorDataType data_type,
                                 void *p_v,
                                 const void *p_min,
                                 const void *p_max,
                                 const char *format,
                                 AnchorSliderFlags flags,
                                 AnchorBBox *out_grab_bb);
  ANCHOR_API bool SplitterBehavior(const AnchorBBox &bb,
                                   ANCHOR_ID id,
                                   ANCHOR_Axis axis,
                                   float *size1,
                                   float *size2,
                                   float min_size1,
                                   float min_size2,
                                   float hover_extend = 0.0f,
                                   float hover_visibility_delay = 0.0f);
  ANCHOR_API bool TreeNodeBehavior(ANCHOR_ID id,
                                   AnchorTreeNodeFlags flags,
                                   const char *label,
                                   const char *label_end = NULL);
  // Consume previous SetNextItemOpen() data, if any. May return true when logging
  ANCHOR_API bool TreeNodeBehaviorIsOpen(ANCHOR_ID id, AnchorTreeNodeFlags flags = 0);
  ANCHOR_API void TreePushOverrideID(ANCHOR_ID id);

  // Template functions are instantiated in ANCHOR_widgets.cpp for a finite number of types.
  // To use them externally (for custom widget) you may need an "extern template" statement in your
  // code in order to link to existing instances and silence Clang warnings (see #2036). e.g. "
  // extern template ANCHOR_API float RoundScalarWithFormatT<float, float>(const char* format,
  // AnchorDataType data_type, float v); "
  template<typename T, typename SIGNED_T, typename FLOAT_T>
  ANCHOR_API float ScaleRatioFromValueT(AnchorDataType data_type,
                                        T v,
                                        T v_min,
                                        T v_max,
                                        bool is_logarithmic,
                                        float logarithmic_zero_epsilon,
                                        float zero_deadzone_size);
  template<typename T, typename SIGNED_T, typename FLOAT_T>
  ANCHOR_API T ScaleValueFromRatioT(AnchorDataType data_type,
                                    float t,
                                    T v_min,
                                    T v_max,
                                    bool is_logarithmic,
                                    float logarithmic_zero_epsilon,
                                    float zero_deadzone_size);
  template<typename T, typename SIGNED_T, typename FLOAT_T>
  ANCHOR_API bool DragBehaviorT(AnchorDataType data_type,
                                T *v,
                                float v_speed,
                                T v_min,
                                T v_max,
                                const char *format,
                                AnchorSliderFlags flags);
  template<typename T, typename SIGNED_T, typename FLOAT_T>
  ANCHOR_API bool SliderBehaviorT(const AnchorBBox &bb,
                                  ANCHOR_ID id,
                                  AnchorDataType data_type,
                                  T *v,
                                  T v_min,
                                  T v_max,
                                  const char *format,
                                  AnchorSliderFlags flags,
                                  AnchorBBox *out_grab_bb);
  template<typename T, typename SIGNED_T>
  ANCHOR_API T RoundScalarWithFormatT(const char *format, AnchorDataType data_type, T v);
  template<typename T> ANCHOR_API bool CheckboxFlagsT(const char *label, T *flags, T flags_value);

  // Data type helpers
  ANCHOR_API const AnchorDataTypeInfo *DataTypeGetInfo(AnchorDataType data_type);
  ANCHOR_API int DataTypeFormatString(char *buf,
                                      int buf_size,
                                      AnchorDataType data_type,
                                      const void *p_data,
                                      const char *format);
  ANCHOR_API void DataTypeApplyOp(AnchorDataType data_type,
                                  int op,
                                  void *output,
                                  const void *arg_1,
                                  const void *arg_2);
  ANCHOR_API bool DataTypeApplyOpFromText(const char *buf,
                                          const char *initial_value_buf,
                                          AnchorDataType data_type,
                                          void *p_data,
                                          const char *format);
  ANCHOR_API int DataTypeCompare(AnchorDataType data_type, const void *arg_1, const void *arg_2);
  ANCHOR_API bool DataTypeClamp(AnchorDataType data_type,
                                void *p_data,
                                const void *p_min,
                                const void *p_max);

  // InputText
  ANCHOR_API bool InputTextEx(const char *label,
                              const char *hint,
                              char *buf,
                              int buf_size,
                              const wabi::GfVec2f &size_arg,
                              AnchorInputTextFlags flags,
                              ANCHORInputTextCallback callback = NULL,
                              void *user_data = NULL);
  ANCHOR_API bool TempInputText(const AnchorBBox &bb,
                                ANCHOR_ID id,
                                const char *label,
                                char *buf,
                                int buf_size,
                                AnchorInputTextFlags flags);
  ANCHOR_API bool TempInputScalar(const AnchorBBox &bb,
                                  ANCHOR_ID id,
                                  const char *label,
                                  AnchorDataType data_type,
                                  void *p_data,
                                  const char *format,
                                  const void *p_clamp_min = NULL,
                                  const void *p_clamp_max = NULL);
  inline bool TempInputIsActive(ANCHOR_ID id)
  {
    AnchorContext &g = *G_CTX;
    return (g.ActiveId == id && g.TempInputId == id);
  }
  inline AnchorInputTextState *GetInputTextState(ANCHOR_ID id)
  {
    AnchorContext &g = *G_CTX;
    return (g.InputTextState.ID == id) ? &g.InputTextState : NULL;
  }  // Get input text state if active

  // Color
  ANCHOR_API void ColorTooltip(const char *text, const float *col, AnchorColorEditFlags flags);
  ANCHOR_API void ColorEditOptionsPopup(const float *col, AnchorColorEditFlags flags);
  ANCHOR_API void ColorPickerOptionsPopup(const float *ref_col, AnchorColorEditFlags flags);

  // Plot
  ANCHOR_API int PlotEx(ANCHORPlotType plot_type,
                        const char *label,
                        float (*values_getter)(void *data, int idx),
                        void *data,
                        int values_count,
                        int values_offset,
                        const char *overlay_text,
                        float scale_min,
                        float scale_max,
                        wabi::GfVec2f frame_size);

  // Shade functions (write over already created vertices)
  ANCHOR_API void ShadeVertsLinearColorGradientKeepAlpha(AnchorDrawList *draw_list,
                                                         int vert_start_idx,
                                                         int vert_end_idx,
                                                         wabi::GfVec2f gradient_p0,
                                                         wabi::GfVec2f gradient_p1,
                                                         AnchorU32 col0,
                                                         AnchorU32 col1);
  ANCHOR_API void ShadeVertsLinearUV(AnchorDrawList *draw_list,
                                     int vert_start_idx,
                                     int vert_end_idx,
                                     const wabi::GfVec2f &a,
                                     const wabi::GfVec2f &b,
                                     const wabi::GfVec2f &uv_a,
                                     const wabi::GfVec2f &uv_b,
                                     bool clamp);

  // Garbage collection
  ANCHOR_API void GcCompactTransientMiscBuffers();
  ANCHOR_API void GcCompactTransientWindowBuffers(AnchorWindow *window);
  ANCHOR_API void GcAwakeTransientWindowBuffers(AnchorWindow *window);

  // Debug Tools
  ANCHOR_API void ErrorCheckEndFrameRecover(AnchorErrorLogCallback log_callback,
                                            void *user_data = NULL);
  inline void DebugDrawItemRect(AnchorU32 col = ANCHOR_COL32(255, 0, 0, 255))
  {
    AnchorContext &g = *G_CTX;
    AnchorWindow *window = g.CurrentWindow;
    GetForegroundDrawList(window)->AddRect(window->DC.LastItemRect.Min,
                                           window->DC.LastItemRect.Max,
                                           col);
  }
  inline void DebugStartItemPicker()
  {
    AnchorContext &g = *G_CTX;
    g.DebugItemPickerActive = true;
  }

  ANCHOR_API void ShowFontAtlas(AnchorFontAtlas *atlas);
  ANCHOR_API void DebugNodeColumns(AnchorOldColumns *columns);
  ANCHOR_API void DebugNodeDrawList(AnchorWindow *window,
                                    const AnchorDrawList *draw_list,
                                    const char *label);
  ANCHOR_API void DebugNodeDrawCmdShowMeshAndBoundingBox(AnchorDrawList *out_draw_list,
                                                         const AnchorDrawList *draw_list,
                                                         const AnchorDrawCmd *draw_cmd,
                                                         bool show_mesh,
                                                         bool show_aabb);
  ANCHOR_API void DebugNodeFont(AnchorFont *font);
  ANCHOR_API void DebugNodeStorage(AnchorStorage *storage, const char *label);
  ANCHOR_API void DebugNodeTabBar(AnchorTabBar *tab_bar, const char *label);
  ANCHOR_API void DebugNodeTable(AnchorTable *table);
  ANCHOR_API void DebugNodeTableSettings(AnchorTableSettings *settings);
  ANCHOR_API void DebugNodeWindow(AnchorWindow *window, const char *label);
  ANCHOR_API void DebugNodeWindowSettings(AnchorWindowSettings *settings);
  ANCHOR_API void DebugNodeWindowsList(AnchorVector<AnchorWindow *> *windows, const char *label);
  ANCHOR_API void DebugNodeViewport(AnchorViewportP *viewport);
  ANCHOR_API void DebugRenderViewportThumbnail(AnchorDrawList *draw_list,
                                               AnchorViewportP *viewport,
                                               const AnchorBBox &bb);

}  // namespace ANCHOR

//-----------------------------------------------------------------------------
// [SECTION] AnchorFontAtlas internal API
//-----------------------------------------------------------------------------

// This structure is likely to evolve as we add support for incremental atlas updates
struct AnchorFontBuilderIO
{
  bool (*FontBuilder_Build)(AnchorFontAtlas *atlas);
};

// Helper for font builder
ANCHOR_API const AnchorFontBuilderIO *AnchorFontAtlasGetBuilderForStbTruetype();
ANCHOR_API void AnchorFontAtlasBuildInit(AnchorFontAtlas *atlas);
ANCHOR_API void AnchorFontAtlasBuildSetupFont(AnchorFontAtlas *atlas,
                                              AnchorFont *font,
                                              AnchorFontConfig *font_config,
                                              float ascent,
                                              float descent);
ANCHOR_API void AnchorFontAtlasBuildPackCustomRects(AnchorFontAtlas *atlas,
                                                    void *stbrp_context_opaque);
ANCHOR_API void AnchorFontAtlasBuildFinish(AnchorFontAtlas *atlas);
ANCHOR_API void AnchorFontAtlasBuildRender8bppRectFromString(AnchorFontAtlas *atlas,
                                                             int x,
                                                             int y,
                                                             int w,
                                                             int h,
                                                             const char *in_str,
                                                             char in_marker_char,
                                                             unsigned char in_marker_pixel_value);
ANCHOR_API void AnchorFontAtlasBuildRender32bppRectFromString(AnchorFontAtlas *atlas,
                                                              int x,
                                                              int y,
                                                              int w,
                                                              int h,
                                                              const char *in_str,
                                                              char in_marker_char,
                                                              unsigned int in_marker_pixel_value);
ANCHOR_API void AnchorFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256],
                                                            float in_multiply_factor);
ANCHOR_API void AnchorFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256],
                                                       unsigned char *pixels,
                                                       int x,
                                                       int y,
                                                       int w,
                                                       int h,
                                                       int stride);

#ifdef ANCHOR_ENABLE_TEST_ENGINE
extern void ANCHORTestEngineHook_ItemAdd(AnchorContext *ctx, const AnchorBBox &bb, ANCHOR_ID id);
extern void ANCHORTestEngineHook_ItemInfo(AnchorContext *ctx,
                                          ANCHOR_ID id,
                                          const char *label,
                                          AnchorItemStatusFlags flags);
extern void ANCHORTestEngineHook_IdInfo(AnchorContext *ctx,
                                        AnchorDataType data_type,
                                        ANCHOR_ID id,
                                        const void *data_id);
extern void ANCHORTestEngineHook_IdInfo(AnchorContext *ctx,
                                        AnchorDataType data_type,
                                        ANCHOR_ID id,
                                        const void *data_id,
                                        const void *data_id_end);
extern void ANCHORTestEngineHook_Log(AnchorContext *ctx, const char *fmt, ...);
#  define ANCHOR_TEST_ENGINE_ITEM_ADD(_BB, _ID) \
    if (g.TestEngineHookItems)                  \
    ANCHORTestEngineHook_ItemAdd(&g, _BB, _ID)  // Register item bounding box
#  define ANCHOR_TEST_ENGINE_ITEM_INFO(_ID, _LABEL, _FLAGS) \
    if (g.TestEngineHookItems)                              \
    ANCHORTestEngineHook_ItemInfo(&g,                       \
                                  _ID,                      \
                                  _LABEL,                   \
                                  _FLAGS)  // Register item label and status flags (optional)
#  define ANCHOR_TEST_ENGINE_LOG(_FMT, ...) \
    if (g.TestEngineHookItems)              \
    ANCHORTestEngineHook_Log(&g,            \
                             _FMT,          \
                             __VA_ARGS__)  // Custom log entry from user land into test log
#  define ANCHOR_TEST_ENGINE_ID_INFO(_ID, _TYPE, _DATA) \
    if (g.TestEngineHookIdInfo == id)                   \
      ANCHORTestEngineHook_IdInfo(&g, _TYPE, _ID, (const void *)(_DATA));
#  define ANCHOR_TEST_ENGINE_ID_INFO2(_ID, _TYPE, _DATA, _DATA2) \
    if (g.TestEngineHookIdInfo == id)                            \
      ANCHORTestEngineHook_IdInfo(&g, _TYPE, _ID, (const void *)(_DATA), (const void *)(_DATA2));
#else
#  define ANCHOR_TEST_ENGINE_ITEM_ADD(_BB, _ID) \
    do {                                        \
    } while (0)
#  define ANCHOR_TEST_ENGINE_ITEM_INFO(_ID, _LABEL, _FLAGS) \
    do {                                                    \
    } while (0)
#  define ANCHOR_TEST_ENGINE_LOG(_FMT, ...) \
    do {                                    \
    } while (0)
#  define ANCHOR_TEST_ENGINE_ID_INFO(_ID, _TYPE, _DATA) \
    do {                                                \
    } while (0)
#  define ANCHOR_TEST_ENGINE_ID_INFO2(_ID, _TYPE, _DATA, _DATA2) \
    do {                                                         \
    } while (0)
#endif

//-----------------------------------------------------------------------------

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

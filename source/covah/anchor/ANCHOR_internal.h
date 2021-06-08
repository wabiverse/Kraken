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

// ANCHOR, v1.84 WIP
// (internal structures/api)

/*

Index of this file:

// [SECTION] Header mess
// [SECTION] Forward declarations
// [SECTION] Context pointer
// [SECTION] STB libraries includes
// [SECTION] Macros
// [SECTION] Generic helpers
// [SECTION] ImDrawList support
// [SECTION] Widgets support: flags, enums, data structures
// [SECTION] Columns support
// [SECTION] Multi-select support
// [SECTION] Docking support
// [SECTION] Viewport support
// [SECTION] Settings support
// [SECTION] Metrics, Debug
// [SECTION] Generic context hooks
// [SECTION] ANCHOR_Context (main ANCHOR context)
// [SECTION] ANCHOR_WindowTempData, ANCHOR_Window
// [SECTION] Tab bar, Tab item support
// [SECTION] Table support
// [SECTION] ANCHOR internal API
// [SECTION] AnchorFontAtlas internal API
// [SECTION] Test Engine specific hooks (ANCHOR_test_engine)

*/

#pragma once
#ifndef ANCHOR_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#  ifndef ANCHOR_VERSION
#    error Must include ANCHOR_api.h before ANCHOR_internal.h
#  endif

#  include <limits.h>  // INT_MIN, INT_MAX
#  include <math.h>    // sqrtf, fabsf, fmodf, powf, floorf, ceilf, cosf, sinf
#  include <stdio.h>   // FILE*, sscanf
#  include <stdlib.h>  // NULL, malloc, free, qsort, atoi, atof

// Enable SSE intrinsics if available
#  if defined __SSE__ || defined __x86_64__ || defined _M_X64
#    define ANCHOR_ENABLE_SSE
#    include <immintrin.h>
#  endif

#  include <wabi/imaging/hd/driver.h>
#  include <wabi/imaging/hgi/hgi.h>
#  include <wabi/imaging/hgi/tokens.h>

#  include <wabi/usdImaging/usdApollo/engine.h>

// Visual Studio warnings
#  ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning( \
        disable : 4251)  // class 'xxx' needs to have dll-interface to be used by clients of struct
                         // 'xxx' // when ANCHOR_API is set to__declspec(dllexport)
#    pragma warning(disable : 26812)  // The enum type 'xxx' is unscoped. Prefer 'enum class' over
                                      // 'enum' (Enum.3). [MSVC Static Analyzer)
#    pragma warning(disable : 26495)  // [Static Analyzer] Variable 'XXX' is uninitialized. Always
                                      // initialize a member variable (type.6).
#    if defined(_MSC_VER) && _MSC_VER >= 1922  // MSVC 2019 16.2 or later
#      pragma warning( \
          disable : 5054)  // operator '|': deprecated between enumerations of different types
#    endif
#  endif

// Clang/GCC warnings with -Weverything
#  if defined(__clang__)
#    pragma clang diagnostic push
#    if __has_warning("-Wunknown-warning-option")
#      pragma clang diagnostic ignored \
          "-Wunknown-warning-option"  // warning: unknown warning group 'xxx'
#    endif
#    pragma clang diagnostic ignored "-Wunknown-pragmas"  // warning: unknown warning group 'xxx'
#    pragma clang diagnostic ignored \
        "-Wfloat-equal"  // warning: comparing floating point with == or != is unsafe // storing
                         // and comparing against same constants ok, for ImFloorSigned()
#    pragma clang diagnostic ignored "-Wunused-function"     // for stb_textedit.h
#    pragma clang diagnostic ignored "-Wmissing-prototypes"  // for stb_textedit.h
#    pragma clang diagnostic ignored "-Wold-style-cast"
#    pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#    pragma clang diagnostic ignored "-Wdouble-promotion"
#    pragma clang diagnostic ignored \
        "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float'
                                           // may lose precision
#  elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored \
        "-Wpragmas"  // warning: unknown option after '#pragma GCC diagnostic' kind
#    pragma GCC diagnostic ignored \
        "-Wclass-memaccess"  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object
                             // of type 'xxxx' with no trivial copy-assignment; use assignment or
                             // value-initialization instead
#  endif

// Legacy defines
#  ifdef ANCHOR_DISABLE_FORMAT_STRING_FUNCTIONS  // Renamed in 1.74
#    error Use ANCHOR_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#  endif
#  ifdef ANCHOR_DISABLE_MATH_FUNCTIONS  // Renamed in 1.74
#    error Use ANCHOR_DISABLE_DEFAULT_MATH_FUNCTIONS
#  endif

// Enable stb_truetype by default unless FreeType is enabled.
// You can compile with both by defining both ANCHOR_ENABLE_FREETYPE and ANCHOR_ENABLE_STB_TRUETYPE
// together.
#  ifndef ANCHOR_ENABLE_FREETYPE
#    define ANCHOR_ENABLE_STB_TRUETYPE
#  endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

/** Store 1-bit per value */
struct AnchorBitVector;
/** An axis-aligned rectangle (2 points) */
struct ImRect;
/** Helper to build a ImDrawData instance */
struct ImDrawDataBuilder;
/** Data shared between all ImDrawList instances */
struct ImDrawListSharedData;
/** Stacked color modifier, backup of modified data so we can restore it */
struct ANCHOR_ColorMod;
/** Main ANCHOR context */
struct ANCHOR_Context;
/** Hook for extensions like ANCHORTestEngine */
struct ANCHOR_ContextHook;
/** Type information associated to a ANCHOR_DataType enum */
struct ANCHOR_DataTypeInfo;
/** Stacked storage data for BeginGroup()/EndGroup() */
struct ANCHOR_GroupData;
/** Internal state of the currently focused/edited text input box */
struct ANCHOR_InputTextState;
/** Backup and restore IsItemHovered() internal data */
struct ANCHOR_LastItemDataBackup;
/** Simple column measurement, currently used for MenuItem() only */
struct ANCHOR_MenuColumns;
/** Result of a gamepad/keyboard directional navigation move query result */
struct ANCHOR_NavItemData;
/** Storage for ShowMetricsWindow() and DebugNodeXXX() functions */
struct ANCHOR_MetricsConfig;
/** Storage for SetNextWindow** functions */
struct ANCHOR_NextWindowData;
/** Storage for SetNextItem** functions */
struct ANCHOR_NextItemData;
/** Storage data for a single column for legacy Columns() api */
struct ANCHOR_OldColumnData;
/** Storage data for a columns set for legacy Columns() api */
struct ANCHOR_OldColumns;
/** Storage for current popup stack */
struct ANCHOR_PopupData;
/** Storage for one type registered in the .ini file */
struct ANCHOR_SettingsHandler;
/** Storage of stack sizes for debugging/asserting */
struct ANCHOR_StackSizes;
/** Stacked style modifier, backup of modified data so we can restore it */
struct ANCHOR_StyleMod;
/** Storage for a tab bar */
struct ANCHOR_TabBar;
/** Storage for a tab item (within a tab bar) */
struct ANCHOR_TabItem;
/** Storage for a table */
struct ANCHOR_Table;
/** Storage for one column of a table */
struct ANCHOR_TableColumn;
/** Temporary storage for one table (one per table in the stack), shared between tables. */
struct ANCHOR_TableTempData;
/** Storage for a table .ini settings */
struct ANCHOR_TableSettings;
/** Storage for a column .ini settings */
struct ANCHOR_TableColumnsSettings;
/** Storage for one window */
struct ANCHOR_Window;
/** Temporary storage for one window */
struct ANCHOR_WindowTempData;
/** Storage for a window .ini settings */
struct ANCHOR_WindowSettings;

// Use your programming IDE "Go to definition" facility on the names of the center columns to find
// the actual flags/enum lists.
/** -> enum ANCHOR_LayoutType_           Enum: Horizontal or vertical */
typedef int ANCHOR_LayoutType;
/** -> enum ANCHOR_ItemFlags_            Flags: for PushItemFlag() */
typedef int ANCHOR_ItemFlags;
/** -> enum ANCHOR_ItemAddFlags_         Flags: for ItemAdd() */
typedef int ANCHOR_ItemAddFlags;
/** -> enum ANCHOR_ItemStatusFlags_      Flags: for DC.LastItemStatusFlags */
typedef int ANCHOR_ItemStatusFlags;
/** -> enum ANCHOR_OldColumnFlags_       Flags: for BeginColumns() */
typedef int ANCHOR_OldColumnFlags;
/** -> enum ANCHOR_NavHighlightFlags_    Flags: for RenderNavHighlight() */
typedef int ANCHOR_NavHighlightFlags;
/** -> enum ANCHOR_NavDirSourceFlags_    Flags: for GetNavInputAmount2d() */
typedef int ANCHOR_NavDirSourceFlags;
/** -> enum ANCHOR_NavMoveFlags_         Flags: for navigation requests */
typedef int ANCHOR_NavMoveFlags;
/** -> enum ANCHOR_NextItemDataFlags_    Flags: for SetNextItemXXX() functions */
typedef int ANCHOR_NextItemDataFlags;
/** -> enum ANCHOR_NextWindowDataFlags_  Flags: for SetNextWindowXXX() functions */
typedef int ANCHOR_NextWindowDataFlags;
/** -> enum ANCHOR_SeparatorFlags_       Flags: for SeparatorEx() */
typedef int ANCHOR_SeparatorFlags;
/** -> enum ANCHOR_TextFlags_            Flags: for TextEx() */
typedef int ANCHOR_TextFlags;
/** -> enum ANCHOR_TooltipFlags_         Flags: for BeginTooltipEx() */
typedef int ANCHOR_TooltipFlags;

typedef void (*ANCHOR_ErrorLogCallback)(void *user_data, const char *fmt, ...);

//-----------------------------------------------------------------------------
// [SECTION] Context pointer
// See implementation of this variable in ANCHOR.cpp for comments and details.
//-----------------------------------------------------------------------------

#  ifndef G_CTX
extern ANCHOR_API ANCHOR_Context *G_CTX; /** Current implicit context pointer. */
#  endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries includes
//-------------------------------------------------------------------------

namespace ImStb {

#  undef STB_TEXTEDIT_STRING
#  undef STB_TEXTEDIT_CHARTYPE
#  define STB_TEXTEDIT_STRING ANCHOR_InputTextState
#  define STB_TEXTEDIT_CHARTYPE AnchorWChar
#  define STB_TEXTEDIT_GETWIDTH_NEWLINE (-1.0f)
#  define STB_TEXTEDIT_UNDOSTATECOUNT 99
#  define STB_TEXTEDIT_UNDOCHARCOUNT 999
#  include "ANCHOR_textedit.h"

}  // namespace ImStb

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// Debug Logging
#  ifndef ANCHOR_DEBUG_LOG
#    define ANCHOR_DEBUG_LOG(_FMT, ...) printf("[%05d] " _FMT, G_CTX->FrameCount, __VA_ARGS__)
#  endif

// Debug Logging for selected systems. Remove the '((void)0) //' to enable.
//#define ANCHOR_DEBUG_LOG_POPUP         ANCHOR_DEBUG_LOG // Enable log
//#define ANCHOR_DEBUG_LOG_NAV           ANCHOR_DEBUG_LOG // Enable log
#  define ANCHOR_DEBUG_LOG_POPUP(...) ((void)0)  // Disable log
#  define ANCHOR_DEBUG_LOG_NAV(...) ((void)0)    // Disable log

// Static Asserts
#  if (__cplusplus >= 201100) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201100)
#    define IM_STATIC_ASSERT(_COND) static_assert(_COND, "")
#  else
#    define IM_STATIC_ASSERT(_COND) typedef char static_assertion_##__line__[(_COND) ? 1 : -1]
#  endif

// "Paranoid" Debug Asserts are meant to only be enabled during specific debugging/work, otherwise
// would slow down the code too much. We currently don't have many of those so the effect is
// currently negligible, but onward intent to add more aggressive ones in the code.
//#define ANCHOR_DEBUG_PARANOID
#  ifdef ANCHOR_DEBUG_PARANOID
#    define ANCHOR_ASSERT_PARANOID(_EXPR) ANCHOR_ASSERT(_EXPR)
#  else
#    define ANCHOR_ASSERT_PARANOID(_EXPR)
#  endif

// Error handling
// Down the line in some frameworks/languages we would like to have a way to redirect those to the
// programmer and recover from more faults.
#  ifndef ANCHOR_ASSERT_USER_ERROR
#    define ANCHOR_ASSERT_USER_ERROR(_EXP, _MSG) \
      ANCHOR_ASSERT((_EXP) && _MSG)  // Recoverable User Error
#  endif

// Misc Macros
#  define IM_PI 3.14159265358979323846f
#  ifdef _WIN32
#    define IM_NEWLINE \
      "\r\n"  // Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to
              // support Unix-style carriage returns!)
#  else
#    define IM_NEWLINE "\n"
#  endif
#  define IM_TABSIZE (4)
#  define IM_MEMALIGN(_OFF, _ALIGN) \
    (((_OFF) + (_ALIGN - 1)) & \
     ~(_ALIGN - 1))  // Memory align e.g. IM_ALIGN(0,4)=0, IM_ALIGN(1,4)=4, IM_ALIGN(4,4)=4,
                     // IM_ALIGN(5,4)=8
#  define IM_F32_TO_INT8_UNBOUND(_VAL) \
    ((int)((_VAL)*255.0f + ((_VAL) >= 0 ? 0.5f : -0.5f)))  // Unsaturated, for display purpose
#  define IM_F32_TO_INT8_SAT(_VAL) \
    ((int)(ImSaturate(_VAL) * 255.0f + 0.5f))  // Saturated, always output 0..255
#  define IM_FLOOR(_VAL) ((float)(int)(_VAL))  // ImFloor() is not inlined in MSVC debug builds
#  define IM_ROUND(_VAL) ((float)(int)((_VAL) + 0.5f))  //

// Enforce cdecl calling convention for functions called by the standard library, in case
// compilation settings changed the default to e.g. __vectorcall
#  ifdef _MSC_VER
#    define ANCHOR_CDECL __cdecl
#  else
#    define ANCHOR_CDECL
#  endif

// Warnings
#  if defined(_MSC_VER) && !defined(__clang__)
#    define ANCHOR_MSVC_WARNING_SUPPRESS(XXXX) __pragma(warning(suppress : XXXX))
#  else
#    define ANCHOR_MSVC_WARNING_SUPPRESS(XXXX)
#  endif

// Debug Tools
// Use 'Metrics->Tools->Item Picker' to break into the call-stack of a specific item.
#  ifndef IM_DEBUG_BREAK
#    if defined(__clang__)
#      define IM_DEBUG_BREAK() __builtin_debugtrap()
#    elif defined(_MSC_VER)
#      define IM_DEBUG_BREAK() __debugbreak()
#    else
#      define IM_DEBUG_BREAK() \
        ANCHOR_ASSERT( \
            0)  // It is expected that you define IM_DEBUG_BREAK() into something that will
   // break nicely in a debugger!
#    endif
#  endif  // #ifndef IM_DEBUG_BREAK

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
// Note that the ImXXX helpers functions are lower-level than ANCHOR functions.
// ANCHOR functions or the ANCHOR context are never called/used from other ImXXX functions.
//-----------------------------------------------------------------------------
// - Helpers: Hashing
// - Helpers: Sorting
// - Helpers: Bit manipulation
// - Helpers: String, Formatting
// - Helpers: UTF-8 <> wchar conversions
// - Helpers: wabi::GfVec2f/wabi::GfVec4f operators
// - Helpers: Maths
// - Helpers: Geometry
// - Helper: ImVec1
// - Helper: ImRect
// - Helper: AnchorBitArray
// - Helper: AnchorBitVector
// - Helper: ImSpan<>, ImSpanAllocator<>
// - Helper: ImPool<>
// - Helper: ImChunkStream<>
//-----------------------------------------------------------------------------

// Helpers: Hashing
ANCHOR_API ANCHOR_ID ImHashData(const void *data, size_t data_size, AnchorU32 seed = 0);
ANCHOR_API ANCHOR_ID ImHashStr(const char *data, size_t data_size = 0, AnchorU32 seed = 0);
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
static inline ANCHOR_ID ImHash(const void *data, int size, AnchorU32 seed = 0)
{
  return size ? ImHashData(data, (size_t)size, seed) : ImHashStr((const char *)data, 0, seed);
}  // [moved to ImHashStr/ImHashData in 1.68]
#  endif

// Helpers: Sorting
#  define ImQsort qsort

// Helpers: Color Blending
ANCHOR_API AnchorU32 ImAlphaBlendColors(AnchorU32 col_a, AnchorU32 col_b);

// Helpers: Bit manipulation
static inline bool ImIsPowerOfTwo(int v)
{
  return v != 0 && (v & (v - 1)) == 0;
}
static inline bool ImIsPowerOfTwo(ImU64 v)
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

// Helpers: String, Formatting
ANCHOR_API int ImStricmp(const char *str1, const char *str2);
ANCHOR_API int ImStrnicmp(const char *str1, const char *str2, size_t count);
ANCHOR_API void ImStrncpy(char *dst, const char *src, size_t count);
ANCHOR_API char *ImStrdup(const char *str);
ANCHOR_API char *ImStrdupcpy(char *dst, size_t *p_dst_size, const char *str);
ANCHOR_API const char *ImStrchrRange(const char *str_begin, const char *str_end, char c);
ANCHOR_API int ImStrlenW(const AnchorWChar *str);
ANCHOR_API const char *ImStreolRange(const char *str, const char *str_end);
ANCHOR_API const AnchorWChar *ImStrbolW(const AnchorWChar *buf_mid_line,
                                        const AnchorWChar *buf_begin);
ANCHOR_API const char *ImStristr(const char *haystack,
                                 const char *haystack_end,
                                 const char *needle,
                                 const char *needle_end);
ANCHOR_API void ImStrTrimBlanks(char *str);
ANCHOR_API const char *ImStrSkipBlank(const char *str);
ANCHOR_API int ImFormatString(char *buf, size_t buf_size, const char *fmt, ...) ANCHOR_FMTARGS(3);
ANCHOR_API int ImFormatStringV(char *buf, size_t buf_size, const char *fmt, va_list args)
    ANCHOR_FMTLIST(3);
ANCHOR_API const char *ImParseFormatFindStart(const char *format);
ANCHOR_API const char *ImParseFormatFindEnd(const char *format);
ANCHOR_API const char *ImParseFormatTrimDecorations(const char *format,
                                                    char *buf,
                                                    size_t buf_size);
ANCHOR_API int ImParseFormatPrecision(const char *format, int default_value);
static inline bool ImCharIsBlankA(char c)
{
  return c == ' ' || c == '\t';
}
static inline bool ImCharIsBlankW(unsigned int c)
{
  return c == ' ' || c == '\t' || c == 0x3000;
}

// Helpers: UTF-8 <> wchar conversions
// return out_buf
ANCHOR_API const char *ImTextCharToUtf8(char out_buf[5], unsigned int c);

// return output UTF-8 bytes count
ANCHOR_API int ImTextStrToUtf8(char *out_buf,
                               int out_buf_size,
                               const AnchorWChar *in_text,
                               const AnchorWChar *in_text_end);

// read one character. return input UTF-8 bytes count
ANCHOR_API int ImTextCharFromUtf8(unsigned int *out_char,
                                  const char *in_text,
                                  const char *in_text_end);
// return input UTF-8 bytes count
ANCHOR_API int ImTextStrFromUtf8(AnchorWChar *out_buf,
                                 int out_buf_size,
                                 const char *in_text,
                                 const char *in_text_end,
                                 const char **in_remaining = NULL);
ANCHOR_API int ImTextCountCharsFromUtf8(
    const char *in_text,
    const char *in_text_end);  // return number of UTF-8 code-points (NOT bytes count)
ANCHOR_API int ImTextCountUtf8BytesFromChar(
    const char *in_text,
    const char *in_text_end);  // return number of bytes to express one char in UTF-8
ANCHOR_API int ImTextCountUtf8BytesFromStr(
    const AnchorWChar *in_text,
    const AnchorWChar *in_text_end);  // return number of bytes to express string in UTF-8

// Helpers: File System
#  ifdef ANCHOR_DISABLE_FILE_FUNCTIONS
#    define ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void *ImFileHandle;
static inline ImFileHandle ImFileOpen(const char *, const char *)
{
  return NULL;
}
static inline bool ImFileClose(ImFileHandle)
{
  return false;
}
static inline ImU64 ImFileGetSize(ImFileHandle)
{
  return (ImU64)-1;
}
static inline ImU64 ImFileRead(void *, ImU64, ImU64, ImFileHandle)
{
  return 0;
}
static inline ImU64 ImFileWrite(const void *, ImU64, ImU64, ImFileHandle)
{
  return 0;
}
#  endif
#  ifndef ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE *ImFileHandle;
ANCHOR_API ImFileHandle ImFileOpen(const char *filename, const char *mode);
ANCHOR_API bool ImFileClose(ImFileHandle file);
ANCHOR_API ImU64 ImFileGetSize(ImFileHandle file);
ANCHOR_API ImU64 ImFileRead(void *data, ImU64 size, ImU64 count, ImFileHandle file);
ANCHOR_API ImU64 ImFileWrite(const void *data, ImU64 size, ImU64 count, ImFileHandle file);
#  else
#    define ANCHOR_DISABLE_TTY_FUNCTIONS  // Can't use stdout, fflush if we are not using default
                                          // file functions
#  endif
ANCHOR_API void *ImFileLoadToMemory(const char *filename,
                                    const char *mode,
                                    size_t *out_file_size = NULL,
                                    int padding_bytes     = 0);

// Helpers: Maths
ANCHOR_MSVC_RUNTIME_CHECKS_OFF
// - Wrapper for standard libs functions. (Note that ANCHOR_demo.cpp does _not_ use them to keep
// the code easy to copy)
#  ifndef ANCHOR_DISABLE_DEFAULT_MATH_FUNCTIONS
#    define ImFabs(X) fabsf(X)
#    define ImSqrt(X) sqrtf(X)
#    define ImFmod(X, Y) fmodf((X), (Y))
#    define ImCos(X) cosf(X)
#    define ImSin(X) sinf(X)
#    define ImAcos(X) acosf(X)
#    define ImAtan2(Y, X) atan2f((Y), (X))
#    define ImAtof(STR) atof(STR)
//#define ImFloorStd(X)     floorf(X)           // We use our own, see ImFloor() and
// ImFloorSigned()
#    define ImCeil(X) ceilf(X)
static inline float ImPow(float x, float y)
{
  return powf(x, y);
}  // DragBehaviorT/SliderBehaviorT uses ImPow with either float/double and need the precision
static inline double ImPow(double x, double y)
{
  return pow(x, y);
}
static inline float ImLog(float x)
{
  return logf(x);
}  // DragBehaviorT/SliderBehaviorT uses ImLog with either float/double and need the precision
static inline double ImLog(double x)
{
  return log(x);
}
static inline int ImAbs(int x)
{
  return x < 0 ? -x : x;
}
static inline float ImAbs(float x)
{
  return fabsf(x);
}
static inline double ImAbs(double x)
{
  return fabs(x);
}
static inline float ImSign(float x)
{
  return (x < 0.0f) ? -1.0f : ((x > 0.0f) ? 1.0f : 0.0f);
}  // Sign operator - returns -1, 0 or 1 based on sign of argument
static inline double ImSign(double x)
{
  return (x < 0.0) ? -1.0 : ((x > 0.0) ? 1.0 : 0.0);
}
#    ifdef ANCHOR_ENABLE_SSE
static inline float ImRsqrt(float x)
{
  return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
}
#    else
static inline float ImRsqrt(float x)
{
  return 1.0f / sqrtf(x);
}
#    endif
static inline double ImRsqrt(double x)
{
  return 1.0 / sqrt(x);
}
#  endif
// - ImMin/AnchorMax/ImClamp/ImLerp/ImSwap are used by widgets which support variety of types:
// signed/unsigned int/long long float/double (Exceptionally using templates here but we could also
// redefine them for those types)
template<typename T> static inline T ImMin(T lhs, T rhs)
{
  return lhs < rhs ? lhs : rhs;
}
template<typename T> static inline T AnchorMax(T lhs, T rhs)
{
  return lhs >= rhs ? lhs : rhs;
}
template<typename T> static inline T ImClamp(T v, T mn, T mx)
{
  return (v < mn) ? mn : (v > mx) ? mx : v;
}
template<typename T> static inline T ImLerp(T a, T b, float t)
{
  return (T)(a + (b - a) * t);
}
template<typename T> static inline void ImSwap(T &a, T &b)
{
  T tmp = a;
  a     = b;
  b     = tmp;
}
template<typename T> static inline T ImAddClampOverflow(T a, T b, T mn, T mx)
{
  if (b < 0 && (a < mn - b))
    return mn;
  if (b > 0 && (a > mx - b))
    return mx;
  return a + b;
}
template<typename T> static inline T ImSubClampOverflow(T a, T b, T mn, T mx)
{
  if (b > 0 && (a < mn + b))
    return mn;
  if (b < 0 && (a > mx + b))
    return mx;
  return a - b;
}
// - Misc maths helpers
static inline wabi::GfVec2f ImMin(const wabi::GfVec2f &lhs, const wabi::GfVec2f &rhs)
{
  return wabi::GfVec2f(lhs[0] < rhs[0] ? lhs[0] : rhs[0], lhs[1] < rhs[1] ? lhs[1] : rhs[1]);
}
static inline wabi::GfVec2f AnchorMax(const wabi::GfVec2f &lhs, const wabi::GfVec2f &rhs)
{
  return wabi::GfVec2f(lhs[0] >= rhs[0] ? lhs[0] : rhs[0], lhs[1] >= rhs[1] ? lhs[1] : rhs[1]);
}
static inline wabi::GfVec2f ImClamp(const wabi::GfVec2f &v,
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
static inline wabi::GfVec2f ImLerp(const wabi::GfVec2f &a, const wabi::GfVec2f &b, float t)
{
  return wabi::GfVec2f(a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t);
}
static inline wabi::GfVec2f ImLerp(const wabi::GfVec2f &a,
                                   const wabi::GfVec2f &b,
                                   const wabi::GfVec2f &t)
{
  return wabi::GfVec2f(a[0] + (b[0] - a[0]) * t[0], a[1] + (b[1] - a[1]) * t[1]);
}
static inline wabi::GfVec4f ImLerp(const wabi::GfVec4f &a, const wabi::GfVec4f &b, float t)
{
  return wabi::GfVec4f(a[0] + (b[0] - a[0]) * t,
                       a[1] + (b[1] - a[1]) * t,
                       a[2] + (b[2] - a[2]) * t,
                       a[3] + (b[3] - a[3]) * t);
}
static inline float ImSaturate(float f)
{
  return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f;
}
static inline float ImLengthSqr(const wabi::GfVec2f &lhs)
{
  return (lhs[0] * lhs[0]) + (lhs[1] * lhs[1]);
}
static inline float ImLengthSqr(const wabi::GfVec4f &lhs)
{
  return (lhs[0] * lhs[0]) + (lhs[1] * lhs[1]) + (lhs[2] * lhs[2]) + (lhs[3] * lhs[3]);
}
static inline float ImInvLength(const wabi::GfVec2f &lhs, float fail_value)
{
  float d = (lhs[0] * lhs[0]) + (lhs[1] * lhs[1]);
  if (d > 0.0f)
    return ImRsqrt(d);
  return fail_value;
}
static inline float ImFloor(float f)
{
  return (float)(int)(f);
}
static inline float ImFloorSigned(float f)
{
  return (float)((f >= 0 || (int)f == f) ? (int)f : (int)f - 1);
}  // Decent replacement for floorf()
static inline wabi::GfVec2f ImFloor(const wabi::GfVec2f &v)
{
  return wabi::GfVec2f((float)(int)(v[0]), (float)(int)(v[1]));
}
static inline int ImModPositive(int a, int b)
{
  return (a + b) % b;
}
static inline float ImDot(const wabi::GfVec2f &a, const wabi::GfVec2f &b)
{
  return a[0] * b[0] + a[1] * b[1];
}
static inline wabi::GfVec2f ImRotate(const wabi::GfVec2f &v, float cos_a, float sin_a)
{
  return wabi::GfVec2f(v[0] * cos_a - v[1] * sin_a, v[0] * sin_a + v[1] * cos_a);
}
static inline float ImLinearSweep(float current, float target, float speed)
{
  if (current < target)
    return ImMin(current + speed, target);
  if (current > target)
    return AnchorMax(current - speed, target);
  return current;
}
static inline wabi::GfVec2f ImMul(const wabi::GfVec2f &lhs, const wabi::GfVec2f &rhs)
{
  return wabi::GfVec2f(lhs[0] * rhs[0], lhs[1] * rhs[1]);
}
ANCHOR_MSVC_RUNTIME_CHECKS_RESTORE

// Helpers: Geometry
ANCHOR_API wabi::GfVec2f ImBezierCubicCalc(const wabi::GfVec2f &p1,
                                           const wabi::GfVec2f &p2,
                                           const wabi::GfVec2f &p3,
                                           const wabi::GfVec2f &p4,
                                           float t);
ANCHOR_API wabi::GfVec2f ImBezierCubicClosestPoint(
    const wabi::GfVec2f &p1,
    const wabi::GfVec2f &p2,
    const wabi::GfVec2f &p3,
    const wabi::GfVec2f &p4,
    const wabi::GfVec2f &p,
    int num_segments);  // For curves with explicit number of segments
ANCHOR_API wabi::GfVec2f ImBezierCubicClosestPointCasteljau(
    const wabi::GfVec2f &p1,
    const wabi::GfVec2f &p2,
    const wabi::GfVec2f &p3,
    const wabi::GfVec2f &p4,
    const wabi::GfVec2f &p,
    float tess_tol);  // For auto-tessellated curves you can use
                      // tess_tol = style.CurveTessellationTol
ANCHOR_API wabi::GfVec2f ImBezierQuadraticCalc(const wabi::GfVec2f &p1,
                                               const wabi::GfVec2f &p2,
                                               const wabi::GfVec2f &p3,
                                               float t);
ANCHOR_API wabi::GfVec2f ImLineClosestPoint(const wabi::GfVec2f &a,
                                            const wabi::GfVec2f &b,
                                            const wabi::GfVec2f &p);
ANCHOR_API bool ImTriangleContainsPoint(const wabi::GfVec2f &a,
                                        const wabi::GfVec2f &b,
                                        const wabi::GfVec2f &c,
                                        const wabi::GfVec2f &p);
ANCHOR_API wabi::GfVec2f ImTriangleClosestPoint(const wabi::GfVec2f &a,
                                                const wabi::GfVec2f &b,
                                                const wabi::GfVec2f &c,
                                                const wabi::GfVec2f &p);
ANCHOR_API void ImTriangleBarycentricCoords(const wabi::GfVec2f &a,
                                            const wabi::GfVec2f &b,
                                            const wabi::GfVec2f &c,
                                            const wabi::GfVec2f &p,
                                            float &out_u,
                                            float &out_v,
                                            float &out_w);
inline float ImTriangleArea(const wabi::GfVec2f &a, const wabi::GfVec2f &b, const wabi::GfVec2f &c)
{
  return ImFabs((a[0] * (b[1] - c[1])) + (b[0] * (c[1] - a[1])) + (c[0] * (a[1] - b[1]))) * 0.5f;
}
ANCHOR_API ANCHOR_Dir ImGetDirQuadrantFromDelta(float dx, float dy);

// Helper: ImVec1 (1D vector)
// (this odd construct is used to facilitate the transition between 1D and 2D, and the maintenance
// of some branches/patches)
ANCHOR_MSVC_RUNTIME_CHECKS_OFF
struct ImVec1 {
  float x;
  ImVec1()
  {
    x = 0.0f;
  }
  ImVec1(float _x)
  {
    x = _x;
  }
};

// Helper: ImRect (2D axis aligned bounding-box)
// NB: we can't rely on wabi::GfVec2f math operators being available here!
struct ANCHOR_API ImRect {
  wabi::GfVec2f Min;  // Upper-left
  wabi::GfVec2f Max;  // Lower-right

  ImRect() : Min(0.0f, 0.0f), Max(0.0f, 0.0f)
  {}
  ImRect(const wabi::GfVec2f &min, const wabi::GfVec2f &max) : Min(min), Max(max)
  {}
  ImRect(const wabi::GfVec4f &v) : Min(v[0], v[1]), Max(v[2], v[3])
  {}
  ImRect(float x1, float y1, float x2, float y2) : Min(x1, y1), Max(x2, y2)
  {}

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
  }  // Top-left
  wabi::GfVec2f GetTR() const
  {
    return wabi::GfVec2f(Max[0], Min[1]);
  }  // Top-right
  wabi::GfVec2f GetBL() const
  {
    return wabi::GfVec2f(Min[0], Max[1]);
  }  // Bottom-left
  wabi::GfVec2f GetBR() const
  {
    return Max;
  }  // Bottom-right
  bool Contains(const wabi::GfVec2f &p) const
  {
    return p[0] >= Min[0] && p[1] >= Min[1] && p[0] < Max[0] && p[1] < Max[1];
  }
  bool Contains(const ImRect &r) const
  {
    return r.Min[0] >= Min[0] && r.Min[1] >= Min[1] && r.Max[0] <= Max[0] && r.Max[1] <= Max[1];
  }
  bool Overlaps(const ImRect &r) const
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
  void Add(const ImRect &r)
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
  void ClipWith(const ImRect &r)
  {
    Min = AnchorMax(Min, r.Min);
    Max = ImMin(Max, r.Max);
  }  // Simple version, may lead to an inverted rectangle, which is fine for Contains/Overlaps test
     // but not for display.
  void ClipWithFull(const ImRect &r)
  {
    Min = ImClamp(Min, r.Min, r.Max);
    Max = ImClamp(Max, r.Min, r.Max);
  }  // Full version, ensure both points are fully clipped.
  void Floor()
  {
    Min[0] = IM_FLOOR(Min[0]);
    Min[1] = IM_FLOOR(Min[1]);
    Max[0] = IM_FLOOR(Max[0]);
    Max[1] = IM_FLOOR(Max[1]);
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

// Helper: AnchorBitArray
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
    int a_mod      = (n & 31);
    int b_mod      = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
    AnchorU32 mask = (AnchorU32)(((ImU64)1 << b_mod) - 1) & ~(AnchorU32)(((ImU64)1 << a_mod) - 1);
    arr[n >> 5] |= mask;
    n = (n + 32) & ~31;
  }
}

// Helper: AnchorBitArray class (wrapper over AnchorBitArray functions)
// Store 1-bit per value.
template<int BITCOUNT> struct ANCHOR_API AnchorBitArray {
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
  void SetBitRange(int n, int n2)
  {
    AnchorBitArraySetBitRange(Storage, n, n2);
  }  // Works on range [n..n2)
};

// Helper: AnchorBitVector
// Store 1-bit per value.
struct ANCHOR_API AnchorBitVector {
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

// Helper: ImSpan<>
// Pointing to a span of data we don't own.
template<typename T> struct ImSpan {
  T *Data;
  T *DataEnd;

  // Constructors, destructor
  inline ImSpan()
  {
    Data = DataEnd = NULL;
  }
  inline ImSpan(T *data, int size)
  {
    Data    = data;
    DataEnd = data + size;
  }
  inline ImSpan(T *data, T *data_end)
  {
    Data    = data;
    DataEnd = data_end;
  }

  inline void set(T *data, int size)
  {
    Data    = data;
    DataEnd = data + size;
  }
  inline void set(T *data, T *data_end)
  {
    Data    = data;
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

  // Utilities
  inline int index_from_ptr(const T *it) const
  {
    ANCHOR_ASSERT(it >= Data && it < DataEnd);
    const ptrdiff_t off = it - Data;
    return (int)off;
  }
};

// Helper: ImSpanAllocator<>
// Facilitate storing multiple chunks into a single large block (the "arena")
// - Usage: call Reserve() N times, allocate GetArenaSizeInBytes() worth, pass it to
// SetArenaBasePtr(), call GetSpan() N times to retrieve the aligned ranges.
template<int CHUNKS> struct ImSpanAllocator {
  char *BasePtr;
  int CurrOff;
  int CurrIdx;
  int Offsets[CHUNKS];
  int Sizes[CHUNKS];

  ImSpanAllocator()
  {
    memset(this, 0, sizeof(*this));
  }
  inline void Reserve(int n, size_t sz, int a = 4)
  {
    ANCHOR_ASSERT(n == CurrIdx && n < CHUNKS);
    CurrOff    = IM_MEMALIGN(CurrOff, a);
    Offsets[n] = CurrOff;
    Sizes[n]   = (int)sz;
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
  template<typename T> inline void GetSpan(int n, ImSpan<T> *span)
  {
    span->set((T *)GetSpanPtrBegin(n), (T *)GetSpanPtrEnd(n));
  }
};

// Helper: ImPool<>
// Basic keyed storage for contiguous instances, slow/amortized insertion, O(1) indexable, O(Log N)
// queries by ID over a dense/hot buffer, Honor constructor/destructor. Add/remove invalidate all
// pointers. Indexes have the same lifetime as the associated object.
typedef int ImPoolIdx;
template<typename T> struct ANCHOR_API ImPool {
  AnchorVector<T> Buf;  // Contiguous data
  ANCHORStorage Map;    // ID->Index
  ImPoolIdx FreeIdx;    // Next free idx to use

  ImPool()
  {
    FreeIdx = 0;
  }
  ~ImPool()
  {
    Clear();
  }
  T *GetByKey(ANCHOR_ID key)
  {
    int idx = Map.GetInt(key, -1);
    return (idx != -1) ? &Buf[idx] : NULL;
  }
  T *GetByIndex(ImPoolIdx n)
  {
    return &Buf[n];
  }
  ImPoolIdx GetIndex(const T *p) const
  {
    ANCHOR_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size);
    return (ImPoolIdx)(p - Buf.Data);
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
    }
    else {
      FreeIdx = *(int *)&Buf[idx];
    }
    IM_PLACEMENT_NEW(&Buf[idx]) T();
    return &Buf[idx];
  }
  void Remove(ANCHOR_ID key, const T *p)
  {
    Remove(key, GetIndex(p));
  }
  void Remove(ANCHOR_ID key, ImPoolIdx idx)
  {
    Buf[idx].~T();
    *(int *)&Buf[idx] = FreeIdx;
    FreeIdx           = idx;
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

// Helper: ImChunkStream<>
// Build and iterate a contiguous stream of variable-sized structures.
// This is used by Settings to store persistent data while reducing allocation count.
// We store the chunk size first, and align the final size on 4 bytes boundaries.
// The tedious/zealous amount of casting is to avoid -Wcast-align warnings.
template<typename T> struct ANCHOR_API ImChunkStream {
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
    sz            = IM_MEMALIGN(HDR_SZ + sz, 4u);
    int off       = Buf.Size;
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
  void swap(ImChunkStream<T> &rhs)
  {
    rhs.Buf.swap(Buf);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] ImDrawList support
//-----------------------------------------------------------------------------

// ImDrawList: Helper function to calculate a circle's segment count given its radius and a
// "maximum error" value. Estimation of number of circle segment based on error is derived using
// method described in https://stackoverflow.com/a/2244088/15194693 Number of segments (N) is
// calculated using equation:
//   N = ceil ( pi / acos(1 - error / r) )     where r > 0, error <= r
// Our equation is significantly simpler that one in the post thanks for choosing segment that is
// perpendicular to X axis. Follow steps in the article from this starting condition and you will
// will get this result.
//
// Rendering circles with an odd number of segments, while mathematically correct will produce
// asymmetrical results on the raster grid. Therefore we're rounding N to next even number (7->8,
// 8->8, 9->10 etc.)
//
#  define IM_ROUNDUP_TO_EVEN(_V) ((((_V) + 1) / 2) * 2)
#  define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN 4
#  define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX 512
#  define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD, _MAXERROR) \
    ImClamp( \
        IM_ROUNDUP_TO_EVEN((int)ImCeil(IM_PI / ImAcos(1 - ImMin((_MAXERROR), (_RAD)) / (_RAD)))), \
        IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, \
        IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

// Raw equation from IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC rewritten for 'r' and 'error'.
#  define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N, _MAXERROR) \
    ((_MAXERROR) / (1 - ImCos(IM_PI / AnchorMax((float)(_N), IM_PI))))
#  define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N, _RAD) \
    ((1 - ImCos(IM_PI / AnchorMax((float)(_N), IM_PI))) / (_RAD))

// ImDrawList: Lookup table size for adaptive arc drawing, cover full circle.
#  ifndef IM_DRAWLIST_ARCFAST_TABLE_SIZE
#    define IM_DRAWLIST_ARCFAST_TABLE_SIZE 48  // Number of samples in lookup table.
#  endif
#  define IM_DRAWLIST_ARCFAST_SAMPLE_MAX \
    IM_DRAWLIST_ARCFAST_TABLE_SIZE  // Sample index _PathArcToFastEx() for 360 angle.

// Data shared between all ImDrawList instances
// You may want to create your own instance of this if you want to use ImDrawList completely
// without ANCHOR. In that case, watch out for future changes to this structure.
struct ANCHOR_API ImDrawListSharedData {
  wabi::GfVec2f TexUvWhitePixel;  // UV of white pixel in the atlas
  AnchorFont *Font;  // Current/default font (optional, for simplified AddText overload)
  float FontSize;    // Current/default font size (optional, for simplified AddText overload)
  float CurveTessellationTol;        // Tessellation tolerance when using PathBezierCurveTo()
  float CircleSegmentMaxError;       // Number of circle segments to use per pixel of radius for
                                     // AddCircle() etc
  wabi::GfVec4f ClipRectFullscreen;  // Value for PushClipRectFullscreen()
  ImDrawListFlags InitialFlags;  // Initial flags at the beginning of the frame (it is possible to
                                 // alter flags on a per-drawlist basis afterwards)

  // [Internal] Lookup tables
  wabi::GfVec2f
      ArcFastVtx[IM_DRAWLIST_ARCFAST_TABLE_SIZE];  // Sample points on the quarter of the circle.
  float ArcFastRadiusCutoff;     // Cutoff radius after which arc drawing will fallback to slower
                                 // PathArcTo()
  ImU8 CircleSegmentCounts[64];  // Precomputed segment count for given radius before we calculate
                                 // it dynamically (to avoid calculation overhead)
  const wabi::GfVec4f *TexUvLines;  // UV of anti-aliased lines in the atlas

  ImDrawListSharedData();
  void SetCircleTessellationMaxError(float max_error);
};

struct ImDrawDataBuilder {
  AnchorVector<ImDrawList *> Layers[2];  // Global layers for: regular, tooltip

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

//-----------------------------------------------------------------------------
// [SECTION] Widgets support: flags, enums, data structures
//-----------------------------------------------------------------------------

// Transient per-window flags, reset at the beginning of the frame. For child window, inherited
// from parent on first Begin(). This is going to be exposed in ANCHOR_api.h when stabilized
// enough.
enum ANCHOR_ItemFlags_ {
  ANCHOR_ItemFlags_None      = 0,
  ANCHOR_ItemFlags_NoTabStop = 1 << 0,  // false
  ANCHOR_ItemFlags_ButtonRepeat =
      1 << 1,  // false    // Button() will return true multiple times based on io.KeyRepeatDelay
               // and io.KeyRepeatRate settings.
  ANCHOR_ItemFlags_Disabled =
      1 << 2,  // false    // [BETA] Disable interactions but doesn't affect visuals yet. See
               // github.com/ocornut/ANCHOR/issues/211
  ANCHOR_ItemFlags_NoNav             = 1 << 3,  // false
  ANCHOR_ItemFlags_NoNavDefaultFocus = 1 << 4,  // false
  ANCHOR_ItemFlags_SelectableDontClosePopup =
      1 << 5,  // false    // MenuItem/Selectable() automatically closes current Popup window
  ANCHOR_ItemFlags_MixedValue =
      1 << 6,  // false    // [BETA] Represent a mixed/indeterminate value, generally
               // multi-selection where values differ. Currently only supported by Checkbox()
               // (later should support all sorts of widgets)
  ANCHOR_ItemFlags_ReadOnly = 1 << 7  // false    // [ALPHA] Allow hovering interactions but
                                      // underlying value is not changed.
};

// Flags for ItemAdd()
// FIXME-NAV: _Focusable is _ALMOST_ what you would expect to be called '_TabStop' but because
// SetKeyboardFocusHere() works on items with no TabStop we distinguish Focusable from TabStop.
enum ANCHOR_ItemAddFlags_ {
  ANCHOR_ItemAddFlags_None = 0,
  ANCHOR_ItemAddFlags_Focusable =
      1 << 0  // FIXME-NAV: In current/legacy scheme, Focusable+TabStop support are opt-in by
              // widgets. We will transition it toward being opt-out, so this flag is expected to
              // eventually disappear.
};

// Storage for LastItem data
enum ANCHOR_ItemStatusFlags_ {
  ANCHOR_ItemStatusFlags_None = 0,
  ANCHOR_ItemStatusFlags_HoveredRect =
      1 << 0,  // Mouse position is within item rectangle (does NOT mean that the window is in
               // correct z-order and can be hovered!, this is only one part of the most-common
               // IsItemHovered test)
  ANCHOR_ItemStatusFlags_HasDisplayRect = 1 << 1,  // window->DC.LastItemDisplayRect is valid
  ANCHOR_ItemStatusFlags_Edited = 1 << 2,  // Value exposed by item was edited in the current frame
                                           // (should match the bool return value of most widgets)
  ANCHOR_ItemStatusFlags_ToggledSelection =
      1 << 3,  // Set when Selectable(), TreeNode() reports toggling a selection. We can't report
               // "Selected", only state changes, in order to easily handle clipping with less
               // issues.
  ANCHOR_ItemStatusFlags_ToggledOpen =
      1 << 4,  // Set when TreeNode() reports toggling their open state.
  ANCHOR_ItemStatusFlags_HasDeactivated =
      1 << 5,  // Set if the widget/group is able to provide data for the
               // ANCHOR_ItemStatusFlags_Deactivated flag.
  ANCHOR_ItemStatusFlags_Deactivated =
      1 << 6,  // Only valid if ANCHOR_ItemStatusFlags_HasDeactivated is set.
  ANCHOR_ItemStatusFlags_HoveredWindow =
      1 << 7,  // Override the HoveredWindow test to allow cross-window hover testing.
  ANCHOR_ItemStatusFlags_FocusedByCode =
      1 << 8,  // Set when the Focusable item just got focused from code.
  ANCHOR_ItemStatusFlags_FocusedByTabbing =
      1 << 9,  // Set when the Focusable item just got focused by Tabbing.
  ANCHOR_ItemStatusFlags_Focused = ANCHOR_ItemStatusFlags_FocusedByCode |
                                   ANCHOR_ItemStatusFlags_FocusedByTabbing

#  ifdef ANCHOR_ENABLE_TEST_ENGINE
  ,                                            // [ANCHOR_tests only]
  ANCHOR_ItemStatusFlags_Openable  = 1 << 20,  //
  ANCHOR_ItemStatusFlags_Opened    = 1 << 21,  //
  ANCHOR_ItemStatusFlags_Checkable = 1 << 22,  //
  ANCHOR_ItemStatusFlags_Checked   = 1 << 23   //
#  endif
};

// Extend ANCHORInputTextFlags_
enum ANCHORInputTextFlagsPrivate_ {
  // [Internal]
  ANCHORInputTextFlags_Multiline = 1 << 26,  // For internal use by InputTextMultiline()
  ANCHORInputTextFlags_NoMarkEdited =
      1 << 27,  // For internal use by functions using InputText() before reformatting data
  ANCHORInputTextFlags_MergedItem =
      1 << 28  // For internal use by TempInputText(), will skip calling ItemAdd(). Require
               // bounding-box to strictly match.
};

// Extend ANCHOR_ButtonFlags_
enum ANCHOR_ButtonFlagsPrivate_ {
  ANCHOR_ButtonFlags_PressedOnClick = 1 << 4,  // return true on click (mouse down event)
  ANCHOR_ButtonFlags_PressedOnClickRelease =
      1 << 5,  // [Default] return true on click + release on same item <-- this is what the
               // majority of Button are using
  ANCHOR_ButtonFlags_PressedOnClickReleaseAnywhere =
      1 << 6,  // return true on click + release even if the release event is not done while
               // hovering the item
  ANCHOR_ButtonFlags_PressedOnRelease =
      1 << 7,  // return true on release (default requires click+release)
  ANCHOR_ButtonFlags_PressedOnDoubleClick =
      1 << 8,  // return true on double-click (default requires click+release)
  ANCHOR_ButtonFlags_PressedOnDragDropHold =
      1 << 9,  // return true when held into while we are drag and dropping another item (used by
               // e.g. tree nodes, collapsing headers)
  ANCHOR_ButtonFlags_Repeat = 1 << 10,  // hold to repeat
  ANCHOR_ButtonFlags_FlattenChildren =
      1 << 11,  // allow interactions even if a child window is overlapping
  ANCHOR_ButtonFlags_AllowItemOverlap =
      1 << 12,  // require previous frame HoveredId to either match id or be null before being
                // usable, use along with SetItemAllowOverlap()
  ANCHOR_ButtonFlags_DontClosePopups =
      1 << 13,  // disable automatically closing parent popup on press // [UNUSED]
  ANCHOR_ButtonFlags_Disabled = 1 << 14,  // disable interactions
  ANCHOR_ButtonFlags_AlignTextBaseLine =
      1 << 15,  // vertically align button to match text baseline - ButtonEx() only // FIXME:
                // Should be removed and handled by SmallButton(), not possible currently because
                // of DC.CursorPosPrevLine
  ANCHOR_ButtonFlags_NoKeyModifiers =
      1 << 16,  // disable mouse interaction if a key modifier is held
  ANCHOR_ButtonFlags_NoHoldingActiveId = 1 << 17,  // don't set ActiveId while holding the mouse
                                                   // (ANCHOR_ButtonFlags_PressedOnClick only)
  ANCHOR_ButtonFlags_NoNavFocus = 1 << 18,  // don't override navigation focus when activated
  ANCHOR_ButtonFlags_NoHoveredOnFocus =
      1 << 19,  // don't report as hovered when nav focus is on this item
  ANCHOR_ButtonFlags_PressedOnMask_ = ANCHOR_ButtonFlags_PressedOnClick |
                                      ANCHOR_ButtonFlags_PressedOnClickRelease |
                                      ANCHOR_ButtonFlags_PressedOnClickReleaseAnywhere |
                                      ANCHOR_ButtonFlags_PressedOnRelease |
                                      ANCHOR_ButtonFlags_PressedOnDoubleClick |
                                      ANCHOR_ButtonFlags_PressedOnDragDropHold,
  ANCHOR_ButtonFlags_PressedOnDefault_ = ANCHOR_ButtonFlags_PressedOnClickRelease
};

// Extend ANCHOR_SliderFlags_
enum ANCHOR_SliderFlagsPrivate_ {
  ANCHOR_SliderFlags_Vertical = 1 << 20,  // Should this slider be orientated vertically?
  ANCHOR_SliderFlags_ReadOnly = 1 << 21
};

// Extend ANCHORSelectableFlags_
enum ANCHORSelectableFlagsPrivate_ {
  // NB: need to be in sync with last value of ANCHORSelectableFlags_
  ANCHORSelectableFlags_NoHoldingActiveID = 1 << 20,
  ANCHORSelectableFlags_SelectOnClick =
      1 << 21,  // Override button behavior to react on Click (default is Click+Release)
  ANCHORSelectableFlags_SelectOnRelease =
      1 << 22,  // Override button behavior to react on Release (default is Click+Release)
  ANCHORSelectableFlags_SpanAvailWidth =
      1 << 23,  // Span all avail width even if we declared less for layout purpose. FIXME: We may
                // be able to remove this (added in 6251d379, 2bcafc86 for menus)
  ANCHORSelectableFlags_DrawHoveredWhenHeld =
      1 << 24,  // Always show active when held, even is not hovered. This concept could probably
                // be renamed/formalized somehow.
  ANCHORSelectableFlags_SetNavIdOnHover =
      1 << 25,  // Set Nav/Focus ID on mouse hover (used by MenuItem)
  ANCHORSelectableFlags_NoPadWithHalfSpacing =
      1 << 26  // Disable padding each side with ItemSpacing * 0.5f
};

// Extend ANCHOR_TreeNodeFlags_
enum ANCHOR_TreeNodeFlagsPrivate_ { ANCHOR_TreeNodeFlags_ClipLabelForTrailingButton = 1 << 20 };

enum ANCHOR_SeparatorFlags_ {
  ANCHOR_SeparatorFlags_None       = 0,
  ANCHOR_SeparatorFlags_Horizontal = 1 << 0,  // Axis default to current layout type, so generally
                                              // Horizontal unless e.g. in a menu bar
  ANCHOR_SeparatorFlags_Vertical       = 1 << 1,
  ANCHOR_SeparatorFlags_SpanAllColumns = 1 << 2
};

enum ANCHOR_TextFlags_ {
  ANCHOR_TextFlags_None                       = 0,
  ANCHOR_TextFlags_NoWidthForLargeClippedText = 1 << 0
};

enum ANCHOR_TooltipFlags_ {
  ANCHOR_TooltipFlags_None = 0,
  ANCHOR_TooltipFlags_OverridePreviousTooltip =
      1 << 0  // Override will clear/ignore previously submitted tooltip (defaults to append)
};

// FIXME: this is in development, not exposed/functional as a generic feature yet.
// Horizontal/Vertical enums are fixed to 0/1 so they may be used to index wabi::GfVec2f
enum ANCHOR_LayoutType_ { ANCHOR_LayoutType_Horizontal = 0, ANCHOR_LayoutType_Vertical = 1 };

enum ANCHORLogType {
  ANCHORLogType_None = 0,
  ANCHORLogType_TTY,
  ANCHORLogType_File,
  ANCHORLogType_Buffer,
  ANCHORLogType_Clipboard
};

// X/Y enums are fixed to 0/1 so they may be used to index wabi::GfVec2f
enum ANCHOR_Axis { ANCHOR_Axis_None = -1, ANCHOR_Axis_X = 0, ANCHOR_Axis_Y = 1 };

enum ANCHORPlotType { ANCHORPlotType_Lines, ANCHORPlotType_Histogram };

enum ANCHORInputSource {
  ANCHORInputSource_None = 0,
  ANCHORInputSource_Mouse,
  ANCHORInputSource_Keyboard,
  ANCHORInputSource_Gamepad,
  ANCHORInputSource_Nav,        // Stored in g.ActiveIdSource only
  ANCHORInputSource_Clipboard,  // Currently only used by InputText()
  ANCHORInputSource_COUNT
};

// FIXME-NAV: Clarify/expose various repeat delay/rate
enum ANCHOR_InputReadMode {
  ANCHOR_InputReadMode_Down,
  ANCHOR_InputReadMode_Pressed,
  ANCHOR_InputReadMode_Released,
  ANCHOR_InputReadMode_Repeat,
  ANCHOR_InputReadMode_RepeatSlow,
  ANCHOR_InputReadMode_RepeatFast
};

enum ANCHOR_NavHighlightFlags_ {
  ANCHOR_NavHighlightFlags_None        = 0,
  ANCHOR_NavHighlightFlags_TypeDefault = 1 << 0,
  ANCHOR_NavHighlightFlags_TypeThin    = 1 << 1,
  ANCHOR_NavHighlightFlags_AlwaysDraw =
      1 << 2,  // Draw rectangular highlight if (g.NavId == id) _even_ when using the mouse.
  ANCHOR_NavHighlightFlags_NoRounding = 1 << 3
};

enum ANCHOR_NavDirSourceFlags_ {
  ANCHOR_NavDirSourceFlags_None      = 0,
  ANCHOR_NavDirSourceFlags_Keyboard  = 1 << 0,
  ANCHOR_NavDirSourceFlags_PadDPad   = 1 << 1,
  ANCHOR_NavDirSourceFlags_PadLStick = 1 << 2
};

enum ANCHOR_NavMoveFlags_ {
  ANCHOR_NavMoveFlags_None  = 0,
  ANCHOR_NavMoveFlags_LoopX = 1 << 0,  // On failed request, restart from opposite side
  ANCHOR_NavMoveFlags_LoopY = 1 << 1,
  ANCHOR_NavMoveFlags_WrapX =
      1 << 2,  // On failed request, request from opposite side one line down (when NavDir==right)
               // or one line up (when NavDir==left)
  ANCHOR_NavMoveFlags_WrapY = 1 << 3,  // This is not super useful for provided for completeness
  ANCHOR_NavMoveFlags_AllowCurrentNavId =
      1 << 4,  // Allow scoring and considering the current NavId as a move target candidate. This
               // is used when the move source is offset (e.g. pressing PageDown actually needs to
               // send a Up move request, if we are pressing PageDown from the bottom-most item we
               // need to stay in place)
  ANCHOR_NavMoveFlags_AlsoScoreVisibleSet =
      1 << 5,  // Store alternate result in NavMoveResultLocalVisibleSet that only comprise
               // elements that are already fully visible.
  ANCHOR_NavMoveFlags_ScrollToEdge = 1 << 6
};

enum ANCHORNavForward {
  ANCHORNavForward_None,
  ANCHORNavForward_ForwardQueued,
  ANCHORNavForward_ForwardActive
};

enum ANCHORNavLayer {
  ANCHORNavLayer_Main = 0,  // Main scrolling layer
  ANCHORNavLayer_Menu = 1,  // Menu layer (access with Alt/ANCHOR_NavInput_Menu)
  ANCHORNavLayer_COUNT
};

enum ANCHORPopupPositionPolicy {
  ANCHORPopupPositionPolicy_Default,
  ANCHORPopupPositionPolicy_ComboBox,
  ANCHORPopupPositionPolicy_Tooltip
};

struct ANCHOR_DataTypeTempStorage {
  ImU8 Data[8];  // Can fit any data up to ANCHOR_DataType_COUNT
};

// Type information associated to one ANCHOR_DataType. Retrieve with DataTypeGetInfo().
struct ANCHOR_DataTypeInfo {
  size_t Size;           // Size in bytes
  const char *Name;      // Short descriptive name for the type, for debugging
  const char *PrintFmt;  // Default printf format for the type
  const char *ScanFmt;   // Default scanf format for the type
};

// Extend ANCHOR_DataType_
enum ANCHOR_DataTypePrivate_ {
  ANCHOR_DataType_String = ANCHOR_DataType_COUNT + 1,
  ANCHOR_DataType_Pointer,
  ANCHOR_DataType_ID
};

// Stacked color modifier, backup of modified data so we can restore it
struct ANCHOR_ColorMod {
  ANCHOR_Col Col;
  wabi::GfVec4f BackupValue;
};

// Stacked style modifier, backup of modified data so we can restore it. Data type inferred from
// the variable.
struct ANCHOR_StyleMod {
  ANCHOR_StyleVar VarIdx;
  union {
    int BackupInt[2];
    float BackupFloat[2];
  };
  ANCHOR_StyleMod(ANCHOR_StyleVar idx, int v)
  {
    VarIdx       = idx;
    BackupInt[0] = v;
  }
  ANCHOR_StyleMod(ANCHOR_StyleVar idx, float v)
  {
    VarIdx         = idx;
    BackupFloat[0] = v;
  }
  ANCHOR_StyleMod(ANCHOR_StyleVar idx, wabi::GfVec2f v)
  {
    VarIdx         = idx;
    BackupFloat[0] = v[0];
    BackupFloat[1] = v[1];
  }
};

// Stacked storage data for BeginGroup()/EndGroup()
struct ANCHOR_API ANCHOR_GroupData {
  ANCHOR_ID WindowID;
  wabi::GfVec2f BackupCursorPos;
  wabi::GfVec2f BackupCursorMaxPos;
  ImVec1 BackupIndent;
  ImVec1 BackupGroupOffset;
  wabi::GfVec2f BackupCurrLineSize;
  float BackupCurrLineTextBaseOffset;
  ANCHOR_ID BackupActiveIdIsAlive;
  bool BackupActiveIdPreviousFrameIsAlive;
  bool BackupHoveredIdIsAlive;
  bool EmitItem;
};

// Simple column measurement, currently used for MenuItem() only.. This is very
// short-sighted/throw-away code and NOT a generic helper.
struct ANCHOR_API ANCHOR_MenuColumns {
  float Spacing;
  float Width, NextWidth;
  float Pos[3], NextWidths[3];

  ANCHOR_MenuColumns()
  {
    memset(this, 0, sizeof(*this));
  }
  void Update(int count, float spacing, bool clear);
  float DeclColumns(float w0, float w1, float w2);
  float CalcExtraSpace(float avail_w) const;
};

// Internal state of the currently focused/edited text input box
// For a given item ID, access with ANCHOR::GetInputTextState()
struct ANCHOR_API ANCHOR_InputTextState {
  ANCHOR_ID ID;          // widget id owning the text state
  int CurLenW, CurLenA;  // we need to maintain our buffer length in both UTF-8 and wchar format.
                         // UTF-8 length is valid even if TextA is not.
  AnchorVector<AnchorWChar>
      TextW;                 // edit buffer, we need to persist but can't guarantee the persistence
                             // of the user-provided buffer. so we copy into own buffer.
  AnchorVector<char> TextA;  // temporary UTF8 buffer for callbacks and other operations. this is
                             // not updated in every code-path! size=capacity.
  AnchorVector<char>
      InitialTextA;   // backup of end-user buffer at the time of focus (in UTF-8, unaltered)
  bool TextAIsValid;  // temporary UTF8 buffer is not initially valid before we make the widget
                      // active (until then we pull the data from user argument)
  int BufCapacityA;   // end-user buffer capacity
  float ScrollX;      // horizontal scrolling/offset
  ImStb::STB_TexteditState Stb;  // state for stb_textedit.h
  float CursorAnim;   // timer for cursor blink, reset on every user action so the cursor reappears
                      // immediately
  bool CursorFollow;  // set when we want scrolling to follow the current cursor position (not
                      // always!)
  bool SelectedAllMouseLock;   // after a double-click to select all, we ignore further mouse drags
                               // to update selection
  bool Edited;                 // edited this frame
  ANCHORInputTextFlags Flags;  // copy of InputText() flags
  ANCHORInputTextCallback UserCallback;  // "
  void *UserCallbackData;                // "

  ANCHOR_InputTextState()
  {
    memset(this, 0, sizeof(*this));
  }
  void ClearText()
  {
    CurLenW = CurLenA = 0;
    TextW[0]          = 0;
    TextA[0]          = 0;
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
    Stb.cursor       = ImMin(Stb.cursor, CurLenW);
    Stb.select_start = ImMin(Stb.select_start, CurLenW);
    Stb.select_end   = ImMin(Stb.select_end, CurLenW);
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
    Stb.has_preferred_x         = 0;
  }
};

// Storage for current popup stack
struct ANCHOR_PopupData {
  ANCHOR_ID PopupId;  // Set on OpenPopup()
  ANCHOR_Window
      *Window;  // Resolved on BeginPopup() - may stay unresolved if user never calls OpenPopup()
  ANCHOR_Window
      *SourceWindow;       // Set on OpenPopup() copy of NavWindow at the time of opening the popup
  int OpenFrameCount;      // Set on OpenPopup()
  ANCHOR_ID OpenParentId;  // Set on OpenPopup(), we need this to differentiate multiple menu sets
                           // from each others (e.g. inside menu bar vs loose menu items)
  wabi::GfVec2f OpenPopupPos;  // Set on OpenPopup(), preferred popup position (typically ==
                               // OpenMousePos when using mouse)
  wabi::GfVec2f
      OpenMousePos;  // Set on OpenPopup(), copy of mouse position at the time of opening popup

  ANCHOR_PopupData()
  {
    memset(this, 0, sizeof(*this));
    OpenFrameCount = -1;
  }
};

struct ANCHOR_NavItemData {
  ANCHOR_Window *Window;   // Init,Move    // Best candidate window
                           // (result->ItemWindow->RootWindowForNav == request->Window)
  ANCHOR_ID ID;            // Init,Move    // Best candidate item ID
  ANCHOR_ID FocusScopeId;  // Init,Move    // Best candidate focus scope ID
  ImRect RectRel;          // Init,Move    // Best candidate bounding box in window relative space
  float DistBox;           //      Move    // Best candidate box distance to current NavId
  float DistCenter;        //      Move    // Best candidate center distance to current NavId
  float DistAxial;         //      Move    // Best candidate axial distance to current NavId

  ANCHOR_NavItemData()
  {
    Clear();
  }
  void Clear()
  {
    Window = NULL;
    ID = FocusScopeId = 0;
    RectRel           = ImRect();
    DistBox = DistCenter = DistAxial = FLT_MAX;
  }
};

enum ANCHOR_NextWindowDataFlags_ {
  ANCHOR_NextWindowDataFlags_None              = 0,
  ANCHOR_NextWindowDataFlags_HasPos            = 1 << 0,
  ANCHOR_NextWindowDataFlags_HasSize           = 1 << 1,
  ANCHOR_NextWindowDataFlags_HasContentSize    = 1 << 2,
  ANCHOR_NextWindowDataFlags_HasCollapsed      = 1 << 3,
  ANCHOR_NextWindowDataFlags_HasSizeConstraint = 1 << 4,
  ANCHOR_NextWindowDataFlags_HasFocus          = 1 << 5,
  ANCHOR_NextWindowDataFlags_HasBgAlpha        = 1 << 6,
  ANCHOR_NextWindowDataFlags_HasScroll         = 1 << 7
};

// Storage for SetNexWindow** functions
struct ANCHOR_NextWindowData {
  ANCHOR_NextWindowDataFlags Flags;
  ANCHOR_Cond PosCond;
  ANCHOR_Cond SizeCond;
  ANCHOR_Cond CollapsedCond;
  wabi::GfVec2f PosVal;
  wabi::GfVec2f PosPivotVal;
  wabi::GfVec2f SizeVal;
  wabi::GfVec2f ContentSizeVal;
  wabi::GfVec2f ScrollVal;
  bool CollapsedVal;
  ImRect SizeConstraintRect;
  ANCHORSizeCallback SizeCallback;
  void *SizeCallbackUserData;
  float BgAlphaVal;  // Override background alpha
  wabi::GfVec2f
      MenuBarOffsetMinVal;  // *Always on* This is not exposed publicly, so we don't clear it.

  ANCHOR_NextWindowData()
  {
    memset(this, 0, sizeof(*this));
  }
  inline void ClearFlags()
  {
    Flags = ANCHOR_NextWindowDataFlags_None;
  }
};

enum ANCHOR_NextItemDataFlags_ {
  ANCHOR_NextItemDataFlags_None     = 0,
  ANCHOR_NextItemDataFlags_HasWidth = 1 << 0,
  ANCHOR_NextItemDataFlags_HasOpen  = 1 << 1
};

struct ANCHOR_NextItemData {
  ANCHOR_NextItemDataFlags Flags;
  float Width;  // Set by SetNextItemWidth()
  ANCHOR_ID
  FocusScopeId;  // Set by SetNextItemMultiSelectData() (!= 0 signify value has been set, so
                 // it's an alternate version of HasSelectionData, we don't use Flags for this
                 // because they are cleared too early. This is mostly used for debugging)
  ANCHOR_Cond OpenCond;
  bool OpenVal;  // Set by SetNextItemOpen()

  ANCHOR_NextItemData()
  {
    memset(this, 0, sizeof(*this));
  }
  inline void ClearFlags()
  {
    Flags = ANCHOR_NextItemDataFlags_None;
  }  // Also cleared manually by ItemAdd()!
};

struct ANCHOR_ShrinkWidthItem {
  int Index;
  float Width;
};

struct ANCHOR_PtrOrIndex {
  void *Ptr;  // Either field can be set, not both. e.g. Dock node tab bars are loose while
              // BeginTabBar() ones are in a pool.
  int Index;  // Usually index in a main pool.

  ANCHOR_PtrOrIndex(void *ptr)
  {
    Ptr   = ptr;
    Index = -1;
  }
  ANCHOR_PtrOrIndex(int index)
  {
    Ptr   = NULL;
    Index = index;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Columns support
//-----------------------------------------------------------------------------

// Flags for internal's BeginColumns(). Prefix using BeginTable() nowadays!
enum ANCHOR_OldColumnFlags_ {
  ANCHOR_OldColumnFlags_None     = 0,
  ANCHOR_OldColumnFlags_NoBorder = 1 << 0,  // Disable column dividers
  ANCHOR_OldColumnFlags_NoResize = 1
                                   << 1,  // Disable resizing columns when clicking on the dividers
  ANCHOR_OldColumnFlags_NoPreserveWidths =
      1 << 2,  // Disable column width preservation when adjusting columns
  ANCHOR_OldColumnFlags_NoForceWithinWindow =
      1 << 3,  // Disable forcing columns to fit within window
  ANCHOR_OldColumnFlags_GrowParentContentsSize =
      1 << 4  // (WIP) Restore pre-1.51 behavior of extending the parent window contents size but
              // _without affecting the columns width at all_. Will eventually remove.

// Obsolete names (will be removed)
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  ,
  ANCHOR_ColumnsFlags_None                   = ANCHOR_OldColumnFlags_None,
  ANCHOR_ColumnsFlags_NoBorder               = ANCHOR_OldColumnFlags_NoBorder,
  ANCHOR_ColumnsFlags_NoResize               = ANCHOR_OldColumnFlags_NoResize,
  ANCHOR_ColumnsFlags_NoPreserveWidths       = ANCHOR_OldColumnFlags_NoPreserveWidths,
  ANCHOR_ColumnsFlags_NoForceWithinWindow    = ANCHOR_OldColumnFlags_NoForceWithinWindow,
  ANCHOR_ColumnsFlags_GrowParentContentsSize = ANCHOR_OldColumnFlags_GrowParentContentsSize
#  endif
};

struct ANCHOR_OldColumnData {
  float OffsetNorm;  // Column start offset, normalized 0.0 (far left) -> 1.0 (far right)
  float OffsetNormBeforeResize;
  ANCHOR_OldColumnFlags Flags;  // Not exposed
  ImRect ClipRect;

  ANCHOR_OldColumnData()
  {
    memset(this, 0, sizeof(*this));
  }
};

struct ANCHOR_OldColumns {
  ANCHOR_ID ID;
  ANCHOR_OldColumnFlags Flags;
  bool IsFirstFrame;
  bool IsBeingResized;
  int Current;
  int Count;
  float OffMinX, OffMaxX;  // Offsets from HostWorkRect.Min[0]
  float LineMinY, LineMaxY;
  float HostCursorPosY;             // Backup of CursorPos at the time of BeginColumns()
  float HostCursorMaxPosX;          // Backup of CursorMaxPos at the time of BeginColumns()
  ImRect HostInitialClipRect;       // Backup of ClipRect at the time of BeginColumns()
  ImRect HostBackupClipRect;        // Backup of ClipRect during
                                    // PushColumnsBackground()/PopColumnsBackground()
  ImRect HostBackupParentWorkRect;  // Backup of WorkRect at the time of BeginColumns()
  AnchorVector<ANCHOR_OldColumnData> Columns;
  ImDrawListSplitter Splitter;

  ANCHOR_OldColumns()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Multi-select support
//-----------------------------------------------------------------------------

#  ifdef ANCHOR_HAS_MULTI_SELECT
// <this is filled in 'range_select' branch>
#  endif  // #ifdef ANCHOR_HAS_MULTI_SELECT

//-----------------------------------------------------------------------------
// [SECTION] Docking support
//-----------------------------------------------------------------------------

#  ifdef ANCHOR_HAS_DOCK
// <this is filled in 'docking' branch>
#  endif  // #ifdef ANCHOR_HAS_DOCK

//-----------------------------------------------------------------------------
// [SECTION] Viewport support
//-----------------------------------------------------------------------------

// ANCHORViewport Private/Internals fields (cardinal sin: we are using inheritance!)
// Every instance of ANCHORViewport is in fact a ANCHORViewportP.
struct ANCHORViewportP : public ANCHORViewport {
  int DrawListsLastFrame[2];  // Last frame number the background (0) and foreground (1) draw lists
                              // were used
  ImDrawList *DrawLists[2];   // Convenience background (0) and foreground (1) draw lists. We use
                              // them to draw software mouser cursor when io.MouseDrawCursor is set
                              // and to draw most debug overlays.
  ImDrawData DrawDataP;
  ImDrawDataBuilder DrawDataBuilder;

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

  ANCHORViewportP()
  {
    DrawListsLastFrame[0] = DrawListsLastFrame[1] = -1;
    DrawLists[0] = DrawLists[1] = NULL;
  }
  ~ANCHORViewportP()
  {
    if (DrawLists[0])
      IM_DELETE(DrawLists[0]);
    if (DrawLists[1])
      IM_DELETE(DrawLists[1]);
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
    WorkPos  = CalcWorkRectPos(WorkOffsetMin);
    WorkSize = CalcWorkRectSize(WorkOffsetMin, WorkOffsetMax);
  }  // Update public fields

  // Helpers to retrieve ImRect (we don't need to store BuildWorkRect as every access tend to
  // change it, hence the code asymmetry)
  ImRect GetMainRect() const
  {
    return ImRect(Pos[0], Pos[1], Pos[0] + Size[0], Pos[1] + Size[1]);
  }
  ImRect GetWorkRect() const
  {
    return ImRect(WorkPos[0], WorkPos[1], WorkPos[0] + WorkSize[0], WorkPos[1] + WorkSize[1]);
  }
  ImRect GetBuildWorkRect() const
  {
    wabi::GfVec2f pos  = CalcWorkRectPos(BuildWorkOffsetMin);
    wabi::GfVec2f size = CalcWorkRectSize(BuildWorkOffsetMin, BuildWorkOffsetMax);
    return ImRect(pos[0], pos[1], pos[0] + size[0], pos[1] + size[1]);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Settings support
//-----------------------------------------------------------------------------

// Windows data saved in ANCHOR.ini file
// Because we never destroy or rename ANCHOR_WindowSettings, we can store the names in a separate
// buffer easily. (this is designed to be stored in a ImChunkStream buffer, with the
// variable-length Name following our structure)
struct ANCHOR_WindowSettings {
  ANCHOR_ID ID;
  wabi::GfVec2h Pos;
  wabi::GfVec2h Size;
  bool Collapsed;
  bool WantApply;  // Set when loaded from .ini data (to enable merging/loading .ini data into an
                   // already running context)

  ANCHOR_WindowSettings()
  {
    memset(this, 0, sizeof(*this));
  }
  char *GetName()
  {
    return (char *)(this + 1);
  }
};

struct ANCHOR_SettingsHandler {
  const char *TypeName;  // Short description stored in .ini file. Disallowed characters: '[' ']'
  ANCHOR_ID TypeHash;    // == ImHashStr(TypeName)
  void (*ClearAllFn)(ANCHOR_Context *ctx,
                     ANCHOR_SettingsHandler *handler);  // Clear all settings data
  void (*ReadInitFn)(
      ANCHOR_Context *ctx,
      ANCHOR_SettingsHandler *handler);  // Read: Called before reading (in registration order)
  void *(*ReadOpenFn)(
      ANCHOR_Context *ctx,
      ANCHOR_SettingsHandler *handler,
      const char *name);  // Read: Called when entering into a new ini entry e.g. "[Window][Name]"
  void (*ReadLineFn)(ANCHOR_Context *ctx,
                     ANCHOR_SettingsHandler *handler,
                     void *entry,
                     const char *line);  // Read: Called for every line of text within an ini entry
  void (*ApplyAllFn)(
      ANCHOR_Context *ctx,
      ANCHOR_SettingsHandler *handler);  // Read: Called after reading (in registration order)
  void (*WriteAllFn)(ANCHOR_Context *ctx,
                     ANCHOR_SettingsHandler *handler,
                     ANCHORTextBuffer *out_buf);  // Write: Output every entries into 'out_buf'
  void *UserData;

  ANCHOR_SettingsHandler()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Metrics, Debug
//-----------------------------------------------------------------------------

struct ANCHOR_MetricsConfig {
  bool ShowWindowsRects;
  bool ShowWindowsBeginOrder;
  bool ShowTablesRects;
  bool ShowDrawCmdMesh;
  bool ShowDrawCmdBoundingBoxes;
  int ShowWindowsRectsType;
  int ShowTablesRectsType;

  ANCHOR_MetricsConfig()
  {
    ShowWindowsRects         = false;
    ShowWindowsBeginOrder    = false;
    ShowTablesRects          = false;
    ShowDrawCmdMesh          = true;
    ShowDrawCmdBoundingBoxes = true;
    ShowWindowsRectsType     = -1;
    ShowTablesRectsType      = -1;
  }
};

struct ANCHOR_API ANCHOR_StackSizes {
  short SizeOfIDStack;
  short SizeOfColorStack;
  short SizeOfStyleVarStack;
  short SizeOfFontStack;
  short SizeOfFocusScopeStack;
  short SizeOfGroupStack;
  short SizeOfBeginPopupStack;

  ANCHOR_StackSizes()
  {
    memset(this, 0, sizeof(*this));
  }
  void SetToCurrentState();
  void CompareWithCurrentState();
};

//-----------------------------------------------------------------------------
// [SECTION] Generic context hooks
//-----------------------------------------------------------------------------

typedef void (*ANCHOR_ContextHookCallback)(ANCHOR_Context *ctx, ANCHOR_ContextHook *hook);
enum ANCHOR_ContextHookType {
  ANCHOR_ContextHookType_NewFramePre,
  ANCHOR_ContextHookType_NewFramePost,
  ANCHOR_ContextHookType_EndFramePre,
  ANCHOR_ContextHookType_EndFramePost,
  ANCHOR_ContextHookType_RenderPre,
  ANCHOR_ContextHookType_RenderPost,
  ANCHOR_ContextHookType_Shutdown,
  ANCHOR_ContextHookType_PendingRemoval_
};

struct ANCHOR_ContextHook {
  ANCHOR_ID HookId;  // A unique ID assigned by AddContextHook()
  ANCHOR_ContextHookType Type;
  ANCHOR_ID Owner;
  ANCHOR_ContextHookCallback Callback;
  void *UserData;

  ANCHOR_ContextHook()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] ANCHOR_Context (main ANCHOR context)
//-----------------------------------------------------------------------------

struct ANCHOR_Context {
  bool Initialized;
  bool FontAtlasOwnedByContext;  // IO.Fonts-> is owned by the ANCHOR_Context and will be
                                 // destructed along with it.
  /**
   * Pixar Hydra Driver.
   * Points to the Rendering Device and is
   * responsible for instancing any number
   * of Hydra Engines -- all using the same
   * underlying & shared Graphics Resources. */
  wabi::HdDriver HydraDriver;
  APOLLO_EnginePtr ApolloEngine;

  ANCHOR_IO IO;
  ANCHOR_Style Style;
  AnchorFont *Font;    // (Shortcut) == FontStack.empty() ? IO.Font : FontStack.back()
  float FontSize;      // (Shortcut) == FontBaseSize * g.CurrentWindow->FontWindowScale ==
                       // window->FontSize(). Text height for current window.
  float FontBaseSize;  // (Shortcut) == IO.FontGlobalScale * Font->Scale * Font->FontSize. Base
                       // text height.
  ImDrawListSharedData DrawListSharedData;
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
  AnchorVector<ANCHOR_Window *> Windows;  // Windows, sorted in display order, back to front
  AnchorVector<ANCHOR_Window *>
      WindowsFocusOrder;  // Root windows, sorted in focus order, back to front.
  AnchorVector<ANCHOR_Window *>
      WindowsTempSortBuffer;  // Temporary buffer used in EndFrame() to reorder windows so parents
                              // are kept before their child
  AnchorVector<ANCHOR_Window *> CurrentWindowStack;
  ANCHORStorage WindowsById;          // Map window's ANCHOR_ID to ANCHOR_Window*
  int WindowsActiveCount;             // Number of unique windows submitted by frame
  wabi::GfVec2f WindowsHoverPadding;  // Padding around resizable windows for which hovering on
                                      // counts as hovering the window ==
                                      // AnchorMax(style.TouchExtraPadding, WINDOWS_HOVER_PADDING)
  ANCHOR_Window *CurrentWindow;       // Window being drawn into
  ANCHOR_Window
      *HoveredWindow;  // Window the mouse is hovering. Will typically catch mouse inputs.
  ANCHOR_Window *HoveredWindowUnderMovingWindow;  // Hovered window ignoring MovingWindow. Only set
                                                  // if MovingWindow is set.
  ANCHOR_Window
      *MovingWindow;  // Track the window we clicked on (in order to preserve focus). The actual
                      // window that is moved is generally MovingWindow->RootWindow.
  ANCHOR_Window
      *WheelingWindow;  // Track the window we started mouse-wheeling on. Until a timer elapse or
                        // mouse has moved, generally keep scrolling the same window even if during
                        // the course of scrolling the mouse ends up hovering a child window.
  wabi::GfVec2f WheelingWindowRefMousePos;
  float WheelingWindowTimer;

  // Item/widgets state and tracking information
  ANCHOR_ItemFlags CurrentItemFlags;  // == g.ItemFlagsStack.back()
  ANCHOR_ID HoveredId;                // Hovered widget, filled during the frame
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
  ImU64 ActiveIdUsingKeyInputMask;    // Active widget will want to read those key inputs. When we
                                      // grow the ANCHOR_Key enum we'll need to either to order the
                                      // enum to make useful keys come first, either redesign this
                                      // into e.g. a small array.
  wabi::GfVec2f ActiveIdClickOffset;  // Clicked offset from upper-left corner, if applicable
                                      // (currently only set by ButtonBehavior)
  ANCHOR_Window *ActiveIdWindow;
  ANCHORInputSource ActiveIdSource;  // Activating with mouse or nav (gamepad/keyboard)
  int ActiveIdMouseButton;
  ANCHOR_ID ActiveIdPreviousFrame;
  bool ActiveIdPreviousFrameIsAlive;
  bool ActiveIdPreviousFrameHasBeenEditedBefore;
  ANCHOR_Window *ActiveIdPreviousFrameWindow;
  ANCHOR_ID LastActiveId;   // Store the last non-zero ActiveId, useful for animation.
  float LastActiveIdTimer;  // Store the last non-zero ActiveId timer since the beginning of
                            // activation, useful for animation.

  // Next window/item data
  ANCHOR_NextWindowData NextWindowData;  // Storage for SetNextWindow** functions
  ANCHOR_NextItemData NextItemData;      // Storage for SetNextItem** functions

  // Shared stacks
  AnchorVector<ANCHOR_ColorMod>
      ColorStack;  // Stack for PushStyleColor()/PopStyleColor() - inherited by Begin()
  AnchorVector<ANCHOR_StyleMod>
      StyleVarStack;  // Stack for PushStyleVar()/PopStyleVar() - inherited by Begin()
  AnchorVector<AnchorFont *> FontStack;  // Stack for PushFont()/PopFont() - inherited by Begin()
  AnchorVector<ANCHOR_ID> FocusScopeStack;  // Stack for PushFocusScope()/PopFocusScope() - not
                                            // inherited by Begin(), unless child window
  AnchorVector<ANCHOR_ItemFlags>
      ItemFlagsStack;  // Stack for PushItemFlag()/PopItemFlag() - inherited by Begin()
  AnchorVector<ANCHOR_GroupData>
      GroupStack;  // Stack for BeginGroup()/EndGroup() - not inherited by Begin()
  AnchorVector<ANCHOR_PopupData> OpenPopupStack;  // Which popups are open (persistent)
  AnchorVector<ANCHOR_PopupData>
      BeginPopupStack;  // Which level of BeginPopup() we are in (reset every frame)

  // Viewports
  AnchorVector<ANCHORViewportP *> Viewports;  // Active viewports (Size==1 in 'master' branch).
                                              // Each viewports hold their copy of ImDrawData.

  // Gamepad/keyboard Navigation
  ANCHOR_Window *NavWindow;   // Focused window for navigation. Could be called 'FocusWindow'
  ANCHOR_ID NavId;            // Focused item for navigation
  ANCHOR_ID NavFocusScopeId;  // Identify a selection scope (selection code often wants to "clear
                              // other items" when landing on an item of the selection set)
  ANCHOR_ID NavActivateId;  // ~~ (g.ActiveId == 0) && IsNavInputPressed(ANCHOR_NavInput_Activate)
                            // ? NavId : 0, also set when calling ActivateItem()
  ANCHOR_ID NavActivateDownId;     // ~~ IsNavInputDown(ANCHOR_NavInput_Activate) ? NavId : 0
  ANCHOR_ID NavActivatePressedId;  // ~~ IsNavInputPressed(ANCHOR_NavInput_Activate) ? NavId : 0
  ANCHOR_ID NavInputId;            // ~~ IsNavInputPressed(ANCHOR_NavInput_Input) ? NavId : 0
  ANCHOR_ID NavJustTabbedId;       // Just tabbed to this id.
  ANCHOR_ID NavJustMovedToId;  // Just navigated to this id (result of a successfully MoveRequest).
  ANCHOR_ID NavJustMovedToFocusScopeId;  // Just navigated to this focus scope id (result of a
                                         // successfully MoveRequest).
  ANCHOR_KeyModFlags NavJustMovedToKeyMods;
  ANCHOR_ID NavNextActivateId;       // Set by ActivateItem(), queued until next frame.
  ANCHORInputSource NavInputSource;  // Keyboard or Gamepad mode? THIS WILL ONLY BE None or
                                     // NavGamepad or NavKeyboard.
  ImRect NavScoringRect;             // Rectangle used for scoring, in screen space. Based of
                          // window->NavRectRel[], modified for directional navigation scoring.
  int NavScoringCount;      // Metrics for debugging
  ANCHORNavLayer NavLayer;  // Layer we are navigating on. For now the system is hard-coded for
                            // 0=main contents and 1=menu/title bar, may expose layers later.
  int NavIdTabCounter;      // == NavWindow->DC.FocusIdxTabCounter at time of NavId processing
  bool NavIdIsAlive;        // Nav widget has been seen this frame ~~ NavRectRel is valid
  bool NavMousePosDirty;    // When set we will update mouse position if (io.ConfigFlags &
                            // ANCHORConfigFlags_NavEnableSetMousePos) if set (NB: this not enabled
                            // by default)
  bool NavDisableHighlight;  // When user starts using mouse, we hide gamepad/keyboard highlight
                             // (NB: but they are still available, which is why NavDisableHighlight
                             // isn't always != NavDisableMouseHover)
  bool NavDisableMouseHover;  // When user starts using gamepad/keyboard, we hide mouse hovering
                              // highlight until mouse is touched again.
  bool NavAnyRequest;         // ~~ NavMoveRequest || NavInitRequest
  bool NavInitRequest;        // Init request for appearing window to select first item
  bool NavInitRequestFromMove;
  ANCHOR_ID NavInitResultId;    // Init request result (first item of the window, or one for which
                                // SetItemDefaultFocus() was called)
  ImRect NavInitResultRectRel;  // Init request result rectangle (relative to parent window)
  bool NavMoveRequest;          // Move request for this frame
  ANCHOR_NavMoveFlags NavMoveRequestFlags;
  ANCHORNavForward NavMoveRequestForward;  // None / ForwardQueued / ForwardActive (this is used to
                                           // navigate sibling parent menus from a child menu)
  ANCHOR_KeyModFlags NavMoveRequestKeyMods;
  ANCHOR_Dir NavMoveDir, NavMoveDirLast;  // Direction of the move request (left/right/up/down),
                                          // direction of the previous move request
  ANCHOR_Dir
      NavMoveClipDir;  // FIXME-NAV: Describe the purpose of this better. Might want to rename?
  ANCHOR_NavItemData NavMoveResultLocal;  // Best move request candidate within NavWindow
  ANCHOR_NavItemData
      NavMoveResultLocalVisibleSet;  // Best move request candidate within NavWindow that are
                                     // mostly visible (when using
                                     // ANCHOR_NavMoveFlags_AlsoScoreVisibleSet flag)
  ANCHOR_NavItemData
      NavMoveResultOther;  // Best move request candidate within NavWindow's flattened hierarchy
                           // (when using ANCHOR_WindowFlags_NavFlattened flag)
  ANCHOR_Window *NavWrapRequestWindow;      // Window which requested trying nav wrap-around.
  ANCHOR_NavMoveFlags NavWrapRequestFlags;  // Wrap-around operation flags.

  // Navigation: Windowing (CTRL+TAB for list, or Menu button + keys or directional pads to
  // move/resize)
  ANCHOR_Window
      *NavWindowingTarget;  // Target window when doing CTRL+Tab (or Pad Menu + FocusPrev/Next),
                            // this window is temporarily displayed top-most!
  ANCHOR_Window *NavWindowingTargetAnim;  // Record of last valid NavWindowingTarget until
                                          // DimBgRatio and NavWindowingHighlightAlpha becomes
                                          // 0.0f, so the fade-out can stay on it.
  ANCHOR_Window *NavWindowingListWindow;  // Internal window actually listing the CTRL+Tab contents
  float NavWindowingTimer;
  float NavWindowingHighlightAlpha;
  bool NavWindowingToggleLayer;

  // Legacy Focus/Tabbing system (older than Nav, active even if Nav is disabled, misnamed.
  // FIXME-NAV: This needs a redesign!)
  ANCHOR_Window *TabFocusRequestCurrWindow;  //
  ANCHOR_Window *TabFocusRequestNextWindow;  //
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
  ANCHOR_MouseCursor MouseCursor;

  // Drag and Drop
  bool DragDropActive;
  bool DragDropWithinSource;  // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag
                              // source.
  bool DragDropWithinTarget;  // Set when within a BeginDragDropXXX/EndDragDropXXX block for a drag
                              // target.
  ANCHORDragDropFlags DragDropSourceFlags;
  int DragDropSourceFrameCount;
  int DragDropMouseButton;
  ANCHORPayload DragDropPayload;
  ImRect DragDropTargetRect;  // Store rectangle of current target candidate (we favor small
                              // targets when overlapping)
  ANCHOR_ID DragDropTargetId;
  ANCHORDragDropFlags DragDropAcceptFlags;
  float DragDropAcceptIdCurrRectSurface;  // Target item surface (we resolve overlapping targets by
                                          // prioritizing the smaller surface)
  ANCHOR_ID DragDropAcceptIdCurr;  // Target item id (set at the time of accepting the payload)
  ANCHOR_ID DragDropAcceptIdPrev;  // Target item id from previous frame (we need to store this to
                                   // allow for overlapping drag and drop targets)
  int DragDropAcceptFrameCount;    // Last time a target expressed a desire to accept the source
  ANCHOR_ID DragDropHoldJustPressedId;  // Set when holding a payload just made ButtonBehavior()
                                        // return a press.
  AnchorVector<unsigned char>
      DragDropPayloadBufHeap;                 // We don't expose the AnchorVector<> directly,
                                              // ANCHORPayload only holds pointer+size
  unsigned char DragDropPayloadBufLocal[16];  // Local buffer for small payloads

  // Table
  ANCHOR_Table *CurrentTable;
  int CurrentTableStackIdx;
  ImPool<ANCHOR_Table> Tables;
  AnchorVector<ANCHOR_TableTempData> TablesTempDataStack;
  AnchorVector<float>
      TablesLastTimeActive;  // Last used timestamp of each tables (SOA, for efficient GC)
  AnchorVector<ImDrawChannel> DrawChannelsTempMergeBuffer;

  // Tab bars
  ANCHOR_TabBar *CurrentTabBar;
  ImPool<ANCHOR_TabBar> TabBars;
  AnchorVector<ANCHOR_PtrOrIndex> CurrentTabBarStack;
  AnchorVector<ANCHOR_ShrinkWidthItem> ShrinkWidthBuffer;

  // Widget state
  wabi::GfVec2f LastValidMousePos;
  ANCHOR_InputTextState InputTextState;
  AnchorFont InputTextPasswordFont;
  ANCHOR_ID TempInputId;  // Temporary text input when CTRL+clicking on a slider, etc.
  ANCHOR_ColorEditFlags ColorEditOptions;  // Store user options for color edit widgets
  float ColorEditLastHue;  // Backup of last Hue associated to LastColor[3], so we can restore Hue
                           // in lossy RGB<>HSV round trips
  float ColorEditLastSat;  // Backup of last Saturation associated to LastColor[3], so we can
                           // restore Saturation in lossy RGB<>HSV round trips
  float ColorEditLastColor[3];
  wabi::GfVec4f
      ColorPickerRef;        // Initial/reference color at the time of opening the color picker.
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
  ANCHORTextBuffer SettingsIniData;  // In memory .ini settings
  AnchorVector<ANCHOR_SettingsHandler> SettingsHandlers;  // List of .ini settings handlers
  ImChunkStream<ANCHOR_WindowSettings> SettingsWindows;   // ANCHOR_Window .ini settings entries
  ImChunkStream<ANCHOR_TableSettings> SettingsTables;     // ANCHOR_Table .ini settings entries
  AnchorVector<ANCHOR_ContextHook> Hooks;  // Hooks for extensions (e.g. test engine)
  ANCHOR_ID HookIdNext;                    // Next available HookId

  // Capture/Logging
  bool LogEnabled;             // Currently capturing
  ANCHORLogType LogType;       // Capture target
  ImFileHandle LogFile;        // If != NULL log to stdout/ file
  ANCHORTextBuffer LogBuffer;  // Accumulation buffer when log to clipboard. This is pointer so our
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
  ANCHOR_MetricsConfig DebugMetricsConfig;

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

  ANCHOR_Context(AnchorFontAtlas *shared_font_atlas)
  {
    Initialized             = false;
    FontAtlasOwnedByContext = shared_font_atlas ? false : true;

    HydraDriver.name   = wabi::HgiTokens->renderDriver;
    HydraDriver.driver = wabi::VtValue();

    Font     = NULL;
    FontSize = FontBaseSize = 0.0f;
    IO.Fonts                = shared_font_atlas ? shared_font_atlas : IM_NEW(AnchorFontAtlas)();
    Time                    = 0.0f;
    FrameCount              = 0;
    FrameCountEnded = FrameCountRendered = -1;
    WithinFrameScope = WithinFrameScopeWithImplicitWindow = WithinEndChild = false;
    GcCompactAll                                                           = false;
    TestEngineHookItems                                                    = false;
    TestEngineHookIdInfo                                                   = 0;
    TestEngine                                                             = NULL;

    WindowsActiveCount             = 0;
    CurrentWindow                  = NULL;
    HoveredWindow                  = NULL;
    HoveredWindowUnderMovingWindow = NULL;
    MovingWindow                   = NULL;
    WheelingWindow                 = NULL;
    WheelingWindowTimer            = 0.0f;

    CurrentItemFlags = ANCHOR_ItemFlags_None;
    HoveredId = HoveredIdPreviousFrame = 0;
    HoveredIdAllowOverlap              = false;
    HoveredIdUsingMouseWheel = HoveredIdPreviousFrameUsingMouseWheel = false;
    HoveredIdDisabled                                                = false;
    HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
    ActiveId                                 = 0;
    ActiveIdIsAlive                          = 0;
    ActiveIdTimer                            = 0.0f;
    ActiveIdIsJustActivated                  = false;
    ActiveIdAllowOverlap                     = false;
    ActiveIdNoClearOnFocusLoss               = false;
    ActiveIdHasBeenPressedBefore             = false;
    ActiveIdHasBeenEditedBefore              = false;
    ActiveIdHasBeenEditedThisFrame           = false;
    ActiveIdUsingMouseWheel                  = false;
    ActiveIdUsingNavDirMask                  = 0x00;
    ActiveIdUsingNavInputMask                = 0x00;
    ActiveIdUsingKeyInputMask                = 0x00;
    ActiveIdClickOffset                      = wabi::GfVec2f(-1, -1);
    ActiveIdWindow                           = NULL;
    ActiveIdSource                           = ANCHORInputSource_None;
    ActiveIdMouseButton                      = -1;
    ActiveIdPreviousFrame                    = 0;
    ActiveIdPreviousFrameIsAlive             = false;
    ActiveIdPreviousFrameHasBeenEditedBefore = false;
    ActiveIdPreviousFrameWindow              = NULL;
    LastActiveId                             = 0;
    LastActiveIdTimer                        = 0.0f;

    NavWindow = NULL;
    NavId = NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId =
        NavInputId                                              = 0;
    NavJustTabbedId = NavJustMovedToId = NavJustMovedToFocusScopeId = NavNextActivateId = 0;
    NavJustMovedToKeyMods  = ANCHOR_KeyModFlags_None;
    NavInputSource         = ANCHORInputSource_None;
    NavScoringRect         = ImRect();
    NavScoringCount        = 0;
    NavLayer               = ANCHORNavLayer_Main;
    NavIdTabCounter        = INT_MAX;
    NavIdIsAlive           = false;
    NavMousePosDirty       = false;
    NavDisableHighlight    = true;
    NavDisableMouseHover   = false;
    NavAnyRequest          = false;
    NavInitRequest         = false;
    NavInitRequestFromMove = false;
    NavInitResultId        = 0;
    NavMoveRequest         = false;
    NavMoveRequestFlags    = ANCHOR_NavMoveFlags_None;
    NavMoveRequestForward  = ANCHORNavForward_None;
    NavMoveRequestKeyMods  = ANCHOR_KeyModFlags_None;
    NavMoveDir = NavMoveDirLast = NavMoveClipDir = ANCHOR_Dir_None;
    NavWrapRequestWindow                         = NULL;
    NavWrapRequestFlags                          = ANCHOR_NavMoveFlags_None;

    NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
    NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
    NavWindowingToggleLayer                        = false;

    TabFocusRequestCurrWindow = TabFocusRequestNextWindow = NULL;
    TabFocusRequestCurrCounterRegular = TabFocusRequestCurrCounterTabStop = INT_MAX;
    TabFocusRequestNextCounterRegular = TabFocusRequestNextCounterTabStop = INT_MAX;
    TabFocusPressed                                                       = false;

    DimBgRatio  = 0.0f;
    MouseCursor = ANCHOR_MouseCursor_Arrow;

    DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
    DragDropSourceFlags                                          = ANCHORDragDropFlags_None;
    DragDropSourceFrameCount                                     = -1;
    DragDropMouseButton                                          = -1;
    DragDropTargetId                                             = 0;
    DragDropAcceptFlags                                          = ANCHORDragDropFlags_None;
    DragDropAcceptIdCurrRectSurface                              = 0.0f;
    DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
    DragDropAcceptFrameCount                    = -1;
    DragDropHoldJustPressedId                   = 0;
    memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

    CurrentTable         = NULL;
    CurrentTableStackIdx = -1;
    CurrentTabBar        = NULL;

    LastValidMousePos = wabi::GfVec2f(0.0f, 0.0f);
    TempInputId       = 0;
    ColorEditOptions  = ANCHOR_ColorEditFlags__OptionsDefault;
    ColorEditLastHue = ColorEditLastSat = 0.0f;
    ColorEditLastColor[0] = ColorEditLastColor[1] = ColorEditLastColor[2] = FLT_MAX;
    SliderCurrentAccum                                                    = 0.0f;
    SliderCurrentAccumDirty                                               = false;
    DragCurrentAccumDirty                                                 = false;
    DragCurrentAccum                                                      = 0.0f;
    DragSpeedDefaultRatio                                                 = 1.0f / 100.0f;
    ScrollbarClickDeltaToGrabCenter                                       = 0.0f;
    TooltipOverrideCount                                                  = 0;
    TooltipSlowDelay                                                      = 0.50f;

    PlatformImePos = PlatformImeLastPos = wabi::GfVec2f(FLT_MAX, FLT_MAX);
    PlatformLocaleDecimalPoint          = '.';

    SettingsLoaded     = false;
    SettingsDirtyTimer = 0.0f;
    HookIdNext         = 0;

    LogEnabled    = false;
    LogType       = ANCHORLogType_None;
    LogNextPrefix = LogNextSuffix = NULL;
    LogFile                       = NULL;
    LogLinePosY                   = FLT_MAX;
    LogLineFirstItem              = false;
    LogDepthRef                   = 0;
    LogDepthToExpand = LogDepthToExpandDefault = 2;

    DebugItemPickerActive  = false;
    DebugItemPickerBreakId = 0;

    memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
    FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
    FramerateSecPerFrameAccum                           = 0.0f;
    WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
    memset(TempBuffer, 0, sizeof(TempBuffer));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] ANCHOR_WindowTempData, ANCHOR_Window
//-----------------------------------------------------------------------------

// Transient per-window data, reset at the beginning of the frame. This used to be called
// ANCHORDrawContext, hence the DC variable name in ANCHOR_Window. (That's theory, in practice the
// delimitation between ANCHOR_Window and ANCHOR_WindowTempData is quite tenuous and could be
// reconsidered..) (This doesn't need a constructor because we zero-clear it as part of
// ANCHOR_Window and all frame-temporary data are setup on Begin)
struct ANCHOR_API ANCHOR_WindowTempData {
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
  ImVec1 Indent;         // Indentation / start position from left of window (increased by
                         // TreePush/TreePop, etc.)
  ImVec1 ColumnsOffset;  // Offset to the current column (if ColumnsCurrent > 0). FIXME: This and
                         // the above should be a stack to allow use cases like Tree->Column->Tree.
                         // Need revamp columns API.
  ImVec1 GroupOffset;

  // Last item status
  ANCHOR_ID LastItemId;  // ID for last item
  ANCHOR_ItemStatusFlags
      LastItemStatusFlags;     // Status flags for last item (see ANCHOR_ItemStatusFlags_)
  ImRect LastItemRect;         // Interaction rect for last item
  ImRect LastItemDisplayRect;  // End-user display rect for last item (only valid if
                               // LastItemStatusFlags & ANCHOR_ItemStatusFlags_HasDisplayRect)

  // Keyboard/Gamepad navigation
  ANCHORNavLayer NavLayerCurrent;  // Current layer, 0..31 (we currently only use 0..1)
  short NavLayersActiveMask;      // Which layers have been written to (result from previous frame)
  short NavLayersActiveMaskNext;  // Which layers have been written to (accumulator for current
                                  // frame)
  ANCHOR_ID NavFocusScopeIdCurrent;  // Current focus scope ID while appending
  bool NavHideHighlightOneFrame;
  bool NavHasScroll;  // Set when scrolling can be used (ScrollMax > 0.0f)

  // Miscellaneous
  bool MenuBarAppending;        // FIXME: Remove this
  wabi::GfVec2f MenuBarOffset;  // MenuBarOffset[0] is sort of equivalent of a per-layer
                                // CursorPos[0], saved/restored as we switch to the menu bar. The
                                // only situation when MenuBarOffset[1] is > 0 if when
                                // (SafeAreaPadding[1] > FramePadding[1]), often used on TVs.
  ANCHOR_MenuColumns MenuColumns;       // Simplified columns storage for menu items measurement
  int TreeDepth;                        // Current tree depth.
  AnchorU32 TreeJumpToParentOnPopMask;  // Store a copy of !g.NavIdIsAlive for TreeDepth 0..31..
                                        // Could be turned into a ImU64 if necessary.
  AnchorVector<ANCHOR_Window *> ChildWindows;
  ANCHORStorage *StateStorage;  // Current persistent per-window storage (store e.g. tree node
                                // open/close state)
  ANCHOR_OldColumns *CurrentColumns;  // Current columns set
  int CurrentTableIdx;                // Current table index (into g.Tables)
  ANCHOR_LayoutType LayoutType;
  ANCHOR_LayoutType ParentLayoutType;  // Layout type of parent window at the time of Begin()
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
      TextWrapPosStack;  // Store text wrap pos to restore (attention: .back() is not
                         // == TextWrapPos)
  ANCHOR_StackSizes StackSizesOnBegin;  // Store size of various stacks for asserting
};

// Storage for one window
struct ANCHOR_API ANCHOR_Window {
  char *Name;                 // Window name, owned by the window.
  ANCHOR_ID ID;               // == ImHashStr(Name)
  ANCHOR_WindowFlags Flags;   // See enum ANCHOR_WindowFlags_
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
  ImS8 AutoFitFramesX, AutoFitFramesY;
  ImS8 AutoFitChildAxises;
  bool AutoFitOnlyGrows;
  ANCHOR_Dir AutoPosLastDirection;
  ImS8 HiddenFramesCanSkipItems;     // Hide the window for N frames
  ImS8 HiddenFramesCannotSkipItems;  // Hide the window for N frames while allowing items to be
                                     // submitted so we can measure their size
  ImS8 HiddenFramesForRenderOnly;    // Hide the window until frame N at Render() time only
  ImS8 DisableInputsFrames;          // Disable window interactions for N frames
  ANCHOR_Cond
      SetWindowPosAllowFlags : 8;  // store acceptable condition flags for SetNextWindowPos() use.
  ANCHOR_Cond SetWindowSizeAllowFlags : 8;       // store acceptable condition flags for
                                                 // SetNextWindowSize() use.
  ANCHOR_Cond SetWindowCollapsedAllowFlags : 8;  // store acceptable condition flags for
                                                 // SetNextWindowCollapsed() use.
  wabi::GfVec2f SetWindowPosVal;    // store window position when using a non-zero Pivot (position
                                    // set needs to be processed when we know the window size)
  wabi::GfVec2f SetWindowPosPivot;  // store window pivot for positioning. wabi::GfVec2f(0, 0) when
                                    // positioning from top-left corner; wabi::GfVec2f(0.5f, 0.5f)
                                    // for centering; wabi::GfVec2f(1, 1) for bottom right.

  AnchorVector<ANCHOR_ID>
      IDStack;  // ID stack. ID are hashes seeded with the value at the top of the
                // stack. (In theory this should be in the TempData structure)
  ANCHOR_WindowTempData
      DC;  // Temporary per-window data, reset at the beginning of the frame. This used to be
           // called ANCHORDrawContext, hence the "DC" variable name.

  // The best way to understand what those rectangles are is to use the 'Metrics->Tools->Show
  // Windows Rectangles' viewer. The main 'OuterRect', omitted as a field, is window->Rect().
  ImRect OuterRectClipped;  // == Window->Rect() just after setup in Begin(). == window->Rect() for
                            // root window.
  ImRect InnerRect;         // Inner rectangle (omit title bar, menu bar, scroll bar)
  ImRect InnerClipRect;  // == InnerRect shrunk by WindowPadding*0.5f on each side, clipped within
                         // viewport or parent clip rect.
  ImRect WorkRect;       // Initially covers the whole scrolling region. Reduced by containers e.g
                    // columns/tables when active. Shrunk by WindowPadding*1.0f on each side. This
                    // is meant to replace ContentRegionRect over time (from 1.71+ onward).
  ImRect ParentWorkRect;  // Backup of WorkRect before entering a container such as columns/tables.
                          // Used by e.g. SpanAllColumns functions to easily access. Stacked
                          // containers are responsible for maintaining this. // FIXME-WORKRECT:
                          // Could be a stack?
  ImRect ClipRect;        // Current clipping/scissoring rectangle, evolve as we are using
                          // PushClipRect(), etc. == DrawList->clip_rect_stack.back().
  ImRect ContentRegionRect;  // FIXME: This is currently confusing/misleading. It is essentially
                             // WorkRect but not handling of scrolling. We currently rely on it as
                             // right/bottom aligned sizing operation need some size to rely on.
  wabi::GfVec2h HitTestHoleSize;  // Define an optional rectangular hole where mouse will
                                  // pass-through the window.
  wabi::GfVec2h HitTestHoleOffset;

  int LastFrameActive;   // Last frame number the window was Active.
  float LastTimeActive;  // Last timestamp the window was Active (using float as we don't need high
                         // precision there)
  float ItemWidthDefault;
  ANCHORStorage StateStorage;
  AnchorVector<ANCHOR_OldColumns> ColumnsStorage;
  float FontWindowScale;  // User scale multiplier per-window, via SetWindowFontScale()
  int SettingsOffset;  // Offset into SettingsWindows[] (offsets are always valid as we only grow
                       // the array from the back)

  ImDrawList *DrawList;  // == &DrawListInst (for backward compatibility reason with code using
                         // ANCHOR_internal.h we keep this a pointer)
  ImDrawList DrawListInst;
  ANCHOR_Window *ParentWindow;  // If we are a child _or_ popup window, this is pointing to our
                                // parent. Otherwise NULL.
  ANCHOR_Window *RootWindow;    // Point to ourself or first ancestor that is not a child window ==
                                // Top-level window.
  ANCHOR_Window
      *RootWindowForTitleBarHighlight;  // Point to ourself or first ancestor which will display
                                        // TitleBgActive color when this window is active.
  ANCHOR_Window *RootWindowForNav;  // Point to ourself or first ancestor which doesn't have the
                                    // NavFlattened flag.

  ANCHOR_Window
      *NavLastChildNavWindow;  // When going to the menu bar, we remember the child window we came
                               // from. (This could probably be made implicit if we kept g.Windows
                               // sorted by last focused including child window.)
  ANCHOR_ID NavLastIds[ANCHORNavLayer_COUNT];  // Last known NavId for this window, per layer (0/1)
  ImRect NavRectRel[ANCHORNavLayer_COUNT];     // Reference rectangle, in window relative space

  int MemoryDrawListIdxCapacity;  // Backup of last idx/vtx count, so when waking up the window we
                                  // can preallocate and avoid iterative alloc/copy
  int MemoryDrawListVtxCapacity;
  bool MemoryCompacted;  // Set when window extraneous data have been garbage collected

 public:
  ANCHOR_Window(ANCHOR_Context *context, const char *name);
  ~ANCHOR_Window();

  ANCHOR_ID GetID(const char *str, const char *str_end = NULL);
  ANCHOR_ID GetID(const void *ptr);
  ANCHOR_ID GetID(int n);
  ANCHOR_ID GetIDNoKeepAlive(const char *str, const char *str_end = NULL);
  ANCHOR_ID GetIDNoKeepAlive(const void *ptr);
  ANCHOR_ID GetIDNoKeepAlive(int n);
  ANCHOR_ID GetIDFromRectangle(const ImRect &r_abs);

  // We don't use g.FontSize because the window may be != g.CurrentWidow.
  ImRect Rect() const
  {
    return ImRect(Pos[0], Pos[1], Pos[0] + Size[0], Pos[1] + Size[1]);
  }
  float CalcFontSize() const
  {
    ANCHOR_Context &g = *G_CTX;
    float scale       = g.FontBaseSize * FontWindowScale;
    if (ParentWindow)
      scale *= ParentWindow->FontWindowScale;
    return scale;
  }
  float TitleBarHeight() const
  {
    ANCHOR_Context &g = *G_CTX;
    return (Flags & ANCHOR_WindowFlags_NoTitleBar) ?
               0.0f :
               CalcFontSize() + g.Style.FramePadding[1] * 2.0f;
  }
  ImRect TitleBarRect() const
  {
    return ImRect(Pos, wabi::GfVec2f(Pos[0] + SizeFull[0], Pos[1] + TitleBarHeight()));
  }
  float MenuBarHeight() const
  {
    ANCHOR_Context &g = *G_CTX;
    return (Flags & ANCHOR_WindowFlags_MenuBar) ?
               DC.MenuBarOffset[1] + CalcFontSize() + g.Style.FramePadding[1] * 2.0f :
               0.0f;
  }
  ImRect MenuBarRect() const
  {
    float y1 = Pos[1] + TitleBarHeight();
    return ImRect(Pos[0], y1, Pos[0] + SizeFull[0], y1 + MenuBarHeight());
  }
};

// Backup and restore just enough data to be able to use IsItemHovered() on item A after another B
// in the same window has overwritten the data.
struct ANCHOR_LastItemDataBackup {
  ANCHOR_ID LastItemId;
  ANCHOR_ItemStatusFlags LastItemStatusFlags;
  ImRect LastItemRect;
  ImRect LastItemDisplayRect;

  ANCHOR_LastItemDataBackup()
  {
    Backup();
  }
  void Backup()
  {
    ANCHOR_Window *window = G_CTX->CurrentWindow;
    LastItemId            = window->DC.LastItemId;
    LastItemStatusFlags   = window->DC.LastItemStatusFlags;
    LastItemRect          = window->DC.LastItemRect;
    LastItemDisplayRect   = window->DC.LastItemDisplayRect;
  }
  void Restore() const
  {
    ANCHOR_Window *window          = G_CTX->CurrentWindow;
    window->DC.LastItemId          = LastItemId;
    window->DC.LastItemStatusFlags = LastItemStatusFlags;
    window->DC.LastItemRect        = LastItemRect;
    window->DC.LastItemDisplayRect = LastItemDisplayRect;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Tab bar, Tab item support
//-----------------------------------------------------------------------------

// Extend ANCHOR_TabBarFlags_
enum ANCHOR_TabBarFlagsPrivate_ {
  ANCHOR_TabBarFlags_DockNode =
      1 << 20,  // Part of a dock node [we don't use this in the master branch but it facilitate
                // branch syncing to keep this around]
  ANCHOR_TabBarFlags_IsFocused = 1 << 21,
  ANCHOR_TabBarFlags_SaveSettings =
      1 << 22  // FIXME: Settings are handled by the docking system, this only request the tab bar
               // to mark settings dirty when reordering tabs
};

// Extend ANCHOR_TabItemFlags_
enum ANCHOR_TabItemFlagsPrivate_ {
  ANCHOR_TabItemFlags_SectionMask_ = ANCHOR_TabItemFlags_Leading | ANCHOR_TabItemFlags_Trailing,
  ANCHOR_TabItemFlags_NoCloseButton =
      1 << 20,  // Track whether p_open was set or not (we'll need this info on the next frame to
                // recompute ContentWidth during layout)
  ANCHOR_TabItemFlags_Button =
      1 << 21  // Used by TabItemButton, change the tab item behavior to mimic a button
};

// Storage for one active tab item (sizeof() 40 bytes)
struct ANCHOR_TabItem {
  ANCHOR_ID ID;
  ANCHOR_TabItemFlags Flags;
  int LastFrameVisible;
  int LastFrameSelected;  // This allows us to infer an ordered list of the last activated tabs
                          // with little maintenance
  float Offset;           // Position relative to beginning of tab
  float Width;            // Width currently displayed
  float ContentWidth;     // Width of label, stored during BeginTabItem() call
  ImS32 NameOffset;  // When Window==NULL, offset to name within parent ANCHOR_TabBar::TabsNames
  ImS16 BeginOrder;  // BeginTabItem() order, used to re-order tabs after toggling
                     // ANCHOR_TabBarFlags_Reorderable
  ImS16 IndexDuringLayout;  // Index only used during TabBarLayout()
  bool WantClose;           // Marked as closed by SetTabItemClosed()

  ANCHOR_TabItem()
  {
    memset(this, 0, sizeof(*this));
    LastFrameVisible = LastFrameSelected = -1;
    NameOffset                           = -1;
    BeginOrder = IndexDuringLayout = -1;
  }
};

// Storage for a tab bar (sizeof() 152 bytes)
struct ANCHOR_TabBar {
  AnchorVector<ANCHOR_TabItem> Tabs;
  ANCHOR_TabBarFlags Flags;
  ANCHOR_ID ID;             // Zero for tab-bars used by docking
  ANCHOR_ID SelectedTabId;  // Selected tab/window
  ANCHOR_ID
  NextSelectedTabId;       // Next selected tab/window. Will also trigger a scrolling animation
  ANCHOR_ID VisibleTabId;  // Can occasionally be != SelectedTabId (e.g. when previewing contents
                           // for CTRL+TAB preview)
  int CurrFrameVisible;
  int PrevFrameVisible;
  ImRect BarRect;
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
  ImS16 ReorderRequestOffset;
  ImS8 BeginCount;
  bool WantLayout;
  bool VisibleTabWasSubmitted;
  bool TabsAddedNew;  // Set to true when a new tab item or button has been added to the tab bar
                      // during last frame
  ImS16 TabsActiveCount;  // Number of tabs submitted this frame.
  ImS16 LastTabItemIdx;   // Index of last BeginTabItem() tab for use by EndTabItem()
  float ItemSpacingY;
  wabi::GfVec2f FramePadding;  // style.FramePadding locked at the time of BeginTabBar()
  wabi::GfVec2f BackupCursorPos;
  ANCHORTextBuffer
      TabsNames;  // For non-docking tab bar we re-append names in a contiguous buffer.

  ANCHOR_TabBar();
  int GetTabOrder(const ANCHOR_TabItem *tab) const
  {
    return Tabs.index_from_ptr(tab);
  }
  const char *GetTabName(const ANCHOR_TabItem *tab) const
  {
    ANCHOR_ASSERT(tab->NameOffset != -1 && tab->NameOffset < TabsNames.Buf.Size);
    return TabsNames.Buf.Data + tab->NameOffset;
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Table support
//-----------------------------------------------------------------------------

#  define ANCHOR_COL32_DISABLE \
    ANCHOR_COL32(0, 0, 0, 1)  // Special sentinel code which cannot be used as a regular color.
#  define ANCHOR_TABLE_MAX_COLUMNS \
    64  // sizeof(ImU64) * 8. This is solely because we frequently encode columns set in a ImU64.
#  define ANCHOR_TABLE_MAX_DRAW_CHANNELS (4 + 64 * 2)  // See TableSetupDrawChannels()

// Our current column maximum is 64 but we may raise that in the future.
typedef ImS8 ANCHOR_TableColumnIdx;
typedef ImU8 ANCHOR_TableDrawChannelIdx;

// [Internal] sizeof() ~ 104
// We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
// We use the terminology "Clipped" to refer to a column that is out of sight because of
// scrolling/clipping. This is in contrast with some user-facing api such as IsItemVisible() /
// IsRectVisible() which use "Visible" to mean "not clipped".
struct ANCHOR_TableColumn {
  ANCHOR_TableColumnFlags Flags;  // Flags after some patching (not directly same as provided by
                                  // user). See ANCHOR_TableColumnFlags_
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
  ImRect ClipRect;                 // Clipping rectangle for the column
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
  ImS16 NameOffset;                    // Offset into parent ColumnsNames[]
  ANCHOR_TableColumnIdx DisplayOrder;  // Index within Table's IndexToDisplayOrder[] (column may be
                                       // reordered by users)
  ANCHOR_TableColumnIdx
      IndexWithinEnabledSet;  // Index within enabled/visible set (<= IndexToDisplayOrder)
  ANCHOR_TableColumnIdx PrevEnabledColumn;  // Index of prev enabled/visible column within
                                            // Columns[], -1 if first enabled/visible column
  ANCHOR_TableColumnIdx NextEnabledColumn;  // Index of next enabled/visible column within
                                            // Columns[], -1 if last enabled/visible column
  ANCHOR_TableColumnIdx SortOrder;  // Index of this column within sort specs, -1 if not sorting on
                                    // this column, 0 for single-sort, may be >0 on multi-sort
  ANCHOR_TableDrawChannelIdx DrawChannelCurrent;  // Index within DrawSplitter.Channels[]
  ANCHOR_TableDrawChannelIdx DrawChannelFrozen;
  ANCHOR_TableDrawChannelIdx DrawChannelUnfrozen;
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
  ImS8 NavLayerCurrent;  // ANCHORNavLayer in 1 byte
  ImU8 AutoFitQueue;     // Queue of 8 values for the next 8 frames to request auto-fit
  ImU8
      CannotSkipItemsQueue;  // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
  ImU8 SortDirection : 2;    // ANCHOR_SortDirection_Ascending or ANCHOR_SortDirection_Descending
  ImU8 SortDirectionsAvailCount : 2;  // Number of available sort directions (0 to 3)
  ImU8 SortDirectionsAvailMask  : 4;  // Mask of available sort directions (1-bit each)
  ImU8 SortDirectionsAvailList;       // Ordered of available sort directions (2-bits each)

  ANCHOR_TableColumn()
  {
    memset(this, 0, sizeof(*this));
    StretchWeight = WidthRequest = -1.0f;
    NameOffset                   = -1;
    DisplayOrder = IndexWithinEnabledSet = -1;
    PrevEnabledColumn = NextEnabledColumn = -1;
    SortOrder                             = -1;
    SortDirection                         = ANCHOR_SortDirection_None;
    DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (ImU8)-1;
  }
};

// Transient cell data stored per row.
// sizeof() ~ 6
struct ANCHOR_TableCellData {
  AnchorU32 BgColor;             // Actual color
  ANCHOR_TableColumnIdx Column;  // Column number
};

// FIXME-TABLE: more transient data could be stored in a per-stacked table structure: DrawSplitter,
// SortSpecs, incoming RowData
struct ANCHOR_Table {
  ANCHOR_ID ID;
  ANCHOR_TableFlags Flags;
  void *RawData;  // Single allocation to hold Columns[], DisplayOrderToIndex[] and RowCellData[]
  ANCHOR_TableTempData
      *TempData;  // Transient data while table is active. Point within g.CurrentTableStack[]
  ImSpan<ANCHOR_TableColumn> Columns;  // Point within RawData[]
  ImSpan<ANCHOR_TableColumnIdx>
      DisplayOrderToIndex;  // Point within RawData[]. Store display order of columns (when not
                            // reordered, the values are 0...Count-1)
  ImSpan<ANCHOR_TableCellData>
      RowCellData;  // Point within RawData[]. Store cells background requests for current row.
  ImU64 EnabledMaskByDisplayOrder;  // Column DisplayOrder -> IsEnabled map
  ImU64 EnabledMaskByIndex;  // Column Index -> IsEnabled map (== not hidden by user/api) in a
                             // format adequate for iterating column without touching cold data
  ImU64 VisibleMaskByIndex;  // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by
                             // user/api && not hidden by scrolling/cliprect)
  ImU64 RequestOutputMaskByIndex;  // Column Index -> IsVisible || AutoFit (== expect user to
                                   // submit items)
  ANCHOR_TableFlags SettingsLoadedFlags;  // Which data were loaded from the .ini file (e.g. when
                                          // order is not altered we won't save order)
  int SettingsOffset;                     // Offset in g.SettingsTables
  int LastFrameActive;
  int ColumnsCount;  // Number of columns declared in BeginTable()
  int CurrentRow;
  int CurrentColumn;
  ImS16 InstanceCurrent;  // Count of BeginTable() calls with same ID in the same frame (generally
                          // 0). This is a little bit similar to BeginCount for a window, but
                          // multiple table with same ID look are multiple tables, they are just
                          // synched.
  ImS16 InstanceInteracted;  // Mark which instance (generally 0) of the same ID is being
                             // interacted with
  float RowPosY1;
  float RowPosY2;
  float RowMinHeight;  // Height submitted to TableNextRow()
  float RowTextBaseline;
  float RowIndentOffsetX;
  ANCHOR_TableRowFlags RowFlags     : 16;  // Current row flags, see ANCHOR_TableRowFlags_
  ANCHOR_TableRowFlags LastRowFlags : 16;
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
  float RefScale;    // Reference scale to be able to rescale columns on font/dpi changes.
  ImRect OuterRect;  // Note: for non-scrolling table, OuterRect.Max[1] is often FLT_MAX until
                     // EndTable(), unless a height has been specified in BeginTable().
  ImRect InnerRect;  // InnerRect but without decoration. As with OuterRect, for non-scrolling
                     // tables, InnerRect.Max[1] is
  ImRect WorkRect;
  ImRect InnerClipRect;
  ImRect BgClipRect;             // We use this to cpu-clip cell background color fill
  ImRect Bg0ClipRectForDrawCmd;  // Actual ImDrawCmd clip rect for BG0/1 channel. This tends to be
                                 // == OuterWindow->ClipRect at BeginTable() because output in
                                 // BG0/BG1 is cpu-clipped
  ImRect Bg2ClipRectForDrawCmd;  // Actual ImDrawCmd clip rect for BG2 channel. This tends to be a
                                 // correct, tight-fit, because output to BG2 are done by widgets
                                 // relying on regular ClipRect.
  ImRect HostClipRect;  // This is used to check if we can eventually merge our columns draw calls
                        // into the current draw call of the current window.
  ImRect HostBackupInnerClipRect;  // Backup of InnerWindow->ClipRect during
                                   // PushTableBackground()/PopTableBackground()
  ANCHOR_Window *OuterWindow;      // Parent window for the table
  ANCHOR_Window *InnerWindow;  // Window holding the table data (== OuterWindow or a child window)
  ANCHORTextBuffer ColumnsNames;  // Contiguous buffer holding columns names
  ImDrawListSplitter
      *DrawSplitter;  // Shortcut to TempData->DrawSplitter while in table. Isolate draw commands
                      // per columns to avoid switching clip rect constantly
  ANCHOR_TableSortSpecs
      SortSpecs;  // Public facing sorts specs, this is what we return in TableGetSortSpecs()
  ANCHOR_TableColumnIdx SortSpecsCount;
  ANCHOR_TableColumnIdx ColumnsEnabledCount;       // Number of enabled columns (<= ColumnsCount)
  ANCHOR_TableColumnIdx ColumnsEnabledFixedCount;  // Number of enabled columns (<= ColumnsCount)
  ANCHOR_TableColumnIdx DeclColumnsCount;          // Count calls to TableSetupColumn()
  ANCHOR_TableColumnIdx
      HoveredColumnBody;  // Index of column whose visible region is being hovered. Important: ==
                          // ColumnsCount when hovering empty region after the right-most column!
  ANCHOR_TableColumnIdx
      HoveredColumnBorder;  // Index of column whose right-border is being hovered (for resizing).
  ANCHOR_TableColumnIdx AutoFitSingleColumn;  // Index of single column requesting auto-fit.
  ANCHOR_TableColumnIdx
      ResizedColumn;  // Index of column being resized. Reset when InstanceCurrent==0.
  ANCHOR_TableColumnIdx LastResizedColumn;  // Index of column being resized from previous frame.
  ANCHOR_TableColumnIdx HeldHeaderColumn;   // Index of column header being held.
  ANCHOR_TableColumnIdx ReorderColumn;      // Index of column being reordered. (not cleared)
  ANCHOR_TableColumnIdx ReorderColumnDir;   // -1 or +1
  ANCHOR_TableColumnIdx LeftMostEnabledColumn;     // Index of left-most non-hidden column.
  ANCHOR_TableColumnIdx RightMostEnabledColumn;    // Index of right-most non-hidden column.
  ANCHOR_TableColumnIdx LeftMostStretchedColumn;   // Index of left-most stretched column.
  ANCHOR_TableColumnIdx RightMostStretchedColumn;  // Index of right-most stretched column.
  ANCHOR_TableColumnIdx ContextPopupColumn;  // Column right-clicked on, of -1 if opening context
                                             // menu from a neutral/empty spot
  ANCHOR_TableColumnIdx FreezeRowsRequest;   // Requested frozen rows count
  ANCHOR_TableColumnIdx FreezeRowsCount;  // Actual frozen row count (== FreezeRowsRequest, or == 0
                                          // when no scrolling offset)
  ANCHOR_TableColumnIdx FreezeColumnsRequest;  // Requested frozen columns count
  ANCHOR_TableColumnIdx
      FreezeColumnsCount;  // Actual frozen columns count (== FreezeColumnsRequest, or == 0 when no
                           // scrolling offset)
  ANCHOR_TableColumnIdx RowCellDataCurrent;  // Index of current RowCellData[] entry in current row
  ANCHOR_TableDrawChannelIdx DummyDrawChannel;  // Redirect non-visible columns here.
  ANCHOR_TableDrawChannelIdx
      Bg2DrawChannelCurrent;  // For Selectable() and other widgets drawing across columns after
                              // the freezing line. Index within DrawSplitter.Channels[]
  ANCHOR_TableDrawChannelIdx Bg2DrawChannelUnfrozen;
  bool IsLayoutLocked;  // Set by TableUpdateLayout() which is called when beginning the first row.
  bool IsInsideRow;     // Set when inside TableBeginRow()/TableEndRow().
  bool IsInitializing;
  bool IsSortSpecsDirty;
  bool IsUsingHeaders;      // Set when the first row had the ANCHOR_TableRowFlags_Headers flag.
  bool IsContextPopupOpen;  // Set when default context menu is open (also see: ContextPopupColumn,
                            // InstanceInteracted).
  bool IsSettingsRequestLoad;
  bool IsSettingsDirty;  // Set when table settings have changed and needs to be reported into
                         // ANCHOR_TableSettings data.
  bool IsDefaultDisplayOrder;  // Set when display order is unchanged from default (DisplayOrder
                               // contains 0...Count-1)
  bool IsResetAllRequest;
  bool IsResetDisplayOrderRequest;
  bool IsUnfrozenRows;         // Set when we got past the frozen row.
  bool IsDefaultSizingPolicy;  // Set if user didn't explicitly set a sizing policy in BeginTable()
  bool MemoryCompacted;
  bool HostSkipItems;  // Backup of InnerWindow->SkipItem at the end of BeginTable(), because we
                       // will overwrite InnerWindow->SkipItem on a per-column basis

  ANCHOR_API ANCHOR_Table()
  {
    memset(this, 0, sizeof(*this));
    LastFrameActive = -1;
  }
  ANCHOR_API ~ANCHOR_Table()
  {
    IM_FREE(RawData);
  }
};

// Transient data that are only needed between BeginTable() and EndTable(), those buffers are
// shared (1 per level of stacked table).
// - Accessing those requires chasing an extra pointer so for very frequently used data we leave
// them in the main table structure.
// - We also leave out of this structure data that tend to be particularly useful for
// debugging/metrics.
// FIXME-TABLE: more transient data could be stored here: DrawSplitter, incoming RowData?
struct ANCHOR_TableTempData {
  int TableIndex;        // Index in g.Tables.Buf[] pool
  float LastTimeActive;  // Last timestamp this structure was used

  wabi::GfVec2f UserOuterSize;  // outer_size[0] passed to BeginTable()
  ImDrawListSplitter DrawSplitter;
  ANCHOR_TableColumnSortSpecs SortSpecsSingle;
  AnchorVector<ANCHOR_TableColumnSortSpecs>
      SortSpecsMulti;  // FIXME-OPT: Using a small-vector pattern would be good.

  ImRect HostBackupWorkRect;        // Backup of InnerWindow->WorkRect at the end of BeginTable()
  ImRect HostBackupParentWorkRect;  // Backup of InnerWindow->ParentWorkRect at the end of
                                    // BeginTable()
  wabi::GfVec2f
      HostBackupPrevLineSize;  // Backup of InnerWindow->DC.PrevLineSize at the end of BeginTable()
  wabi::GfVec2f
      HostBackupCurrLineSize;  // Backup of InnerWindow->DC.CurrLineSize at the end of BeginTable()
  wabi::GfVec2f
      HostBackupCursorMaxPos;  // Backup of InnerWindow->DC.CursorMaxPos at the end of BeginTable()
  ImVec1 HostBackupColumnsOffset;  // Backup of OuterWindow->DC.ColumnsOffset at the end of
                                   // BeginTable()
  float HostBackupItemWidth;  // Backup of OuterWindow->DC.ItemWidth at the end of BeginTable()
  int HostBackupItemWidthStackSize;  // Backup of OuterWindow->DC.ItemWidthStack.Size at the end of
                                     // BeginTable()

  ANCHOR_API ANCHOR_TableTempData()
  {
    memset(this, 0, sizeof(*this));
    LastTimeActive = -1.0f;
  }
};

// sizeof() ~ 12
struct ANCHOR_TableColumnSettings {
  float WidthOrWeight;
  ANCHOR_ID UserID;
  ANCHOR_TableColumnIdx Index;
  ANCHOR_TableColumnIdx DisplayOrder;
  ANCHOR_TableColumnIdx SortOrder;
  ImU8 SortDirection : 2;
  ImU8 IsEnabled     : 1;  // "Visible" in ini file
  ImU8 IsStretch     : 1;

  ANCHOR_TableColumnSettings()
  {
    WidthOrWeight = 0.0f;
    UserID        = 0;
    Index         = -1;
    DisplayOrder = SortOrder = -1;
    SortDirection            = ANCHOR_SortDirection_None;
    IsEnabled                = 1;
    IsStretch                = 0;
  }
};

// This is designed to be stored in a single ImChunkStream (1 header followed by N
// ANCHOR_TableColumnSettings, etc.)
struct ANCHOR_TableSettings {
  ANCHOR_ID ID;  // Set to 0 to invalidate/delete the setting
  ANCHOR_TableFlags
      SaveFlags;  // Indicate data we want to save using the
                  // Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
  float RefScale;  // Reference scale to be able to rescale columns on font/dpi changes.
  ANCHOR_TableColumnIdx ColumnsCount;
  ANCHOR_TableColumnIdx
      ColumnsCountMax;  // Maximum number of columns this settings instance can store, we can
                        // recycle a settings instance with lower number of columns but not higher
  bool WantApply;  // Set when loaded from .ini data (to enable merging/loading .ini data into an
                   // already running context)

  ANCHOR_TableSettings()
  {
    memset(this, 0, sizeof(*this));
  }
  ANCHOR_TableColumnSettings *GetColumnSettings()
  {
    return (ANCHOR_TableColumnSettings *)(this + 1);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] ANCHOR internal API
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

namespace ANCHOR {
// Windows
// We should always have a CurrentWindow in the stack (there is an implicit "Debug" window)
// If this ever crash because g.CurrentWindow is NULL it means that either
// - ANCHOR::NewFrame() has never been called, which is illegal.
// - You are calling ANCHOR functions after ANCHOR::EndFrame()/ANCHOR::Render() and before the next
// ANCHOR::NewFrame(), which is also illegal.
inline ANCHOR_Window *GetCurrentWindowRead()
{
  ANCHOR_Context &g = *G_CTX;
  return g.CurrentWindow;
}
inline ANCHOR_Window *GetCurrentWindow()
{
  ANCHOR_Context &g              = *G_CTX;
  g.CurrentWindow->WriteAccessed = true;
  return g.CurrentWindow;
}
ANCHOR_API ANCHOR_Window *FindWindowByID(ANCHOR_ID id);
ANCHOR_API ANCHOR_Window *FindWindowByName(const char *name);
ANCHOR_API void UpdateWindowParentAndRootLinks(ANCHOR_Window *window,
                                               ANCHOR_WindowFlags flags,
                                               ANCHOR_Window *parent_window);
ANCHOR_API wabi::GfVec2f CalcWindowNextAutoFitSize(ANCHOR_Window *window);
ANCHOR_API bool IsWindowChildOf(ANCHOR_Window *window, ANCHOR_Window *potential_parent);
ANCHOR_API bool IsWindowAbove(ANCHOR_Window *potential_above, ANCHOR_Window *potential_below);
ANCHOR_API bool IsWindowNavFocusable(ANCHOR_Window *window);
ANCHOR_API void SetWindowPos(ANCHOR_Window *window,
                             const wabi::GfVec2f &pos,
                             ANCHOR_Cond cond = 0);
ANCHOR_API void SetWindowSize(ANCHOR_Window *window,
                              const wabi::GfVec2f &size,
                              ANCHOR_Cond cond = 0);
ANCHOR_API void SetWindowCollapsed(ANCHOR_Window *window, bool collapsed, ANCHOR_Cond cond = 0);
ANCHOR_API void SetWindowHitTestHole(ANCHOR_Window *window,
                                     const wabi::GfVec2f &pos,
                                     const wabi::GfVec2f &size);

// Windows: Display Order and Focus Order
ANCHOR_API void FocusWindow(ANCHOR_Window *window);
ANCHOR_API void FocusTopMostWindowUnderOne(ANCHOR_Window *under_this_window,
                                           ANCHOR_Window *ignore_window);
ANCHOR_API void BringWindowToFocusFront(ANCHOR_Window *window);
ANCHOR_API void BringWindowToDisplayFront(ANCHOR_Window *window);
ANCHOR_API void BringWindowToDisplayBack(ANCHOR_Window *window);

// Fonts, drawing
ANCHOR_API void SetCurrentFont(AnchorFont *font);
inline AnchorFont *GetDefaultFont()
{
  ANCHOR_Context &g = *G_CTX;
  return g.IO.FontDefault ? g.IO.FontDefault : g.IO.Fonts->Fonts[0];
}
inline ImDrawList *GetForegroundDrawList(ANCHOR_Window *window)
{
  TF_UNUSED(window);
  // This seemingly unnecessary wrapper simplifies compatibility between the 'master' and
  // 'docking' branches.
  return GetForegroundDrawList();
}

// get background draw list for the given viewport. this draw list
// will be the first rendering one. Useful to quickly draw
// shapes/text behind ANCHOR contents.
ANCHOR_API ImDrawList *GetBackgroundDrawList(ANCHORViewport *viewport);
// get foreground draw list for the given viewport. this draw list will be the
// last rendered one. Useful to quickly draw shapes/text over ANCHOR contents.
ANCHOR_API ImDrawList *GetForegroundDrawList(ANCHORViewport *viewport);

// Init
ANCHOR_API void Initialize(ANCHOR_Context *context);
ANCHOR_API void Shutdown(ANCHOR_Context *context);  // Since 1.60 this is a _private_ function. You
                                                    // can call DestroyContext() to destroy the
                                                    // context created by CreateContext().

// NewFrame
ANCHOR_API void UpdateHoveredWindowAndCaptureFlags();
ANCHOR_API void StartMouseMovingWindow(ANCHOR_Window *window);
ANCHOR_API void UpdateMouseMovingWindowNewFrame();
ANCHOR_API void UpdateMouseMovingWindowEndFrame();

// Generic context hooks
ANCHOR_API ANCHOR_ID AddContextHook(ANCHOR_Context *context, const ANCHOR_ContextHook *hook);
ANCHOR_API void RemoveContextHook(ANCHOR_Context *context, ANCHOR_ID hook_to_remove);
ANCHOR_API void CallContextHooks(ANCHOR_Context *context, ANCHOR_ContextHookType type);

// Settings
ANCHOR_API void MarkIniSettingsDirty();
ANCHOR_API void MarkIniSettingsDirty(ANCHOR_Window *window);
ANCHOR_API void ClearIniSettings();
ANCHOR_API ANCHOR_WindowSettings *CreateNewWindowSettings(const char *name);
ANCHOR_API ANCHOR_WindowSettings *FindWindowSettings(ANCHOR_ID id);
ANCHOR_API ANCHOR_WindowSettings *FindOrCreateWindowSettings(const char *name);
ANCHOR_API ANCHOR_SettingsHandler *FindSettingsHandler(const char *type_name);

// Scrolling
ANCHOR_API void SetNextWindowScroll(
    const wabi::GfVec2f &scroll);  // Use -1.0f on one axis to leave as-is
ANCHOR_API void SetScrollX(ANCHOR_Window *window, float scroll_x);
ANCHOR_API void SetScrollY(ANCHOR_Window *window, float scroll_y);
ANCHOR_API void SetScrollFromPosX(ANCHOR_Window *window, float local_x, float center_x_ratio);
ANCHOR_API void SetScrollFromPosY(ANCHOR_Window *window, float local_y, float center_y_ratio);
ANCHOR_API wabi::GfVec2f ScrollToBringRectIntoView(ANCHOR_Window *window, const ImRect &item_rect);

// Basic Accessors
inline ANCHOR_ID GetItemID()
{
  ANCHOR_Context &g = *G_CTX;
  return g.CurrentWindow->DC.LastItemId;
}  // Get ID of last item (~~ often same ANCHOR::GetID(label) beforehand)
inline ANCHOR_ItemStatusFlags GetItemStatusFlags()
{
  ANCHOR_Context &g = *G_CTX;
  return g.CurrentWindow->DC.LastItemStatusFlags;
}
inline ANCHOR_ID GetActiveID()
{
  ANCHOR_Context &g = *G_CTX;
  return g.ActiveId;
}
inline ANCHOR_ID GetFocusID()
{
  ANCHOR_Context &g = *G_CTX;
  return g.NavId;
}
inline ANCHOR_ItemFlags GetItemFlags()
{
  ANCHOR_Context &g = *G_CTX;
  return g.CurrentItemFlags;
}
ANCHOR_API void SetActiveID(ANCHOR_ID id, ANCHOR_Window *window);
ANCHOR_API void SetFocusID(ANCHOR_ID id, ANCHOR_Window *window);
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
ANCHOR_API void ItemSize(const ImRect &bb, float text_baseline_y = -1.0f);
ANCHOR_API bool ItemAdd(const ImRect &bb,
                        ANCHOR_ID id,
                        const ImRect *nav_bb      = NULL,
                        ANCHOR_ItemAddFlags flags = 0);
ANCHOR_API bool ItemHoverable(const ImRect &bb, ANCHOR_ID id);
ANCHOR_API void ItemFocusable(ANCHOR_Window *window, ANCHOR_ID id);
ANCHOR_API bool IsClippedEx(const ImRect &bb, ANCHOR_ID id, bool clip_even_when_logged);
ANCHOR_API void SetLastItemData(ANCHOR_Window *window,
                                ANCHOR_ID item_id,
                                ANCHOR_ItemStatusFlags status_flags,
                                const ImRect &item_rect);
ANCHOR_API wabi::GfVec2f CalcItemSize(wabi::GfVec2f size, float default_w, float default_h);
ANCHOR_API float CalcWrapWidthForPos(const wabi::GfVec2f &pos, float wrap_pos_x);
ANCHOR_API void PushMultiItemsWidths(int components, float width_full);
ANCHOR_API void PushItemFlag(ANCHOR_ItemFlags option, bool enabled);
ANCHOR_API void PopItemFlag();

// Was the last item selection toggled? (after
// Selectable(), TreeNode() etc. We only returns toggle
// _event_ in order to handle clipping correctly)
ANCHOR_API bool IsItemToggledSelection();
ANCHOR_API wabi::GfVec2f GetContentRegionMaxAbs();
ANCHOR_API void ShrinkWidths(ANCHOR_ShrinkWidthItem *items, int count, float width_excess);

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
// If you have old/custom copy-and-pasted widgets that used FocusableItemRegister():
//  (Old) ANCHOR_VERSION_NUM  < 18209: using 'ItemAdd(....)'                              and 'bool
//  focused = FocusableItemRegister(...)' (New) ANCHOR_VERSION_NUM >= 18209: using 'ItemAdd(...,
//  ANCHOR_ItemAddFlags_Focusable)'  and 'bool focused = (GetItemStatusFlags() &
//  ANCHOR_ItemStatusFlags_Focused) != 0'
// Widget code are simplified as there's no need to call FocusableItemUnregister() while managing
// the transition from regular widget to TempInputText()
inline bool FocusableItemRegister(ANCHOR_Window *window, ANCHOR_ID id)
{
  ANCHOR_ASSERT(0);
  TF_UNUSED(window);
  TF_UNUSED(id);
  return false;
}  // -> pass ANCHOR_ItemAddFlags_Focusable flag to ItemAdd()
inline void FocusableItemUnregister(ANCHOR_Window *window)
{
  ANCHOR_ASSERT(0);
  TF_UNUSED(window);
}  // -> unnecessary: TempInputText() uses ANCHORInputTextFlags_MergedItem
#  endif

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
                             ANCHOR_WindowFlags flags);
ANCHOR_API void OpenPopupEx(ANCHOR_ID id, ANCHORPopupFlags popup_flags = ANCHORPopupFlags_None);
ANCHOR_API void ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
ANCHOR_API void ClosePopupsOverWindow(ANCHOR_Window *ref_window,
                                      bool restore_focus_to_window_under_popup);
ANCHOR_API bool IsPopupOpen(ANCHOR_ID id, ANCHORPopupFlags popup_flags);
ANCHOR_API bool BeginPopupEx(ANCHOR_ID id, ANCHOR_WindowFlags extra_flags);
ANCHOR_API void BeginTooltipEx(ANCHOR_WindowFlags extra_flags, ANCHOR_TooltipFlags tooltip_flags);
ANCHOR_API ImRect GetPopupAllowedExtentRect(ANCHOR_Window *window);
ANCHOR_API ANCHOR_Window *GetTopMostPopupModal();
ANCHOR_API wabi::GfVec2f FindBestWindowPosForPopup(ANCHOR_Window *window);
ANCHOR_API wabi::GfVec2f FindBestWindowPosForPopupEx(const wabi::GfVec2f &ref_pos,
                                                     const wabi::GfVec2f &size,
                                                     ANCHOR_Dir *last_dir,
                                                     const ImRect &r_outer,
                                                     const ImRect &r_avoid,
                                                     ANCHORPopupPositionPolicy policy);
ANCHOR_API bool BeginViewportSideBar(const char *name,
                                     ANCHORViewport *viewport,
                                     ANCHOR_Dir dir,
                                     float size,
                                     ANCHOR_WindowFlags window_flags);

// Combos
ANCHOR_API bool BeginComboPopup(ANCHOR_ID popup_id, const ImRect &bb, ANCHORComboFlags flags);

// Gamepad/Keyboard Navigation
ANCHOR_API void NavInitWindow(ANCHOR_Window *window, bool force_reinit);
ANCHOR_API bool NavMoveRequestButNoResultYet();
ANCHOR_API void NavMoveRequestCancel();
ANCHOR_API void NavMoveRequestForward(ANCHOR_Dir move_dir,
                                      ANCHOR_Dir clip_dir,
                                      const ImRect &bb_rel,
                                      ANCHOR_NavMoveFlags move_flags);
ANCHOR_API void NavMoveRequestTryWrapping(ANCHOR_Window *window, ANCHOR_NavMoveFlags move_flags);
ANCHOR_API float GetNavInputAmount(ANCHOR_NavInput n, ANCHOR_InputReadMode mode);
ANCHOR_API wabi::GfVec2f GetNavInputAmount2d(ANCHOR_NavDirSourceFlags dir_sources,
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
                         const ImRect &rect_rel);

// Focus Scope (WIP)
// This is generally used to identify a selection set (multiple of which may be in the same
// window), as selection patterns generally need to react (e.g. clear selection) when landing on an
// item of the set.
ANCHOR_API void PushFocusScope(ANCHOR_ID id);
ANCHOR_API void PopFocusScope();
inline ANCHOR_ID GetFocusedFocusScope()
{
  ANCHOR_Context &g = *G_CTX;
  return g.NavFocusScopeId;
}  // Focus scope which is actually active
inline ANCHOR_ID GetFocusScope()
{
  ANCHOR_Context &g = *G_CTX;
  return g.CurrentWindow->DC.NavFocusScopeIdCurrent;
}  // Focus scope we are outputting into, set by PushFocusScope()

// Inputs
// FIXME: Eventually we should aim to move e.g. IsActiveIdUsingKey() into IsKeyXXX functions.
ANCHOR_API void SetItemUsingMouseWheel();
inline bool IsActiveIdUsingNavDir(ANCHOR_Dir dir)
{
  ANCHOR_Context &g = *G_CTX;
  return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0;
}
inline bool IsActiveIdUsingNavInput(ANCHOR_NavInput input)
{
  ANCHOR_Context &g = *G_CTX;
  return (g.ActiveIdUsingNavInputMask & (1 << input)) != 0;
}
inline bool IsActiveIdUsingKey(ANCHOR_Key key)
{
  ANCHOR_Context &g = *G_CTX;
  ANCHOR_ASSERT(key < 64);
  return (g.ActiveIdUsingKeyInputMask & ((ImU64)1 << key)) != 0;
}
ANCHOR_API bool IsMouseDragPastThreshold(ANCHOR_MouseButton button, float lock_threshold = -1.0f);
inline bool IsKeyPressedMap(ANCHOR_Key key, bool repeat = true)
{
  ANCHOR_Context &g   = *G_CTX;
  const int key_index = g.IO.KeyMap[key];
  return (key_index >= 0) ? IsKeyPressed(key_index, repeat) : false;
}
inline bool IsNavInputDown(ANCHOR_NavInput n)
{
  ANCHOR_Context &g = *G_CTX;
  return g.IO.NavInputs[n] > 0.0f;
}
inline bool IsNavInputTest(ANCHOR_NavInput n, ANCHOR_InputReadMode rm)
{
  return (GetNavInputAmount(n, rm) > 0.0f);
}
ANCHOR_API ANCHOR_KeyModFlags GetMergedKeyModFlags();

// Drag and Drop
ANCHOR_API bool BeginDragDropTargetCustom(const ImRect &bb, ANCHOR_ID id);
ANCHOR_API void ClearDragDrop();
ANCHOR_API bool IsDragDropPayloadBeingAccepted();

// Internal Columns API (this is not exposed because we will encourage transitioning to the Tables
// API)
ANCHOR_API void SetWindowClipRectBeforeSetChannel(ANCHOR_Window *window, const ImRect &clip_rect);
ANCHOR_API void BeginColumns(
    const char *str_id,
    int count,
    ANCHOR_OldColumnFlags flags = 0);  // setup number of columns. use an identifier to distinguish
                                       // multiple column sets. close with EndColumns().
ANCHOR_API void EndColumns();          // close columns
ANCHOR_API void PushColumnClipRect(int column_index);
ANCHOR_API void PushColumnsBackground();
ANCHOR_API void PopColumnsBackground();
ANCHOR_API ANCHOR_ID GetColumnsID(const char *str_id, int count);
ANCHOR_API ANCHOR_OldColumns *FindOrCreateColumns(ANCHOR_Window *window, ANCHOR_ID id);
ANCHOR_API float GetColumnOffsetFromNorm(const ANCHOR_OldColumns *columns, float offset_norm);
ANCHOR_API float GetColumnNormFromOffset(const ANCHOR_OldColumns *columns, float offset);

// Tables: Candidates for public API
ANCHOR_API void TableOpenContextMenu(int column_n = -1);
ANCHOR_API void TableSetColumnWidth(int column_n, float width);
ANCHOR_API void TableSetColumnSortDirection(int column_n,
                                            ANCHOR_SortDirection sort_direction,
                                            bool append_to_sort_specs);
ANCHOR_API int TableGetHoveredColumn();  // May use (TableGetColumnFlags() &
                                         // ANCHOR_TableColumnFlags_IsHovered) instead. Return
                                         // hovered column. return -1 when table is not hovered.
                                         // return columns_count if the unused space at the right
                                         // of visible columns is hovered.
ANCHOR_API float TableGetHeaderRowHeight();
ANCHOR_API void TablePushBackgroundChannel();
ANCHOR_API void TablePopBackgroundChannel();

// Tables: Internals
inline ANCHOR_Table *GetCurrentTable()
{
  ANCHOR_Context &g = *G_CTX;
  return g.CurrentTable;
}
ANCHOR_API ANCHOR_Table *TableFindByID(ANCHOR_ID id);
ANCHOR_API bool BeginTableEx(const char *name,
                             ANCHOR_ID id,
                             int columns_count,
                             ANCHOR_TableFlags flags         = 0,
                             const wabi::GfVec2f &outer_size = wabi::GfVec2f(0, 0),
                             float inner_width               = 0.0f);
ANCHOR_API void TableBeginInitMemory(ANCHOR_Table *table, int columns_count);
ANCHOR_API void TableBeginApplyRequests(ANCHOR_Table *table);
ANCHOR_API void TableSetupDrawChannels(ANCHOR_Table *table);
ANCHOR_API void TableUpdateLayout(ANCHOR_Table *table);
ANCHOR_API void TableUpdateBorders(ANCHOR_Table *table);
ANCHOR_API void TableUpdateColumnsWeightFromWidth(ANCHOR_Table *table);
ANCHOR_API void TableDrawBorders(ANCHOR_Table *table);
ANCHOR_API void TableDrawContextMenu(ANCHOR_Table *table);
ANCHOR_API void TableMergeDrawChannels(ANCHOR_Table *table);
ANCHOR_API void TableSortSpecsSanitize(ANCHOR_Table *table);
ANCHOR_API void TableSortSpecsBuild(ANCHOR_Table *table);
ANCHOR_API ANCHOR_SortDirection TableGetColumnNextSortDirection(ANCHOR_TableColumn *column);
ANCHOR_API void TableFixColumnSortDirection(ANCHOR_Table *table, ANCHOR_TableColumn *column);
ANCHOR_API float TableGetColumnWidthAuto(ANCHOR_Table *table, ANCHOR_TableColumn *column);
ANCHOR_API void TableBeginRow(ANCHOR_Table *table);
ANCHOR_API void TableEndRow(ANCHOR_Table *table);
ANCHOR_API void TableBeginCell(ANCHOR_Table *table, int column_n);
ANCHOR_API void TableEndCell(ANCHOR_Table *table);
ANCHOR_API ImRect TableGetCellBgRect(const ANCHOR_Table *table, int column_n);
ANCHOR_API const char *TableGetColumnName(const ANCHOR_Table *table, int column_n);
ANCHOR_API ANCHOR_ID TableGetColumnResizeID(const ANCHOR_Table *table,
                                            int column_n,
                                            int instance_no = 0);
ANCHOR_API float TableGetMaxColumnWidth(const ANCHOR_Table *table, int column_n);
ANCHOR_API void TableSetColumnWidthAutoSingle(ANCHOR_Table *table, int column_n);
ANCHOR_API void TableSetColumnWidthAutoAll(ANCHOR_Table *table);
ANCHOR_API void TableRemove(ANCHOR_Table *table);
ANCHOR_API void TableGcCompactTransientBuffers(ANCHOR_Table *table);
ANCHOR_API void TableGcCompactTransientBuffers(ANCHOR_TableTempData *table);
ANCHOR_API void TableGcCompactSettings();

// Tables: Settings
ANCHOR_API void TableLoadSettings(ANCHOR_Table *table);
ANCHOR_API void TableSaveSettings(ANCHOR_Table *table);
ANCHOR_API void TableResetSettings(ANCHOR_Table *table);
ANCHOR_API ANCHOR_TableSettings *TableGetBoundSettings(ANCHOR_Table *table);
ANCHOR_API void TableSettingsInstallHandler(ANCHOR_Context *context);
ANCHOR_API ANCHOR_TableSettings *TableSettingsCreate(ANCHOR_ID id, int columns_count);
ANCHOR_API ANCHOR_TableSettings *TableSettingsFindByID(ANCHOR_ID id);

// Tab Bars
ANCHOR_API bool BeginTabBarEx(ANCHOR_TabBar *tab_bar, const ImRect &bb, ANCHOR_TabBarFlags flags);
ANCHOR_API ANCHOR_TabItem *TabBarFindTabByID(ANCHOR_TabBar *tab_bar, ANCHOR_ID tab_id);
ANCHOR_API void TabBarRemoveTab(ANCHOR_TabBar *tab_bar, ANCHOR_ID tab_id);
ANCHOR_API void TabBarCloseTab(ANCHOR_TabBar *tab_bar, ANCHOR_TabItem *tab);
ANCHOR_API void TabBarQueueReorder(ANCHOR_TabBar *tab_bar, const ANCHOR_TabItem *tab, int offset);
ANCHOR_API void TabBarQueueReorderFromMousePos(ANCHOR_TabBar *tab_bar,
                                               const ANCHOR_TabItem *tab,
                                               wabi::GfVec2f mouse_pos);
ANCHOR_API bool TabBarProcessReorder(ANCHOR_TabBar *tab_bar);
ANCHOR_API bool TabItemEx(ANCHOR_TabBar *tab_bar,
                          const char *label,
                          bool *p_open,
                          ANCHOR_TabItemFlags flags);
ANCHOR_API wabi::GfVec2f TabItemCalcSize(const char *label, bool has_close_button);
ANCHOR_API void TabItemBackground(ImDrawList *draw_list,
                                  const ImRect &bb,
                                  ANCHOR_TabItemFlags flags,
                                  AnchorU32 col);
ANCHOR_API void TabItemLabelAndCloseButton(ImDrawList *draw_list,
                                           const ImRect &bb,
                                           ANCHOR_TabItemFlags flags,
                                           wabi::GfVec2f frame_padding,
                                           const char *label,
                                           ANCHOR_ID tab_id,
                                           ANCHOR_ID close_button_id,
                                           bool is_contents_visible,
                                           bool *out_just_closed,
                                           bool *out_text_clipped);

// Render helpers
// AVOID USING OUTSIDE OF ANCHOR.CPP! NOT FOR PUBLIC CONSUMPTION. THOSE FUNCTIONS ARE A MESS. THEIR
// SIGNATURE AND BEHAVIOR WILL CHANGE, THEY NEED TO BE REFACTORED INTO SOMETHING DECENT. NB: All
// position are in absolute pixels coordinates (we are never using window coordinates internally)
ANCHOR_API void RenderText(wabi::GfVec2f pos,
                           const char *text,
                           const char *text_end      = NULL,
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
                                  const ImRect *clip_rect    = NULL);
ANCHOR_API void RenderTextClippedEx(ImDrawList *draw_list,
                                    const wabi::GfVec2f &pos_min,
                                    const wabi::GfVec2f &pos_max,
                                    const char *text,
                                    const char *text_end,
                                    const wabi::GfVec2f *text_size_if_known,
                                    const wabi::GfVec2f &align = wabi::GfVec2f(0, 0),
                                    const ImRect *clip_rect    = NULL);
ANCHOR_API void RenderTextEllipsis(ImDrawList *draw_list,
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
                            bool border    = true,
                            float rounding = 0.0f);
ANCHOR_API void RenderFrameBorder(wabi::GfVec2f p_min, wabi::GfVec2f p_max, float rounding = 0.0f);
ANCHOR_API void RenderColorRectWithAlphaCheckerboard(ImDrawList *draw_list,
                                                     wabi::GfVec2f p_min,
                                                     wabi::GfVec2f p_max,
                                                     AnchorU32 fill_col,
                                                     float grid_step,
                                                     wabi::GfVec2f grid_off,
                                                     float rounding    = 0.0f,
                                                     ImDrawFlags flags = 0);
ANCHOR_API void RenderNavHighlight(
    const ImRect &bb,
    ANCHOR_ID id,
    ANCHOR_NavHighlightFlags flags =
        ANCHOR_NavHighlightFlags_TypeDefault);  // Navigation highlight
ANCHOR_API const char *FindRenderedTextEnd(
    const char *text,
    const char *text_end = NULL);  // Find the optional ## from which we stop displaying text.

// Render helpers (those functions don't access any ANCHOR state!)
ANCHOR_API void RenderArrow(ImDrawList *draw_list,
                            wabi::GfVec2f pos,
                            AnchorU32 col,
                            ANCHOR_Dir dir,
                            float scale = 1.0f);
ANCHOR_API void RenderBullet(ImDrawList *draw_list, wabi::GfVec2f pos, AnchorU32 col);
ANCHOR_API void RenderCheckMark(ImDrawList *draw_list, wabi::GfVec2f pos, AnchorU32 col, float sz);
ANCHOR_API void RenderMouseCursor(ImDrawList *draw_list,
                                  wabi::GfVec2f pos,
                                  float scale,
                                  ANCHOR_MouseCursor mouse_cursor,
                                  AnchorU32 col_fill,
                                  AnchorU32 col_border,
                                  AnchorU32 col_shadow);
ANCHOR_API void RenderArrowPointingAt(ImDrawList *draw_list,
                                      wabi::GfVec2f pos,
                                      wabi::GfVec2f half_sz,
                                      ANCHOR_Dir direction,
                                      AnchorU32 col);
ANCHOR_API void RenderRectFilledRangeH(ImDrawList *draw_list,
                                       const ImRect &rect,
                                       AnchorU32 col,
                                       float x_start_norm,
                                       float x_end_norm,
                                       float rounding);
ANCHOR_API void RenderRectFilledWithHole(ImDrawList *draw_list,
                                         ImRect outer,
                                         ImRect inner,
                                         AnchorU32 col,
                                         float rounding);

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
// [1.71: 2019/06/07: Updating prototypes of some of the internal functions. Leaving those for
// reference for a short while]
inline void RenderArrow(wabi::GfVec2f pos, ANCHOR_Dir dir, float scale = 1.0f)
{
  ANCHOR_Window *window = GetCurrentWindow();
  RenderArrow(window->DrawList, pos, GetColorU32(ANCHOR_Col_Text), dir, scale);
}
inline void RenderBullet(wabi::GfVec2f pos)
{
  ANCHOR_Window *window = GetCurrentWindow();
  RenderBullet(window->DrawList, pos, GetColorU32(ANCHOR_Col_Text));
}
#  endif

// Widgets
ANCHOR_API void TextEx(const char *text, const char *text_end = NULL, ANCHOR_TextFlags flags = 0);
ANCHOR_API bool ButtonEx(const char *label,
                         const wabi::GfVec2f &size_arg = wabi::GfVec2f(0, 0),
                         ANCHOR_ButtonFlags flags      = 0);
ANCHOR_API bool CloseButton(ANCHOR_ID id, const wabi::GfVec2f &pos);
ANCHOR_API bool CollapseButton(ANCHOR_ID id, const wabi::GfVec2f &pos);
ANCHOR_API bool ArrowButtonEx(const char *str_id,
                              ANCHOR_Dir dir,
                              wabi::GfVec2f size_arg,
                              ANCHOR_ButtonFlags flags = 0);
ANCHOR_API void Scrollbar(ANCHOR_Axis axis);
ANCHOR_API bool ScrollbarEx(const ImRect &bb,
                            ANCHOR_ID id,
                            ANCHOR_Axis axis,
                            float *p_scroll_v,
                            float avail_v,
                            float contents_v,
                            ImDrawFlags flags);
ANCHOR_API bool ImageButtonEx(ANCHOR_ID id,
                              AnchorTextureID texture_id,
                              const wabi::GfVec2f &size,
                              const wabi::GfVec2f &uv0,
                              const wabi::GfVec2f &uv1,
                              const wabi::GfVec2f &padding,
                              const wabi::GfVec4f &bg_col,
                              const wabi::GfVec4f &tint_col);
ANCHOR_API ImRect GetWindowScrollbarRect(ANCHOR_Window *window, ANCHOR_Axis axis);
ANCHOR_API ANCHOR_ID GetWindowScrollbarID(ANCHOR_Window *window, ANCHOR_Axis axis);
ANCHOR_API ANCHOR_ID GetWindowResizeCornerID(ANCHOR_Window *window, int n);  // 0..3: corners
ANCHOR_API ANCHOR_ID GetWindowResizeBorderID(ANCHOR_Window *window, ANCHOR_Dir dir);
ANCHOR_API void SeparatorEx(ANCHOR_SeparatorFlags flags);
ANCHOR_API bool CheckboxFlags(const char *label, ImS64 *flags, ImS64 flags_value);
ANCHOR_API bool CheckboxFlags(const char *label, ImU64 *flags, ImU64 flags_value);

// Widgets low-level behaviors
ANCHOR_API bool ButtonBehavior(const ImRect &bb,
                               ANCHOR_ID id,
                               bool *out_hovered,
                               bool *out_held,
                               ANCHOR_ButtonFlags flags = 0);
ANCHOR_API bool DragBehavior(ANCHOR_ID id,
                             ANCHOR_DataType data_type,
                             void *p_v,
                             float v_speed,
                             const void *p_min,
                             const void *p_max,
                             const char *format,
                             ANCHOR_SliderFlags flags);
ANCHOR_API bool SliderBehavior(const ImRect &bb,
                               ANCHOR_ID id,
                               ANCHOR_DataType data_type,
                               void *p_v,
                               const void *p_min,
                               const void *p_max,
                               const char *format,
                               ANCHOR_SliderFlags flags,
                               ImRect *out_grab_bb);
ANCHOR_API bool SplitterBehavior(const ImRect &bb,
                                 ANCHOR_ID id,
                                 ANCHOR_Axis axis,
                                 float *size1,
                                 float *size2,
                                 float min_size1,
                                 float min_size2,
                                 float hover_extend           = 0.0f,
                                 float hover_visibility_delay = 0.0f);
ANCHOR_API bool TreeNodeBehavior(ANCHOR_ID id,
                                 ANCHOR_TreeNodeFlags flags,
                                 const char *label,
                                 const char *label_end = NULL);
// Consume previous SetNextItemOpen() data, if any. May return true when logging
ANCHOR_API bool TreeNodeBehaviorIsOpen(ANCHOR_ID id, ANCHOR_TreeNodeFlags flags = 0);
ANCHOR_API void TreePushOverrideID(ANCHOR_ID id);

// Template functions are instantiated in ANCHOR_widgets.cpp for a finite number of types.
// To use them externally (for custom widget) you may need an "extern template" statement in your
// code in order to link to existing instances and silence Clang warnings (see #2036). e.g. "
// extern template ANCHOR_API float RoundScalarWithFormatT<float, float>(const char* format,
// ANCHOR_DataType data_type, float v); "
template<typename T, typename SIGNED_T, typename FLOAT_T>
ANCHOR_API float ScaleRatioFromValueT(ANCHOR_DataType data_type,
                                      T v,
                                      T v_min,
                                      T v_max,
                                      bool is_logarithmic,
                                      float logarithmic_zero_epsilon,
                                      float zero_deadzone_size);
template<typename T, typename SIGNED_T, typename FLOAT_T>
ANCHOR_API T ScaleValueFromRatioT(ANCHOR_DataType data_type,
                                  float t,
                                  T v_min,
                                  T v_max,
                                  bool is_logarithmic,
                                  float logarithmic_zero_epsilon,
                                  float zero_deadzone_size);
template<typename T, typename SIGNED_T, typename FLOAT_T>
ANCHOR_API bool DragBehaviorT(ANCHOR_DataType data_type,
                              T *v,
                              float v_speed,
                              T v_min,
                              T v_max,
                              const char *format,
                              ANCHOR_SliderFlags flags);
template<typename T, typename SIGNED_T, typename FLOAT_T>
ANCHOR_API bool SliderBehaviorT(const ImRect &bb,
                                ANCHOR_ID id,
                                ANCHOR_DataType data_type,
                                T *v,
                                T v_min,
                                T v_max,
                                const char *format,
                                ANCHOR_SliderFlags flags,
                                ImRect *out_grab_bb);
template<typename T, typename SIGNED_T>
ANCHOR_API T RoundScalarWithFormatT(const char *format, ANCHOR_DataType data_type, T v);
template<typename T> ANCHOR_API bool CheckboxFlagsT(const char *label, T *flags, T flags_value);

// Data type helpers
ANCHOR_API const ANCHOR_DataTypeInfo *DataTypeGetInfo(ANCHOR_DataType data_type);
ANCHOR_API int DataTypeFormatString(char *buf,
                                    int buf_size,
                                    ANCHOR_DataType data_type,
                                    const void *p_data,
                                    const char *format);
ANCHOR_API void DataTypeApplyOp(ANCHOR_DataType data_type,
                                int op,
                                void *output,
                                const void *arg_1,
                                const void *arg_2);
ANCHOR_API bool DataTypeApplyOpFromText(const char *buf,
                                        const char *initial_value_buf,
                                        ANCHOR_DataType data_type,
                                        void *p_data,
                                        const char *format);
ANCHOR_API int DataTypeCompare(ANCHOR_DataType data_type, const void *arg_1, const void *arg_2);
ANCHOR_API bool DataTypeClamp(ANCHOR_DataType data_type,
                              void *p_data,
                              const void *p_min,
                              const void *p_max);

// InputText
ANCHOR_API bool InputTextEx(const char *label,
                            const char *hint,
                            char *buf,
                            int buf_size,
                            const wabi::GfVec2f &size_arg,
                            ANCHORInputTextFlags flags,
                            ANCHORInputTextCallback callback = NULL,
                            void *user_data                  = NULL);
ANCHOR_API bool TempInputText(const ImRect &bb,
                              ANCHOR_ID id,
                              const char *label,
                              char *buf,
                              int buf_size,
                              ANCHORInputTextFlags flags);
ANCHOR_API bool TempInputScalar(const ImRect &bb,
                                ANCHOR_ID id,
                                const char *label,
                                ANCHOR_DataType data_type,
                                void *p_data,
                                const char *format,
                                const void *p_clamp_min = NULL,
                                const void *p_clamp_max = NULL);
inline bool TempInputIsActive(ANCHOR_ID id)
{
  ANCHOR_Context &g = *G_CTX;
  return (g.ActiveId == id && g.TempInputId == id);
}
inline ANCHOR_InputTextState *GetInputTextState(ANCHOR_ID id)
{
  ANCHOR_Context &g = *G_CTX;
  return (g.InputTextState.ID == id) ? &g.InputTextState : NULL;
}  // Get input text state if active

// Color
ANCHOR_API void ColorTooltip(const char *text, const float *col, ANCHOR_ColorEditFlags flags);
ANCHOR_API void ColorEditOptionsPopup(const float *col, ANCHOR_ColorEditFlags flags);
ANCHOR_API void ColorPickerOptionsPopup(const float *ref_col, ANCHOR_ColorEditFlags flags);

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
ANCHOR_API void ShadeVertsLinearColorGradientKeepAlpha(ImDrawList *draw_list,
                                                       int vert_start_idx,
                                                       int vert_end_idx,
                                                       wabi::GfVec2f gradient_p0,
                                                       wabi::GfVec2f gradient_p1,
                                                       AnchorU32 col0,
                                                       AnchorU32 col1);
ANCHOR_API void ShadeVertsLinearUV(ImDrawList *draw_list,
                                   int vert_start_idx,
                                   int vert_end_idx,
                                   const wabi::GfVec2f &a,
                                   const wabi::GfVec2f &b,
                                   const wabi::GfVec2f &uv_a,
                                   const wabi::GfVec2f &uv_b,
                                   bool clamp);

// Garbage collection
ANCHOR_API void GcCompactTransientMiscBuffers();
ANCHOR_API void GcCompactTransientWindowBuffers(ANCHOR_Window *window);
ANCHOR_API void GcAwakeTransientWindowBuffers(ANCHOR_Window *window);

// Debug Tools
ANCHOR_API void ErrorCheckEndFrameRecover(ANCHOR_ErrorLogCallback log_callback,
                                          void *user_data = NULL);
inline void DebugDrawItemRect(AnchorU32 col = ANCHOR_COL32(255, 0, 0, 255))
{
  ANCHOR_Context &g     = *G_CTX;
  ANCHOR_Window *window = g.CurrentWindow;
  GetForegroundDrawList(window)->AddRect(
      window->DC.LastItemRect.Min, window->DC.LastItemRect.Max, col);
}
inline void DebugStartItemPicker()
{
  ANCHOR_Context &g       = *G_CTX;
  g.DebugItemPickerActive = true;
}

ANCHOR_API void ShowFontAtlas(AnchorFontAtlas *atlas);
ANCHOR_API void DebugNodeColumns(ANCHOR_OldColumns *columns);
ANCHOR_API void DebugNodeDrawList(ANCHOR_Window *window,
                                  const ImDrawList *draw_list,
                                  const char *label);
ANCHOR_API void DebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList *out_draw_list,
                                                       const ImDrawList *draw_list,
                                                       const ImDrawCmd *draw_cmd,
                                                       bool show_mesh,
                                                       bool show_aabb);
ANCHOR_API void DebugNodeFont(AnchorFont *font);
ANCHOR_API void DebugNodeStorage(ANCHORStorage *storage, const char *label);
ANCHOR_API void DebugNodeTabBar(ANCHOR_TabBar *tab_bar, const char *label);
ANCHOR_API void DebugNodeTable(ANCHOR_Table *table);
ANCHOR_API void DebugNodeTableSettings(ANCHOR_TableSettings *settings);
ANCHOR_API void DebugNodeWindow(ANCHOR_Window *window, const char *label);
ANCHOR_API void DebugNodeWindowSettings(ANCHOR_WindowSettings *settings);
ANCHOR_API void DebugNodeWindowsList(AnchorVector<ANCHOR_Window *> *windows, const char *label);
ANCHOR_API void DebugNodeViewport(ANCHORViewportP *viewport);
ANCHOR_API void DebugRenderViewportThumbnail(ImDrawList *draw_list,
                                             ANCHORViewportP *viewport,
                                             const ImRect &bb);

}  // namespace ANCHOR

//-----------------------------------------------------------------------------
// [SECTION] AnchorFontAtlas internal API
//-----------------------------------------------------------------------------

// This structure is likely to evolve as we add support for incremental atlas updates
struct AnchorFontBuilderIO {
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

//-----------------------------------------------------------------------------
// [SECTION] Test Engine specific hooks (ANCHOR_test_engine)
//-----------------------------------------------------------------------------

#  ifdef ANCHOR_ENABLE_TEST_ENGINE
extern void ANCHORTestEngineHook_ItemAdd(ANCHOR_Context *ctx, const ImRect &bb, ANCHOR_ID id);
extern void ANCHORTestEngineHook_ItemInfo(ANCHOR_Context *ctx,
                                          ANCHOR_ID id,
                                          const char *label,
                                          ANCHOR_ItemStatusFlags flags);
extern void ANCHORTestEngineHook_IdInfo(ANCHOR_Context *ctx,
                                        ANCHOR_DataType data_type,
                                        ANCHOR_ID id,
                                        const void *data_id);
extern void ANCHORTestEngineHook_IdInfo(ANCHOR_Context *ctx,
                                        ANCHOR_DataType data_type,
                                        ANCHOR_ID id,
                                        const void *data_id,
                                        const void *data_id_end);
extern void ANCHORTestEngineHook_Log(ANCHOR_Context *ctx, const char *fmt, ...);
#    define ANCHOR_TEST_ENGINE_ITEM_ADD(_BB, _ID) \
      if (g.TestEngineHookItems) \
      ANCHORTestEngineHook_ItemAdd(&g, _BB, _ID)  // Register item bounding box
#    define ANCHOR_TEST_ENGINE_ITEM_INFO(_ID, _LABEL, _FLAGS) \
      if (g.TestEngineHookItems) \
      ANCHORTestEngineHook_ItemInfo( \
          &g, _ID, _LABEL, _FLAGS)  // Register item label and status flags (optional)
#    define ANCHOR_TEST_ENGINE_LOG(_FMT, ...) \
      if (g.TestEngineHookItems) \
      ANCHORTestEngineHook_Log( \
          &g, _FMT, __VA_ARGS__)  // Custom log entry from user land into test log
#    define ANCHOR_TEST_ENGINE_ID_INFO(_ID, _TYPE, _DATA) \
      if (g.TestEngineHookIdInfo == id) \
        ANCHORTestEngineHook_IdInfo(&g, _TYPE, _ID, (const void *)(_DATA));
#    define ANCHOR_TEST_ENGINE_ID_INFO2(_ID, _TYPE, _DATA, _DATA2) \
      if (g.TestEngineHookIdInfo == id) \
        ANCHORTestEngineHook_IdInfo(&g, _TYPE, _ID, (const void *)(_DATA), (const void *)(_DATA2));
#  else
#    define ANCHOR_TEST_ENGINE_ITEM_ADD(_BB, _ID) \
      do { \
      } while (0)
#    define ANCHOR_TEST_ENGINE_ITEM_INFO(_ID, _LABEL, _FLAGS) \
      do { \
      } while (0)
#    define ANCHOR_TEST_ENGINE_LOG(_FMT, ...) \
      do { \
      } while (0)
#    define ANCHOR_TEST_ENGINE_ID_INFO(_ID, _TYPE, _DATA) \
      do { \
      } while (0)
#    define ANCHOR_TEST_ENGINE_ID_INFO2(_ID, _TYPE, _DATA, _DATA2) \
      do { \
      } while (0)
#  endif

//-----------------------------------------------------------------------------

#  if defined(__clang__)
#    pragma clang diagnostic pop
#  elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#  endif

#  ifdef _MSC_VER
#    pragma warning(pop)
#  endif

#endif  // #ifndef ANCHOR_DISABLE

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
// (headers)

// Help:
// - Read FAQ at http://dearANCHOR.org/faq
// - Newcomers, read 'Programmer guide' in ANCHOR.cpp for notes on how to setup ANCHOR in your
// codebase.
// - Call and read ANCHOR::ShowDemoWindow() in ANCHOR_demo.cpp. All applications in examples/ are
// doing that. Read ANCHOR.cpp for details, links and comments.

// Resources:
// - FAQ                   http://dearANCHOR.org/faq
// - Homepage & latest     https://github.com/ocornut/ANCHOR
// - Releases & changelog  https://github.com/ocornut/ANCHOR/releases
// - Gallery               https://github.com/ocornut/ANCHOR/issues/3793 (please post your
// screenshots/video there!)
// - Wiki                  https://github.com/ocornut/ANCHOR/wiki (lots of good stuff there)
// - Glossary              https://github.com/ocornut/ANCHOR/wiki/Glossary
// - Issues & support      https://github.com/ocornut/ANCHOR/issues
// - Discussions           https://github.com/ocornut/ANCHOR/discussions

/*

Index of this file:
// [SECTION] Header mess
// [SECTION] Forward declarations and basic types
// [SECTION] ANCHOR end-user API functions
// [SECTION] Flags & Enumerations
// [SECTION] Helpers: Memory allocations macros, AnchorVector<>
// [SECTION] ANCHOR_Style
// [SECTION] ANCHOR_IO
// [SECTION] Misc data structures (ANCHORInputTextCallbackData, ANCHOR_SizeCallbackData,
ANCHORPayload, ANCHOR_TableSortSpecs, ANCHOR_TableColumnSortSpecs)
// [SECTION] Helpers (ANCHOROnceUponAFrame, ANCHORTextFilter, ANCHORTextBuffer, ANCHORStorage,
ANCHORListClipper, AnchorColor)
// [SECTION] Drawing API (ImDrawCallback, ImDrawCmd, ImDrawIdx, ImDrawVert, ImDrawChannel,
ImDrawListSplitter, ImDrawFlags, ImDrawListFlags, ImDrawList, ImDrawData)
// [SECTION] Font API (AnchorFontConfig, AnchorFontGlyph, AnchorFontGlyphRangesBuilder,
AnchorFontAtlasFlags, AnchorFontAtlas, AnchorFont)
// [SECTION] Viewports (ANCHORViewportFlags, ANCHORViewport)
// [SECTION] Obsolete functions and types

*/

#pragma once

#ifdef ANCHOR_USER_CONFIG
#  include ANCHOR_USER_CONFIG
#endif

#ifndef ANCHOR_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

// Includes
#  include <float.h>   // FLT_MIN, FLT_MAX
#  include <stdarg.h>  // va_list, va_start, va_end
#  include <stddef.h>  // ptrdiff_t, NULL
#  include <string.h>  // memset, memmove, memcpy, strlen, strchr, strcpy, strcmp

#  include <wabi/base/gf/vec2f.h>
#  include <wabi/base/gf/vec2h.h>
#  include <wabi/base/gf/vec4f.h>
#  include <wabi/base/tf/tf.h>

#  include <wabi/imaging/hd/driver.h>
#  include <wabi/imaging/hd/engine.h>

#  include <wabi/usdImaging/usdApollo/engine.h>

// Version
// (Integer encoded as XYYZZ for use in #if preprocessor conditionals. Work in progress versions
// typically starts at XYY99 then bounce up to XYY00, XYY01 etc. when release tagging happens)
#  define ANCHOR_VERSION "1.84 WIP"
#  define ANCHOR_VERSION_NUM 18302
#  define ANCHOR_CHECKVERSION() \
    ANCHOR::DebugCheckVersionAndDataLayout(ANCHOR_VERSION, \
                                           sizeof(ANCHOR_IO), \
                                           sizeof(ANCHOR_Style), \
                                           sizeof(wabi::GfVec2f), \
                                           sizeof(wabi::GfVec4f), \
                                           sizeof(ImDrawVert), \
                                           sizeof(ImDrawIdx))
#  define ANCHOR_HAS_TABLE

// Define attributes of all API symbols declarations (e.g. for DLL under Windows)
// ANCHOR_API is used for core ANCHOR functions, ANCHOR_IMPL_API is used for the default backends
// files (ANCHOR_impl_xxx.h) Using ANCHOR via a shared library is not recommended, because we don't
// guarantee backward nor forward ABI compatibility (also function call overhead, as ANCHOR is a
// call-heavy API)
#  ifndef ANCHOR_API
#    define ANCHOR_API
#  endif
#  ifndef ANCHOR_IMPL_API
#    define ANCHOR_IMPL_API ANCHOR_API
#  endif

// Helper Macros
#  ifndef ANCHOR_ASSERT
#    include <assert.h>
#    define ANCHOR_ASSERT(_EXPR) \
      assert(_EXPR)  // You can override the default assert handler by editing ANCHOR_config.h
#  endif
#  define ANCHOR_ARRAYSIZE(_ARR) \
    ((int)(sizeof(_ARR) / sizeof(*(_ARR))))  // Size of a static C-style array. Don't use on pointers!
#  if (__cplusplus >= 201100) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201100)
#    define ANCHOR_OFFSETOF(_TYPE, _MEMBER) \
      offsetof(_TYPE, _MEMBER)  // Offset of _MEMBER within _TYPE. Standardized as offsetof() in C++11
#  else
#    define ANCHOR_OFFSETOF(_TYPE, _MEMBER) \
      ((size_t) & (((_TYPE *)0)->_MEMBER))  // Offset of _MEMBER within _TYPE. Old style macro.
#  endif

// Helper Macros - ANCHOR_FMTARGS, ANCHOR_FMTLIST: Apply printf-style warnings to our formatting
// functions.
#  if !defined(ANCHOR_USE_STB_SPRINTF) && defined(__MINGW32__)
#    define ANCHOR_FMTARGS(FMT) __attribute__((format(gnu_printf, FMT, FMT + 1)))
#    define ANCHOR_FMTLIST(FMT) __attribute__((format(gnu_printf, FMT, 0)))
#  elif !defined(ANCHOR_USE_STB_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#    define ANCHOR_FMTARGS(FMT) __attribute__((format(printf, FMT, FMT + 1)))
#    define ANCHOR_FMTLIST(FMT) __attribute__((format(printf, FMT, 0)))
#  else
#    define ANCHOR_FMTARGS(FMT)
#    define ANCHOR_FMTLIST(FMT)
#  endif

// Disable some of MSVC most aggressive Debug runtime checks in function header/footer (used in
// some simple/low-level functions)
#  if defined(_MSC_VER) && !defined(__clang__) && !defined(ANCHOR_DEBUG_PARANOID)
#    define ANCHOR_MSVC_RUNTIME_CHECKS_OFF \
      __pragma(runtime_checks("", off)) __pragma(check_stack(off)) __pragma(strict_gs_check(push, off))
#    define ANCHOR_MSVC_RUNTIME_CHECKS_RESTORE \
      __pragma(runtime_checks("", restore)) __pragma(check_stack()) __pragma(strict_gs_check(pop))
#  else
#    define ANCHOR_MSVC_RUNTIME_CHECKS_OFF
#    define ANCHOR_MSVC_RUNTIME_CHECKS_RESTORE
#  endif

// Warnings
#  ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 26495)  // [Static Analyzer] Variable 'XXX' is uninitialized. Always
                                      // initialize a member variable (type.6).
#  endif
#  if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wold-style-cast"
#    if __has_warning("-Wzero-as-null-pointer-constant")
#      pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#    endif
#  elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored \
      "-Wpragmas"  // warning: unknown option after '#pragma GCC diagnostic' kind
#    pragma GCC diagnostic ignored \
      "-Wclass-memaccess"  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object
                           // of type 'xxxx' with no trivial copy-assignment; use assignment or
                           // value-initialization instead
#  endif

/**
 * -----------------------------------------------------------------------------
 * [SECTION] Forward declarations and basic types
 * ----------------------------------------------------------------------------- */

/** Cross-platform compatible handles to backends. */
#  define ANCHOR_DECLARE_HANDLE(name) \
    typedef struct name##__ { \
      int unused; \
    } * name

/**
 * Anchor System :: Handles
 *
 * - These are the handles which a client
 *   application is safe to hold reference
 *   pointers to -- as the client application
 *   maintains the lifetime of their own unique
 *   Anchor handles. */
ANCHOR_DECLARE_HANDLE(ANCHOR_EventHandle);
ANCHOR_DECLARE_HANDLE(ANCHOR_EventConsumerHandle);
ANCHOR_DECLARE_HANDLE(ANCHOR_SystemHandle);
ANCHOR_DECLARE_HANDLE(ANCHOR_SystemWindowHandle);

/**
 * Anchor System :: Interfaces
 *
 * - Provides the main abstract API
 *   schema for the platform agnostic
 *   Anchor backend system.                                           */
class ANCHOR_IEvent;         /** <- Anchor Events Interface.          */
class ANCHOR_IEventConsumer; /** <- Anchor Event Consumers Interface. */
class ANCHOR_ISystem;        /** <- Anchor System Backends Interface. */
class ANCHOR_ISystemWindow;  /** <- Anchor System Windows Interface.  */

/**
 * Anchor System :: Platform Agnostic Implementation
 *
 * - Provides the concrete classes which
 *   are to be inherited by the various
 *   platform specific backends.                           */
class ANCHOR_Event;         /** <- Anchor Events.          */
class ANCHOR_EventConsumer; /** <- Anchor Event Consumers. */
class ANCHOR_System;        /** <- Anchor System Backends. */
class ANCHOR_SystemWindow;  /** <- Anchor System Windows.  */

/**
 * Anchor System :: Managers
 *
 * - Provides the concrete classes which
 *   define the event, display, as well
 *   as  the  window  management system.
 *   All, which power Anchor's backend.                        */
class ANCHOR_DisplayManager; /** <- Anchor Display Management. */
class ANCHOR_EventManager;   /** <- Anchor Event Management.   */
class ANCHOR_WindowManager;  /** <- Anchor Window Management.  */

enum eAnchorStatus { ANCHOR_ERROR = -1, ANCHOR_SUCCESS, ANCHOR_RUN, ANCHOR_EVENT };

enum eAnchorDrawingContextType {
  ANCHOR_DrawingContextTypeNone = 0,
  ANCHOR_DrawingContextTypeAllegro,
  ANCHOR_DrawingContextTypeAndroid,
  ANCHOR_DrawingContextTypeDX9,
  ANCHOR_DrawingContextTypeDX10,
  ANCHOR_DrawingContextTypeDX11,
  ANCHOR_DrawingContextTypeDX12,
  ANCHOR_DrawingContextTypeGLFW,
  ANCHOR_DrawingContextTypeGLUT,
  ANCHOR_DrawingContextTypeMarmalade,
  ANCHOR_DrawingContextTypeMetal,
  ANCHOR_DrawingContextTypeOpenGL,
  ANCHOR_DrawingContextTypeOpenXR,
  ANCHOR_DrawingContextTypeOSX,
  ANCHOR_DrawingContextTypeSDL,
  ANCHOR_DrawingContextTypeVulkan,
  ANCHOR_DrawingContextTypeWGPU,
  ANCHOR_DrawingContextTypeWIN32
};

enum eAnchorWindowState {
  ANCHOR_WindowStateNormal = 0,
  ANCHOR_WindowStateMaximized,
  ANCHOR_WindowStateMinimized,
  ANCHOR_WindowStateFullScreen,
  ANCHOR_WindowStateEmbedded,
};

enum eAnchorGrabCursorMode {
  /** Grab not set. */
  ANCHOR_GrabDisable = 0,
  /** No cursor adjustments. */
  ANCHOR_GrabNormal,
  /** Wrap the mouse location to prevent limiting screen bounds. */
  ANCHOR_GrabWrap,
  /**
   * Hide the mouse while grabbing and restore the original location on release
   * (used for number buttons and some other draggable UI elements).
   */
  ANCHOR_GrabHide,
};

enum eAnchorStandardCursor {
  ANCHOR_StandardCursorFirstCursor = 0,
  ANCHOR_StandardCursorDefault = 0,
  ANCHOR_StandardCursorRightArrow,
  ANCHOR_StandardCursorLeftArrow,
  ANCHOR_StandardCursorInfo,
  ANCHOR_StandardCursorDestroy,
  ANCHOR_StandardCursorHelp,
  ANCHOR_StandardCursorWait,
  ANCHOR_StandardCursorText,
  ANCHOR_StandardCursorCrosshair,
  ANCHOR_StandardCursorCrosshairA,
  ANCHOR_StandardCursorCrosshairB,
  ANCHOR_StandardCursorCrosshairC,
  ANCHOR_StandardCursorPencil,
  ANCHOR_StandardCursorUpArrow,
  ANCHOR_StandardCursorDownArrow,
  ANCHOR_StandardCursorVerticalSplit,
  ANCHOR_StandardCursorHorizontalSplit,
  ANCHOR_StandardCursorEraser,
  ANCHOR_StandardCursorKnife,
  ANCHOR_StandardCursorEyedropper,
  ANCHOR_StandardCursorZoomIn,
  ANCHOR_StandardCursorZoomOut,
  ANCHOR_StandardCursorMove,
  ANCHOR_StandardCursorNSEWScroll,
  ANCHOR_StandardCursorNSScroll,
  ANCHOR_StandardCursorEWScroll,
  ANCHOR_StandardCursorStop,
  ANCHOR_StandardCursorUpDown,
  ANCHOR_StandardCursorLeftRight,
  ANCHOR_StandardCursorTopSide,
  ANCHOR_StandardCursorBottomSide,
  ANCHOR_StandardCursorLeftSide,
  ANCHOR_StandardCursorRightSide,
  ANCHOR_StandardCursorTopLeftCorner,
  ANCHOR_StandardCursorTopRightCorner,
  ANCHOR_StandardCursorBottomRightCorner,
  ANCHOR_StandardCursorBottomLeftCorner,
  ANCHOR_StandardCursorCopy,
  ANCHOR_StandardCursorCustom,

  ANCHOR_StandardCursorNumCursors
};

// Forward declarations
struct ImDrawChannel;         // Temporary storage to output draw commands out of order, used by
                              // ImDrawListSplitter and ImDrawList::ChannelsSplit()
struct ImDrawCmd;             // A single draw command within a parent ImDrawList (generally maps to 1 GPU
                              // draw call, unless it is a callback)
struct ImDrawData;            // All draw command lists required to render the frame + pos/size coordinates
                              // to use for the projection matrix.
struct ImDrawList;            // A single draw command list (generally one per window, conceptually you may
                              // see this as a dynamic "mesh" builder)
struct ImDrawListSharedData;  // Data shared among multiple draw lists (typically owned by parent
                              // ANCHOR context, but you may create one yourself)
struct ImDrawListSplitter;    // Helper to split a draw list into different layers which can be drawn
                              // into out of order, then flattened back.
struct ImDrawVert;            // A single vertex (pos + uv + col = 20 bytes by default. Override layout with
                              // ANCHOR_OVERRIDE_DRAWVERT_STRUCT_LAYOUT)
struct AnchorFont;            // Runtime data for a single font within a parent AnchorFontAtlas
struct AnchorFontAtlas;       // Runtime data for multiple fonts, bake multiple fonts into a single
                              // texture, TTF/OTF font loader
struct AnchorFontBuilderIO;   // Opaque interface to a font builder (stb_truetype or FreeType).
struct AnchorFontConfig;      // Configuration data when adding a font or merging fonts
struct AnchorFontGlyph;       // A single font glyph (code point + coordinates within in AnchorFontAtlas
                              // + offset)
struct AnchorFontGlyphRangesBuilder;  // Helper to build glyph ranges from text/string data
struct AnchorColor;     // Helper functions to create a color that can be converted to either u32 or
                        // float4 (*OBSOLETE* please avoid using)
struct ANCHOR_Context;  // ANCHOR context (opaque structure, unless including ANCHOR_internal.h)
struct ANCHOR_IO;       // Main configuration and I/O between your application and ANCHOR
struct ANCHORInputTextCallbackData;  // Shared state of InputText() when using custom
                                     // ANCHORInputTextCallback (rare/advanced use)
struct ANCHORListClipper;            // Helper to manually clip large list of items
struct ANCHOROnceUponAFrame;         // Helper for running a block of code not more than once a frame,
                                     // used by ANCHOR_ONCE_UPON_A_FRAME macro
struct ANCHORPayload;                // User data payload for drag and drop operations
struct ANCHOR_SizeCallbackData;      // Callback data when using SetNextWindowSizeConstraints()
                                     // (rare/advanced use)
struct ANCHORStorage;                // Helper for key->value storage
struct ANCHOR_Style;                 // Runtime data for styling/colors
struct ANCHOR_TableSortSpecs;        // Sorting specifications for a table (often handling sort specs for
                                     // a single column, occasionally more)
struct ANCHOR_TableColumnSortSpecs;  // Sorting specification for one column of a table
struct ANCHORTextBuffer;             // Helper to hold and append into a text buffer (~string builder)
struct ANCHORTextFilter;             // Helper to parse and apply text filters (e.g. "aaaaa[,bbbbb][,ccccc]")
struct ANCHORViewport;               // A Platform Window (always only one in 'master' branch), in the future
                                     // may represent Platform Monitor

/**
 * Event Types ----------- */

enum ANCHOR_EventType {
  ANCHOR_EventUnknown = 0,

  ANCHOR_EventCursorMove,  /// Mouse move event
  ANCHOR_EventButtonDown,  /// Mouse button event
  ANCHOR_EventButtonUp,    /// Mouse button event
  ANCHOR_EventWheel,       /// Mouse wheel event
  ANCHOR_EventTrackpad,    /// Trackpad event

  ANCHOR_EventKeyDown,
  ANCHOR_EventKeyUp,

  ANCHOR_EventQuitRequest,

  ANCHOR_EventWindowClose,
  ANCHOR_EventWindowActivate,
  ANCHOR_EventWindowDeactivate,
  ANCHOR_EventWindowUpdate,
  ANCHOR_EventWindowSize,
  ANCHOR_EventWindowMove,
  ANCHOR_EventWindowDPIHintChanged,

  ANCHOR_EventDraggingEntered,
  ANCHOR_EventDraggingUpdated,
  ANCHOR_EventDraggingExited,
  ANCHOR_EventDraggingDropDone,

  ANCHOR_EventOpenMainFile,            // Needed for Cocoa to open double-clicked .blend file at startup
  ANCHOR_EventNativeResolutionChange,  // Needed for Cocoa when window moves to other display

  ANCHOR_EventTimer,

  ANCHOR_EventImeCompositionStart,
  ANCHOR_EventImeComposition,
  ANCHOR_EventImeCompositionEnd,

  ANCHOR_NumEventTypes
};

typedef int ANCHOR_Col;               /// Enum: A color identifier for styling
typedef int ANCHOR_Cond;              /// Enum: A condition for many Set*() functions
typedef int ANCHOR_DataType;          /// Enum: A primary data type
typedef int ANCHOR_Dir;               /// Enum: A cardinal direction
typedef int ANCHOR_Key;               /// Enum: A key identifier (ANCHOR-side enum)
typedef int ANCHOR_NavInput;          /// Enum: An input identifier for navigation
typedef int ANCHOR_MouseButton;       /// Enum: A mouse button
typedef int ANCHOR_MouseCursor;       /// Enum: A mouse cursor identifier
typedef int ANCHOR_SortDirection;     /// Enum: A sorting direction (ascending or descending)
typedef int ANCHOR_StyleVar;          /// Enum: A variable identifier for styling
typedef int ANCHOR_TableBgTarget;     /// Enum: A color target for TableSetBgColor()
typedef int ImDrawFlags;              /// Flags: For ImDrawList functions
typedef int ImDrawListFlags;          /// Flags: For ImDrawList instance
typedef int AnchorFontAtlasFlags;     /// Flags: For AnchorFontAtlas build
typedef int ANCHORBackendFlags;       /// Flags: For io.BackendFlags
typedef int ANCHOR_ButtonFlags;       /// Flags: For InvisibleButton()
typedef int ANCHOR_ColorEditFlags;    /// Flags: For ColorEdit4(), ColorPicker4() etc.
typedef int ANCHORConfigFlags;        /// Flags: For io.ConfigFlags
typedef int ANCHORComboFlags;         /// Flags: For BeginCombo()
typedef int ANCHORDragDropFlags;      /// Flags: For BeginDragDropSource(), AcceptDragDropPayload()
typedef int ANCHORFocusedFlags;       /// Flags: For IsWindowFocused()
typedef int ANCHORHoveredFlags;       /// Flags: For IsItemHovered(), IsWindowHovered() etc.
typedef int ANCHORInputTextFlags;     /// Flags: For InputText(), InputTextMultiline()
typedef int ANCHOR_KeyModFlags;       /// Flags: For io.KeyMods (Ctrl/Shift/Alt/Super)
typedef int ANCHORPopupFlags;         /// Flags: For OpenPopup(), IsPopupOpen()
typedef int ANCHORSelectableFlags;    /// Flags: For Selectable()
typedef int ANCHOR_SliderFlags;       /// Flags: For DragFloat(), DragInt(), SliderFloat()
typedef int ANCHOR_TabBarFlags;       /// Flags: For BeginTabBar()
typedef int ANCHOR_TabItemFlags;      /// Flags: For BeginTabItem()
typedef int ANCHOR_TableFlags;        /// Flags: For BeginTable()
typedef int ANCHOR_TableColumnFlags;  /// Flags: For TableSetupColumn()
typedef int ANCHOR_TableRowFlags;     /// Flags: For TableNextRow()
typedef int ANCHOR_TreeNodeFlags;     /// Flags: for TreeNode(), TreeNodeEx(), CollapsingHeader()
typedef int ANCHORViewportFlags;      /// Flags: for ANCHORViewport
typedef int ANCHOR_WindowFlags;       /// Flags: for Begin(), BeginChild()

/**
 * Other types ----------- */

/**
 * AnchorTextureID
 * [configurable type: #define AnchorTextureID xxx'] */
#  ifndef AnchorTextureID
/**
 * User data for rendering backend to identify a texture.
 * This is whatever you want it to be!*/
typedef void *AnchorTextureID;
#  endif

/**
 * For event handling with client applications,
 * in the case of covah  --  ANCHOR_UserPtr is
 * assigned to the #cContext data structure. */
typedef void *ANCHOR_UserPtr;
typedef void *ANCHOR_EventPtr;

/**
 * The Anchor backend measures time with
 * 64-bit unsigned integer precision. */
typedef AnchorU64 ANCHOR_Time;

/**
 * A unique ID used by widgets, hashed from a stack of string. */
typedef unsigned int ANCHOR_ID;
/**
 * Callback function for ANCHOR::InputText() */
typedef int (*ANCHORInputTextCallback)(ANCHORInputTextCallbackData *data);
/**
 * Callback function for ANCHOR::SetNextWindowSizeConstraints() */
typedef void (*ANCHORSizeCallback)(ANCHOR_SizeCallbackData *data);
/**
 * Function signature for ANCHOR::SetAllocatorFunctions() */
typedef void *(*ANCHORMemAllocFunc)(size_t sz, void *user_data);
/**
 * Function signature for ANCHOR::SetAllocatorFunctions() */
typedef void (*ANCHORMemFreeFunc)(void *ptr, void *user_data);

// Character types
// (we generally use UTF-8 encoded string in the API. This is storage specifically for a decoded
// character used for keyboard input and display)
typedef unsigned short AnchorWChar16;  // A single decoded U16 character/code point. We encode them
                                       // as multi bytes UTF-8 when used in strings.
typedef unsigned int AnchorWChar32;    // A single decoded U32 character/code point. We encode them
                                       // as multi bytes UTF-8 when used in strings.
#  ifdef ANCHOR_USE_WCHAR32            // AnchorWChar [configurable type: override in ANCHOR_config.h with
                                       // '#define ANCHOR_USE_WCHAR32' to support Unicode planes 1-16]
typedef AnchorWChar32 AnchorWChar;
#  else
typedef AnchorWChar16 AnchorWChar;
#  endif

// Basic scalar data types
typedef signed char AnchorS8;      // 8-bit signed integer
typedef unsigned char AnchorU8;    // 8-bit unsigned integer
typedef signed short AnchorS16;    // 16-bit signed integer
typedef unsigned short AnchorU16;  // 16-bit unsigned integer
typedef signed int AnchorS32;      // 32-bit signed integer == int
typedef unsigned int AnchorU32;    // 32-bit unsigned integer (often used to store packed colors)
#  if defined(_MSC_VER) && !defined(__clang__)
typedef signed __int64 AnchorS64;    // 64-bit signed integer (pre and post C++11 with MSVC)
typedef unsigned __int64 AnchorU64;  // 64-bit unsigned integer (pre and post C++11 with MSVC)
#  elif (defined(__clang__) || defined(__GNUC__)) && (__cplusplus < 201100)
#    include <stdint.h>
typedef int64_t AnchorS64;   // 64-bit signed integer (pre C++11)
typedef uint64_t AnchorU64;  // 64-bit unsigned integer (pre C++11)
#  else
typedef signed long long AnchorS64;    // 64-bit signed integer (post C++11)
typedef unsigned long long AnchorU64;  // 64-bit unsigned integer (post C++11)
#  endif

typedef struct {
  /** Number of pixels on a line. */
  AnchorU32 xPixels;
  /** Number of lines. */
  AnchorU32 yPixels;
  /** Number of bits per pixel. */
  AnchorU32 bpp;
  /** Refresh rate (in Hertz). */
  AnchorU32 frequency;
} ANCHOR_DisplaySetting;

namespace ANCHOR {
/**
 * Context creation and access.
 *  - Each context create its own AnchorFontAtlas by default.
 *    you may instance one yourself and pass it to CreateContext()
 *    to share a font atlas between contexts.
 *  - DLL users: heaps and globals are not shared across DLL boundaries!
 *    you will need to call SetCurrentContext() + SetAllocatorFunctions()
 *    for each static/DLL boundary you are calling from. Read "Context and
 *    Memory Allocators" section of ANCHOR.cpp for details. */

ANCHOR_API
ANCHOR_Context *CreateContext(AnchorFontAtlas *shared_font_atlas = NULL);

/**
 * NULL = destroy current context */
ANCHOR_API
void DestroyContext(ANCHOR_Context *ctx = NULL);

ANCHOR_API
ANCHOR_Context *GetCurrentContext();

ANCHOR_API
void SetCurrentContext(ANCHOR_Context *ctx);

/**
 * ⚓︎ Anchor :: Main -------------------- */

/**
 * Process Events (User Actions).
 *
 *  - mouse
 *  - keyboard
 *  - gamepad inputs
 *  - time
 *
 * @param systemhandle: Handle to backend system.
 * @param waitForEvent: To indicate that this call
 * should wait (block) until the next event before
 * returning.
 * @return Indication of the presence of events. */

ANCHOR_API
bool ProcessEvents(ANCHOR_SystemHandle systemhandle, bool waitForEvent);

/**
 * Preforms a swap on the swapchain.
 *
 * This is what one may refer to as
 * the "display update" which takes
 * all old 'cache', and swaps it to
 * the new 'cache'. This is intended
 * to be called at a bare minium of
 * a monitor's refresh rate. Any bit
 * slower than that and a user will
 * experience graphics 'lag'.
 * @return Indication of success. */
ANCHOR_API
eAnchorStatus SwapChain(ANCHOR_SystemWindowHandle windowhandle);

/**
 * Adds a given event consumer to anchor.
 *
 * An event consumer is a client who
 * recieves Anchor events on the stack
 * usually in the form of a callback
 * function.
 *
 * @param consumer: The event consumer to add.
 * @return Indication of success. */
ANCHOR_API
eAnchorStatus AddEventConsumer(ANCHOR_EventConsumerHandle consumer);

/**
 * Access the Pixar Hydra Driver.
 *
 *  - Central Shared GPU resources in which
 *    Covah (and plugins) share. Because they
 *    are all shared, and because you can
 *    activate any number of rendering engines
 *    at once ↓
 *  → This is the basis for Hybrid Rendering:
 *      - Arnold     →  Human
 *      - Cycles     →  House
 *      - Renderman  →  Glass
 *      - Phoenix    →  Sky
 *  → Each individual engine rendering their
 *    own respective Prims in a single scene.
 *  → All within the same active viewport.
 *  → All within @em Real-Time. */
ANCHOR_API
wabi::HdDriver &GetPixarDriver();

/**
 * Access the Hydra Engine.
 *
 *  - The Hydra Engine is responsible
 *    for locating Render Engine Plugins
 *    such as Arnold, Renderman, Cycles,
 *    Phoenix, etc. And allowing you to
 *    interface with all of them using
 *    the same underlying agnostic API.
 *  - Hydra Engines can be specialized,
 *    Apollo is a General Purpose engine
 *    for general scene layout and
 *    animation purposes.
 *  - @em Specialization-Engines created
 *    & optimized for:
 *      - VFX (pyro, physics)
 *      - Rigging (AAA animation)
 *      - Hair & Fur
 *      - Game Engine (RTX)
 *      - Misc. */
ANCHOR_API
APOLLO_EnginePtr GetEngineApollo();

/**
 * Access the IO structure.
 *
 *  - mouse
 *  - keyboard
 *  - gamepad inputs
 *  - time
 *  - config options/flags */
ANCHOR_API
ANCHOR_IO &GetIO();

/**
 * Access the Style structure (colors, sizes).
 *
 * Always use:
 *  - PushStyleCol()
 *  - PushStyleVar()
 * to modify style mid-frame! */
ANCHOR_API
ANCHOR_Style &GetStyle();

/**
 * Start a new ANCHOR frame.
 *
 * You can submit any command
 * from this point until
 * Render() / EndFrame(). */
ANCHOR_API
void NewFrame();

/**
 * Ends the ANCHOR frame.
 *
 * Automatically called by Render().
 * If you don't need to render data
 * (skipping rendering) you may call
 * EndFrame() without Render()... but
 * you'll have wasted CPU already! If
 * you don't need to render, better to
 * not create any windows and not call
 * NewFrame() at all! */
ANCHOR_API
void EndFrame();

/**
 * Ends the ANCHOR frame,
 *
 * finalize the draw data.
 * You can then get call
 * GetDrawData(). */
ANCHOR_API
void Render();

/**
 * Draw Data.
 *
 * Valid after Render() and
 * until the next call to
 * NewFrame(). This is what
 * you have to render. */
ANCHOR_API
ImDrawData *GetDrawData();

/**
 * Diagnostic, Debug Window.
 *
 * Demonstrate most ANCHOR
 * features. Call this to
 * learn about the library! */
ANCHOR_API
void ShowDemoWindow(bool *p_open = NULL);

/**
 * Create Metrics/Debugger window.
 *
 * Display ANCHOR internals: windows,
 * draw commands, various internal
 * state, etc. */
ANCHOR_API
void ShowMetricsWindow(bool *p_open = NULL);

/**
 * Create About window.
 *
 * Display ANCHOR version,
 * credits and build/system
 * information. */
ANCHOR_API
void ShowAboutWindow(bool *p_open = NULL);

/**
 * Add style editor block (not a window).
 *
 * You can pass in a reference ANCHOR_Style
 * structure to compare to, revert to and
 * save to (else it uses the default style) */
ANCHOR_API
void ShowStyleEditor(ANCHOR_Style *ref = NULL);

/**
 * Add style selector block (not a window)
 *
 * Essentially a combo listing the default
 * styles. */
ANCHOR_API
bool ShowStyleSelector(const char *label);

/**
 * Add font selector block (not a window)
 *
 * Essentially a combo listing the loaded
 * system fonts. */
ANCHOR_API
void ShowFontSelector(const char *label);

/**
 * Add basic help/info block (not a window)
 *
 * How to manipulate ANCHOR as a end-user
 * (mouse/keyboard controls). */
ANCHOR_API
void ShowUserGuide();

/**
 * Get the compiled version string
 *
 *  - e.g. "1.80 WIP"
 *  - (The value for ANCHOR_VERSION) */
ANCHOR_API
const char *GetVersion();

/**
 * ⚓︎ Anchor :: Styles -------------------- */

/**
 * Covah Default (Recommended).
 *
 * Covah default color theme.
 * For all your digital content
 * creation needs. */
ANCHOR_API
void StyleColorsDefault(ANCHOR_Style *dst = NULL);

/**
 * Dark Side.
 *
 * It's 2021 and everyone
 * needs a dark mode. */
ANCHOR_API
void StyleColorsDark(ANCHOR_Style *dst = NULL);

/**
 * Jedi.
 *
 * Best used with borders
 * and a custom, thicker
 * font */
ANCHOR_API
void StyleColorsLight(ANCHOR_Style *dst = NULL);

/**
 * ⚓︎ Anchor :: Windowing -------------------- */

/**
 * Windows
 *  - Begin() = push window to the stack and start appending to it.
 *  - End() = pop window from the stack.
 *  - Passing 'bool* p_open != NULL' shows a window-closing widget
 *    in the upper-right corner of the window, which clicking will
 *    set the boolean to false when clicked.
 *  - You may append multiple times to the same window during the
 *    same frame by calling Begin() / End() pairs multiple times.
 *  - Some information such as 'flags' or 'p_open' will only be
 *    considered by the first call to Begin().
 *  - Begin() return false to indicate the window is collapsed or
 *    fully clipped, so you  may early out  and omit  submitting
 *    anything to the window. Always call a matching End() for each
 *    Begin() call, regardless of its return value! [Important: due
 *    to legacy reason, this is inconsistent with most other functions
 *    such as BeginMenu / EndMenu, BeginPopup/EndPopup, etc. where the
 *    EndXXX call should only be called if the corresponding BeginXXX
 *    function returned true. Begin and BeginChild are the only odd
 *    ones out. Will be fixed in a future update.]
 *  - Note that the bottom of window stack always contains a window
 *    called "Debug". */
ANCHOR_API
bool Begin(const char *name, bool *p_open = NULL, ANCHOR_WindowFlags flags = 0);

ANCHOR_API
void End();

// Child Windows
// - Use child windows to begin into a self-contained independent scrolling/clipping regions within
// a host window. Child windows can embed their own child.
// - For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed
// size / <0.0f: use remaining window size minus abs(size) / Each axis can use a different mode,
// e.g. wabi::GfVec2f(0,400).
// - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may
// early out and omit submitting anything to the window.
//   Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
//   [Important: due to legacy reason, this is inconsistent with most other functions such as
//   BeginMenu/EndMenu,
//    BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding
//    BeginXXX function returned true. Begin and BeginChild are the only odd ones out. Will be
//    fixed in a future update.]
ANCHOR_API bool BeginChild(const char *str_id,
                           const wabi::GfVec2f &size = wabi::GfVec2f(0, 0),
                           bool border = false,
                           ANCHOR_WindowFlags flags = 0);
ANCHOR_API bool BeginChild(ANCHOR_ID id,
                           const wabi::GfVec2f &size = wabi::GfVec2f(0, 0),
                           bool border = false,
                           ANCHOR_WindowFlags flags = 0);
ANCHOR_API void EndChild();

// Windows Utilities
// - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next
// window' = next window we will Begin() into.
ANCHOR_API bool IsWindowAppearing();
ANCHOR_API bool IsWindowCollapsed();
// is current window focused? or its root/child, depending on
// flags. see flags for options.
ANCHOR_API bool IsWindowFocused(ANCHORFocusedFlags flags = 0);

// is current window hovered (and typically: not blocked by a popup/modal)? see flags
// for options. NB: If you are trying to check whether your mouse should be dispatched
// to ANCHOR or to your app, you should use the 'io.WantCaptureMouse' boolean for
// that! Please read the FAQ!
ANCHOR_API bool IsWindowHovered(ANCHORHoveredFlags flags = 0);

// get draw list associated to the current window, to
// append your own drawing primitives
ANCHOR_API ImDrawList *GetWindowDrawList();
// get current window position in screen space (useful if
// you want to do your own drawing via the DrawList API)
ANCHOR_API wabi::GfVec2f GetWindowPos();
// get current window size
ANCHOR_API wabi::GfVec2f GetWindowSize();
// get current window width (shortcut for GetWindowSize()[0])
ANCHOR_API float GetWindowWidth();
// get current window height (shortcut for GetWindowSize()[1])
ANCHOR_API float GetWindowHeight();

// Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
ANCHOR_API void SetNextWindowPos(
  const wabi::GfVec2f &pos,
  ANCHOR_Cond cond = 0,
  const wabi::GfVec2f &pivot = wabi::GfVec2f(0,
                                             0));  // set next window position. call before Begin(). use
                                                   // pivot=(0.5f,0.5f) to center on given point, etc.
ANCHOR_API void SetNextWindowSize(const wabi::GfVec2f &size,
                                  ANCHOR_Cond cond = 0);  // set next window size. set axis to 0.0f to force
                                                          // an auto-fit on this axis. call before Begin()
ANCHOR_API void SetNextWindowSizeConstraints(
  const wabi::GfVec2f &size_min,
  const wabi::GfVec2f &size_max,
  ANCHORSizeCallback custom_callback = NULL,
  void *custom_callback_data = NULL);  // set next window size limits. use -1,-1 on either X/Y axis to
                                       // preserve the current size. Sizes will be rounded down. Use callback
                                       // to apply non-trivial programmatic constraints.
ANCHOR_API void SetNextWindowContentSize(
  const wabi::GfVec2f
    &size);  // set next window content size (~ scrollable client area, which enforce the range
             // of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor
             // WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
ANCHOR_API void SetNextWindowCollapsed(
  bool collapsed,
  ANCHOR_Cond cond = 0);               // set next window collapsed state. call before Begin()
ANCHOR_API void SetNextWindowFocus();  // set next window to be focused / top-most. call before
                                       // Begin()
ANCHOR_API void SetNextWindowBgAlpha(
  float alpha);  // set next window background color alpha. helper to easily override the Alpha
                 // component of ANCHOR_Col_WindowBg/ChildBg/PopupBg. you may also use
                 // ANCHOR_WindowFlags_NoBackground.
ANCHOR_API void SetWindowPos(
  const wabi::GfVec2f &pos,
  ANCHOR_Cond cond = 0);  // (not recommended) set current window position - call within Begin()/End().
                          // prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
ANCHOR_API void SetWindowSize(
  const wabi::GfVec2f &size,
  ANCHOR_Cond cond = 0);  // (not recommended) set current window size - call within Begin()/End().
                          // set to wabi::GfVec2f(0, 0) to force an auto-fit. prefer using
                          // SetNextWindowSize(), as this may incur tearing and minor side-effects.
ANCHOR_API void SetWindowCollapsed(bool collapsed,
                                   ANCHOR_Cond cond = 0);  // (not recommended) set current window collapsed
                                                           // state. prefer using SetNextWindowCollapsed().
ANCHOR_API void SetWindowFocus();  // (not recommended) set current window to be focused /
                                   // top-most. prefer using SetNextWindowFocus().
ANCHOR_API void SetWindowFontScale(
  float scale);  // set font scale. Adjust IO.FontGlobalScale if you want to scale all windows.
                 // This is an old API! For correct scaling, prefer to reload font + rebuild
                 // AnchorFontAtlas + call style.ScaleAllSizes().
ANCHOR_API void SetWindowPos(const char *name,
                             const wabi::GfVec2f &pos,
                             ANCHOR_Cond cond = 0);  // set named window position.
ANCHOR_API void SetWindowSize(const char *name,
                              const wabi::GfVec2f &size,
                              ANCHOR_Cond cond = 0);  // set named window size. set axis to 0.0f to
                                                      // force an auto-fit on this axis.
ANCHOR_API void SetWindowCollapsed(const char *name,
                                   bool collapsed,
                                   ANCHOR_Cond cond = 0);  // set named window collapsed state
ANCHOR_API void SetWindowFocus(
  const char *name);  // set named window to be focused / top-most. use NULL to remove focus.

// Content region
// - Retrieve available space from a given point. GetContentRegionAvail() is frequently useful.
// - Those functions are bound to be redesigned (they are confusing, incomplete and the Min/Max
// return values are in local window coordinates which increases confusion)
ANCHOR_API wabi::GfVec2f GetContentRegionAvail();      // == GetContentRegionMax() - GetCursorPos()
ANCHOR_API wabi::GfVec2f GetContentRegionMax();        // current content boundaries (typically window
                                                       // boundaries including scrolling, or current
                                                       // column boundaries), in windows coordinates
ANCHOR_API wabi::GfVec2f GetWindowContentRegionMin();  // content boundaries min (roughly
                                                       // (0,0)-Scroll), in window coordinates
ANCHOR_API wabi::GfVec2f GetWindowContentRegionMax();  // content boundaries max (roughly
                                                       // (0,0)+Size-Scroll) where Size can be
                                                       // override with SetNextWindowContentSize(),
                                                       // in window coordinates
ANCHOR_API float GetWindowContentRegionWidth();        //

// Windows Scrolling
ANCHOR_API float GetScrollX();               // get scrolling amount [0 .. GetScrollMaxX()]
ANCHOR_API float GetScrollY();               // get scrolling amount [0 .. GetScrollMaxY()]
ANCHOR_API void SetScrollX(float scroll_x);  // set scrolling amount [0 .. GetScrollMaxX()]
ANCHOR_API void SetScrollY(float scroll_y);  // set scrolling amount [0 .. GetScrollMaxY()]
ANCHOR_API float GetScrollMaxX();            // get maximum scrolling amount ~~ ContentSize[0] -
                                             // WindowSize[0] - DecorationsSize[0]
ANCHOR_API float GetScrollMaxY();            // get maximum scrolling amount ~~ ContentSize[1] -
                                             // WindowSize[1] - DecorationsSize[1]
ANCHOR_API void SetScrollHereX(
  float center_x_ratio =
    0.5f);  // adjust scrolling amount to make current cursor position visible.
            // center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a
            // "default/current item" visible, consider using SetItemDefaultFocus() instead.
ANCHOR_API void SetScrollHereY(
  float center_y_ratio =
    0.5f);  // adjust scrolling amount to make current cursor position visible.
            // center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a
            // "default/current item" visible, consider using SetItemDefaultFocus() instead.
ANCHOR_API void SetScrollFromPosX(
  float local_x,
  float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally
                                 // GetCursorStartPos() + offset to compute a valid position.
ANCHOR_API void SetScrollFromPosY(
  float local_y,
  float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally
                                 // GetCursorStartPos() + offset to compute a valid position.

// Parameters stacks (shared)
ANCHOR_API void PushFont(AnchorFont *font);  // use NULL as a shortcut to push default font
ANCHOR_API void PopFont();
ANCHOR_API void PushStyleColor(ANCHOR_Col idx,
                               AnchorU32 col);  // modify a style color. always use this if you
                                                // modify the style after NewFrame().
ANCHOR_API void PushStyleColor(ANCHOR_Col idx, const wabi::GfVec4f &col);
ANCHOR_API void PopStyleColor(int count = 1);
ANCHOR_API void PushStyleVar(ANCHOR_StyleVar idx,
                             float val);  // modify a style float variable. always use this if you
                                          // modify the style after NewFrame().
ANCHOR_API void PushStyleVar(ANCHOR_StyleVar idx,
                             const wabi::GfVec2f &val);  // modify a style wabi::GfVec2f variable. always use
                                                         // this if you modify the style after NewFrame().
ANCHOR_API void PopStyleVar(int count = 1);
ANCHOR_API void PushAllowKeyboardFocus(
  bool allow_keyboard_focus);  // == tab stop enable. Allow focusing using TAB/Shift-TAB, enabled
                               // by default but you can disable it for certain widgets
ANCHOR_API void PopAllowKeyboardFocus();
ANCHOR_API void PushButtonRepeat(
  bool repeat);  // in 'repeat' mode, Button*() functions return repeated true in a typematic
                 // manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can
                 // call IsItemActive() after any Button() to tell if the button is held in the
                 // current frame.
ANCHOR_API void PopButtonRepeat();

// Parameters stacks (current window)
ANCHOR_API void PushItemWidth(
  float item_width);  // push width of items for common large "item+label" widgets. >0.0f: width
                      // in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN
                      // always align width to the right side).
ANCHOR_API void PopItemWidth();
ANCHOR_API void SetNextItemWidth(
  float item_width);               // set width of the _next_ common large "item+label" widget. >0.0f: width
                                   // in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN
                                   // always align width to the right side)
ANCHOR_API float CalcItemWidth();  // width of item given pushed settings and current cursor
                                   // position. NOT necessarily the width of last item unlike most
                                   // 'Item' functions.
ANCHOR_API void PushTextWrapPos(
  float wrap_local_pos_x = 0.0f);  // push word-wrapping position for Text*() commands. < 0.0f:
                                   // no wrapping; 0.0f: wrap to end of window (or column); >
                                   // 0.0f: wrap at 'wrap_pos_x' position in window local space
ANCHOR_API void PopTextWrapPos();

// Style read access
ANCHOR_API AnchorFont *GetFont();  // get current font
ANCHOR_API float GetFontSize();    // get current font size (= height in pixels) of current font with
                                   // current scale applied
ANCHOR_API wabi::GfVec2f GetFontTexUvWhitePixel();  // get UV coordinate for a while pixel, useful
                                                    // to draw custom shapes via the ImDrawList API
ANCHOR_API AnchorU32 GetColorU32(
  ANCHOR_Col idx,
  float alpha_mul = 1.0f);  // retrieve given style color with style alpha applied and optional extra
                            // alpha multiplier, packed as a 32-bit value suitable for ImDrawList
ANCHOR_API AnchorU32
GetColorU32(const wabi::GfVec4f &col);            // retrieve given color with style alpha applied, packed as
                                                  // a 32-bit value suitable for ImDrawList
ANCHOR_API AnchorU32 GetColorU32(AnchorU32 col);  // retrieve given color with style alpha applied, packed as
                                                  // a 32-bit value suitable for ImDrawList
ANCHOR_API const wabi::GfVec4f &GetStyleColorVec4(
  ANCHOR_Col idx);  // retrieve style color as stored in ANCHOR_Style structure. use to feed back
                    // into PushStyleColor(), otherwise use GetColorU32() to get style color with
                    // style alpha baked in.

// Cursor / Layout
// - By "cursor" we mean the current output position.
// - The typical widget behavior is to output themselves at the current cursor position, then move
// the cursor one line down.
// - You can call SameLine() between widgets to undo the last carriage return and output at the
// right of the preceding widget.
// - Attention! We currently have inconsistencies between window-local and absolute positions we
// will aim to fix with future API:
//    Window-local coordinates:   SameLine(), GetCursorPos(), SetCursorPos(), GetCursorStartPos(),
//    GetContentRegionMax(), GetWindowContentRegion*(), PushTextWrapPos() Absolute coordinate:
//    GetCursorScreenPos(), SetCursorScreenPos(), all ImDrawList:: functions.
ANCHOR_API void Separator();  // separator, generally horizontal. inside a menu bar or in
                              // horizontal layout mode, this becomes a vertical separator.
ANCHOR_API void SameLine(float offset_from_start_x = 0.0f,
                         float spacing = -1.0f);  // call between widgets or groups to layout them
                                                  // horizontally. X position given in window coordinates.
ANCHOR_API void NewLine();  // undo a SameLine() or force a new line when in an horizontal-layout
                            // context.
ANCHOR_API void Spacing();  // add vertical spacing.
ANCHOR_API void Dummy(
  const wabi::GfVec2f &size);                   // add a dummy item of given size. unlike InvisibleButton(),
                                                // Dummy() won't take the mouse click or be navigable into.
ANCHOR_API void Indent(float indent_w = 0.0f);  // move content position toward the right, by indent_w, or
                                                // style.IndentSpacing if indent_w <= 0
ANCHOR_API void Unindent(float indent_w = 0.0f);  // move content position back to the left, by indent_w, or
                                                  // style.IndentSpacing if indent_w <= 0
ANCHOR_API void BeginGroup();                     // lock horizontal starting position
ANCHOR_API void EndGroup();               // unlock horizontal starting position + capture the whole group
                                          // bounding box into one "item" (so you can use IsItemHovered() or
                                          // layout primitives such as SameLine() on whole group, etc.)
ANCHOR_API wabi::GfVec2f GetCursorPos();  // cursor position in window coordinates (relative to
                                          // window position)
ANCHOR_API float GetCursorPosX();         //   (some functions are using window-relative coordinates, such
                                          //   as: GetCursorPos, GetCursorStartPos, GetContentRegionMax,
                                          //   GetWindowContentRegion* etc.
ANCHOR_API float GetCursorPosY();         //    other functions such as GetCursorScreenPos or everything
                                          //    in ImDrawList::
ANCHOR_API void SetCursorPos(
  const wabi::GfVec2f &local_pos);  //    are using the main, absolute coordinate system.
ANCHOR_API void SetCursorPosX(
  float local_x);  //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
ANCHOR_API void SetCursorPosY(float local_y);   //
ANCHOR_API wabi::GfVec2f GetCursorStartPos();   // initial cursor position in window coordinates
ANCHOR_API wabi::GfVec2f GetCursorScreenPos();  // cursor position in absolute coordinates (useful
                                                // to work with ImDrawList API). generally top-left
                                                // == GetMainViewport()->Pos == (0,0) in single
                                                // viewport mode, and bottom-right ==
                                                // GetMainViewport()->Pos+Size == io.DisplaySize in
                                                // single-viewport mode.
ANCHOR_API void SetCursorScreenPos(const wabi::GfVec2f &pos);  // cursor position in absolute coordinates
ANCHOR_API void AlignTextToFramePadding();                     // vertically align upcoming text baseline to
                                            // FramePadding[1] so that it will align properly to
                                            // regularly framed items (call if you have text on a
                                            // line before a framed item)
ANCHOR_API float GetTextLineHeight();             // ~ FontSize
ANCHOR_API float GetTextLineHeightWithSpacing();  // ~ FontSize + style.ItemSpacing[1] (distance in
                                                  // pixels between 2 consecutive lines of text)
ANCHOR_API float GetFrameHeight();                // ~ FontSize + style.FramePadding[1] * 2
ANCHOR_API float GetFrameHeightWithSpacing();     // ~ FontSize + style.FramePadding[1] * 2 +
                                                  // style.ItemSpacing[1] (distance in pixels between
                                                  // 2 consecutive lines of framed widgets)

// ID stack/scopes
// - Read the FAQ for more details about how ID are handled in ANCHOR. If you are creating widgets
// in a loop you most
//   likely want to push a unique identifier (e.g. object pointer, loop index) to uniquely
//   differentiate them.
// - The resulting ID are hashes of the entire stack.
// - You can also use the "Label##foobar" syntax within widget label to distinguish them from each
// others.
// - In this header file we use the "label"/"name" terminology to denote a string that will be
// displayed and used as an ID,
//   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
ANCHOR_API void PushID(const char *str_id);  // push string into the ID stack (will hash string).
ANCHOR_API void PushID(const char *str_id_begin,
                       const char *str_id_end);  // push string into the ID stack (will hash string).
ANCHOR_API void PushID(const void *ptr_id);      // push pointer into the ID stack (will hash pointer).
ANCHOR_API void PushID(int int_id);              // push integer into the ID stack (will hash integer).
ANCHOR_API void PopID();                         // pop from the ID stack.
ANCHOR_API ANCHOR_ID
GetID(const char *str_id);  // calculate unique ID (hash of whole ID stack + given parameter). e.g.
                            // if you want to query into ANCHORStorage yourself
ANCHOR_API ANCHOR_ID GetID(const char *str_id_begin, const char *str_id_end);
ANCHOR_API ANCHOR_ID GetID(const void *ptr_id);

// Widgets: Text
ANCHOR_API void TextUnformatted(
  const char *text,
  const char *text_end = NULL);  // raw text without formatting. Roughly equivalent to Text("%s", text) but:
                                 // A) doesn't require null terminated string if 'text_end' is specified, B)
                                 // it's faster, no memory copy is done, no buffer size limits, recommended
                                 // for long chunks of text.
ANCHOR_API void Text(const char *fmt, ...) ANCHOR_FMTARGS(1);  // formatted text
ANCHOR_API void TextV(const char *fmt, va_list args) ANCHOR_FMTLIST(1);
ANCHOR_API void TextColored(const wabi::GfVec4f &col, const char *fmt, ...)
  ANCHOR_FMTARGS(2);  // shortcut for PushStyleColor(ANCHOR_Col_Text, col); Text(fmt, ...); PopStyleColor();
ANCHOR_API void TextColoredV(const wabi::GfVec4f &col, const char *fmt, va_list args) ANCHOR_FMTLIST(2);
ANCHOR_API void TextDisabled(const char *fmt, ...)
  ANCHOR_FMTARGS(1);  // shortcut for PushStyleColor(ANCHOR_Col_Text,
                      // style.Colors[ANCHOR_Col_TextDisabled]); Text(fmt, ...); PopStyleColor();
ANCHOR_API void TextDisabledV(const char *fmt, va_list args) ANCHOR_FMTLIST(1);
ANCHOR_API void TextWrapped(const char *fmt, ...)
  ANCHOR_FMTARGS(1);  // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that
                      // this won't work on an auto-resizing window if there's no other widgets to extend the
                      // window width, yoy may need to set a size using SetNextWindowSize().
ANCHOR_API void TextWrappedV(const char *fmt, va_list args) ANCHOR_FMTLIST(1);
ANCHOR_API void LabelText(const char *label, const char *fmt, ...)
  ANCHOR_FMTARGS(2);  // display text+label aligned the same way as value+label widgets
ANCHOR_API void LabelTextV(const char *label, const char *fmt, va_list args) ANCHOR_FMTLIST(2);
ANCHOR_API void BulletText(const char *fmt, ...) ANCHOR_FMTARGS(1);  // shortcut for Bullet()+Text()
ANCHOR_API void BulletTextV(const char *fmt, va_list args) ANCHOR_FMTLIST(1);

// Widgets: Main
// - Most widgets return true when the value has been changed or when pressed/selected
// - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.)
// to query widget state.
ANCHOR_API bool Button(const char *label, const wabi::GfVec2f &size = wabi::GfVec2f(0, 0));  // button
ANCHOR_API bool SmallButton(
  const char *label);  // button with FramePadding=(0,0) to easily embed within text
ANCHOR_API bool InvisibleButton(
  const char *str_id,
  const wabi::GfVec2f &size,
  ANCHOR_ButtonFlags flags =
    0);  // flexible button behavior without the visuals, frequently useful to build custom
         // behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
ANCHOR_API bool ArrowButton(const char *str_id,
                            ANCHOR_Dir dir);  // square button with an arrow shape
ANCHOR_API void Image(AnchorTextureID user_texture_id,
                      const wabi::GfVec2f &size,
                      const wabi::GfVec2f &uv0 = wabi::GfVec2f(0, 0),
                      const wabi::GfVec2f &uv1 = wabi::GfVec2f(1, 1),
                      const wabi::GfVec4f &tint_col = wabi::GfVec4f(1, 1, 1, 1),
                      const wabi::GfVec4f &border_col = wabi::GfVec4f(0, 0, 0, 0));
ANCHOR_API bool ImageButton(
  AnchorTextureID user_texture_id,
  const wabi::GfVec2f &size,
  const wabi::GfVec2f &uv0 = wabi::GfVec2f(0, 0),
  const wabi::GfVec2f &uv1 = wabi::GfVec2f(1, 1),
  int frame_padding = -1,
  const wabi::GfVec4f &bg_col = wabi::GfVec4f(0, 0, 0, 0),
  const wabi::GfVec4f &tint_col = wabi::GfVec4f(1, 1, 1, 1));  // <0 frame_padding uses default frame
                                                               // padding settings. 0 for no padding
ANCHOR_API bool Checkbox(const char *label, bool *v);
ANCHOR_API bool CheckboxFlags(const char *label, int *flags, int flags_value);
ANCHOR_API bool CheckboxFlags(const char *label, unsigned int *flags, unsigned int flags_value);
ANCHOR_API bool RadioButton(
  const char *label,
  bool active);  // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
ANCHOR_API bool RadioButton(const char *label,
                            int *v,
                            int v_button);  // shortcut to handle the above pattern when value is an integer
ANCHOR_API void ProgressBar(float fraction,
                            const wabi::GfVec2f &size_arg = wabi::GfVec2f(-FLT_MIN, 0),
                            const char *overlay = NULL);
ANCHOR_API void Bullet();  // draw a small circle + keep the cursor on the same line. advance
                           // cursor x position by GetTreeNodeToLabelSpacing(), same distance that
                           // TreeNode() uses

// Widgets: Combo Box
// - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however
// you want it, by creating e.g. Selectable() items.
// - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for
// convenience purpose. This is analogous to how ListBox are created.
ANCHOR_API bool BeginCombo(const char *label, const char *preview_value, ANCHORComboFlags flags = 0);
ANCHOR_API void EndCombo();  // only call EndCombo() if BeginCombo() returns true!
ANCHOR_API bool Combo(const char *label,
                      int *current_item,
                      const char *const items[],
                      int items_count,
                      int popup_max_height_in_items = -1);
ANCHOR_API bool Combo(const char *label,
                      int *current_item,
                      const char *items_separated_by_zeros,
                      int popup_max_height_in_items = -1);  // Separate items with \0 within a string, end
                                                            // item-list with \0\0. e.g. "One\0Two\0Three\0"
ANCHOR_API bool Combo(const char *label,
                      int *current_item,
                      bool (*items_getter)(void *data, int idx, const char **out_text),
                      void *data,
                      int items_count,
                      int popup_max_height_in_items = -1);

// Widgets: Drag Sliders
// - CTRL+Click on any drag box to turn them into an input box. Manually input values aren't
// clamped and can go off-bounds.
// - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every functions, note that a
// 'float v[X]' function argument is the same as 'float* v', the array syntax is just a way to
// document the number of elements that are expected to be accessible. You can pass address of your
// first element out of a contiguous set, e.g. &myvector[0]
// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and
// display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit:
// 1; etc.
// - Format string may also be set to NULL or use the default format ("%f" or "%d").
// - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to
// increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed,
// minimum_step_at_given_precision).
// - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can
// override those limits.
// - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX /
// INT_MIN to avoid clamping to a minimum.
// - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the
// same and it makes it easier to swap them.
// - Legacy: Pre-1.78 there are DragXXX() function signatures that takes a final `float power=1.0f'
// argument instead of the `ANCHOR_SliderFlags flags=0' argument.
//   If you get a warning converting a float to ANCHOR_SliderFlags, read
//   https://github.com/ocornut/ANCHOR/issues/3361
ANCHOR_API bool DragFloat(const char *label,
                          float *v,
                          float v_speed = 1.0f,
                          float v_min = 0.0f,
                          float v_max = 0.0f,
                          const char *format = "%.3f",
                          ANCHOR_SliderFlags flags = 0);  // If v_min >= v_max we have no bound
ANCHOR_API bool DragFloat2(const char *label,
                           float v[2],
                           float v_speed = 1.0f,
                           float v_min = 0.0f,
                           float v_max = 0.0f,
                           const char *format = "%.3f",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragFloat3(const char *label,
                           float v[3],
                           float v_speed = 1.0f,
                           float v_min = 0.0f,
                           float v_max = 0.0f,
                           const char *format = "%.3f",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragFloat4(const char *label,
                           float v[4],
                           float v_speed = 1.0f,
                           float v_min = 0.0f,
                           float v_max = 0.0f,
                           const char *format = "%.3f",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragFloatRange2(const char *label,
                                float *v_current_min,
                                float *v_current_max,
                                float v_speed = 1.0f,
                                float v_min = 0.0f,
                                float v_max = 0.0f,
                                const char *format = "%.3f",
                                const char *format_max = NULL,
                                ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragInt(const char *label,
                        int *v,
                        float v_speed = 1.0f,
                        int v_min = 0,
                        int v_max = 0,
                        const char *format = "%d",
                        ANCHOR_SliderFlags flags = 0);  // If v_min >= v_max we have no bound
ANCHOR_API bool DragInt2(const char *label,
                         int v[2],
                         float v_speed = 1.0f,
                         int v_min = 0,
                         int v_max = 0,
                         const char *format = "%d",
                         ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragInt3(const char *label,
                         int v[3],
                         float v_speed = 1.0f,
                         int v_min = 0,
                         int v_max = 0,
                         const char *format = "%d",
                         ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragInt4(const char *label,
                         int v[4],
                         float v_speed = 1.0f,
                         int v_min = 0,
                         int v_max = 0,
                         const char *format = "%d",
                         ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragIntRange2(const char *label,
                              int *v_current_min,
                              int *v_current_max,
                              float v_speed = 1.0f,
                              int v_min = 0,
                              int v_max = 0,
                              const char *format = "%d",
                              const char *format_max = NULL,
                              ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragScalar(const char *label,
                           ANCHOR_DataType data_type,
                           void *p_data,
                           float v_speed = 1.0f,
                           const void *p_min = NULL,
                           const void *p_max = NULL,
                           const char *format = NULL,
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool DragScalarN(const char *label,
                            ANCHOR_DataType data_type,
                            void *p_data,
                            int components,
                            float v_speed = 1.0f,
                            const void *p_min = NULL,
                            const void *p_max = NULL,
                            const char *format = NULL,
                            ANCHOR_SliderFlags flags = 0);

// Widgets: Regular Sliders
// - CTRL+Click on any slider to turn them into an input box. Manually input values aren't clamped
// and can go off-bounds.
// - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and
// display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit:
// 1; etc.
// - Format string may also be set to NULL or use the default format ("%f" or "%d").
// - Legacy: Pre-1.78 there are SliderXXX() function signatures that takes a final `float
// power=1.0f' argument instead of the `ANCHOR_SliderFlags flags=0' argument.
//   If you get a warning converting a float to ANCHOR_SliderFlags, read
//   https://github.com/ocornut/ANCHOR/issues/3361
ANCHOR_API bool SliderFloat(
  const char *label,
  float *v,
  float v_min,
  float v_max,
  const char *format = "%.3f",
  ANCHOR_SliderFlags flags = 0);  // adjust format to decorate the value with a prefix or a
                                  // suffix for in-slider labels or unit display.
ANCHOR_API bool SliderFloat2(const char *label,
                             float v[2],
                             float v_min,
                             float v_max,
                             const char *format = "%.3f",
                             ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderFloat3(const char *label,
                             float v[3],
                             float v_min,
                             float v_max,
                             const char *format = "%.3f",
                             ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderFloat4(const char *label,
                             float v[4],
                             float v_min,
                             float v_max,
                             const char *format = "%.3f",
                             ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderAngle(const char *label,
                            float *v_rad,
                            float v_degrees_min = -360.0f,
                            float v_degrees_max = +360.0f,
                            const char *format = "%.0f deg",
                            ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderInt(const char *label,
                          int *v,
                          int v_min,
                          int v_max,
                          const char *format = "%d",
                          ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderInt2(const char *label,
                           int v[2],
                           int v_min,
                           int v_max,
                           const char *format = "%d",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderInt3(const char *label,
                           int v[3],
                           int v_min,
                           int v_max,
                           const char *format = "%d",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderInt4(const char *label,
                           int v[4],
                           int v_min,
                           int v_max,
                           const char *format = "%d",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderScalar(const char *label,
                             ANCHOR_DataType data_type,
                             void *p_data,
                             const void *p_min,
                             const void *p_max,
                             const char *format = NULL,
                             ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool SliderScalarN(const char *label,
                              ANCHOR_DataType data_type,
                              void *p_data,
                              int components,
                              const void *p_min,
                              const void *p_max,
                              const char *format = NULL,
                              ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool VSliderFloat(const char *label,
                             const wabi::GfVec2f &size,
                             float *v,
                             float v_min,
                             float v_max,
                             const char *format = "%.3f",
                             ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool VSliderInt(const char *label,
                           const wabi::GfVec2f &size,
                           int *v,
                           int v_min,
                           int v_max,
                           const char *format = "%d",
                           ANCHOR_SliderFlags flags = 0);
ANCHOR_API bool VSliderScalar(const char *label,
                              const wabi::GfVec2f &size,
                              ANCHOR_DataType data_type,
                              void *p_data,
                              const void *p_min,
                              const void *p_max,
                              const char *format = NULL,
                              ANCHOR_SliderFlags flags = 0);

// Widgets: Input with Keyboard
// - If you want to use InputText() with std::string or any custom dynamic string type, see
// misc/cpp/ANCHOR_stdlib.h and comments in ANCHOR_demo.cpp.
// - Most of the ANCHORInputTextFlags flags are only useful for InputText() and not for
// InputFloatX, InputIntX, InputDouble etc.
ANCHOR_API bool InputText(const char *label,
                          char *buf,
                          size_t buf_size,
                          ANCHORInputTextFlags flags = 0,
                          ANCHORInputTextCallback callback = NULL,
                          void *user_data = NULL);
ANCHOR_API bool InputTextMultiline(const char *label,
                                   char *buf,
                                   size_t buf_size,
                                   const wabi::GfVec2f &size = wabi::GfVec2f(0, 0),
                                   ANCHORInputTextFlags flags = 0,
                                   ANCHORInputTextCallback callback = NULL,
                                   void *user_data = NULL);
ANCHOR_API bool InputTextWithHint(const char *label,
                                  const char *hint,
                                  char *buf,
                                  size_t buf_size,
                                  ANCHORInputTextFlags flags = 0,
                                  ANCHORInputTextCallback callback = NULL,
                                  void *user_data = NULL);
ANCHOR_API bool InputFloat(const char *label,
                           float *v,
                           float step = 0.0f,
                           float step_fast = 0.0f,
                           const char *format = "%.3f",
                           ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputFloat2(const char *label,
                            float v[2],
                            const char *format = "%.3f",
                            ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputFloat3(const char *label,
                            float v[3],
                            const char *format = "%.3f",
                            ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputFloat4(const char *label,
                            float v[4],
                            const char *format = "%.3f",
                            ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputInt(const char *label,
                         int *v,
                         int step = 1,
                         int step_fast = 100,
                         ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputInt2(const char *label, int v[2], ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputInt3(const char *label, int v[3], ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputInt4(const char *label, int v[4], ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputDouble(const char *label,
                            double *v,
                            double step = 0.0,
                            double step_fast = 0.0,
                            const char *format = "%.6f",
                            ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputScalar(const char *label,
                            ANCHOR_DataType data_type,
                            void *p_data,
                            const void *p_step = NULL,
                            const void *p_step_fast = NULL,
                            const char *format = NULL,
                            ANCHORInputTextFlags flags = 0);
ANCHOR_API bool InputScalarN(const char *label,
                             ANCHOR_DataType data_type,
                             void *p_data,
                             int components,
                             const void *p_step = NULL,
                             const void *p_step_fast = NULL,
                             const char *format = NULL,
                             ANCHORInputTextFlags flags = 0);

// Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can
// be left-clicked to open a picker, and right-clicked to open an option menu.)
// - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array
// syntax is just a way to document the number of elements that are expected to be accessible.
// - You can pass the address of a first float element out of a contiguous structure, e.g.
// &myvector[0]
ANCHOR_API bool ColorEdit3(const char *label, float col[3], ANCHOR_ColorEditFlags flags = 0);
ANCHOR_API bool ColorEdit4(const char *label, float col[4], ANCHOR_ColorEditFlags flags = 0);
ANCHOR_API bool ColorPicker3(const char *label, float col[3], ANCHOR_ColorEditFlags flags = 0);
ANCHOR_API bool ColorPicker4(const char *label,
                             float col[4],
                             ANCHOR_ColorEditFlags flags = 0,
                             const float *ref_col = NULL);
ANCHOR_API bool ColorButton(
  const char *desc_id,
  const wabi::GfVec4f &col,
  ANCHOR_ColorEditFlags flags = 0,
  wabi::GfVec2f size =
    wabi::GfVec2f(0,
                  0));  // display a color square/button, hover for details, return true when pressed.
ANCHOR_API void SetColorEditOptions(
  ANCHOR_ColorEditFlags flags);  // initialize current options (generally on application startup) if you want
                                 // to select a default format, picker type, etc. User will be able to change
                                 // many settings, unless you pass the _NoOptions flag to your calls.

// Widgets: Trees
// - TreeNode functions return true when the node is open, in which case you need to also call
// TreePop() when you are finished displaying the tree node contents.
ANCHOR_API bool TreeNode(const char *label);
ANCHOR_API bool TreeNode(const char *str_id, const char *fmt, ...)
  ANCHOR_FMTARGS(2);  // helper variation to easily decorelate the id from the displayed string.
                      // Read the FAQ about why and how to use ID. to align arbitrary text at the
                      // same level as a TreeNode() you can use Bullet().
ANCHOR_API bool TreeNode(const void *ptr_id, const char *fmt, ...) ANCHOR_FMTARGS(2);  // "
ANCHOR_API bool TreeNodeV(const char *str_id, const char *fmt, va_list args) ANCHOR_FMTLIST(2);
ANCHOR_API bool TreeNodeV(const void *ptr_id, const char *fmt, va_list args) ANCHOR_FMTLIST(2);
ANCHOR_API bool TreeNodeEx(const char *label, ANCHOR_TreeNodeFlags flags = 0);
ANCHOR_API bool TreeNodeEx(const char *str_id, ANCHOR_TreeNodeFlags flags, const char *fmt, ...)
  ANCHOR_FMTARGS(3);
ANCHOR_API bool TreeNodeEx(const void *ptr_id, ANCHOR_TreeNodeFlags flags, const char *fmt, ...)
  ANCHOR_FMTARGS(3);
ANCHOR_API bool TreeNodeExV(const char *str_id, ANCHOR_TreeNodeFlags flags, const char *fmt, va_list args)
  ANCHOR_FMTLIST(3);
ANCHOR_API bool TreeNodeExV(const void *ptr_id, ANCHOR_TreeNodeFlags flags, const char *fmt, va_list args)
  ANCHOR_FMTLIST(3);
ANCHOR_API void TreePush(
  const char *str_id);  // ~ Indent()+PushId(). Already called by TreeNode() when returning true,
                        // but you can call TreePush/TreePop yourself if desired.
ANCHOR_API void TreePush(const void *ptr_id = NULL);  // "
ANCHOR_API void TreePop();                            // ~ Unindent()+PopId()
ANCHOR_API float GetTreeNodeToLabelSpacing();         // horizontal distance preceding label when using
                                                      // TreeNode*() or Bullet() == (g.FontSize +
                                                      // style.FramePadding[0]*2) for a regular unframed
                                                      // TreeNode
ANCHOR_API bool CollapsingHeader(
  const char *label,
  ANCHOR_TreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor
                                    // push on ID stack. user doesn't have to call TreePop().
ANCHOR_API bool CollapsingHeader(
  const char *label,
  bool *p_visible,
  ANCHOR_TreeNodeFlags flags =
    0);  // when 'p_visible != NULL': if '*p_visible==true' display an additional small close
         // button on upper right of the header which will set the bool to false when clicked,
         // if '*p_visible==false' don't display the header.
ANCHOR_API void SetNextItemOpen(bool is_open,
                                ANCHOR_Cond cond = 0);  // set next TreeNode/CollapsingHeader open state.

// Widgets: Selectables
// - A selectable highlights when hovered, and can display another color when selected.
// - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This
// is so a series of selected Selectable appear contiguous.
ANCHOR_API bool Selectable(const char *label,
                           bool selected = false,
                           ANCHORSelectableFlags flags = 0,
                           const wabi::GfVec2f &size = wabi::GfVec2f(
                             0,
                             0));  // "bool selected" carry the selection state (read-only). Selectable()
                                   // is clicked is returns true so you can modify your selection state.
                                   // size[0]==0.0: use remaining width, size[0]>0.0: specify width.
                                   // size[1]==0.0: use label height, size[1]>0.0: specify height
ANCHOR_API bool Selectable(
  const char *label,
  bool *p_selected,
  ANCHORSelectableFlags flags = 0,
  const wabi::GfVec2f &size = wabi::GfVec2f(0,
                                            0));  // "bool* p_selected" point to the selection
                                                  // state (read-write), as a convenient helper.

// Widgets: List Boxes
// - This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.
// - The BeginListBox()/EndListBox() api allows you to manage your contents and selection state
// however you want it, by creating e.g. Selectable() or any items.
// - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept
// available for convenience purpose. This is analoguous to how Combos are created.
// - Choose frame width:   size[0] > 0.0f: custom  /  size[0] < 0.0f or -FLT_MIN: right-align   /
// size[0] = 0.0f (default): use current ItemWidth
// - Choose frame height:  size[1] > 0.0f: custom  /  size[1] < 0.0f or -FLT_MIN: bottom-align  /
// size[1] = 0.0f (default): arbitrary default height which can fit ~7 items
ANCHOR_API bool BeginListBox(
  const char *label,
  const wabi::GfVec2f &size = wabi::GfVec2f(0, 0));  // open a framed scrolling region
ANCHOR_API void EndListBox();  // only call EndListBox() if BeginListBox() returned true!
ANCHOR_API bool ListBox(const char *label,
                        int *current_item,
                        const char *const items[],
                        int items_count,
                        int height_in_items = -1);
ANCHOR_API bool ListBox(const char *label,
                        int *current_item,
                        bool (*items_getter)(void *data, int idx, const char **out_text),
                        void *data,
                        int items_count,
                        int height_in_items = -1);

// Widgets: Data Plotting
// - Consider using ImPlot (https://github.com/epezent/implot)
ANCHOR_API void PlotLines(const char *label,
                          const float *values,
                          int values_count,
                          int values_offset = 0,
                          const char *overlay_text = NULL,
                          float scale_min = FLT_MAX,
                          float scale_max = FLT_MAX,
                          wabi::GfVec2f graph_size = wabi::GfVec2f(0, 0),
                          int stride = sizeof(float));
ANCHOR_API void PlotLines(const char *label,
                          float (*values_getter)(void *data, int idx),
                          void *data,
                          int values_count,
                          int values_offset = 0,
                          const char *overlay_text = NULL,
                          float scale_min = FLT_MAX,
                          float scale_max = FLT_MAX,
                          wabi::GfVec2f graph_size = wabi::GfVec2f(0, 0));
ANCHOR_API void PlotHistogram(const char *label,
                              const float *values,
                              int values_count,
                              int values_offset = 0,
                              const char *overlay_text = NULL,
                              float scale_min = FLT_MAX,
                              float scale_max = FLT_MAX,
                              wabi::GfVec2f graph_size = wabi::GfVec2f(0, 0),
                              int stride = sizeof(float));
ANCHOR_API void PlotHistogram(const char *label,
                              float (*values_getter)(void *data, int idx),
                              void *data,
                              int values_count,
                              int values_offset = 0,
                              const char *overlay_text = NULL,
                              float scale_min = FLT_MAX,
                              float scale_max = FLT_MAX,
                              wabi::GfVec2f graph_size = wabi::GfVec2f(0, 0));

// Widgets: Value() Helpers.
// - Those are merely shortcut to calling Text() with a format string. Output single value in
// "name: value" format (tip: freely declare more in your code to handle your types. you can add
// functions to the ANCHOR namespace)
ANCHOR_API void Value(const char *prefix, bool b);
ANCHOR_API void Value(const char *prefix, int v);
ANCHOR_API void Value(const char *prefix, unsigned int v);
ANCHOR_API void Value(const char *prefix, float v, const char *float_format = NULL);

// Widgets: Menus
// - Use BeginMenuBar() on a window ANCHOR_WindowFlags_MenuBar to append to its menu bar.
// - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
// - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same
// identifier to append more items to it.
// - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by
// ANCHOR at the moment.
ANCHOR_API bool BeginMenuBar();      // append to menu-bar of current window (requires
                                     // ANCHOR_WindowFlags_MenuBar flag set on parent window).
ANCHOR_API void EndMenuBar();        // only call EndMenuBar() if BeginMenuBar() returns true!
ANCHOR_API bool BeginMainMenuBar();  // create and append to a full screen menu-bar.
ANCHOR_API void EndMainMenuBar();    // only call EndMainMenuBar() if BeginMainMenuBar() returns
                                     // true!
ANCHOR_API bool BeginMenu(
  const char *label,
  bool enabled = true);     // create a sub-menu entry. only call EndMenu() if this returns true!
ANCHOR_API void EndMenu();  // only call EndMenu() if BeginMenu() returns true!
ANCHOR_API bool MenuItem(const char *label,
                         const char *shortcut = NULL,
                         bool selected = false,
                         bool enabled = true);  // return true when activated.
ANCHOR_API bool MenuItem(const char *label,
                         const char *shortcut,
                         bool *p_selected,
                         bool enabled = true);  // return true when activated + toggle
                                                // (*p_selected) if p_selected != NULL

// Tooltips
// - Tooltip are windows following the mouse. They do not take focus away.
ANCHOR_API void BeginTooltip();  // begin/append a tooltip window. to create full-featured tooltip
                                 // (with any kind of items).
ANCHOR_API void EndTooltip();
ANCHOR_API void SetTooltip(const char *fmt, ...)
  ANCHOR_FMTARGS(1);  // set a text-only tooltip, typically use with ANCHOR::IsItemHovered().
                      // override any previous call to SetTooltip().
ANCHOR_API void SetTooltipV(const char *fmt, va_list args) ANCHOR_FMTLIST(1);

// Popups, Modals
//  - They block normal mouse hovering detection (and therefore most mouse interactions) behind
//  them.
//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
//  - Their visibility state (~bool) is held internally instead of being held by the programmer as
//  we are used to with regular Begin*() calls.
//  - The 3 properties above are related: we need to retain popup visibility state in the library
//  because popups may be closed as any time.
//  - You can bypass the hovering restriction by using ANCHORHoveredFlags_AllowWhenBlockedByPopup
//  when calling IsItemHovered() or IsWindowHovered().
//  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and
//  BeginPopup generally needs to be at the same level of the stack.
//    This is sometimes leading to confusing mistakes. May rework this in the future.
// Popups: begin/end functions
//  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup()
//  afterwards. ANCHOR_WindowFlags are forwarded to the window.
//  - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add
//  a dimming background, has a title bar.
ANCHOR_API bool BeginPopup(const char *str_id,
                           ANCHOR_WindowFlags flags = 0);  // return true if the popup is open, and
                                                           // you can start outputting to it.
ANCHOR_API bool BeginPopupModal(
  const char *name,
  bool *p_open = NULL,
  ANCHOR_WindowFlags flags = 0);  // return true if the modal is open, and you can start outputting to it.
ANCHOR_API void EndPopup();       // only call EndPopup() if BeginPopupXXX() returns true!
// Popups: open/close functions
//  - OpenPopup(): set popup state to open. ANCHORPopupFlags are available for opening options.
//  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
//  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
//  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME:
//  need some options).
//  - Use ANCHORPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one
//  at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to
//  OpenPopup().
//  - Use IsWindowAppearing() after BeginPopup() to tell if a window just opened.
ANCHOR_API void OpenPopup(
  const char *str_id,
  ANCHORPopupFlags popup_flags = 0);  // call to mark popup as open (don't call every frame!).
ANCHOR_API void OpenPopup(
  ANCHOR_ID id,
  ANCHORPopupFlags popup_flags = 0);  // id overload to facilitate calling from nested stacks
ANCHOR_API void OpenPopupOnItemClick(
  const char *str_id = NULL,
  ANCHORPopupFlags popup_flags = 1);  // helper to open popup when clicked on last item. Default to
                                      // ANCHORPopupFlags_MouseButtonRight == 1. (note: actually triggers on
                                      // the mouse _released_ event to be consistent with popup behaviors)
ANCHOR_API void CloseCurrentPopup();  // manually close the popup we have begin-ed into.
// Popups: open+begin combined functions helpers
//  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an
//  item and right-clicking.
//  - They are convenient to easily create context menus, hence the name.
//  - IMPORTANT: Notice that BeginPopupContextXXX takes ANCHORPopupFlags just like OpenPopup() and
//  unlike BeginPopup(). For full consistency, we may add ANCHOR_WindowFlags to the
//  BeginPopupContextXXX functions in the future.
//  - IMPORTANT: we exceptionally default their flags to 1 (== ANCHORPopupFlags_MouseButtonRight)
//  for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you
//  add other flags remember to re-add the ANCHORPopupFlags_MouseButtonRight.
ANCHOR_API bool BeginPopupContextItem(
  const char *str_id = NULL,
  ANCHORPopupFlags popup_flags =
    1);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup
         // to previous item. If you want to use that on a non-interactive item such as Text()
         // you need to pass in an explicit ID here. read comments in .cpp!
ANCHOR_API bool BeginPopupContextWindow(
  const char *str_id = NULL,
  ANCHORPopupFlags popup_flags = 1);  // open+begin popup when clicked on current window.
ANCHOR_API bool BeginPopupContextVoid(
  const char *str_id = NULL,
  ANCHORPopupFlags popup_flags = 1);  // open+begin popup when clicked in void (where there are no windows).
// Popups: query functions
//  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the
//  popup stack.
//  - IsPopupOpen() with ANCHORPopupFlags_AnyPopupId: return true if any popup is open at the
//  current BeginPopup() level of the popup stack.
//  - IsPopupOpen() with ANCHORPopupFlags_AnyPopupId + ANCHORPopupFlags_AnyPopupLevel: return true
//  if any popup is open.
ANCHOR_API bool IsPopupOpen(const char *str_id,
                            ANCHORPopupFlags flags = 0);  // return true if the popup is open.

// Tables
// [BETA API] API may evolve slightly! If you use this, please update to the next version when it
// comes out!
// - Full-featured replacement for old Columns API.
// - See Demo->Tables for demo code.
// - See top of ANCHOR_tables.cpp for general commentary.
// - See ANCHOR_TableFlags_ and ANCHOR_TableColumnFlags_ enums for a description of available
// flags. The typical call flow is:
// - 1. Call BeginTable().
// - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
// - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
// - 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from
// TableSetupColumn() data.
// - 5. Populate contents:
//    - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending
//    into a column.
//    - If you are using tables as a sort of grid, where every columns is holding the same type of
//    contents,
//      you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
//      TableNextColumn() will automatically wrap-around into the next row if needed.
//    - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for
//    the first column!
//    - Summary of possible call flow:
//        --------------------------------------------------------------------------------------------------------
//        TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) ->
//        Text("Hello 1")  // OK TableNextRow() -> TableNextColumn()      -> Text("Hello 0") ->
//        TableNextColumn()      -> Text("Hello 1")  // OK
//                          TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      ->
//                          Text("Hello 1")  // OK: TableNextColumn() automatically gets to next
//                          row!
//        TableNextRow()                           -> Text("Hello 0") // Not OK! Missing
//        TableSetColumnIndex() or TableNextColumn()! Text will not appear!
//        --------------------------------------------------------------------------------------------------------
// - 5. Call EndTable()
ANCHOR_API bool BeginTable(const char *str_id,
                           int column,
                           ANCHOR_TableFlags flags = 0,
                           const wabi::GfVec2f &outer_size = wabi::GfVec2f(0.0f, 0.0f),
                           float inner_width = 0.0f);
ANCHOR_API void EndTable();  // only call EndTable() if BeginTable() returns true!
ANCHOR_API void TableNextRow(ANCHOR_TableRowFlags row_flags = 0,
                             float min_row_height = 0.0f);  // append into the first cell of a new row.
ANCHOR_API bool TableNextColumn();  // append into the next column (or first column of next row if
                                    // currently in last column). Return true when column is
                                    // visible.
ANCHOR_API bool TableSetColumnIndex(
  int column_n);  // append into the specified column. Return true when column is visible.
// Tables: Headers & Columns declaration
// - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various
// other flags etc.
// - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each
// column.
//   Headers are required to perform: reordering, sorting, and opening the context menu.
//   The context menu can also be made available in columns body using
//   ANCHOR_TableFlags_ContextMenuInBody.
// - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only
// useful in
//   some advanced use cases (e.g. adding custom widgets in header row).
// - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
ANCHOR_API void TableSetupColumn(const char *label,
                                 ANCHOR_TableColumnFlags flags = 0,
                                 float init_width_or_weight = 0.0f,
                                 ANCHOR_ID user_id = 0);
ANCHOR_API void TableSetupScrollFreeze(int cols,
                                       int rows);  // lock columns/rows so they stay visible when scrolled.
ANCHOR_API void TableHeadersRow();                 // submit all headers cells based on data provided to
                                                   // TableSetupColumn() + submit context menu
ANCHOR_API void TableHeader(const char *label);    // submit one header cell manually (rarely used)
// Tables: Sorting
// - Call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
// - When 'SpecsDirty == true' you should sort your data. It will be true when sorting specs have
// changed
//   since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting, else
//   you may wastefully sort your data every frame!
// - Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to
// BeginTable().
ANCHOR_API ANCHOR_TableSortSpecs *TableGetSortSpecs();  // get latest sort specs for the table
                                                        // (NULL if not sorting).
// Tables: Miscellaneous functions
// - Functions args 'int column_n' treat the default value of -1 as the same as passing the current
// column index.
ANCHOR_API int TableGetColumnCount();  // return number of columns (value passed to BeginTable)
ANCHOR_API int TableGetColumnIndex();  // return current column index.
ANCHOR_API int TableGetRowIndex();     // return current row index.
ANCHOR_API const char *TableGetColumnName(
  int column_n = -1);  // return "" if column didn't have a name declared by TableSetupColumn().
                       // Pass -1 to use current column.
ANCHOR_API ANCHOR_TableColumnFlags TableGetColumnFlags(
  int column_n = -1);  // return column flags so you can query their Enabled/Visible/Sorted/Hovered
                       // status flags. Pass -1 to use current column.
ANCHOR_API void TableSetColumnEnabled(
  int column_n,
  bool v);  // change enabled/disabled state of a column, set to false to hide the column. Note
            // that end-user can use the context menu to change this themselves (right-click in
            // headers, or right-click in columns body with ANCHOR_TableFlags_ContextMenuInBody)
ANCHOR_API void TableSetBgColor(ANCHOR_TableBgTarget target,
                                AnchorU32 color,
                                int column_n = -1);  // change the color of a cell, row, or column. See
                                                     // ANCHOR_TableBgTarget_ flags for details.

// Legacy Columns API (2020: prefer using Tables!)
// - You can also use SameLine(pos_x) to mimic simplified columns.
ANCHOR_API void Columns(int count = 1, const char *id = NULL, bool border = true);
ANCHOR_API void NextColumn();     // next column, defaults to current row or next row if the current
                                  // row is finished
ANCHOR_API int GetColumnIndex();  // get current column index
ANCHOR_API float GetColumnWidth(
  int column_index = -1);  // get column width (in pixels). pass -1 to use current column
ANCHOR_API void SetColumnWidth(int column_index,
                               float width);  // set column width (in pixels). pass -1 to use current column
ANCHOR_API float GetColumnOffset(
  int column_index = -1);  // get position of column line (in pixels, from the left side of the
                           // contents region). pass -1 to use current column, otherwise
                           // 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
ANCHOR_API void SetColumnOffset(
  int column_index,
  float offset_x);  // set position of column line (in pixels, from the left side of the contents
                    // region). pass -1 to use current column
ANCHOR_API int GetColumnsCount();

// Tab Bars, Tabs
ANCHOR_API bool BeginTabBar(const char *str_id,
                            ANCHOR_TabBarFlags flags = 0);  // create and append into a TabBar
ANCHOR_API void EndTabBar();  // only call EndTabBar() if BeginTabBar() returns true!
ANCHOR_API bool BeginTabItem(
  const char *label,
  bool *p_open = NULL,
  ANCHOR_TabItemFlags flags = 0);  // create a Tab. Returns true if the Tab is selected.
ANCHOR_API void EndTabItem();      // only call EndTabItem() if BeginTabItem() returns true!
ANCHOR_API bool TabItemButton(
  const char *label,
  ANCHOR_TabItemFlags flags = 0);  // create a Tab behaving like a button. return true when
                                   // clicked. cannot be selected in the tab bar.
ANCHOR_API void SetTabItemClosed(
  const char *tab_or_docked_window_label);  // notify TabBar or Docking system of a closed tab/window
                                            // ahead (useful to reduce visual flicker on reorderable tab
                                            // bars). For tab-bar: call after BeginTabBar() and before
                                            // Tab submissions. Otherwise call with a window name.

// Logging/Capture
// - All text output from the interface can be captured into tty/file/clipboard. By default, tree
// nodes are automatically opened during logging.
ANCHOR_API void LogToTTY(int auto_open_depth = -1);  // start logging to tty (stdout)
ANCHOR_API void LogToFile(int auto_open_depth = -1,
                          const char *filename = NULL);    // start logging to file
ANCHOR_API void LogToClipboard(int auto_open_depth = -1);  // start logging to OS clipboard
ANCHOR_API void LogFinish();                               // stop logging (close file, etc.)
ANCHOR_API void LogButtons();  // helper to display buttons for logging to tty/file/clipboard
ANCHOR_API void LogText(const char *fmt, ...)
  ANCHOR_FMTARGS(1);  // pass text data straight to log (without being displayed)
ANCHOR_API void LogTextV(const char *fmt, va_list args) ANCHOR_FMTLIST(1);

// Drag and Drop
// - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload()
// + EndDragDropSource().
// - On target candidates, call BeginDragDropTarget(), if it returns true also call
// AcceptDragDropPayload() + EndDragDropTarget().
// - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a
// preview tooltip (we currently display a fallback "..." tooltip, see #1725)
// - An item can be both drag source and drop target.
ANCHOR_API bool BeginDragDropSource(
  ANCHORDragDropFlags flags = 0);  // call after submitting an item which may be dragged. when this return
                                   // true, you can call SetDragDropPayload() + EndDragDropSource()
ANCHOR_API bool SetDragDropPayload(
  const char *type,
  const void *data,
  size_t sz,
  ANCHOR_Cond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with
                          // '_' are reserved for ANCHOR internal types. Data is copied and held by ANCHOR.
ANCHOR_API void EndDragDropSource();    // only call EndDragDropSource() if BeginDragDropSource()
                                        // returns true!
ANCHOR_API bool BeginDragDropTarget();  // call after submitting an item that may receive a
                                        // payload. If this returns true, you can call
                                        // AcceptDragDropPayload() + EndDragDropTarget()
ANCHOR_API const ANCHORPayload *AcceptDragDropPayload(
  const char *type,
  ANCHORDragDropFlags flags =
    0);  // accept contents of a given type. If ANCHORDragDropFlags_AcceptBeforeDelivery is set
         // you can peek into the payload before the mouse button is released.
ANCHOR_API void EndDragDropTarget();  // only call EndDragDropTarget() if BeginDragDropTarget()
                                      // returns true!
ANCHOR_API const ANCHORPayload *GetDragDropPayload();  // peek directly into the current payload
                                                       // from anywhere. may return NULL. use
                                                       // ANCHORPayload::IsDataType() to test for
                                                       // the payload type.

// Clipping
// - Mouse hovering is affected by ANCHOR::PushClipRect() calls, unlike direct calls to
// ImDrawList::PushClipRect() which are render only.
ANCHOR_API void PushClipRect(const wabi::GfVec2f &clip_rect_min,
                             const wabi::GfVec2f &clip_rect_max,
                             bool intersect_with_current_clip_rect);
ANCHOR_API void PopClipRect();

// Focus, Activation
// - Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when
// applicable to signify "this is the default item"
ANCHOR_API void SetItemDefaultFocus();  // make last item the default focused item of a window.
ANCHOR_API void SetKeyboardFocusHere(
  int offset = 0);  // focus keyboard on the next widget. Use positive 'offset' to access sub
                    // components of a multiple component widget. Use -1 to access previous widget.

// Item/Widgets Utilities and Query Functions
// - Most of the functions are referring to the previous Item that has been submitted.
// - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of
// those functions.
ANCHOR_API bool IsItemHovered(
  ANCHORHoveredFlags flags = 0);  // is the last item hovered? (and usable, aka not blocked by a
                                  // popup, etc.). See ANCHORHoveredFlags for more options.
ANCHOR_API bool IsItemActive();   // is the last item active? (e.g. button being held, text field
                                  // being edited. This will continuously return true while holding
                                  // mouse button on an item. Items that don't interact will always
                                  // return false)
ANCHOR_API bool IsItemFocused();  // is the last item focused for keyboard/gamepad navigation?
ANCHOR_API bool IsItemClicked(
  ANCHOR_MouseButton mouse_button =
    0);  // is the last item hovered and mouse clicked on? (**)  ==
         // IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this it NOT
         // equivalent to the behavior of e.g. Button(). Read comments in function definition.
ANCHOR_API bool IsItemVisible();      // is the last item visible? (items may be out of sight because
                                      // of clipping/scrolling)
ANCHOR_API bool IsItemEdited();       // did the last item modify its underlying value this frame? or
                                      // was pressed? This is generally the same as the "bool" return
                                      // value of many widgets.
ANCHOR_API bool IsItemActivated();    // was the last item just made active (item was previously
                                      // inactive).
ANCHOR_API bool IsItemDeactivated();  // was the last item just made inactive (item was previously
                                      // active). Useful for Undo/Redo patterns with widgets that
                                      // requires continuous editing.
ANCHOR_API bool IsItemDeactivatedAfterEdit();  // was the last item just made inactive and made a
                                               // value change when it was active? (e.g.
                                               // Slider/Drag moved). Useful for Undo/Redo patterns
                                               // with widgets that requires continuous editing.
                                               // Note that you may get false positives (some
                                               // widgets such as Combo()/ListBox()/Selectable()
                                               // will return true even when clicking an already
                                               // selected item).
ANCHOR_API bool IsItemToggledOpen();           // was the last item open state toggled? set by TreeNode().
ANCHOR_API bool IsAnyItemHovered();            // is any item hovered?
ANCHOR_API bool IsAnyItemActive();             // is any item active?
ANCHOR_API bool IsAnyItemFocused();            // is any item focused?
ANCHOR_API wabi::GfVec2f GetItemRectMin();     // get upper-left bounding rectangle of the last item
                                               // (screen space)
ANCHOR_API wabi::GfVec2f GetItemRectMax();     // get lower-right bounding rectangle of the last item
                                               // (screen space)
ANCHOR_API wabi::GfVec2f GetItemRectSize();    // get size of last item
ANCHOR_API void SetItemAllowOverlap();         // allow last item to be overlapped by a subsequent item.
                                               // sometimes useful with invisible buttons, selectables,
                                               // etc. to catch unused area.

// Viewports
// - Currently represents the Platform Window created by the application which is hosting our
// ANCHOR windows.
// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple
// active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and
// support a "no main platform window" operation mode.
ANCHOR_API ANCHORViewport *GetMainViewport();  // return primary/default viewport. This can never
                                               // be NULL.

// Miscellaneous Utilities
ANCHOR_API bool IsRectVisible(const wabi::GfVec2f &size);  // test if rectangle (of given size, starting from
                                                           // cursor position) is visible / not clipped.
ANCHOR_API bool IsRectVisible(
  const wabi::GfVec2f &rect_min,
  const wabi::GfVec2f &rect_max);  // test if rectangle (in screen space) is visible / not
                                   // clipped. to perform coarse clipping on user's side.
ANCHOR_API double GetTime();       // get global ANCHOR time. incremented by io.DeltaTime every frame.
ANCHOR_API int GetFrameCount();    // get global ANCHOR frame count. incremented by 1 every frame.
ANCHOR_API ImDrawList *GetBackgroundDrawList();            // this draw list will be the first rendering one.
                                                           // Useful to quickly draw shapes/text behind
                                                           // ANCHOR contents.
ANCHOR_API ImDrawList *GetForegroundDrawList();            // this draw list will be the last rendered one.
                                                           // Useful to quickly draw shapes/text over ANCHOR
                                                           // contents.
ANCHOR_API ImDrawListSharedData *GetDrawListSharedData();  // you may use this when creating your
                                                           // own ImDrawList instances.
ANCHOR_API const char *GetStyleColorName(
  ANCHOR_Col idx);  // get a string corresponding to the enum value (for display, saving, etc.).
ANCHOR_API void SetStateStorage(
  ANCHORStorage *storage);  // replace current window storage with our own (if you want to
                            // manipulate it yourself, typically clear subsection of it)
ANCHOR_API ANCHORStorage *GetStateStorage();
ANCHOR_API void CalcListClipping(
  int items_count,
  float items_height,
  int *out_items_display_start,
  int *out_items_display_end);  // calculate coarse clipping for large list of evenly sized
                                // items. Prefer using the ANCHORListClipper higher-level helper
                                // if you can.
ANCHOR_API bool BeginChildFrame(
  ANCHOR_ID id,
  const wabi::GfVec2f &size,
  ANCHOR_WindowFlags flags = 0);  // helper to create a child window / scrolling region that
                                  // looks like a normal widget frame
ANCHOR_API void EndChildFrame();  // always call EndChildFrame() regardless of BeginChildFrame()
                                  // return values (which indicates a collapsed/clipped window)

// Text Utilities
ANCHOR_API wabi::GfVec2f CalcTextSize(const char *text,
                                      const char *text_end = NULL,
                                      bool hide_text_after_double_hash = false,
                                      float wrap_width = -1.0f);

// Color Utilities
ANCHOR_API wabi::GfVec4f ColorConvertU32ToFloat4(AnchorU32 in);
ANCHOR_API AnchorU32 ColorConvertFloat4ToU32(const wabi::GfVec4f &in);
ANCHOR_API void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v);
ANCHOR_API void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b);

// Inputs Utilities: Keyboard
// - For 'int user_key_index' you can use your own indices/enums according to how your
// backend/engine stored them in io.KeysDown[].
// - We don't know the meaning of those value. You can use GetKeyIndex() to map a ANCHOR_Key_ value
// into the user index.
ANCHOR_API int GetKeyIndex(
  ANCHOR_Key ANCHOR_key);  // map ANCHOR_Key_* values into user's key index. == io.KeyMap[key]
ANCHOR_API bool IsKeyDown(int user_key_index);  // is key being held. == io.KeysDown[user_key_index].
ANCHOR_API bool IsKeyPressed(int user_key_index,
                             bool repeat = true);   // was key pressed (went from !Down to Down)? if
                                                    // repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
ANCHOR_API bool IsKeyReleased(int user_key_index);  // was key released (went from Down to !Down)?
ANCHOR_API int GetKeyPressedAmount(
  int key_index,
  float repeat_delay,
  float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might
                // be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
ANCHOR_API void CaptureKeyboardFromApp(
  bool want_capture_keyboard_value =
    true);  // attention: misleading name! manually override io.WantCaptureKeyboard flag next
            // frame (said flag is entirely left for your application to handle). e.g. force
            // capture keyboard when your widget is being hovered. This is equivalent to
            // setting "io.WantCaptureKeyboard = want_capture_keyboard_value"; after the next
            // NewFrame() call.

// Inputs Utilities: Mouse
// - To refer to a mouse button, you may use named enums in your code e.g. ANCHOR_MouseButton_Left,
// ANCHOR_MouseButton_Right.
// - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
// - Dragging operations are only reported after mouse has moved a certain distance away from the
// initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
ANCHOR_API bool IsMouseDown(ANCHOR_MouseButton button);  // is mouse button held?
ANCHOR_API bool IsMouseClicked(ANCHOR_MouseButton button,
                               bool repeat = false);  // did mouse button clicked? (went from !Down to Down)
ANCHOR_API bool IsMouseReleased(
  ANCHOR_MouseButton button);  // did mouse button released? (went from Down to !Down)
ANCHOR_API bool IsMouseDoubleClicked(
  ANCHOR_MouseButton button);  // did mouse button double-clicked? (note that a double-click will
                               // also report IsMouseClicked() == true)
ANCHOR_API bool IsMouseHoveringRect(
  const wabi::GfVec2f &r_min,
  const wabi::GfVec2f &r_max,
  bool clip = true);  // is mouse hovering given bounding rect (in screen space). clipped by
                      // current clipping settings, but disregarding of other consideration of
                      // focus/window ordering/popup-block.
ANCHOR_API bool IsMousePosValid(
  const wabi::GfVec2f *mouse_pos = NULL);  // by convention we use (-FLT_MAX,-FLT_MAX) to denote
                                           // that there is no mouse available
ANCHOR_API bool IsAnyMouseDown();          // is any mouse button held?
ANCHOR_API wabi::GfVec2f GetMousePos();    // shortcut to ANCHOR::GetIO().MousePos provided by user,
                                           // to be consistent with other calls
ANCHOR_API wabi::GfVec2f GetMousePosOnOpeningCurrentPopup();  // retrieve mouse position at the
                                                              // time of opening popup we have
                                                              // BeginPopup() into (helper to avoid
                                                              // user backing that value
                                                              // themselves)
ANCHOR_API bool IsMouseDragging(ANCHOR_MouseButton button,
                                float lock_threshold = -1.0f);  // is mouse dragging? (if lock_threshold <
                                                                // -1.0f, uses io.MouseDraggingThreshold)
ANCHOR_API wabi::GfVec2f GetMouseDragDelta(
  ANCHOR_MouseButton button = 0,
  float lock_threshold = -1.0f);  // return the delta from the initial clicking position while the mouse
                                  // button is pressed or was just released. This is locked and return 0.0f
                                  // until the mouse moves past a distance threshold at least once (if
                                  // lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
ANCHOR_API void ResetMouseDragDelta(ANCHOR_MouseButton button = 0);  //
ANCHOR_API ANCHOR_MouseCursor
GetMouseCursor();  // get desired cursor type, reset in ANCHOR::NewFrame(), this is updated during
                   // the frame. valid before Render(). If you use software rendering by setting
                   // io.MouseDrawCursor ANCHOR will render those for you
ANCHOR_API void SetMouseCursor(ANCHOR_MouseCursor cursor_type);  // set desired cursor type
ANCHOR_API void CaptureMouseFromApp(
  bool want_capture_mouse_value =
    true);  // attention: misleading name! manually override io.WantCaptureMouse flag next
            // frame (said flag is entirely left for your application to handle). This is
            // equivalent to setting "io.WantCaptureMouse = want_capture_mouse_value;" after
            // the next NewFrame() call.

// Clipboard Utilities
// - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text
// data to the clipboard.
ANCHOR_API const char *GetClipboardText();
ANCHOR_API void SetClipboardText(const char *text);

// Settings/.Ini Utilities
// - The disk functions are automatically called if io.IniFilename != NULL (default is
// "ANCHOR.ini").
// - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description
// about handling .ini saving manually.
ANCHOR_API void LoadIniSettingsFromDisk(
  const char *ini_filename);  // call after CreateContext() and before the first call to NewFrame().
                              // NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
ANCHOR_API void LoadIniSettingsFromMemory(
  const char *ini_data,
  size_t ini_size = 0);  // call after CreateContext() and before the first call to NewFrame() to
                         // provide .ini data from your own data source.
ANCHOR_API void SaveIniSettingsToDisk(
  const char *ini_filename);  // this is automatically called (if io.IniFilename is not empty) a
                              // few seconds after any modification that should be reflected in
                              // the .ini file (and also by DestroyContext).
ANCHOR_API const char *SaveIniSettingsToMemory(
  size_t *out_ini_size = NULL);  // return a zero-terminated string with the .ini data which you can save by
                                 // your own mean. call when io.WantSaveIniSettings is set, then save data by
                                 // your own mean and clear io.WantSaveIniSettings.

// Debug Utilities
// - This is used by the ANCHOR_CHECKVERSION() macro.
ANCHOR_API bool DebugCheckVersionAndDataLayout(
  const char *version_str,
  size_t sz_io,
  size_t sz_style,
  size_t sz_vec2,
  size_t sz_vec4,
  size_t sz_drawvert,
  size_t sz_drawidx);  // This is called by ANCHOR_CHECKVERSION() macro.

// Memory Allocators
// - Those functions are not reliant on the current context.
// - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call
// SetCurrentContext() + SetAllocatorFunctions()
//   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators"
//   section of ANCHOR.cpp for more details.
ANCHOR_API void SetAllocatorFunctions(ANCHORMemAllocFunc alloc_func,
                                      ANCHORMemFreeFunc free_func,
                                      void *user_data = NULL);
ANCHOR_API void GetAllocatorFunctions(ANCHORMemAllocFunc *p_alloc_func,
                                      ANCHORMemFreeFunc *p_free_func,
                                      void **p_user_data);
ANCHOR_API void *MemAlloc(size_t size);
ANCHOR_API void MemFree(void *ptr);

}  // namespace ANCHOR

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for ANCHOR::Begin()
enum ANCHOR_WindowFlags_ {
  ANCHOR_WindowFlags_None = 0,
  ANCHOR_WindowFlags_NoTitleBar = 1 << 0,  // Disable title-bar
  ANCHOR_WindowFlags_NoResize = 1 << 1,    // Disable user resizing with the lower-right grip
  ANCHOR_WindowFlags_NoMove = 1 << 2,      // Disable user moving the window
  ANCHOR_WindowFlags_NoScrollbar =
    1 << 3,  // Disable scrollbars (window can still scroll with mouse or programmatically)
  ANCHOR_WindowFlags_NoScrollWithMouse =
    1 << 4,  // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel
             // will be forwarded to the parent unless NoScrollbar is also set.
  ANCHOR_WindowFlags_NoCollapse = 1 << 5,        // Disable user collapsing window by double-clicking on it
  ANCHOR_WindowFlags_AlwaysAutoResize = 1 << 6,  // Resize every window to its content every frame
  ANCHOR_WindowFlags_NoBackground = 1 << 7,  // Disable drawing background color (WindowBg, etc.) and outside
                                             // border. Similar as using SetNextWindowBgAlpha(0.0f).
  ANCHOR_WindowFlags_NoSavedSettings = 1 << 8,  // Never load/save settings in .ini file
  ANCHOR_WindowFlags_NoMouseInputs = 1 << 9,    // Disable catching mouse, hovering test with pass through.
  ANCHOR_WindowFlags_MenuBar = 1 << 10,         // Has a menu-bar
  ANCHOR_WindowFlags_HorizontalScrollbar =
    1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use
              // SetNextWindowContentSize(wabi::GfVec2f(width,0.0f)); prior to calling Begin() to
              // specify width. Read code in ANCHOR_demo in the "Horizontal Scrolling" section.
  ANCHOR_WindowFlags_NoFocusOnAppearing =
    1 << 12,  // Disable taking focus when transitioning from hidden to visible state
  ANCHOR_WindowFlags_NoBringToFrontOnFocus =
    1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or
              // programmatically giving it focus)
  ANCHOR_WindowFlags_AlwaysVerticalScrollbar =
    1 << 14,  // Always show vertical scrollbar (even if ContentSize[1] < Size[1])
  ANCHOR_WindowFlags_AlwaysHorizontalScrollbar =
    1 << 15,  // Always show horizontal scrollbar (even if ContentSize[0] < Size[0])
  ANCHOR_WindowFlags_AlwaysUseWindowPadding =
    1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default
              // for non-bordered child windows, because more convenient)
  ANCHOR_WindowFlags_NoNavInputs = 1 << 18,  // No gamepad/keyboard navigation within the window
  ANCHOR_WindowFlags_NoNavFocus = 1 << 19,   // No focusing toward this window with gamepad/keyboard
                                             // navigation (e.g. skipped by CTRL+TAB)
  ANCHOR_WindowFlags_UnsavedDocument =
    1 << 20,  // Append '*' to title without affecting the ID, as a convenience to avoid using
              // the ### operator. When used in a tab/docking context, tab is selected on closure
              // and closure is deferred by one frame to allow code to cancel the closure (with a
              // confirmation popup, etc.) without flicker.
  ANCHOR_WindowFlags_NoNav = ANCHOR_WindowFlags_NoNavInputs | ANCHOR_WindowFlags_NoNavFocus,
  ANCHOR_WindowFlags_NoDecoration = ANCHOR_WindowFlags_NoTitleBar | ANCHOR_WindowFlags_NoResize |
                                    ANCHOR_WindowFlags_NoScrollbar | ANCHOR_WindowFlags_NoCollapse,
  ANCHOR_WindowFlags_NoInputs = ANCHOR_WindowFlags_NoMouseInputs | ANCHOR_WindowFlags_NoNavInputs |
                                ANCHOR_WindowFlags_NoNavFocus,

  // [Internal]
  ANCHOR_WindowFlags_NavFlattened =
    1 << 23,  // [BETA] Allow gamepad/keyboard navigation to cross over parent border to this
              // child (only use on child that have no scrolling!)
  ANCHOR_WindowFlags_ChildWindow = 1 << 24,  // Don't use! For internal use by BeginChild()
  ANCHOR_WindowFlags_Tooltip = 1 << 25,      // Don't use! For internal use by BeginTooltip()
  ANCHOR_WindowFlags_Popup = 1 << 26,        // Don't use! For internal use by BeginPopup()
  ANCHOR_WindowFlags_Modal = 1 << 27,        // Don't use! For internal use by BeginPopupModal()
  ANCHOR_WindowFlags_ChildMenu = 1 << 28     // Don't use! For internal use by BeginMenu()

  // [Obsolete]
  // ANCHOR_WindowFlags_ResizeFromAnySide    = 1 << 17,  // --> Set
  // io.ConfigWindowsResizeFromEdges=true and make sure mouse cursors are supported by backend
  // (io.BackendFlags & ANCHORBackendFlags_HasMouseCursors)
};

// Flags for ANCHOR::InputText()
enum ANCHORInputTextFlags_ {
  ANCHORInputTextFlags_None = 0,
  ANCHORInputTextFlags_CharsDecimal = 1 << 0,      // Allow 0123456789.+-*/
  ANCHORInputTextFlags_CharsHexadecimal = 1 << 1,  // Allow 0123456789ABCDEFabcdef
  ANCHORInputTextFlags_CharsUppercase = 1 << 2,    // Turn a.[2] into A..Z
  ANCHORInputTextFlags_CharsNoBlank = 1 << 3,      // Filter out spaces, tabs
  ANCHORInputTextFlags_AutoSelectAll = 1 << 4,     // Select entire text when first taking mouse focus
  ANCHORInputTextFlags_EnterReturnsTrue =
    1 << 5,  // Return 'true' when Enter is pressed (as opposed to every time the value was
             // modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
  ANCHORInputTextFlags_CallbackCompletion = 1 << 6,  // Callback on pressing TAB (for completion handling)
  ANCHORInputTextFlags_CallbackHistory = 1
                                         << 7,  // Callback on pressing Up/Down arrows (for history handling)
  ANCHORInputTextFlags_CallbackAlways = 1 << 8,  // Callback on each iteration. User code may query
                                                 // cursor position, modify text buffer.
  ANCHORInputTextFlags_CallbackCharFilter =
    1 << 9,  // Callback on character inputs to replace or discard them. Modify 'EventChar' to
             // replace or discard, or return 1 in callback to discard.
  ANCHORInputTextFlags_AllowTabInput = 1 << 10,  // Pressing TAB input a '\t' character into the text field
  ANCHORInputTextFlags_CtrlEnterForNewLine =
    1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is
              // opposite: unfocus with Ctrl+Enter, add line with Enter).
  ANCHORInputTextFlags_NoHorizontalScroll = 1 << 12,  // Disable following the cursor horizontally
  ANCHORInputTextFlags_AlwaysOverwrite = 1 << 13,     // Overwrite mode
  ANCHORInputTextFlags_ReadOnly = 1 << 14,            // Read-only mode
  ANCHORInputTextFlags_Password = 1 << 15,            // Password mode, display all characters as '*'
  ANCHORInputTextFlags_NoUndoRedo =
    1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you
              // want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
  ANCHORInputTextFlags_CharsScientific = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
  ANCHORInputTextFlags_CallbackResize =
    1 << 18,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value),
              // allowing the string to grow. Notify when the string wants to be resized (for
              // string types which hold a cache of their Size). You will be provided a new
              // BufSize in the callback and NEED to honor it. (see misc/cpp/ANCHOR_stdlib.h for
              // an example of using this)
  ANCHORInputTextFlags_CallbackEdit = 1 << 19  // Callback on any edit (note that InputText() already returns
                                               // true on edit, the callback is useful mainly to manipulate
                                               // the underlying buffer while focus is active)

// Obsolete names (will be removed soon)
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  ,
  ANCHORInputTextFlags_AlwaysInsertMode =
    ANCHORInputTextFlags_AlwaysOverwrite  // [renamed in 1.82] name was not matching behavior
#  endif
};

// Flags for ANCHOR::TreeNodeEx(), ANCHOR::CollapsingHeader*()
enum ANCHOR_TreeNodeFlags_ {
  ANCHOR_TreeNodeFlags_None = 0,
  ANCHOR_TreeNodeFlags_Selected = 1 << 0,  // Draw as selected
  ANCHOR_TreeNodeFlags_Framed = 1 << 1,    // Draw frame with background (e.g. for CollapsingHeader)
  ANCHOR_TreeNodeFlags_AllowItemOverlap =
    1 << 2,  // Hit testing to allow subsequent widgets to overlap this one
  ANCHOR_TreeNodeFlags_NoTreePushOnOpen =
    1 << 3,  // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor
             // pushing on ID stack
  ANCHOR_TreeNodeFlags_NoAutoOpenOnLog =
    1 << 4,  // Don't automatically and temporarily open node when Logging is active (by default
             // logging will automatically open tree nodes)
  ANCHOR_TreeNodeFlags_DefaultOpen = 1 << 5,        // Default node to be open
  ANCHOR_TreeNodeFlags_OpenOnDoubleClick = 1 << 6,  // Need double-click to open node
  ANCHOR_TreeNodeFlags_OpenOnArrow = 1 << 7,        // Only open when clicking on the arrow part. If
                                                    // ANCHOR_TreeNodeFlags_OpenOnDoubleClick is also set,
                                                    // single-click arrow or double-click all box to open.
  ANCHOR_TreeNodeFlags_Leaf = 1 << 8,    // No collapsing, no arrow (use as a convenience for leaf nodes).
  ANCHOR_TreeNodeFlags_Bullet = 1 << 9,  // Display a bullet instead of arrow
  ANCHOR_TreeNodeFlags_FramePadding = 1 << 10,  // Use FramePadding (even for an unframed text node) to
                                                // vertically align text baseline to regular widget height.
                                                // Equivalent to calling AlignTextToFramePadding().
  ANCHOR_TreeNodeFlags_SpanAvailWidth =
    1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the
              // default in order to allow adding other items on the same line. In the future we
              // may refactor the hit system to be front-to-back, allowing natural overlaps and
              // then this can become the default.
  ANCHOR_TreeNodeFlags_SpanFullWidth =
    1 << 12,  // Extend hit box to the left-most and right-most edges (bypass the indented area).
  ANCHOR_TreeNodeFlags_NavLeftJumpsBackHere =
    1 << 13,  // (WIP) Nav: left direction may move to this TreeNode() from any of its child
              // (items submitted between TreeNode and TreePop)
  // ANCHOR_TreeNodeFlags_NoScrollOnOpen     = 1 << 14,  // FIXME: TODO: Disable automatic scroll
  // on TreePop() if node got just open and contents is not visible
  ANCHOR_TreeNodeFlags_CollapsingHeader = ANCHOR_TreeNodeFlags_Framed |
                                          ANCHOR_TreeNodeFlags_NoTreePushOnOpen |
                                          ANCHOR_TreeNodeFlags_NoAutoOpenOnLog
};

// Flags for OpenPopup*(), BeginPopupContext*(), IsPopupOpen() functions.
// - To be backward compatible with older API which took an 'int mouse_button = 1' argument, we
// need to treat
//   small flags values as a mouse button index, so we encode the mouse button in the first few
//   bits of the flags. It is therefore guaranteed to be legal to pass a mouse button index in
//   ANCHORPopupFlags.
// - For the same reason, we exceptionally default the ANCHORPopupFlags argument of
// BeginPopupContextXXX functions to 1 instead of 0.
//   IMPORTANT: because the default parameter is 1 (==ANCHORPopupFlags_MouseButtonRight), if you
//   rely on the default parameter and want to another another flag, you need to pass in the
//   ANCHORPopupFlags_MouseButtonRight flag.
// - Multiple buttons currently cannot be combined/or-ed in those functions (we could allow it
// later).
enum ANCHORPopupFlags_ {
  ANCHORPopupFlags_None = 0,
  ANCHORPopupFlags_MouseButtonLeft = 0,   // For BeginPopupContext*(): open on Left Mouse release. Guaranteed
                                          // to always be == 0 (same as ANCHOR_MouseButton_Left)
  ANCHORPopupFlags_MouseButtonRight = 1,  // For BeginPopupContext*(): open on Right Mouse release.
                                          // Guaranteed to always be == 1 (same as ANCHOR_MouseButton_Right)
  ANCHORPopupFlags_MouseButtonMiddle =
    2,  // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2
        // (same as ANCHOR_MouseButton_Middle)
  ANCHORPopupFlags_MouseButtonMask_ = 0x1F,
  ANCHORPopupFlags_MouseButtonDefault_ = 1,
  ANCHORPopupFlags_NoOpenOverExistingPopup =
    1 << 5,  // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at
             // the same level of the popup stack
  ANCHORPopupFlags_NoOpenOverItems = 1 << 6,  // For BeginPopupContextWindow(): don't return true
                                              // when hovering items, only when hovering empty space
  ANCHORPopupFlags_AnyPopupId =
    1 << 7,  // For IsPopupOpen(): ignore the ANCHOR_ID parameter and test for any popup.
  ANCHORPopupFlags_AnyPopupLevel = 1 << 8,  // For IsPopupOpen(): search/test at any level of the
                                            // popup stack (default test in the current level)
  ANCHORPopupFlags_AnyPopup = ANCHORPopupFlags_AnyPopupId | ANCHORPopupFlags_AnyPopupLevel
};

// Flags for ANCHOR::Selectable()
enum ANCHORSelectableFlags_ {
  ANCHORSelectableFlags_None = 0,
  ANCHORSelectableFlags_DontClosePopups = 1 << 0,  // Clicking this don't close parent popup window
  ANCHORSelectableFlags_SpanAllColumns =
    1 << 1,  // Selectable frame can span all columns (text will still fit in current column)
  ANCHORSelectableFlags_AllowDoubleClick = 1 << 2,  // Generate press events on double clicks too
  ANCHORSelectableFlags_Disabled = 1 << 3,          // Cannot be selected, display grayed out text
  ANCHORSelectableFlags_AllowItemOverlap =
    1 << 4  // (WIP) Hit testing to allow subsequent widgets to overlap this one
};

// Flags for ANCHOR::BeginCombo()
enum ANCHORComboFlags_ {
  ANCHORComboFlags_None = 0,
  ANCHORComboFlags_PopupAlignLeft = 1 << 0,  // Align the popup toward the left by default
  ANCHORComboFlags_HeightSmall =
    1 << 1,  // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you
             // can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
  ANCHORComboFlags_HeightRegular = 1 << 2,  // Max ~8 items visible (default)
  ANCHORComboFlags_HeightLarge = 1 << 3,    // Max ~20 items visible
  ANCHORComboFlags_HeightLargest = 1 << 4,  // As many fitting items as possible
  ANCHORComboFlags_NoArrowButton = 1 << 5,  // Display on the preview box without the square arrow button
  ANCHORComboFlags_NoPreview = 1 << 6,      // Display only a square arrow button
  ANCHORComboFlags_HeightMask_ = ANCHORComboFlags_HeightSmall | ANCHORComboFlags_HeightRegular |
                                 ANCHORComboFlags_HeightLarge | ANCHORComboFlags_HeightLargest
};

// Flags for ANCHOR::BeginTabBar()
enum ANCHOR_TabBarFlags_ {
  ANCHOR_TabBarFlags_None = 0,
  ANCHOR_TabBarFlags_Reorderable = 1 << 0,         // Allow manually dragging tabs to re-order them + New
                                                   // tabs are appended at the end of list
  ANCHOR_TabBarFlags_AutoSelectNewTabs = 1 << 1,   // Automatically select new tabs when they appear
  ANCHOR_TabBarFlags_TabListPopupButton = 1 << 2,  // Disable buttons to open the tab list popup
  ANCHOR_TabBarFlags_NoCloseWithMiddleMouseButton =
    1 << 3,  // Disable behavior of closing tabs (that are submitted with p_open != NULL) with
             // middle mouse button. You can still repro this behavior on user's side with if
             // (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
  ANCHOR_TabBarFlags_NoTabListScrollingButtons = 1
                                                 << 4,  // Disable scrolling buttons (apply when fitting
                                                        // policy is ANCHOR_TabBarFlags_FittingPolicyScroll)
  ANCHOR_TabBarFlags_NoTooltip = 1 << 5,                // Disable tooltips when hovering a tab
  ANCHOR_TabBarFlags_FittingPolicyResizeDown = 1 << 6,  // Resize tabs when they don't fit
  ANCHOR_TabBarFlags_FittingPolicyScroll = 1 << 7,      // Add scroll buttons when tabs don't fit
  ANCHOR_TabBarFlags_FittingPolicyMask_ = ANCHOR_TabBarFlags_FittingPolicyResizeDown |
                                          ANCHOR_TabBarFlags_FittingPolicyScroll,
  ANCHOR_TabBarFlags_FittingPolicyDefault_ = ANCHOR_TabBarFlags_FittingPolicyResizeDown
};

// Flags for ANCHOR::BeginTabItem()
enum ANCHOR_TabItemFlags_ {
  ANCHOR_TabItemFlags_None = 0,
  ANCHOR_TabItemFlags_UnsavedDocument =
    1 << 0,  // Append '*' to title without affecting the ID, as a convenience to avoid using the
             // ### operator. Also: tab is selected on closure and closure is deferred by one
             // frame to allow code to undo it without flicker.
  ANCHOR_TabItemFlags_SetSelected = 1 << 1,  // Trigger flag to programmatically make the tab
                                             // selected when calling BeginTabItem()
  ANCHOR_TabItemFlags_NoCloseWithMiddleMouseButton =
    1 << 2,  // Disable behavior of closing tabs (that are submitted with p_open != NULL) with
             // middle mouse button. You can still repro this behavior on user's side with if
             // (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
  ANCHOR_TabItemFlags_NoPushId =
    1 << 3,  // Don't call PushID(tab->ID)/PopID() on BeginTabItem()/EndTabItem()
  ANCHOR_TabItemFlags_NoTooltip = 1 << 4,  // Disable tooltip for the given tab
  ANCHOR_TabItemFlags_NoReorder =
    1 << 5,  // Disable reordering this tab or having another tab cross over this tab
  ANCHOR_TabItemFlags_Leading = 1 << 6,  // Enforce the tab position to the left of the tab bar
                                         // (after the tab list popup button)
  ANCHOR_TabItemFlags_Trailing = 1 << 7  // Enforce the tab position to the right of the tab bar
                                         // (before the scrolling buttons)
};

// Flags for ANCHOR::BeginTable()
// [BETA API] API may evolve slightly! If you use this, please update to the next version when it
// comes out!
// - Important! Sizing policies have complex and subtle side effects, more so than you would
// expect.
//   Read comments/demos carefully + experiment with live demos to get acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to ANCHOR_TableFlags_SizingFixedFit    if ScrollX is on, or if host window has
//    ANCHOR_WindowFlags_AlwaysAutoResize.
//    - Default to ANCHOR_TableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to ANCHOR_TableFlags_SizingStretchSame -> all Columns defaults to
//    ANCHOR_TableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns will generally obtain their requested width (unless the table cannot fit them
//    all).
//    - Stretch Columns will share the remaining width.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed
//      by one or two TRAILING Stretch columns. (this is because the visible order of columns have
//      subtle but necessary effects on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to ANCHOR_TableFlags_SizingFixedFit -> all Columns defaults to
//    ANCHOR_TableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show an horizontal scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the content width to use
//    item right-alignment e.g. SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a
//    feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified
//    a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling space is known and
//      Stretch or mixed Fixed/Stretch columns become meaningful again.
// - Read on documentation at the top of ANCHOR_tables.cpp for details.
enum ANCHOR_TableFlags_ {
  // Features
  ANCHOR_TableFlags_None = 0,
  ANCHOR_TableFlags_Resizable = 1 << 0,  // Enable resizing columns.
  ANCHOR_TableFlags_Reorderable =
    1 << 1,  // Enable reordering columns in header row (need calling TableSetupColumn() +
             // TableHeadersRow() to display headers)
  ANCHOR_TableFlags_Hideable = 1 << 2,  // Enable hiding/disabling columns in context menu.
  ANCHOR_TableFlags_Sortable =
    1 << 3,  // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see
             // ANCHOR_TableFlags_SortMulti and ANCHOR_TableFlags_SortTristate.
  ANCHOR_TableFlags_NoSavedSettings =
    1 << 4,  // Disable persisting columns order, width and sort settings in the .ini file.
  ANCHOR_TableFlags_ContextMenuInBody =
    1 << 5,  // Right-click on columns body/contents will display table context menu. By default
             // it is available in TableHeadersRow(). Decorations
  ANCHOR_TableFlags_RowBg =
    1 << 6,  // Set each RowBg color with ANCHOR_Col_TableRowBg or ANCHOR_Col_TableRowBgAlt
             // (equivalent of calling TableSetBgColor with ANCHOR_TableBgFlags_RowBg0 on each
             // row manually)
  ANCHOR_TableFlags_BordersInnerH = 1 << 7,   // Draw horizontal borders between rows.
  ANCHOR_TableFlags_BordersOuterH = 1 << 8,   // Draw horizontal borders at the top and bottom.
  ANCHOR_TableFlags_BordersInnerV = 1 << 9,   // Draw vertical borders between columns.
  ANCHOR_TableFlags_BordersOuterV = 1 << 10,  // Draw vertical borders on the left and right sides.
  ANCHOR_TableFlags_BordersH = ANCHOR_TableFlags_BordersInnerH |
                               ANCHOR_TableFlags_BordersOuterH,  // Draw horizontal borders.
  ANCHOR_TableFlags_BordersV = ANCHOR_TableFlags_BordersInnerV |
                               ANCHOR_TableFlags_BordersOuterV,  // Draw vertical borders.
  ANCHOR_TableFlags_BordersInner = ANCHOR_TableFlags_BordersInnerV |
                                   ANCHOR_TableFlags_BordersInnerH,  // Draw inner borders.
  ANCHOR_TableFlags_BordersOuter = ANCHOR_TableFlags_BordersOuterV |
                                   ANCHOR_TableFlags_BordersOuterH,  // Draw outer borders.
  ANCHOR_TableFlags_Borders = ANCHOR_TableFlags_BordersInner |
                              ANCHOR_TableFlags_BordersOuter,  // Draw all borders.
  ANCHOR_TableFlags_NoBordersInBody =
    1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appears in
              // Headers). -> May move to style
  ANCHOR_TableFlags_NoBordersInBodyUntilResize =
    1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize
              // (borders will always appears in Headers). -> May move to style Sizing Policy
              // (read above for defaults)
  ANCHOR_TableFlags_SizingFixedFit =
    1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable),
              // matching contents width.
  ANCHOR_TableFlags_SizingFixedSame =
    2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable),
              // matching the maximum contents width of all columns. Implicitly enable
              // ANCHOR_TableFlags_NoKeepColumnsVisible.
  ANCHOR_TableFlags_SizingStretchProp =
    3 << 13,  // Columns default to _WidthStretch with default weights proportional to each
              // columns contents widths.
  ANCHOR_TableFlags_SizingStretchSame =
    4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless
              // overridden by TableSetupColumn(). Sizing Extra Options
  ANCHOR_TableFlags_NoHostExtendX =
    1 << 16,  // Make outer width auto-fit to columns, overriding outer_size[0] value. Only
              // available when ScrollX/ScrollY are disabled and Stretch columns are not used.
  ANCHOR_TableFlags_NoHostExtendY =
    1 << 17,  // Make outer height stop exactly at outer_size[1] (prevent auto-extending table
              // past the limit). Only available when ScrollX/ScrollY are disabled. Data below
              // the limit will be clipped and not visible.
  ANCHOR_TableFlags_NoKeepColumnsVisible =
    1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table
              // gets too small. Not recommended if columns are resizable.
  ANCHOR_TableFlags_PreciseWidths =
    1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a
              // 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag:
              // 33,33,33). With larger number of columns, resizing will appear to be less
              // smooth. Clipping
  ANCHOR_TableFlags_NoClip =
    1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command
              // count, items will be able to overflow into other columns). Generally
              // incompatible with TableSetupScrollFreeze(). Padding
  ANCHOR_TableFlags_PadOuterX = 1 << 21,    // Default if BordersOuterV is on. Enable outer-most
                                            // padding. Generally desirable if you have headers.
  ANCHOR_TableFlags_NoPadOuterX = 1 << 22,  // Default if BordersOuterV is off. Disable outer-most padding.
  ANCHOR_TableFlags_NoPadInnerX =
    1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is
              // on, single inner padding if BordersOuterV is off). Scrolling
  ANCHOR_TableFlags_ScrollX =
    1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to
              // specify the container size. Changes default sizing policy. Because this create a
              // child window, ScrollY is currently generally recommended when using ScrollX.
  ANCHOR_TableFlags_ScrollY =
    1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to
              // specify the container size. Sorting
  ANCHOR_TableFlags_SortMulti =
    1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs()
              // may return specs where (SpecsCount > 1).
  ANCHOR_TableFlags_SortTristate =
    1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs
              // where (SpecsCount == 0).

  // [Internal] Combinations and masks
  ANCHOR_TableFlags_SizingMask_ = ANCHOR_TableFlags_SizingFixedFit | ANCHOR_TableFlags_SizingFixedSame |
                                  ANCHOR_TableFlags_SizingStretchProp | ANCHOR_TableFlags_SizingStretchSame

// Obsolete names (will be removed soon)
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
//, ANCHOR_TableFlags_ColumnsWidthFixed = ANCHOR_TableFlags_SizingFixedFit,
// ANCHOR_TableFlags_ColumnsWidthStretch = ANCHOR_TableFlags_SizingStretchSame   // WIP Tables
// 2020/12 , ANCHOR_TableFlags_SizingPolicyFixed = ANCHOR_TableFlags_SizingFixedFit,
// ANCHOR_TableFlags_SizingPolicyStretch = ANCHOR_TableFlags_SizingStretchSame   // WIP Tables
// 2021/01
#  endif
};

// Flags for ANCHOR::TableSetupColumn()
enum ANCHOR_TableColumnFlags_ {
  // Input configuration flags
  ANCHOR_TableColumnFlags_None = 0,
  ANCHOR_TableColumnFlags_DefaultHide = 1 << 0,  // Default as a hidden/disabled column.
  ANCHOR_TableColumnFlags_DefaultSort = 1 << 1,  // Default as a sorting column.
  ANCHOR_TableColumnFlags_WidthStretch =
    1 << 2,  // Column will stretch. Preferable with horizontal scrolling disabled (default if
             // table sizing policy is _SizingStretchSame or _SizingStretchProp).
  ANCHOR_TableColumnFlags_WidthFixed =
    1 << 3,  // Column will not stretch. Preferable with horizontal scrolling enabled (default if
             // table sizing policy is _SizingFixedFit and table is resizable).
  ANCHOR_TableColumnFlags_NoResize = 1 << 4,  // Disable manual resizing.
  ANCHOR_TableColumnFlags_NoReorder =
    1 << 5,  // Disable manual reordering this column, this will also prevent other columns from
             // crossing over this column.
  ANCHOR_TableColumnFlags_NoHide = 1 << 6,           // Disable ability to hide/disable this column.
  ANCHOR_TableColumnFlags_NoClip = 1 << 7,           // Disable clipping for this column (all NoClip columns
                                                     // will render in a same draw command).
  ANCHOR_TableColumnFlags_NoSort = 1 << 8,           // Disable ability to sort on this field (even if
                                                     // ANCHOR_TableFlags_Sortable is set on the table).
  ANCHOR_TableColumnFlags_NoSortAscending = 1 << 9,  // Disable ability to sort in the ascending direction.
  ANCHOR_TableColumnFlags_NoSortDescending = 1
                                             << 10,  // Disable ability to sort in the descending direction.
  ANCHOR_TableColumnFlags_NoHeaderWidth =
    1 << 11,  // Disable header text width contribution to automatic column width.
  ANCHOR_TableColumnFlags_PreferSortAscending =
    1 << 12,  // Make the initial sort direction Ascending when first sorting on this column
              // (default).
  ANCHOR_TableColumnFlags_PreferSortDescending =
    1 << 13,  // Make the initial sort direction Descending when first sorting on this column.
  ANCHOR_TableColumnFlags_IndentEnable =
    1 << 14,  // Use current Indent value when entering cell (default for column 0).
  ANCHOR_TableColumnFlags_IndentDisable =
    1 << 15,  // Ignore current Indent value when entering cell (default for columns > 0).
              // Indentation changes _within_ the cell will still be honored.

  // Output status flags, read-only via TableGetColumnFlags()
  ANCHOR_TableColumnFlags_IsEnabled =
    1 << 20,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in
              // _DefaultHide and _NoHide) flags.
  ANCHOR_TableColumnFlags_IsVisible =
    1 << 21,  // Status: is visible == is enabled AND not clipped by scrolling.
  ANCHOR_TableColumnFlags_IsSorted = 1 << 22,   // Status: is currently part of the sort specs
  ANCHOR_TableColumnFlags_IsHovered = 1 << 23,  // Status: is hovered by mouse

  // [Internal] Combinations and masks
  ANCHOR_TableColumnFlags_WidthMask_ = ANCHOR_TableColumnFlags_WidthStretch |
                                       ANCHOR_TableColumnFlags_WidthFixed,
  ANCHOR_TableColumnFlags_IndentMask_ = ANCHOR_TableColumnFlags_IndentEnable |
                                        ANCHOR_TableColumnFlags_IndentDisable,
  ANCHOR_TableColumnFlags_StatusMask_ = ANCHOR_TableColumnFlags_IsEnabled |
                                        ANCHOR_TableColumnFlags_IsVisible |
                                        ANCHOR_TableColumnFlags_IsSorted | ANCHOR_TableColumnFlags_IsHovered,
  ANCHOR_TableColumnFlags_NoDirectResize_ =
    1 << 30  // [Internal] Disable user resizing this column directly (it may however we resized
             // indirectly from its left edge)

// Obsolete names (will be removed soon)
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
// ANCHOR_TableColumnFlags_WidthAuto           = ANCHOR_TableColumnFlags_WidthFixed
// | ANCHOR_TableColumnFlags_NoResize, // Column will not stretch and keep resizing
// based on submitted contents.
#  endif
};

// Flags for ANCHOR::TableNextRow()
enum ANCHOR_TableRowFlags_ {
  ANCHOR_TableRowFlags_None = 0,
  ANCHOR_TableRowFlags_Headers = 1 << 0  // Identify header row (set default background color + width of its
                                         // contents accounted different for auto column width)
};

// Enum for ANCHOR::TableSetBgColor()
// Background colors are rendering in 3 layers:
//  - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
//  - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
//  - Layer 2: draw with CellBg color if set.
// The purpose of the two row/columns layers is to let you decide if a background color changes
// should override or blend with the existing color. When using ANCHOR_TableFlags_RowBg on the
// table, each row has the RowBg0 color automatically set for odd/even rows. If you set the color
// of RowBg0 target, your color will override the existing RowBg0 color. If you set the color of
// RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.
enum ANCHOR_TableBgTarget_ {
  ANCHOR_TableBgTarget_None = 0,
  ANCHOR_TableBgTarget_RowBg0 = 1,  // Set row background color 0 (generally used for background,
                                    // automatically set when ANCHOR_TableFlags_RowBg is used)
  ANCHOR_TableBgTarget_RowBg1 = 2,  // Set row background color 1 (generally used for selection marking)
  ANCHOR_TableBgTarget_CellBg = 3   // Set cell background color (top-most color)
};

// Flags for ANCHOR::IsWindowFocused()
enum ANCHORFocusedFlags_ {
  ANCHORFocusedFlags_None = 0,
  ANCHORFocusedFlags_ChildWindows =
    1 << 0,  // IsWindowFocused(): Return true if any children of the window is focused
  ANCHORFocusedFlags_RootWindow = 1 << 1,  // IsWindowFocused(): Test from root window (top most
                                           // parent of the current hierarchy)
  ANCHORFocusedFlags_AnyWindow =
    1 << 2,  // IsWindowFocused(): Return true if any window is focused. Important: If you are
             // trying to tell how to dispatch your low-level inputs, do NOT use this. Use
             // 'io.WantCaptureMouse' instead! Please read the FAQ!
  ANCHORFocusedFlags_RootAndChildWindows = ANCHORFocusedFlags_RootWindow | ANCHORFocusedFlags_ChildWindows
};

// Flags for ANCHOR::IsItemHovered(), ANCHOR::IsWindowHovered()
// Note: if you are trying to check whether your mouse should be dispatched to ANCHOR or to your
// app, you should use 'io.WantCaptureMouse' instead! Please read the FAQ! Note: windows with the
// ANCHOR_WindowFlags_NoInputs flag are ignored by IsWindowHovered() calls.
enum ANCHORHoveredFlags_ {
  ANCHORHoveredFlags_None =
    0,  // Return true if directly over the item/window, not obstructed by another window, not
        // obstructed by an active popup or modal blocking inputs under them.
  ANCHORHoveredFlags_ChildWindows =
    1 << 0,  // IsWindowHovered() only: Return true if any children of the window is hovered
  ANCHORHoveredFlags_RootWindow = 1 << 1,  // IsWindowHovered() only: Test from root window (top
                                           // most parent of the current hierarchy)
  ANCHORHoveredFlags_AnyWindow = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
  ANCHORHoveredFlags_AllowWhenBlockedByPopup =
    1 << 3,  // Return true even if a popup window is normally blocking access to this item/window
  // ANCHORHoveredFlags_AllowWhenBlockedByModal     = 1 << 4,   // Return true even if a modal
  // popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
  ANCHORHoveredFlags_AllowWhenBlockedByActiveItem =
    1 << 5,  // Return true even if an active item is blocking access to this item/window. Useful
             // for Drag and Drop patterns.
  ANCHORHoveredFlags_AllowWhenOverlapped =
    1 << 6,  // Return true even if the position is obstructed or overlapped by another window
  ANCHORHoveredFlags_AllowWhenDisabled = 1 << 7,  // Return true even if the item is disabled
  ANCHORHoveredFlags_RectOnly = ANCHORHoveredFlags_AllowWhenBlockedByPopup |
                                ANCHORHoveredFlags_AllowWhenBlockedByActiveItem |
                                ANCHORHoveredFlags_AllowWhenOverlapped,
  ANCHORHoveredFlags_RootAndChildWindows = ANCHORHoveredFlags_RootWindow | ANCHORHoveredFlags_ChildWindows
};

// Flags for ANCHOR::BeginDragDropSource(), ANCHOR::AcceptDragDropPayload()
enum ANCHORDragDropFlags_ {
  ANCHORDragDropFlags_None = 0,
  // BeginDragDropSource() flags
  ANCHORDragDropFlags_SourceNoPreviewTooltip =
    1 << 0,  // By default, a successful call to BeginDragDropSource opens a tooltip so you can
             // display a preview or description of the source contents. This flag disable this
             // behavior.
  ANCHORDragDropFlags_SourceNoDisableHover =
    1 << 1,  // By default, when dragging we clear data so that IsItemHovered() will return
             // false, to avoid subsequent user code submitting tooltips. This flag disable this
             // behavior so you can still call IsItemHovered() on the source item.
  ANCHORDragDropFlags_SourceNoHoldToOpenOthers =
    1 << 2,  // Disable the behavior that allows to open tree nodes and collapsing header by
             // holding over them while dragging a source item.
  ANCHORDragDropFlags_SourceAllowNullID =
    1 << 3,  // Allow items such as Text(), Image() that have no unique identifier to be used as
             // drag source, by manufacturing a temporary identifier based on their
             // window-relative position. This is extremely unusual within the ANCHOR ecosystem
             // and so we made it explicit.
  ANCHORDragDropFlags_SourceExtern =
    1 << 4,  // External source (from outside of ANCHOR), won't attempt to read current item/window
             // info. Will always return true. Only one Extern source can be active simultaneously.
  ANCHORDragDropFlags_SourceAutoExpirePayload =
    1 << 5,  // Automatically expire the payload if the source cease to be submitted (otherwise
             // payloads are persisting while being dragged)
  // AcceptDragDropPayload() flags
  ANCHORDragDropFlags_AcceptBeforeDelivery =
    1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is
              // released. You can then call IsDelivery() to test if the payload needs to be
              // delivered.
  ANCHORDragDropFlags_AcceptNoDrawDefaultRect =
    1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
  ANCHORDragDropFlags_AcceptNoPreviewTooltip =
    1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
  ANCHORDragDropFlags_AcceptPeekOnly =
    ANCHORDragDropFlags_AcceptBeforeDelivery |
    ANCHORDragDropFlags_AcceptNoDrawDefaultRect  // For peeking ahead and inspecting the payload
                                                 // before delivery.
};

// Standard Drag and Drop payload types. You can define you own payload types using short strings.
// Types starting with '_' are defined by ANCHOR.
#  define ANCHOR_PAYLOAD_TYPE_COLOR_3F \
    "_COL3F"  // float[3]: Standard type for colors, without alpha. User code may use this type.
#  define ANCHOR_PAYLOAD_TYPE_COLOR_4F \
    "_COL4F"  // float[4]: Standard type for colors. User code may use this type.

// A primary data type
enum ANCHOR_DataType_ {
  ANCHOR_DataType_S8,      // signed char / char (with sensible compilers)
  ANCHOR_DataType_U8,      // unsigned char
  ANCHOR_DataType_S16,     // short
  ANCHOR_DataType_U16,     // unsigned short
  ANCHOR_DataType_S32,     // int
  ANCHOR_DataType_U32,     // unsigned int
  ANCHOR_DataType_S64,     // long long / __int64
  ANCHOR_DataType_U64,     // unsigned long long / unsigned __int64
  ANCHOR_DataType_Float,   // float
  ANCHOR_DataType_Double,  // double
  ANCHOR_DataType_COUNT
};

// A cardinal direction
enum ANCHOR_Dir_ {
  ANCHOR_Dir_None = -1,
  ANCHOR_Dir_Left = 0,
  ANCHOR_Dir_Right = 1,
  ANCHOR_Dir_Up = 2,
  ANCHOR_Dir_Down = 3,
  ANCHOR_Dir_COUNT
};

// A sorting direction
enum ANCHOR_SortDirection_ {
  ANCHOR_SortDirection_None = 0,
  ANCHOR_SortDirection_Ascending = 1,  // Ascending = 0->9, A->Z etc.
  ANCHOR_SortDirection_Descending = 2  // Descending = 9->0, Z->A etc.
};

// User fill ANCHOR_IO.KeyMap[] array with indices into the ANCHOR_IO.KeysDown[512] array
enum ANCHOR_Key_ {
  ANCHOR_Key_Tab,
  ANCHOR_Key_LeftArrow,
  ANCHOR_Key_RightArrow,
  ANCHOR_Key_UpArrow,
  ANCHOR_Key_DownArrow,
  ANCHOR_Key_PageUp,
  ANCHOR_Key_PageDown,
  ANCHOR_Key_Home,
  ANCHOR_Key_End,
  ANCHOR_Key_Insert,
  ANCHOR_Key_Delete,
  ANCHOR_Key_Backspace,
  ANCHOR_Key_Space,
  ANCHOR_Key_Enter,
  ANCHOR_Key_Escape,
  ANCHOR_Key_KeyPadEnter,
  ANCHOR_Key_A,  // for text edit CTRL+A: select all
  ANCHOR_Key_C,  // for text edit CTRL+C: copy
  ANCHOR_Key_V,  // for text edit CTRL+V: paste
  ANCHOR_Key_X,  // for text edit CTRL+X: cut
  ANCHOR_Key_Y,  // for text edit CTRL+Y: redo
  ANCHOR_Key_Z,  // for text edit CTRL+Z: undo
  ANCHOR_Key_COUNT
};

// To test io.KeyMods (which is a combination of individual fields io.KeyCtrl, io.KeyShift,
// io.KeyAlt set by user/backend)
enum ANCHOR_KeyModFlags_ {
  ANCHOR_KeyModFlags_None = 0,
  ANCHOR_KeyModFlags_Ctrl = 1 << 0,
  ANCHOR_KeyModFlags_Shift = 1 << 1,
  ANCHOR_KeyModFlags_Alt = 1 << 2,
  ANCHOR_KeyModFlags_Super = 1 << 3
};

// Gamepad/Keyboard navigation
// Keyboard: Set io.ConfigFlags |= ANCHORConfigFlags_NavEnableKeyboard to enable. NewFrame() will
// automatically fill io.NavInputs[] based on your io.KeysDown[] + io.KeyMap[] arrays. Gamepad: Set
// io.ConfigFlags |= ANCHORConfigFlags_NavEnableGamepad to enable. Backend: set
// ANCHORBackendFlags_HasGamepad and fill the io.NavInputs[] fields before calling NewFrame(). Note
// that io.NavInputs[] is cleared by EndFrame(). Read instructions in ANCHOR.cpp for more details.
// Download PNG/PSD at http://dearANCHOR.org/controls_sheets.
enum ANCHOR_NavInput_ {
  // Gamepad Mapping
  ANCHOR_NavInput_Activate,     // activate / open / toggle / tweak value       // e.g. Cross  (PS4),
                                // A (Xbox), A (Switch), Space (Keyboard)
  ANCHOR_NavInput_Cancel,       // cancel / close / exit                        // e.g. Circle (PS4), B
                                // (Xbox), B (Switch), Escape (Keyboard)
  ANCHOR_NavInput_Input,        // text input / on-screen keyboard              // e.g. Triang.(PS4), Y
                                // (Xbox), X (Switch), Return (Keyboard)
  ANCHOR_NavInput_Menu,         // tap: toggle menu / hold: focus, move, resize // e.g. Square (PS4), X
                                // (Xbox), Y (Switch), Alt (Keyboard)
  ANCHOR_NavInput_DpadLeft,     // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad
                                // Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
  ANCHOR_NavInput_DpadRight,    //
  ANCHOR_NavInput_DpadUp,       //
  ANCHOR_NavInput_DpadDown,     //
  ANCHOR_NavInput_LStickLeft,   // scroll / move window (w/ PadMenu)            // e.g. Left Analog
                                // Stick Left/Right/Up/Down
  ANCHOR_NavInput_LStickRight,  //
  ANCHOR_NavInput_LStickUp,     //
  ANCHOR_NavInput_LStickDown,   //
  ANCHOR_NavInput_FocusPrev,    // next window (w/ PadMenu)                     // e.g. L1 or L2
                                // (PS4), LB or LT (Xbox), L or ZL (Switch)
  ANCHOR_NavInput_FocusNext,    // prev window (w/ PadMenu)                     // e.g. R1 or R2
                                // (PS4), RB or RT (Xbox), R or ZL (Switch)
  ANCHOR_NavInput_TweakSlow,    // slower tweaks                                // e.g. L1 or L2
                                // (PS4), LB or LT (Xbox), L or ZL (Switch)
  ANCHOR_NavInput_TweakFast,    // faster tweaks                                // e.g. R1 or R2
                                // (PS4), RB or RT (Xbox), R or ZL (Switch)

  // [Internal] Don't use directly! This is used internally to differentiate keyboard from gamepad
  // inputs for behaviors that require to differentiate them. Keyboard behavior that have no
  // corresponding gamepad mapping (e.g. CTRL+TAB) will be directly reading from io.KeysDown[]
  // instead of io.NavInputs[].
  ANCHOR_NavInput_KeyMenu_,   // toggle menu                                  // = io.KeyAlt
  ANCHOR_NavInput_KeyLeft_,   // move left                                    // = Arrow keys
  ANCHOR_NavInput_KeyRight_,  // move right
  ANCHOR_NavInput_KeyUp_,     // move up
  ANCHOR_NavInput_KeyDown_,   // move down
  ANCHOR_NavInput_COUNT,
  ANCHOR_NavInput_InternalStart_ = ANCHOR_NavInput_KeyMenu_
};

// Configuration flags stored in io.ConfigFlags. Set by user/application.
enum ANCHORConfigFlags_ {
  ANCHORConfigFlags_None = 0,
  ANCHORConfigFlags_NavEnableKeyboard = 1 << 0,  // Master keyboard navigation enable flag. NewFrame() will
                                                 // automatically fill io.NavInputs[] based on io.KeysDown[].
  ANCHORConfigFlags_NavEnableGamepad = 1 << 1,   // Master gamepad navigation enable flag. This is mostly to
                                                 // instruct your ANCHOR backend to fill io.NavInputs[].
                                                 // Backend also needs to set ANCHORBackendFlags_HasGamepad.
  ANCHORConfigFlags_NavEnableSetMousePos =
    1 << 2,  // Instruct navigation to move the mouse cursor. May be useful on TV/console systems
             // where moving a virtual mouse is awkward. Will update io.MousePos and set
             // io.WantSetMousePos=true. If enabled you MUST honor io.WantSetMousePos requests in
             // your backend, otherwise ANCHOR will react as if the mouse is jumping around back
             // and forth.
  ANCHORConfigFlags_NavNoCaptureKeyboard = 1 << 3,  // Instruct navigation to not set the
                                                    // io.WantCaptureKeyboard flag when io.NavActive is set.
  ANCHORConfigFlags_NoMouse = 1 << 4,  // Instruct ANCHOR to clear mouse position/buttons in NewFrame(). This
                                       // allows ignoring the mouse information set by the backend.
  ANCHORConfigFlags_NoMouseCursorChange =
    1 << 5,  // Instruct backend to not alter mouse cursor shape and visibility. Use if the
             // backend cursor changes are interfering with yours and you don't want to use
             // SetMouseCursor() to change mouse cursor. You may want to honor requests from
             // ANCHOR by reading GetMouseCursor() yourself instead.

  // User storage (to allow your backend/engine to communicate to code that may be shared between
  // multiple projects. Those flags are not used by core ANCHOR)
  ANCHORConfigFlags_IsSRGB = 1 << 20,        // Application is SRGB-aware.
  ANCHORConfigFlags_IsTouchScreen = 1 << 21  // Application is using a touch screen instead of a mouse.
};

// Backend capabilities flags stored in io.BackendFlags. Set by ANCHOR_impl_xxx or custom backend.
enum ANCHORBackendFlags_ {
  ANCHORBackendFlags_None = 0,
  ANCHORBackendFlags_HasGamepad =
    1 << 0,  // Backend Platform supports gamepad and currently has one connected.
  ANCHORBackendFlags_HasMouseCursors = 1 << 1,  // Backend Platform supports honoring GetMouseCursor() value
                                                // to change the OS cursor shape.
  ANCHORBackendFlags_HasSetMousePos =
    1 << 2,  // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse
             // position (only used if ANCHORConfigFlags_NavEnableSetMousePos is set).
  ANCHORBackendFlags_RendererHasVtxOffset =
    1 << 3  // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large
            // meshes (64K+ vertices) while still using 16-bit indices.
};

// Enumeration for PushStyleColor() / PopStyleColor()
enum ANCHOR_Col_ {
  ANCHOR_Col_Text,
  ANCHOR_Col_TextDisabled,
  ANCHOR_Col_WindowBg,  // Background of normal windows
  ANCHOR_Col_ChildBg,   // Background of child windows
  ANCHOR_Col_PopupBg,   // Background of popups, menus, tooltips windows
  ANCHOR_Col_Border,
  ANCHOR_Col_BorderShadow,
  ANCHOR_Col_FrameBg,  // Background of checkbox, radio button, plot, slider, text input
  ANCHOR_Col_FrameBgHovered,
  ANCHOR_Col_FrameBgActive,
  ANCHOR_Col_TitleBg,
  ANCHOR_Col_TitleBgActive,
  ANCHOR_Col_TitleBgCollapsed,
  ANCHOR_Col_MenuBarBg,
  ANCHOR_Col_ScrollbarBg,
  ANCHOR_Col_ScrollbarGrab,
  ANCHOR_Col_ScrollbarGrabHovered,
  ANCHOR_Col_ScrollbarGrabActive,
  ANCHOR_Col_CheckMark,
  ANCHOR_Col_SliderGrab,
  ANCHOR_Col_SliderGrabActive,
  ANCHOR_Col_Button,
  ANCHOR_Col_ButtonHovered,
  ANCHOR_Col_ButtonActive,
  ANCHOR_Col_Header,  // Header* colors are used for CollapsingHeader, TreeNode, Selectable,
                      // MenuItem
  ANCHOR_Col_HeaderHovered,
  ANCHOR_Col_HeaderActive,
  ANCHOR_Col_Separator,
  ANCHOR_Col_SeparatorHovered,
  ANCHOR_Col_SeparatorActive,
  ANCHOR_Col_ResizeGrip,
  ANCHOR_Col_ResizeGripHovered,
  ANCHOR_Col_ResizeGripActive,
  ANCHOR_Col_Tab,
  ANCHOR_Col_TabHovered,
  ANCHOR_Col_TabActive,
  ANCHOR_Col_TabUnfocused,
  ANCHOR_Col_TabUnfocusedActive,
  ANCHOR_Col_PlotLines,
  ANCHOR_Col_PlotLinesHovered,
  ANCHOR_Col_PlotHistogram,
  ANCHOR_Col_PlotHistogramHovered,
  ANCHOR_Col_TableHeaderBg,      // Table header background
  ANCHOR_Col_TableBorderStrong,  // Table outer and header borders (prefer using Alpha=1.0 here)
  ANCHOR_Col_TableBorderLight,   // Table inner borders (prefer using Alpha=1.0 here)
  ANCHOR_Col_TableRowBg,         // Table row background (even rows)
  ANCHOR_Col_TableRowBgAlt,      // Table row background (odd rows)
  ANCHOR_Col_TextSelectedBg,
  ANCHOR_Col_DragDropTarget,
  ANCHOR_Col_NavHighlight,           // Gamepad/keyboard: current highlighted item
  ANCHOR_Col_NavWindowingHighlight,  // Highlight window when using CTRL+TAB
  ANCHOR_Col_NavWindowingDimBg,      // Darken/colorize entire screen behind the CTRL+TAB window list,
                                     // when active
  ANCHOR_Col_ModalWindowDimBg,       // Darken/colorize entire screen behind a modal window, when one is
                                     // active
  ANCHOR_Col_COUNT
};

// Enumeration for PushStyleVar() / PopStyleVar() to temporarily modify the ANCHOR_Style structure.
// - The enum only refers to fields of ANCHOR_Style which makes sense to be pushed/popped inside UI
// code.
//   During initialization or between frames, feel free to just poke into ANCHOR_Style directly.
// - Tip: Use your programming IDE navigation facilities on the names in the _second column_ below
// to find the actual members and their description.
//   In Visual Studio IDE: CTRL+comma ("Edit.NavigateTo") can follow symbols in comments, whereas
//   CTRL+F12 ("Edit.GoToImplementation") cannot. With Visual Assist installed: ALT+G
//   ("VAssistX.GoToImplementation") can also follow symbols in comments.
// - When changing this enum, you need to update the associated internal table GStyleVarInfo[]
// accordingly. This is where we link enum values to members offset/type.
enum ANCHOR_StyleVar_ {
  // Enum name --------------------- // Member in ANCHOR_Style structure (see ANCHOR_Style for
  // descriptions)
  ANCHOR_StyleVar_Alpha,                // float     Alpha
  ANCHOR_StyleVar_WindowPadding,        // wabi::GfVec2f    WindowPadding
  ANCHOR_StyleVar_WindowRounding,       // float     WindowRounding
  ANCHOR_StyleVar_WindowBorderSize,     // float     WindowBorderSize
  ANCHOR_StyleVar_WindowMinSize,        // wabi::GfVec2f    WindowMinSize
  ANCHOR_StyleVar_WindowTitleAlign,     // wabi::GfVec2f    WindowTitleAlign
  ANCHOR_StyleVar_ChildRounding,        // float     ChildRounding
  ANCHOR_StyleVar_ChildBorderSize,      // float     ChildBorderSize
  ANCHOR_StyleVar_PopupRounding,        // float     PopupRounding
  ANCHOR_StyleVar_PopupBorderSize,      // float     PopupBorderSize
  ANCHOR_StyleVar_FramePadding,         // wabi::GfVec2f    FramePadding
  ANCHOR_StyleVar_FrameRounding,        // float     FrameRounding
  ANCHOR_StyleVar_FrameBorderSize,      // float     FrameBorderSize
  ANCHOR_StyleVar_ItemSpacing,          // wabi::GfVec2f    ItemSpacing
  ANCHOR_StyleVar_ItemInnerSpacing,     // wabi::GfVec2f    ItemInnerSpacing
  ANCHOR_StyleVar_IndentSpacing,        // float     IndentSpacing
  ANCHOR_StyleVar_CellPadding,          // wabi::GfVec2f    CellPadding
  ANCHOR_StyleVar_ScrollbarSize,        // float     ScrollbarSize
  ANCHOR_StyleVar_ScrollbarRounding,    // float     ScrollbarRounding
  ANCHOR_StyleVar_GrabMinSize,          // float     GrabMinSize
  ANCHOR_StyleVar_GrabRounding,         // float     GrabRounding
  ANCHOR_StyleVar_TabRounding,          // float     TabRounding
  ANCHOR_StyleVar_ButtonTextAlign,      // wabi::GfVec2f    ButtonTextAlign
  ANCHOR_StyleVar_SelectableTextAlign,  // wabi::GfVec2f    SelectableTextAlign
  ANCHOR_StyleVar_COUNT
};

// Flags for InvisibleButton() [extended in ANCHOR_internal.h]
enum ANCHOR_ButtonFlags_ {
  ANCHOR_ButtonFlags_None = 0,
  ANCHOR_ButtonFlags_MouseButtonLeft = 1 << 0,    // React on left mouse button (default)
  ANCHOR_ButtonFlags_MouseButtonRight = 1 << 1,   // React on right mouse button
  ANCHOR_ButtonFlags_MouseButtonMiddle = 1 << 2,  // React on center mouse button

  // [Internal]
  ANCHOR_ButtonFlags_MouseButtonMask_ = ANCHOR_ButtonFlags_MouseButtonLeft |
                                        ANCHOR_ButtonFlags_MouseButtonRight |
                                        ANCHOR_ButtonFlags_MouseButtonMiddle,
  ANCHOR_ButtonFlags_MouseButtonDefault_ = ANCHOR_ButtonFlags_MouseButtonLeft
};

// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() / ColorButton()
enum ANCHOR_ColorEditFlags_ {
  ANCHOR_ColorEditFlags_None = 0,
  ANCHOR_ColorEditFlags_NoAlpha =
    1 << 1,  //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will
             //              only read 3 components from the input pointer).
  ANCHOR_ColorEditFlags_NoPicker =
    1 << 2,  //              // ColorEdit: disable picker when clicking on color square.
  ANCHOR_ColorEditFlags_NoOptions = 1 << 3,  //              // ColorEdit: disable toggling options menu when
                                             //              right-clicking on inputs/small preview.
  ANCHOR_ColorEditFlags_NoSmallPreview =
    1 << 4,  //              // ColorEdit, ColorPicker: disable color square preview next to the
             //              inputs. (e.g. to show only the inputs)
  ANCHOR_ColorEditFlags_NoInputs =
    1 << 5,  //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g.
             //              to show only the small preview color square).
  ANCHOR_ColorEditFlags_NoTooltip = 1 << 6,  //              // ColorEdit, ColorPicker, ColorButton: disable
                                             //              tooltip when hovering the preview.
  ANCHOR_ColorEditFlags_NoLabel =
    1 << 7,  //              // ColorEdit, ColorPicker: disable display of inline text label (the
             //              label is still forwarded to the tooltip and picker).
  ANCHOR_ColorEditFlags_NoSidePreview =
    1 << 8,  //              // ColorPicker: disable bigger color preview on right side of the
             //              picker, use small color square preview instead.
  ANCHOR_ColorEditFlags_NoDragDrop = 1 << 9,  //              // ColorEdit: disable drag and drop target.
                                              //              ColorButton: disable drag and drop source.
  ANCHOR_ColorEditFlags_NoBorder =
    1 << 10,  //              // ColorButton: disable border (which is enforced by default)

  // User Options (right-click on widget to change some of them).
  ANCHOR_ColorEditFlags_AlphaBar = 1 << 16,  //              // ColorEdit, ColorPicker: show
                                             //              vertical alpha bar/gradient in picker.
  ANCHOR_ColorEditFlags_AlphaPreview =
    1 << 17,  //              // ColorEdit, ColorPicker, ColorButton: display preview as a
              //              transparent color over a checkerboard, instead of opaque.
  ANCHOR_ColorEditFlags_AlphaPreviewHalf =
    1 << 18,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half
              //              checkerboard, instead of opaque.
  ANCHOR_ColorEditFlags_HDR = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f
                                        //              limits in RGBA edition (note: you probably want to
                                        //              use ANCHOR_ColorEditFlags_Float flag as well).
  ANCHOR_ColorEditFlags_DisplayRGB =
    1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex.
              // ColorPicker: select any combination using one or more of RGB/HSV/Hex.
  ANCHOR_ColorEditFlags_DisplayHSV = 1 << 21,  // [Display]    // "
  ANCHOR_ColorEditFlags_DisplayHex = 1 << 22,  // [Display]    // "
  ANCHOR_ColorEditFlags_Uint8 = 1 << 23,       // [DataType]   // ColorEdit, ColorPicker, ColorButton:
                                               // _display_ values formatted as 0..255.
  ANCHOR_ColorEditFlags_Float =
    1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as
              // 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
  ANCHOR_ColorEditFlags_PickerHueBar =
    1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
  ANCHOR_ColorEditFlags_PickerHueWheel =
    1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
  ANCHOR_ColorEditFlags_InputRGB =
    1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
  ANCHOR_ColorEditFlags_InputHSV =
    1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

  // Defaults Options. You can set application defaults using SetColorEditOptions(). The intent is
  // that you probably don't want to override them in most of your calls. Let the user choose via
  // the option menu and/or call SetColorEditOptions() once during startup.
  ANCHOR_ColorEditFlags__OptionsDefault = ANCHOR_ColorEditFlags_Uint8 | ANCHOR_ColorEditFlags_DisplayRGB |
                                          ANCHOR_ColorEditFlags_InputRGB |
                                          ANCHOR_ColorEditFlags_PickerHueBar,

  // [Internal] Masks
  ANCHOR_ColorEditFlags__DisplayMask = ANCHOR_ColorEditFlags_DisplayRGB | ANCHOR_ColorEditFlags_DisplayHSV |
                                       ANCHOR_ColorEditFlags_DisplayHex,
  ANCHOR_ColorEditFlags__DataTypeMask = ANCHOR_ColorEditFlags_Uint8 | ANCHOR_ColorEditFlags_Float,
  ANCHOR_ColorEditFlags__PickerMask = ANCHOR_ColorEditFlags_PickerHueWheel |
                                      ANCHOR_ColorEditFlags_PickerHueBar,
  ANCHOR_ColorEditFlags__InputMask = ANCHOR_ColorEditFlags_InputRGB | ANCHOR_ColorEditFlags_InputHSV

// Obsolete names (will be removed)
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  ,
  ANCHOR_ColorEditFlags_RGB = ANCHOR_ColorEditFlags_DisplayRGB,
  ANCHOR_ColorEditFlags_HSV = ANCHOR_ColorEditFlags_DisplayHSV,
  ANCHOR_ColorEditFlags_HEX = ANCHOR_ColorEditFlags_DisplayHex  // [renamed in 1.69]
#  endif
};

// Flags for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
// We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the
// same and it makes it easier to swap them.
enum ANCHOR_SliderFlags_ {
  ANCHOR_SliderFlags_None = 0,
  ANCHOR_SliderFlags_AlwaysClamp = 1 << 4,  // Clamp value to min/max bounds when input manually with
                                            // CTRL+Click. By default CTRL+Click allows going out of bounds.
  ANCHOR_SliderFlags_Logarithmic = 1 << 5,  // Make the widget logarithmic (linear otherwise). Consider using
                                            // ANCHOR_SliderFlags_NoRoundToFormat with this if using a
                                            // format-string with small amount of digits.
  ANCHOR_SliderFlags_NoRoundToFormat =
    1 << 6,  // Disable rounding underlying value to match precision of the display format string
             // (e.g. %.3f values are rounded to those 3 digits)
  ANCHOR_SliderFlags_NoInput =
    1 << 7,  // Disable CTRL+Click or Enter key allowing to input text directly into the widget
  ANCHOR_SliderFlags_InvalidMask_ =
    0x7000000F  // [Internal] We treat using those bits as being potentially a 'float power'
                // argument from the previous API that has got miscast to this enum, and will
                // trigger an assert if needed.

// Obsolete names (will be removed)
#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  ,
  ANCHOR_SliderFlags_ClampOnInput = ANCHOR_SliderFlags_AlwaysClamp  // [renamed in 1.79]
#  endif
};

// Identify a mouse button.
// Those values are guaranteed to be stable and we frequently use 0/1 directly. Named enums
// provided for convenience.
enum ANCHOR_MouseButton_ {
  ANCHOR_MouseButton_Left = 0,
  ANCHOR_MouseButton_Right = 1,
  ANCHOR_MouseButton_Middle = 2,
  ANCHOR_MouseButton_COUNT = 5
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling SetMouseCursor(), which is why
// we have some cursors that are marked unused here
enum ANCHOR_MouseCursor_ {
  ANCHOR_MouseCursor_None = -1,
  ANCHOR_MouseCursor_Arrow = 0,
  ANCHOR_MouseCursor_TextInput,   // When hovering over InputText, etc.
  ANCHOR_MouseCursor_ResizeAll,   // (Unused by ANCHOR functions)
  ANCHOR_MouseCursor_ResizeNS,    // When hovering over an horizontal border
  ANCHOR_MouseCursor_ResizeEW,    // When hovering over a vertical border or a column
  ANCHOR_MouseCursor_ResizeNESW,  // When hovering over the bottom-left corner of a window
  ANCHOR_MouseCursor_ResizeNWSE,  // When hovering over the bottom-right corner of a window
  ANCHOR_MouseCursor_Hand,        // (Unused by ANCHOR functions. Use for e.g. hyperlinks)
  ANCHOR_MouseCursor_NotAllowed,  // When hovering something with disallowed interaction. Usually a
                                  // crossed circle.
  ANCHOR_MouseCursor_COUNT
};

// Enumeration for ANCHOR::SetWindow***(), SetNextWindow***(), SetNextItem***() functions
// Represent a condition.
// Important: Treat as a regular enum! Do NOT combine multiple values using binary operators! All
// the functions above treat 0 as a shortcut to ANCHOR_Cond_Always.
enum ANCHOR_Cond_ {
  ANCHOR_Cond_None = 0,         // No condition (always set the variable), same as _Always
  ANCHOR_Cond_Always = 1 << 0,  // No condition (always set the variable)
  ANCHOR_Cond_Once = 1 << 1,  // Set the variable once per runtime session (only the first call will succeed)
  ANCHOR_Cond_FirstUseEver = 1 << 2,  // Set the variable if the object/window has no persistently
                                      // saved data (no entry in .ini file)
  ANCHOR_Cond_Appearing = 1 << 3      // Set the variable if the object/window is appearing after being
                                      // hidden/inactive (or the first time)
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers: Memory allocations macros, AnchorVector<>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// IM_MALLOC(), IM_FREE(), IM_NEW(), IM_PLACEMENT_NEW(), IM_DELETE()
// We call C++ constructor on own allocated memory via the placement "new(ptr) Type()" syntax.
// Defining a custom placement new() with a custom parameter allows us to bypass including <new>
// which on some platforms complains when user has disabled exceptions.
//-----------------------------------------------------------------------------

struct AnchorNewWrapper {
};
inline void *operator new(size_t, AnchorNewWrapper, void *ptr)
{
  return ptr;
}
inline void operator delete(void *, AnchorNewWrapper, void *)
{}  // This is only required so we can use the symmetrical new()
#  define IM_ALLOC(_SIZE) ANCHOR::MemAlloc(_SIZE)
#  define IM_FREE(_PTR) ANCHOR::MemFree(_PTR)
#  define IM_PLACEMENT_NEW(_PTR) new (AnchorNewWrapper(), _PTR)
#  define IM_NEW(_TYPE) new (AnchorNewWrapper(), ANCHOR::MemAlloc(sizeof(_TYPE))) _TYPE
template<typename T> void IM_DELETE(T *p)
{
  if (p) {
    p->~T();
    ANCHOR::MemFree(p);
  }
}

//-----------------------------------------------------------------------------
// AnchorVector<>
// Lightweight std::vector<>-like class to avoid dragging dependencies (also, some implementations
// of STL with debug enabled are absurdly slow, we bypass it so our code runs fast in debug).
//-----------------------------------------------------------------------------
// - You generally do NOT need to care or use this ever. But we need to make it available in
// ANCHOR_api.h because some of our public structures are relying on it.
// - We use std-like naming convention here, which is a little unusual for this codebase.
// - Important: clear() frees memory, resize(0) keep the allocated buffer. We use resize(0) a lot
// to intentionally recycle allocated buffers across frames and amortize our costs.
// - Important: our implementation does NOT call C++ constructors/destructors, we treat everything
// as raw data! This is intentional but be extra mindful of that,
//   Do NOT use this class as a std::vector replacement in your own code! Many of the structures
//   used by ANCHOR can be safely initialized by a zero-memset.
//-----------------------------------------------------------------------------

ANCHOR_MSVC_RUNTIME_CHECKS_OFF
template<typename T> struct AnchorVector {
  int Size;
  int Capacity;
  T *Data;

  // Provide standard typedefs but we don't use them ourselves.
  typedef T value_type;
  typedef value_type *iterator;
  typedef const value_type *const_iterator;

  // Constructors, destructor
  inline AnchorVector()
  {
    Size = Capacity = 0;
    Data = NULL;
  }
  inline AnchorVector(const AnchorVector<T> &src)
  {
    Size = Capacity = 0;
    Data = NULL;
    operator=(src);
  }
  inline AnchorVector<T> &operator=(const AnchorVector<T> &src)
  {
    clear();
    resize(src.Size);
    memcpy(Data, src.Data, (size_t)Size * sizeof(T));
    return *this;
  }
  inline ~AnchorVector()
  {
    if (Data)
      IM_FREE(Data);
  }

  inline bool empty() const
  {
    return Size == 0;
  }
  inline int size() const
  {
    return Size;
  }
  inline int size_in_bytes() const
  {
    return Size * (int)sizeof(T);
  }
  inline int max_size() const
  {
    return 0x7FFFFFFF / (int)sizeof(T);
  }
  inline int capacity() const
  {
    return Capacity;
  }
  inline T &operator[](int i)
  {
    ANCHOR_ASSERT(i >= 0 && i < Size);
    return Data[i];
  }
  inline const T &operator[](int i) const
  {
    ANCHOR_ASSERT(i >= 0 && i < Size);
    return Data[i];
  }

  inline void clear()
  {
    if (Data) {
      Size = Capacity = 0;
      IM_FREE(Data);
      Data = NULL;
    }
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
    return Data + Size;
  }
  inline const T *end() const
  {
    return Data + Size;
  }
  inline T &front()
  {
    ANCHOR_ASSERT(Size > 0);
    return Data[0];
  }
  inline const T &front() const
  {
    ANCHOR_ASSERT(Size > 0);
    return Data[0];
  }
  inline T &back()
  {
    ANCHOR_ASSERT(Size > 0);
    return Data[Size - 1];
  }
  inline const T &back() const
  {
    ANCHOR_ASSERT(Size > 0);
    return Data[Size - 1];
  }
  inline void swap(AnchorVector<T> &rhs)
  {
    int rhs_size = rhs.Size;
    rhs.Size = Size;
    Size = rhs_size;
    int rhs_cap = rhs.Capacity;
    rhs.Capacity = Capacity;
    Capacity = rhs_cap;
    T *rhs_data = rhs.Data;
    rhs.Data = Data;
    Data = rhs_data;
  }

  inline int _grow_capacity(int sz) const
  {
    int new_capacity = Capacity ? (Capacity + Capacity / 2) : 8;
    return new_capacity > sz ? new_capacity : sz;
  }
  inline void resize(int new_size)
  {
    if (new_size > Capacity)
      reserve(_grow_capacity(new_size));
    Size = new_size;
  }
  inline void resize(int new_size, const T &v)
  {
    if (new_size > Capacity)
      reserve(_grow_capacity(new_size));
    if (new_size > Size)
      for (int n = Size; n < new_size; n++)
        memcpy(&Data[n], &v, sizeof(v));
    Size = new_size;
  }
  inline void shrink(int new_size)
  {
    ANCHOR_ASSERT(new_size <= Size);
    Size = new_size;
  }  // Resize a vector to a smaller size, guaranteed not to cause a reallocation
  inline void reserve(int new_capacity)
  {
    if (new_capacity <= Capacity)
      return;
    T *new_data = (T *)IM_ALLOC((size_t)new_capacity * sizeof(T));
    if (Data) {
      memcpy(new_data, Data, (size_t)Size * sizeof(T));
      IM_FREE(Data);
    }
    Data = new_data;
    Capacity = new_capacity;
  }

  // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the
  // AnchorVector data itself! e.g. v.push_back(v[10]) is forbidden.
  inline void push_back(const T &v)
  {
    if (Size == Capacity)
      reserve(_grow_capacity(Size + 1));
    memcpy(&Data[Size], &v, sizeof(v));
    Size++;
  }
  inline void pop_back()
  {
    ANCHOR_ASSERT(Size > 0);
    Size--;
  }
  inline void push_front(const T &v)
  {
    if (Size == 0)
      push_back(v);
    else
      insert(Data, v);
  }
  inline T *erase(const T *it)
  {
    ANCHOR_ASSERT(it >= Data && it < Data + Size);
    const ptrdiff_t off = it - Data;
    memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(T));
    Size--;
    return Data + off;
  }
  inline T *erase(const T *it, const T *it_last)
  {
    ANCHOR_ASSERT(it >= Data && it < Data + Size && it_last > it && it_last <= Data + Size);
    const ptrdiff_t count = it_last - it;
    const ptrdiff_t off = it - Data;
    memmove(Data + off, Data + off + count, ((size_t)Size - (size_t)off - count) * sizeof(T));
    Size -= (int)count;
    return Data + off;
  }
  inline T *erase_unsorted(const T *it)
  {
    ANCHOR_ASSERT(it >= Data && it < Data + Size);
    const ptrdiff_t off = it - Data;
    if (it < Data + Size - 1)
      memcpy(Data + off, Data + Size - 1, sizeof(T));
    Size--;
    return Data + off;
  }
  inline T *insert(const T *it, const T &v)
  {
    ANCHOR_ASSERT(it >= Data && it <= Data + Size);
    const ptrdiff_t off = it - Data;
    if (Size == Capacity)
      reserve(_grow_capacity(Size + 1));
    if (off < (int)Size)
      memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(T));
    memcpy(&Data[off], &v, sizeof(v));
    Size++;
    return Data + off;
  }
  inline bool contains(const T &v) const
  {
    const T *data = Data;
    const T *data_end = Data + Size;
    while (data < data_end)
      if (*data++ == v)
        return true;
    return false;
  }
  inline T *find(const T &v)
  {
    T *data = Data;
    const T *data_end = Data + Size;
    while (data < data_end)
      if (*data == v)
        break;
      else
        ++data;
    return data;
  }
  inline const T *find(const T &v) const
  {
    const T *data = Data;
    const T *data_end = Data + Size;
    while (data < data_end)
      if (*data == v)
        break;
      else
        ++data;
    return data;
  }
  inline bool find_erase(const T &v)
  {
    const T *it = find(v);
    if (it < Data + Size) {
      erase(it);
      return true;
    }
    return false;
  }
  inline bool find_erase_unsorted(const T &v)
  {
    const T *it = find(v);
    if (it < Data + Size) {
      erase_unsorted(it);
      return true;
    }
    return false;
  }
  inline int index_from_ptr(const T *it) const
  {
    ANCHOR_ASSERT(it >= Data && it < Data + Size);
    const ptrdiff_t off = it - Data;
    return (int)off;
  }
};
ANCHOR_MSVC_RUNTIME_CHECKS_RESTORE

//-----------------------------------------------------------------------------
// [SECTION] ANCHOR_Style
//-----------------------------------------------------------------------------
// You may modify the ANCHOR::GetStyle() main instance during initialization and before NewFrame().
// During the frame, use ANCHOR::PushStyleVar(ANCHOR_StyleVar_XXXX)/PopStyleVar() to alter the main
// style values, and ANCHOR::PushStyleColor(ANCHOR_Col_XXX)/PopStyleColor() for colors.
//-----------------------------------------------------------------------------

struct ANCHOR_Style {
  float Alpha;                  // Global alpha applies to everything in ANCHOR.
  wabi::GfVec2f WindowPadding;  // Padding within a window.
  float WindowRounding;         // Radius of window corners rounding. Set to 0.0f to have rectangular
                                // windows. Large values tend to lead to variety of artifacts and are not
                                // recommended.
  float WindowBorderSize;       // Thickness of border around windows. Generally set to 0.0f or 1.0f.
                                // (Other values are not well tested and more CPU/GPU costly).
  wabi::GfVec2f WindowMinSize;  // Minimum window size. This is a global setting. If you want to constraint
                                // individual windows, use SetNextWindowSizeConstraints().
  wabi::GfVec2f WindowTitleAlign;       // Alignment for title bar text. Defaults to (0.0f,0.5f) for
                                        // left-aligned,vertically centered.
  ANCHOR_Dir WindowMenuButtonPosition;  // Side of the collapsing/docking button in the title bar
                                        // (None/Left/Right). Defaults to ANCHOR_Dir_Left.
  float ChildRounding;         // Radius of child window corners rounding. Set to 0.0f to have rectangular
                               // windows.
  float ChildBorderSize;       // Thickness of border around child windows. Generally set to 0.0f
                               // or 1.0f. (Other values are not well tested and more CPU/GPU costly).
  float PopupRounding;         // Radius of popup window corners rounding. (Note that tooltip windows use
                               // WindowRounding)
  float PopupBorderSize;       // Thickness of border around popup/tooltip windows. Generally set to 0.0f
                               // or 1.0f. (Other values are not well tested and more CPU/GPU costly).
  wabi::GfVec2f FramePadding;  // Padding within a framed rectangle (used by most widgets).
  float FrameRounding;         // Radius of frame corners rounding. Set to 0.0f to have rectangular frame
                               // (used by most widgets).
  float FrameBorderSize;       // Thickness of border around frames. Generally set to 0.0f or 1.0f.
                               // (Other values are not well tested and more CPU/GPU costly).
  wabi::GfVec2f ItemSpacing;   // Horizontal and vertical spacing between widgets/lines.
  wabi::GfVec2f ItemInnerSpacing;   // Horizontal and vertical spacing between within elements of a
                                    // composed widget (e.g. a slider and its label).
  wabi::GfVec2f CellPadding;        // Padding within a table cell
  wabi::GfVec2f TouchExtraPadding;  // Expand reactive bounding box for touch-based system where
                                    // touch position is not accurate enough. Unfortunately we
                                    // don't sort widgets so priority on overlap will always be
                                    // given to the first widget. So don't grow this too much!
  float IndentSpacing;              // Horizontal indentation when e.g. entering a tree node. Generally ==
                                    // (FontSize + FramePadding[0]*2).
  float ColumnsMinSpacing;          // Minimum horizontal spacing between two columns. Preferably >
                                    // (FramePadding[0] + 1).
  float ScrollbarSize;              // Width of the vertical scrollbar, Height of the horizontal scrollbar.
  float ScrollbarRounding;          // Radius of grab corners for scrollbar.
  float GrabMinSize;                // Minimum width/height of a grab box for slider/scrollbar.
  float GrabRounding;       // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider
                            // grabs.
  float LogSliderDeadzone;  // The size in pixels of the dead-zone around zero on logarithmic
                            // sliders that cross zero.
  float TabRounding;        // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
  float TabBorderSize;      // Thickness of border around tabs.
  float TabMinWidthForCloseButton;     // Minimum width for close button to appears on an unselected
                                       // tab when hovered. Set to 0.0f to always show when hovering,
                                       // set to FLT_MAX to never show close button unless selected.
  ANCHOR_Dir ColorButtonPosition;      // Side of the color button in the ColorEdit4 widget
                                       // (left/right). Defaults to ANCHOR_Dir_Right.
  wabi::GfVec2f ButtonTextAlign;       // Alignment of button text when button is larger than text.
                                       // Defaults to (0.5f, 0.5f) (centered).
  wabi::GfVec2f SelectableTextAlign;   // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left
                                       // aligned). It's generally important to keep this left-aligned if you
                                       // want to lay multiple items on a same line.
  wabi::GfVec2f DisplayWindowPadding;  // Window position are clamped to be visible within the
                                       // display area or monitors by at least this amount. Only
                                       // applies to regular windows.
  wabi::GfVec2f DisplaySafeAreaPadding;  // If you cannot see the edges of your screen (e.g. on a TV)
                                         // increase the safe area padding. Apply to popups/tooltips as well
                                         // regular windows. NB: Prefer configuring your TV sets correctly!
  float MouseCursorScale;                // Scale software rendered mouse cursor (when io.MouseDrawCursor is
                                         // enabled). May be removed later.
  bool AntiAliasedLines;        // Enable anti-aliased lines/borders. Disable if you are really tight on
                                // CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
  bool AntiAliasedLinesUseTex;  // Enable anti-aliased lines/borders using textures where possible.
                                // Require backend to render with bilinear filtering. Latched at
                                // the beginning of the frame (copied to ImDrawList).
  bool AntiAliasedFill;         // Enable anti-aliased edges around filled shapes (rounded rectangles,
                                // circles, etc.). Disable if you are really tight on CPU/GPU. Latched at
                                // the beginning of the frame (copied to ImDrawList).
  float CurveTessellationTol;   // Tessellation tolerance when using PathBezierCurveTo() without a
                                // specific number of segments. Decrease for highly tessellated curves
                                // (higher quality, more polygons), increase to reduce quality.
  float CircleTessellationMaxError;  // Maximum error (in pixels) allowed when using
                                     // AddCircle()/AddCircleFilled() or drawing rounded corner
                                     // rectangles with no explicit segment count specified.
                                     // Decrease for higher quality but more geometry.
  wabi::GfVec4f Colors[ANCHOR_Col_COUNT];

  ANCHOR_API ANCHOR_Style();
  ANCHOR_API void ScaleAllSizes(float scale_factor);
};

//-----------------------------------------------------------------------------
// [SECTION] ANCHOR_IO
//-----------------------------------------------------------------------------
// Communicate most settings and inputs/outputs to ANCHOR using this structure.
// Access via ANCHOR::GetIO(). Read 'Programmer guide' section in .cpp file for general usage.
//-----------------------------------------------------------------------------

enum eCovahFonts { FONT_FALLBACK, FONT_GOTHAM, FONT_DANKMONO, FONT_SANFRANCISCO };

struct ANCHOR_IO {
  //------------------------------------------------------------------
  // Configuration (fill once)                // Default value
  //------------------------------------------------------------------

  ANCHORConfigFlags ConfigFlags;    // = 0              // See ANCHORConfigFlags_ enum. Set by
                                    // user/application. Gamepad/keyboard navigation options, etc.
  ANCHORBackendFlags BackendFlags;  // = 0              // See ANCHORBackendFlags_ enum. Set by
                                    // backend (ANCHOR_impl_xxx files or custom backend) to
                                    // communicate features supported by the backend.
  wabi::GfVec2f DisplaySize;        // <unset>          // Main display size, in pixels (generally ==
                                    // GetMainViewport()->Size)
  float DeltaTime;                  // = 1.0f/60.0f     // Time elapsed since last frame, in seconds.
  float IniSavingRate;         // = 5.0f           // Minimum time between saving positions/sizes to .ini
                               // file, in seconds.
  const char *IniFilename;     // = "ANCHOR.ini"    // Path to .ini file. Set NULL to disable automatic .ini
                               // loading/saving, if e.g. you want to manually load/save from memory.
  const char *LogFilename;     // = "ANCHOR_log.txt"// Path to .log file (default parameter to
                               // ANCHOR::LogToFile when no file is specified).
  float MouseDoubleClickTime;  // = 0.30f          // Time for a double-click, in seconds.
  float MouseDoubleClickMaxDist;  // = 6.0f           // Distance threshold to stay in to validate
                                  // a double-click, in pixels.
  float MouseDragThreshold;       // = 6.0f           // Distance threshold before considering we are
                                  // dragging.
  int KeyMap[ANCHOR_Key_COUNT];   // <unset>          // Map of indices into the KeysDown[512]
                                  // entries array which represent your "native" keyboard state.
  float KeyRepeatDelay;           // = 0.250f         // When holding a key/button, time before it starts
                                  // repeating, in seconds (for buttons in Repeat mode, etc.).
  float KeyRepeatRate;            // = 0.050f         // When holding a key/button, rate at which it repeats,
                                  // in seconds.
  void *UserData;                 // = NULL           // Store your own data for retrieval by callbacks.

  AnchorFontAtlas *Fonts;     // <auto>           // Font atlas: load, rasterize and pack one or more
                              // fonts into a single texture.
  float FontGlobalScale;      // = 1.0f           // Global scale all fonts
  bool FontAllowUserScaling;  // = false          // Allow user scaling text of individual window
                              // with CTRL+Wheel.
  AnchorFont *FontDefault;    // = NULL           // Font to use on NewFrame(). Use NULL to uses
                              // Fonts->Fonts[0].
  wabi::GfVec2f DisplayFramebufferScale;  // = (1, 1)         // For retina display or other situations where
                                          // window coordinates are different from framebuffer coordinates.
                                          // This generally ends up in ImDrawData::FramebufferScale.

  // Miscellaneous options
  bool MouseDrawCursor;        // = false          // Request ANCHOR to draw a mouse cursor for you (if
                               // you are on a platform without a mouse cursor). Cannot be easily renamed
                               // to 'io.ConfigXXX' because this is frequently used by backend
                               // implementations.
  bool ConfigMacOSXBehaviors;  // = defined(__APPLE__) // OS X style: Text editing cursor movement
                               // using Alt instead of Ctrl, Shortcuts using Cmd/Super instead of
                               // Ctrl, Line/Text Start and End using Cmd+Arrows instead of
                               // Home/End, Double click selects by word instead of selecting whole
                               // text, Multi-selection in lists uses Cmd/Super instead of Ctrl.
  bool ConfigInputTextCursorBlink;         // = true           // Enable blinking cursor (optional as some
                                           // users consider it to be distracting).
  bool ConfigDragClickToInputText;         // = false          // [BETA] Enable turning DragXXX widgets
                                           // into text input with a simple mouse click-release (without
                                           // moving). Not desirable on devices without a keyboard.
  bool ConfigWindowsResizeFromEdges;       // = true           // Enable resizing of windows from their
                                           // edges and from the lower-left corner. This requires
                                           // (io.BackendFlags & ANCHORBackendFlags_HasMouseCursors)
                                           // because it needs mouse cursor feedback. (This used to be a
                                           // per-window ANCHOR_WindowFlags_ResizeFromAnySide flag)
  bool ConfigWindowsMoveFromTitleBarOnly;  // = false       // Enable allowing to move windows only
                                           // when clicking on their title bar. Does not apply to
                                           // windows without a title bar.
  float ConfigMemoryCompactTimer;          // = 60.0f          // Timer (in seconds) to free transient
                                           // windows/tables memory buffers when unused. Set to -1.0f to
                                           // disable.

  //------------------------------------------------------------------
  // Platform Functions
  // (the ANCHOR_impl_xxxx backend files are setting those up for you)
  //------------------------------------------------------------------

  // Optional: Platform/Renderer backend name (informational only! will be displayed in About
  // Window) + User data for backend/wrappers to store their own stuff.
  const char *BackendPlatformName;  // = NULL
  const char *BackendRendererName;  // = NULL
  void *BackendPlatformUserData;    // = NULL           // User data for platform backend
  void *BackendRendererUserData;    // = NULL           // User data for renderer backend
  void *BackendLanguageUserData;    // = NULL           // User data for non C++ programming language
                                    // backend

  // Optional: Access OS clipboard
  // (default to use native Win32 clipboard on Windows, otherwise uses a private clipboard.
  // Override to access OS clipboard on other architectures)
  const char *(*GetClipboardTextFn)(void *user_data);
  void (*SetClipboardTextFn)(void *user_data, const char *text);
  void *ClipboardUserData;

  // Optional: Notify OS Input Method Editor of the screen position of your cursor for text input
  // position (e.g. when using Japanese/Chinese IME on Windows) (default to use native imm32 api on
  // Windows)
  void (*ImeSetInputScreenPosFn)(int x, int y);
  void *ImeWindowHandle;  // = NULL           // (Windows) Set this to your HWND to get automatic
                          // IME cursor positioning.

  //------------------------------------------------------------------
  // Input - Fill before calling NewFrame()
  //------------------------------------------------------------------

  wabi::GfVec2f MousePos;  // Mouse position, in pixels. Set to wabi::GfVec2f(-FLT_MAX, -FLT_MAX)
                           // if mouse is unavailable (on another screen, etc.)
  bool MouseDown[5];       // Mouse buttons: 0=left, 1=right, 2=middle + extras (ANCHOR_MouseButton_COUNT
                           // == 5). ANCHOR mostly uses left and right buttons. Others buttons allows us
                           // to track if the mouse is being used by your application + available to
                           // user as a convenience via IsMouse** API.
  float MouseWheel;        // Mouse wheel Vertical: 1 unit scrolls about 5 lines text.
  float MouseWheelH;       // Mouse wheel Horizontal. Most users don't have a mouse with an horizontal
                           // wheel, may not be filled by all backends.
  bool KeyCtrl;            // Keyboard modifier pressed: Control
  bool KeyShift;           // Keyboard modifier pressed: Shift
  bool KeyAlt;             // Keyboard modifier pressed: Alt
  bool KeySuper;           // Keyboard modifier pressed: Cmd/Super/Windows
  bool KeysDown[512];      // Keyboard keys that are pressed (ideally left in the "native" order your
                           // engine has access to keyboard keys, so you can use your own defines/enums
                           // for keys).
  float NavInputs[ANCHOR_NavInput_COUNT];  // Gamepad inputs. Cleared back to zero by EndFrame().
                                           // Keyboard keys will be auto-mapped and be written here
                                           // by NewFrame().

  // Functions
  ANCHOR_API void AddInputCharacter(unsigned int c);  // Queue new character input
  ANCHOR_API void AddInputCharacterUTF16(
    AnchorWChar16 c);  // Queue new character input from an UTF-16 character, it can be a surrogate
  ANCHOR_API void AddInputCharactersUTF8(
    const char *str);                      // Queue new characters input from an UTF-8 string
  ANCHOR_API void ClearInputCharacters();  // Clear the text input buffer manually

  //------------------------------------------------------------------
  // Output - Updated by NewFrame() or EndFrame()/Render()
  // (when reading from the io.WantCaptureMouse, io.WantCaptureKeyboard flags to dispatch your
  // inputs, it is
  //  generally easier and more correct to use their state BEFORE calling NewFrame(). See FAQ for
  //  details!)
  //------------------------------------------------------------------

  bool WantCaptureMouse;     // Set when ANCHOR will use mouse inputs, in this case do not dispatch
                             // them to your main game/application (either way, always pass on mouse
                             // inputs to ANCHOR). (e.g. unclicked mouse is hovering over an ANCHOR
                             // window, widget is active, mouse was clicked over an ANCHOR window,
                             // etc.).
  bool WantCaptureKeyboard;  // Set when ANCHOR will use keyboard inputs, in this case do not
                             // dispatch them to your main game/application (either way, always
                             // pass keyboard inputs to ANCHOR). (e.g. InputText active, or an
                             // ANCHOR window is focused and navigation is enabled, etc.).
  bool WantTextInput;        // Mobile/console: when set, you may display an on-screen keyboard. This is
                             // set by ANCHOR when it wants textual keyboard input to happen (e.g. when a
                             // InputText widget is active).
  bool WantSetMousePos;      // MousePos has been altered, backend should reposition mouse on next
                             // frame. Rarely used! Set only when
                             // ANCHORConfigFlags_NavEnableSetMousePos flag is enabled.
  bool WantSaveIniSettings;  // When manual .ini load/save is active (io.IniFilename == NULL), this
                             // will be set to notify your application that you can call
                             // SaveIniSettingsToMemory() and save yourself. Important: clear
                             // io.WantSaveIniSettings yourself after saving!
  bool NavActive;            // Keyboard/Gamepad navigation is currently allowed (will handle
                             // ANCHOR_Key_NavXXX events) = a window is focused and it doesn't use the
                             // ANCHOR_WindowFlags_NoNavInputs flag.
  bool NavVisible;           // Keyboard/Gamepad navigation is visible and allowed (will handle
                             // ANCHOR_Key_NavXXX events).
  float Framerate;           // Rough estimate of application framerate, in frame per second. Solely for
                    // convenience. Rolling average estimation based on io.DeltaTime over 120 frames.
  int MetricsRenderVertices;     // Vertices output during last call to Render()
  int MetricsRenderIndices;      // Indices output during last call to Render() = number of triangles *
                                 // 3
  int MetricsRenderWindows;      // Number of visible windows
  int MetricsActiveWindows;      // Number of active windows
  int MetricsActiveAllocations;  // Number of active allocations, updated by MemAlloc/MemFree based
                                 // on current context. May be off if you have multiple ANCHOR
                                 // contexts.
  wabi::GfVec2f MouseDelta;      // Mouse delta. Note that this is zero if either current or previous
                                 // position are invalid (-FLT_MAX,-FLT_MAX), so a
                                 // disappearing/reappearing mouse won't have a huge delta.

  //------------------------------------------------------------------
  // [Internal] ANCHOR will maintain those fields. Forward compatibility not guaranteed!
  //------------------------------------------------------------------

  ANCHOR_KeyModFlags KeyMods;        // Key mods flags (same as io.KeyCtrl/KeyShift/KeyAlt/KeySuper but
                                     // merged into flags), updated by NewFrame()
  wabi::GfVec2f MousePosPrev;        // Previous mouse position (note that MouseDelta is not necessary ==
                                     // MousePos-MousePosPrev, in case either position is invalid)
  wabi::GfVec2f MouseClickedPos[5];  // Position at time of clicking
  double MouseClickedTime[5];        // Time of last click (used to figure out double-click)
  bool MouseClicked[5];              // Mouse button went from !Down to Down
  bool MouseDoubleClicked[5];        // Has mouse button been double-clicked?
  bool MouseReleased[5];             // Mouse button went from Down to !Down
  bool MouseDownOwned[5];            // Track if button was clicked inside a ANCHOR window. We don't request
                                     // mouse capture from the application if click started outside ANCHOR
                                     // bounds.
  bool MouseDownWasDoubleClick[5];   // Track if button down was a double-click
  float MouseDownDuration[5];        // Duration the mouse button has been down (0.0f == just clicked)
  float MouseDownDurationPrev[5];    // Previous time the mouse button has been down
  wabi::GfVec2f MouseDragMaxDistanceAbs[5];  // Maximum distance, absolute, on each axis, of how
                                             // much mouse has traveled from the clicking point
  float MouseDragMaxDistanceSqr[5];          // Squared maximum distance of how much mouse has traveled
                                             // from the clicking point
  float KeysDownDuration[512];      // Duration the keyboard key has been down (0.0f == just pressed)
  float KeysDownDurationPrev[512];  // Previous duration the key has been down
  float NavInputsDownDuration[ANCHOR_NavInput_COUNT];
  float NavInputsDownDurationPrev[ANCHOR_NavInput_COUNT];
  float PenPressure;  // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0]
                      // == true). Helper storage currently unused by ANCHOR.
  AnchorWChar16 InputQueueSurrogate;               // For AddInputCharacterUTF16
  AnchorVector<AnchorWChar> InputQueueCharacters;  // Queue of _characters_ input (obtained by platform
                                                   // backend). Fill using AddInputCharacter() helper.

  ANCHOR_API ANCHOR_IO();
};

//-----------------------------------------------------------------------------
// [SECTION] Misc data structures
//-----------------------------------------------------------------------------

// Shared state of InputText(), passed as an argument to your callback when a
// ANCHORInputTextFlags_Callback* flag is used. The callback function should return 0 by default.
// Callbacks (follow a flag name and see comments in ANCHORInputTextFlags_ declarations for more
// details)
// - ANCHORInputTextFlags_CallbackEdit:        Callback on buffer edit (note that InputText()
// already returns true on edit, the callback is useful mainly to manipulate the underlying buffer
// while focus is active)
// - ANCHORInputTextFlags_CallbackAlways:      Callback on each iteration
// - ANCHORInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - ANCHORInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// - ANCHORInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard
// them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// - ANCHORInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond
// 'buf_size' parameter value), allowing the string to grow.
struct ANCHORInputTextCallbackData {
  ANCHORInputTextFlags EventFlag;  // One ANCHORInputTextFlags_Callback*    // Read-only
  ANCHORInputTextFlags Flags;      // What user passed to InputText()      // Read-only
  void *UserData;                  // What user passed to InputText()      // Read-only

  // Arguments for the different callback events
  // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars()
  // function. InsertChars() will take care of calling the resize callback if necessary.
  // - If you know your edits are not going to resize the underlying buffer allocation, you may
  // modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <=
  // BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
  AnchorWChar EventChar;  // Character input                      // Read-write   // [CharFilter]
                          // Replace character with another one, or set to zero to drop. return 1
                          // is equivalent to setting EventChar=0;
  ANCHOR_Key EventKey;    // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
  char *Buf;              // Text buffer                          // Read-write   // [Resize] Can replace
                          // pointer / [Completion,History,Always] Only write to pointed data, don't replace
                          // the actual pointer!
  int BufTextLen;         // Text length (in bytes)               // Read-write   //
                          // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C
                          // land: == strlen(some_text), in C++ land: string.length()
  int BufSize;            // Buffer size (in bytes) = capacity+1  // Read-only    //
                          // [Resize,Completion,History,Always] Include zero-terminator storage. In C land ==
                          // ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
  bool BufDirty;          // Set if you modify Buf/BufTextLen!    // Write        //
                          // [Completion,History,Always]
  int CursorPos;          //                                      // Read-write   //
                          //                                      [Completion,History,Always]
  int SelectionStart;     //                                      // Read-write   //
                          //                                      [Completion,History,Always] == to
                          //                                      SelectionEnd when no selection)
  int SelectionEnd;       //                                      // Read-write   //
                          //                                      [Completion,History,Always]

  // Helper functions for text manipulation.
  // Use those function to benefit from the CallbackResize behaviors. Calling those function reset
  // the selection.
  ANCHOR_API ANCHORInputTextCallbackData();
  ANCHOR_API void DeleteChars(int pos, int bytes_count);
  ANCHOR_API void InsertChars(int pos, const char *text, const char *text_end = NULL);
  void SelectAll()
  {
    SelectionStart = 0;
    SelectionEnd = BufTextLen;
  }
  void ClearSelection()
  {
    SelectionStart = SelectionEnd = BufTextLen;
  }
  bool HasSelection() const
  {
    return SelectionStart != SelectionEnd;
  }
};

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints().
// Callback is called during the next Begin(). NB: For basic min/max size constraint on each axis
// you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
struct ANCHOR_SizeCallbackData {
  void *UserData;             // Read-only.   What user passed to SetNextWindowSizeConstraints()
  wabi::GfVec2f Pos;          // Read-only.   Window position, for reference.
  wabi::GfVec2f CurrentSize;  // Read-only.   Current window size.
  wabi::GfVec2f DesiredSize;  // Read-write.  Desired size, based on user's mouse position. Write
                              // to this field to restrain resizing.
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(), GetDragDropPayload()
struct ANCHORPayload {
  // Members
  void *Data;    // Data (copied and owned by ANCHOR)
  int DataSize;  // Data size

  // [Internal]
  ANCHOR_ID SourceId;        // Source item id
  ANCHOR_ID SourceParentId;  // Source parent id (if available)
  int DataFrameCount;        // Data timestamp
  char DataType[32 + 1];     // Data type tag (short user-supplied string, 32 characters max)
  bool Preview;              // Set when AcceptDragDropPayload() was called and mouse has been hovering the
                             // target item (nb: handle overlapping drag targets)
  bool Delivery;             // Set when AcceptDragDropPayload() was called and mouse button is released over
                             // the target item.

  ANCHORPayload()
  {
    Clear();
  }
  void Clear()
  {
    SourceId = SourceParentId = 0;
    Data = NULL;
    DataSize = 0;
    memset(DataType, 0, sizeof(DataType));
    DataFrameCount = -1;
    Preview = Delivery = false;
  }
  bool IsDataType(const char *type) const
  {
    return DataFrameCount != -1 && strcmp(type, DataType) == 0;
  }
  bool IsPreview() const
  {
    return Preview;
  }
  bool IsDelivery() const
  {
    return Delivery;
  }
};

// Sorting specification for one column of a table (sizeof == 12 bytes)
struct ANCHOR_TableColumnSortSpecs {
  ANCHOR_ID ColumnUserID;  // User id of the column (if specified by a TableSetupColumn() call)
  AnchorS16 ColumnIndex;   // Index of the column
  AnchorS16 SortOrder;     // Index within parent ANCHOR_TableSortSpecs (always stored in order starting
                           // from 0, tables sorted on a single criteria will always have a 0 here)
  ANCHOR_SortDirection SortDirection : 8;  // ANCHOR_SortDirection_Ascending or
                                           // ANCHOR_SortDirection_Descending (you can use this or SortSign,
                                           // whichever is more convenient for your sort function)

  ANCHOR_TableColumnSortSpecs()
  {
    memset(this, 0, sizeof(*this));
  }
};

// Sorting specifications for a table (often handling sort specs for a single column, occasionally
// more) Obtained by calling TableGetSortSpecs(). When 'SpecsDirty == true' you can sort your data.
// It will be true with sorting specs have changed since last call, or the first time. Make sure to
// set 'SpecsDirty = false' after sorting, else you may wastefully sort your data every frame!
struct ANCHOR_TableSortSpecs {
  const ANCHOR_TableColumnSortSpecs *Specs;  // Pointer to sort spec array.
  int SpecsCount;   // Sort spec count. Most often 1. May be > 1 when ANCHOR_TableFlags_SortMulti is
                    // enabled. May be == 0 when ANCHOR_TableFlags_SortTristate is enabled.
  bool SpecsDirty;  // Set to true when specs have changed since last time! Use this to sort again,
                    // then clear the flag.

  ANCHOR_TableSortSpecs()
  {
    memset(this, 0, sizeof(*this));
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Helpers (ANCHOROnceUponAFrame, ANCHORTextFilter, ANCHORTextBuffer, ANCHORStorage,
// ANCHORListClipper, AnchorColor)
//-----------------------------------------------------------------------------

// Helper: Unicode defines
#  define IM_UNICODE_CODEPOINT_INVALID 0xFFFD  // Invalid Unicode code point (standard value).
#  ifdef ANCHOR_USE_WCHAR32
#    define IM_UNICODE_CODEPOINT_MAX 0x10FFFF  // Maximum Unicode code point supported by this build.
#  else
#    define IM_UNICODE_CODEPOINT_MAX 0xFFFF  // Maximum Unicode code point supported by this build.
#  endif

// Helper: Execute a block of code at maximum once a frame. Convenient if you want to quickly
// create an UI within deep-nested code that runs multiple times every frame. Usage: static
// ANCHOROnceUponAFrame oaf; if (oaf) ANCHOR::Text("This will be called only once per frame");
struct ANCHOROnceUponAFrame {
  ANCHOROnceUponAFrame()
  {
    RefFrame = -1;
  }
  mutable int RefFrame;
  operator bool() const
  {
    int current_frame = ANCHOR::GetFrameCount();
    if (RefFrame == current_frame)
      return false;
    RefFrame = current_frame;
    return true;
  }
};

// Helper: Parse and apply text filters. In format "aaaaa[,bbbb][,ccccc]"
struct ANCHORTextFilter {
  ANCHOR_API ANCHORTextFilter(const char *default_filter = "");
  ANCHOR_API bool Draw(const char *label = "Filter (inc,-exc)",
                       float width = 0.0f);  // Helper calling InputText+Build
  ANCHOR_API bool PassFilter(const char *text, const char *text_end = NULL) const;
  ANCHOR_API void Build();
  void Clear()
  {
    InputBuf[0] = 0;
    Build();
  }
  bool IsActive() const
  {
    return !Filters.empty();
  }

  // [Internal]
  struct ANCHORTextRange {
    const char *b;
    const char *e;

    ANCHORTextRange()
    {
      b = e = NULL;
    }
    ANCHORTextRange(const char *_b, const char *_e)
    {
      b = _b;
      e = _e;
    }
    bool empty() const
    {
      return b == e;
    }
    ANCHOR_API void split(char separator, AnchorVector<ANCHORTextRange> *out) const;
  };
  char InputBuf[256];
  AnchorVector<ANCHORTextRange> Filters;
  int CountGrep;
};

// Helper: Growable text buffer for logging/accumulating text
// (this could be called 'ANCHORTextBuilder' / 'ANCHORStringBuilder')
struct ANCHORTextBuffer {
  AnchorVector<char> Buf;
  ANCHOR_API static char EmptyString[1];

  ANCHORTextBuffer()
  {}
  inline char operator[](int i) const
  {
    ANCHOR_ASSERT(Buf.Data != NULL);
    return Buf.Data[i];
  }
  const char *begin() const
  {
    return Buf.Data ? &Buf.front() : EmptyString;
  }
  const char *end() const
  {
    return Buf.Data ? &Buf.back() : EmptyString;
  }  // Buf is zero-terminated, so end() will point on the zero-terminator
  int size() const
  {
    return Buf.Size ? Buf.Size - 1 : 0;
  }
  bool empty() const
  {
    return Buf.Size <= 1;
  }
  void clear()
  {
    Buf.clear();
  }
  void reserve(int capacity)
  {
    Buf.reserve(capacity);
  }
  const char *c_str() const
  {
    return Buf.Data ? Buf.Data : EmptyString;
  }
  ANCHOR_API void append(const char *str, const char *str_end = NULL);
  ANCHOR_API void appendf(const char *fmt, ...) ANCHOR_FMTARGS(2);
  ANCHOR_API void appendfv(const char *fmt, va_list args) ANCHOR_FMTLIST(2);
};

// Helper: Key->Value storage
// Typically you don't have to worry about this since a storage is held within each Window.
// We use it to e.g. store collapse state for a tree (Int 0/1)
// This is optimized for efficient lookup (dichotomy into a contiguous buffer) and rare insertion
// (typically tied to user interactions aka max once a frame) You can use it as custom user storage
// for temporary values. Declare your own storage if, for example:
// - You want to manipulate the open/close state of a particular sub-tree in your interface (tree
// node uses Int 0/1 to store their state).
// - You want to store custom debug data easily without adding or editing structures in your code
// (probably not efficient, but convenient) Types are NOT stored, so it is up to you to make sure
// your Key don't collide with different types.
struct ANCHORStorage {
  // [Internal]
  struct ANCHORStoragePair {
    ANCHOR_ID key;
    union {
      int val_i;
      float val_f;
      void *val_p;
    };
    ANCHORStoragePair(ANCHOR_ID _key, int _val_i)
    {
      key = _key;
      val_i = _val_i;
    }
    ANCHORStoragePair(ANCHOR_ID _key, float _val_f)
    {
      key = _key;
      val_f = _val_f;
    }
    ANCHORStoragePair(ANCHOR_ID _key, void *_val_p)
    {
      key = _key;
      val_p = _val_p;
    }
  };

  AnchorVector<ANCHORStoragePair> Data;

  // - Get***() functions find pair, never add/allocate. Pairs are sorted so a query is O(log N)
  // - Set***() functions find pair, insertion on demand if missing.
  // - Sorted insertion is costly, paid once. A typical frame shouldn't need to insert any new
  // pair.
  void Clear()
  {
    Data.clear();
  }
  ANCHOR_API int GetInt(ANCHOR_ID key, int default_val = 0) const;
  ANCHOR_API void SetInt(ANCHOR_ID key, int val);
  ANCHOR_API bool GetBool(ANCHOR_ID key, bool default_val = false) const;
  ANCHOR_API void SetBool(ANCHOR_ID key, bool val);
  ANCHOR_API float GetFloat(ANCHOR_ID key, float default_val = 0.0f) const;
  ANCHOR_API void SetFloat(ANCHOR_ID key, float val);
  ANCHOR_API void *GetVoidPtr(ANCHOR_ID key) const;  // default_val is NULL
  ANCHOR_API void SetVoidPtr(ANCHOR_ID key, void *val);

  // - Get***Ref() functions finds pair, insert on demand if missing, return pointer. Useful if you
  // intend to do Get+Set.
  // - References are only valid until a new value is added to the storage. Calling a Set***()
  // function or a Get***Ref() function invalidates the pointer.
  // - A typical use case where this is convenient for quick hacking (e.g. add storage during a
  // live Edit&Continue session if you can't modify existing struct)
  //      float* pvar = ANCHOR::GetFloatRef(key); ANCHOR::SliderFloat("var", pvar, 0, 100.0f);
  //      some_var += *pvar;
  ANCHOR_API int *GetIntRef(ANCHOR_ID key, int default_val = 0);
  ANCHOR_API bool *GetBoolRef(ANCHOR_ID key, bool default_val = false);
  ANCHOR_API float *GetFloatRef(ANCHOR_ID key, float default_val = 0.0f);
  ANCHOR_API void **GetVoidPtrRef(ANCHOR_ID key, void *default_val = NULL);

  // Use on your own storage if you know only integer are being stored (open/close all tree nodes)
  ANCHOR_API void SetAllInt(int val);

  // For quicker full rebuild of a storage (instead of an incremental one), you may add all your
  // contents and then sort once.
  ANCHOR_API void BuildSortByKey();
};

// Helper: Manually clip large list of items.
// If you are submitting lots of evenly spaced items and you have a random access to the list, you
// can perform coarse clipping based on visibility to save yourself from processing those items at
// all. The clipper calculates the range of visible items and advance the cursor to compensate for
// the non-visible items we have skipped. (ANCHOR already clip items based on their bounds but it
// needs to measure text size to do so, whereas manual coarse clipping before submission makes this
// cost and your own data fetching/submission cost almost null) Usage:
//   ANCHORListClipper clipper;
//   clipper.Begin(1000);         // We have 1000 elements, evenly spaced.
//   while (clipper.Step())
//       for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
//           ANCHOR::Text("line number %d", i);
// Generally what happens is:
// - Clipper lets you process the first element (DisplayStart = 0, DisplayEnd = 1) regardless of it
// being visible or not.
// - User code submit one element.
// - Clipper can measure the height of the first element
// - Clipper calculate the actual range of elements to display based on the current clipping
// rectangle, position the cursor before the first visible element.
// - User code submit visible elements.
struct ANCHORListClipper {
  int DisplayStart;
  int DisplayEnd;

  // [Internal]
  int ItemsCount;
  int StepNo;
  int ItemsFrozen;
  float ItemsHeight;
  float StartPosY;

  ANCHOR_API ANCHORListClipper();
  ANCHOR_API ~ANCHORListClipper();

  // items_count: Use INT_MAX if you don't know how many items you have (in which case the cursor
  // won't be advanced in the final step) items_height: Use -1.0f to be calculated automatically on
  // first step. Otherwise pass in the distance between your items, typically
  // GetTextLineHeightWithSpacing() or GetFrameHeightWithSpacing().
  ANCHOR_API void Begin(int items_count,
                        float items_height = -1.0f);  // Automatically called by constructor if you passed
                                                      // 'items_count' or by Step() in Step 1.
  ANCHOR_API void End();   // Automatically called on the last call of Step() that returns false.
  ANCHOR_API bool Step();  // Call until it returns false. The DisplayStart/DisplayEnd fields will
                           // be set and you can process/draw those items.

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  inline ANCHORListClipper(int items_count, float items_height = -1.0f)
  {
    memset(this, 0, sizeof(*this));
    ItemsCount = -1;
    Begin(items_count, items_height);
  }  // [removed in 1.79]
#  endif
};

// Helpers macros to generate 32-bit encoded colors
#  ifdef ANCHOR_USE_BGRA_PACKED_COLOR
#    define ANCHOR_COL32_R_SHIFT 16
#    define ANCHOR_COL32_G_SHIFT 8
#    define ANCHOR_COL32_B_SHIFT 0
#    define ANCHOR_COL32_A_SHIFT 24
#    define ANCHOR_COL32_A_MASK 0xFF000000
#  else
#    define ANCHOR_COL32_R_SHIFT 0
#    define ANCHOR_COL32_G_SHIFT 8
#    define ANCHOR_COL32_B_SHIFT 16
#    define ANCHOR_COL32_A_SHIFT 24
#    define ANCHOR_COL32_A_MASK 0xFF000000
#  endif
#  define ANCHOR_COL32(R, G, B, A) \
    (((AnchorU32)(A) << ANCHOR_COL32_A_SHIFT) | ((AnchorU32)(B) << ANCHOR_COL32_B_SHIFT) | \
     ((AnchorU32)(G) << ANCHOR_COL32_G_SHIFT) | ((AnchorU32)(R) << ANCHOR_COL32_R_SHIFT))
#  define ANCHOR_COL32_WHITE ANCHOR_COL32(255, 255, 255, 255)  // Opaque white = 0xFFFFFFFF
#  define ANCHOR_COL32_BLACK ANCHOR_COL32(0, 0, 0, 255)        // Opaque black
#  define ANCHOR_COL32_BLACK_TRANS ANCHOR_COL32(0, 0, 0, 0)    // Transparent black = 0x00000000

// Helper: AnchorColor() implicitly converts colors to either AnchorU32 (packed 4x1 byte) or
// wabi::GfVec4f (4x1 float) Prefer using ANCHOR_COL32() macros if you want a guaranteed
// compile-time AnchorU32 for usage with ImDrawList API.
// **Avoid storing AnchorColor! Store either u32 of wabi::GfVec4f. This is not a full-featured
// color class. MAY OBSOLETE.
// **None of the ANCHOR API are using AnchorColor directly but you can use it as a convenience to
// pass colors in either AnchorU32 or wabi::GfVec4f formats. Explicitly cast to AnchorU32 or
// wabi::GfVec4f if needed.
struct AnchorColor {
  wabi::GfVec4f Value;

  AnchorColor()
  {
    Value[0] = Value[1] = Value[2] = Value[3] = 0.0f;
  }
  AnchorColor(int r, int g, int b, int a = 255)
  {
    float sc = 1.0f / 255.0f;
    Value[0] = (float)r * sc;
    Value[1] = (float)g * sc;
    Value[2] = (float)b * sc;
    Value[3] = (float)a * sc;
  }
  AnchorColor(AnchorU32 rgba)
  {
    float sc = 1.0f / 255.0f;
    Value[0] = (float)((rgba >> ANCHOR_COL32_R_SHIFT) & 0xFF) * sc;
    Value[1] = (float)((rgba >> ANCHOR_COL32_G_SHIFT) & 0xFF) * sc;
    Value[2] = (float)((rgba >> ANCHOR_COL32_B_SHIFT) & 0xFF) * sc;
    Value[3] = (float)((rgba >> ANCHOR_COL32_A_SHIFT) & 0xFF) * sc;
  }
  AnchorColor(float r, float g, float b, float a = 1.0f)
  {
    Value[0] = r;
    Value[1] = g;
    Value[2] = b;
    Value[3] = a;
  }
  AnchorColor(const wabi::GfVec4f &col)
  {
    Value = col;
  }
  inline operator AnchorU32() const
  {
    return ANCHOR::ColorConvertFloat4ToU32(Value);
  }
  inline operator wabi::GfVec4f() const
  {
    return Value;
  }

  // FIXME-OBSOLETE: May need to obsolete/cleanup those helpers.
  inline void SetHSV(float h, float s, float v, float a = 1.0f)
  {
    ANCHOR::ColorConvertHSVtoRGB(h, s, v, Value[0], Value[1], Value[2]);
    Value[3] = a;
  }
  static AnchorColor HSV(float h, float s, float v, float a = 1.0f)
  {
    float r, g, b;
    ANCHOR::ColorConvertHSVtoRGB(h, s, v, r, g, b);
    return AnchorColor(r, g, b, a);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Drawing API (ImDrawCmd, ImDrawIdx, ImDrawVert, ImDrawChannel, ImDrawListSplitter,
// ImDrawListFlags, ImDrawList, ImDrawData) Hold a series of drawing commands. The user provides a
// renderer for ImDrawData which essentially contains an array of ImDrawList.
//-----------------------------------------------------------------------------

// The maximum line width to bake anti-aliased textures for. Build atlas with
// AnchorFontAtlasFlags_NoBakedLines to disable baking.
#  ifndef IM_DRAWLIST_TEX_LINES_WIDTH_MAX
#    define IM_DRAWLIST_TEX_LINES_WIDTH_MAX (63)
#  endif

// ImDrawCallback: Draw callbacks for advanced uses [configurable type: override in
// ANCHOR_config.h] NB: You most likely do NOT need to use draw callbacks just to create your own
// widget or customized UI rendering, you can poke into the draw list for that! Draw callback may
// be useful for example to:
//  A) Change your GPU render state,
//  B) render a complex 3D scene inside a UI element without an intermediate texture/render target,
//  etc.
// The expected behavior from your rendering function is 'if (cmd.UserCallback != NULL) {
// cmd.UserCallback(parent_list, cmd); } else { RenderTriangles() }' If you want to override the
// signature of ImDrawCallback, you can simply use e.g. '#define ImDrawCallback MyDrawCallback' (in
// ANCHOR_config.h) + update rendering backend accordingly.
#  ifndef ImDrawCallback
typedef void (*ImDrawCallback)(const ImDrawList *parent_list, const ImDrawCmd *cmd);
#  endif

// Special Draw callback value to request renderer backend to reset the graphics/render state.
// The renderer backend needs to handle this special value, otherwise it will crash trying to call
// a function at this address. This is useful for example if you submitted callbacks which you know
// have altered the render state and you want it to be restored. It is not done by default because
// they are many perfectly useful way of altering render state for ANCHOR contents (e.g. changing
// shader/blending settings before an Image call).
#  define ImDrawCallback_ResetRenderState (ImDrawCallback)(-1)

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
// - VtxOffset/IdxOffset: When 'io.BackendFlags & ANCHORBackendFlags_RendererHasVtxOffset' is
// enabled,
//   those fields allow us to render meshes larger than 64K vertices while keeping 16-bit indices.
//   Pre-1.71 backends will typically ignore the VtxOffset/IdxOffset fields.
// - The ClipRect/TextureId/VtxOffset fields must be contiguous as we memcmp() them together (this
// is asserted for).
struct ImDrawCmd {
  wabi::GfVec4f ClipRect;  // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract ImDrawData->DisplayPos to
                           // get clipping rectangle in "viewport" coordinates
  AnchorTextureID TextureId;    // 4-8  // User-provided texture ID. Set by user in
                                // ImfontAtlas::SetTexID() for fonts or passed to Image*() functions.
                                // Ignore if never using images or multiple fonts atlas.
  unsigned int VtxOffset;       // 4    // Start offset in vertex buffer.
                                // ANCHORBackendFlags_RendererHasVtxOffset: always 0, otherwise may be
                                // >0 to support meshes larger than 64K vertices with 16-bit indices.
  unsigned int IdxOffset;       // 4    // Start offset in index buffer. Always equal to sum of
                                // ElemCount drawn so far.
  unsigned int ElemCount;       // 4    // Number of indices (multiple of 3) to be rendered as
                                // triangles. Vertices are stored in the callee ImDrawList's
                                // vtx_buffer[] array, indices in idx_buffer[].
  ImDrawCallback UserCallback;  // 4-8  // If != NULL, call the function instead of rendering the
                                // vertices. clip_rect and texture_id will be set normally.
  void *UserCallbackData;       // 4-8  // The draw callback code can access this.

  ImDrawCmd()
  {
    memset(this, 0, sizeof(*this));
  }  // Also ensure our padding fields are zeroed

  // Since 1.83: returns AnchorTextureID associated with this draw call. Warning: DO NOT assume
  // this is always same as 'TextureId' (we will change this function for an upcoming feature)
  inline AnchorTextureID GetTexID() const
  {
    return TextureId;
  }
};

// Vertex index, default to 16-bit
// To allow large meshes with 16-bit indices: set 'io.BackendFlags |=
// ANCHORBackendFlags_RendererHasVtxOffset' and handle ImDrawCmd::VtxOffset in the renderer backend
// (recommended). To use 32-bit indices: override with '#define ImDrawIdx unsigned int' in
// ANCHOR_config.h.
#  ifndef ImDrawIdx
typedef unsigned short ImDrawIdx;
#  endif

// Vertex layout
#  ifndef ANCHOR_OVERRIDE_DRAWVERT_STRUCT_LAYOUT
struct ImDrawVert {
  wabi::GfVec2f pos;
  wabi::GfVec2f uv;
  AnchorU32 col;
};
#  else
// You can override the vertex format layout by defining ANCHOR_OVERRIDE_DRAWVERT_STRUCT_LAYOUT in
// ANCHOR_config.h The code expect wabi::GfVec2f pos (8 bytes), wabi::GfVec2f uv (8 bytes),
// AnchorU32 col (4 bytes), but you can re-order them or add other fields as needed to simplify
// integration in your engine. The type has to be described within the macro (you can either
// declare the struct or use a typedef). This is because wabi::GfVec2f/AnchorU32 are likely not
// declared a the time you'd want to set your type up. NOTE: ANCHOR DOESN'T CLEAR THE STRUCTURE AND
// DOESN'T CALL A CONSTRUCTOR SO ANY CUSTOM FIELD WILL BE UNINITIALIZED. IF YOU ADD EXTRA FIELDS
// (SUCH AS A 'Z' COORDINATES) YOU WILL NEED TO CLEAR THEM DURING RENDER OR TO IGNORE THEM.
ANCHOR_OVERRIDE_DRAWVERT_STRUCT_LAYOUT;
#  endif

// [Internal] For use by ImDrawList
struct ImDrawCmdHeader {
  wabi::GfVec4f ClipRect;
  AnchorTextureID TextureId;
  unsigned int VtxOffset;
};

// [Internal] For use by ImDrawListSplitter
struct ImDrawChannel {
  AnchorVector<ImDrawCmd> _CmdBuffer;
  AnchorVector<ImDrawIdx> _IdxBuffer;
};

// Split/Merge functions are used to split the draw list into different layers which can be drawn
// into out of order. This is used by the Columns/Tables API, so items of each column can be
// batched together in a same draw call.
struct ImDrawListSplitter {
  int _Current;  // Current channel number (0)
  int _Count;    // Number of active channels (1+)
  AnchorVector<ImDrawChannel>
    _Channels;  // Draw channels (not resized down so _Count might be < Channels.Size)

  inline ImDrawListSplitter()
  {
    memset(this, 0, sizeof(*this));
  }
  inline ~ImDrawListSplitter()
  {
    ClearFreeMemory();
  }
  inline void Clear()
  {
    _Current = 0;
    _Count = 1;
  }  // Do not clear Channels[] so our allocations are reused next frame
  ANCHOR_API void ClearFreeMemory();
  ANCHOR_API void Split(ImDrawList *draw_list, int count);
  ANCHOR_API void Merge(ImDrawList *draw_list);
  ANCHOR_API void SetCurrentChannel(ImDrawList *draw_list, int channel_idx);
};

// Flags for ImDrawList functions
// (Legacy: bit 0 must always correspond to ImDrawFlags_Closed to be backward compatible with old
// API using a bool. Bits 1..3 must be unused)
enum ImDrawFlags_ {
  ImDrawFlags_None = 0,
  ImDrawFlags_Closed = 1 << 0,  // PathStroke(), AddPolyline(): specify that shape should be closed
                                // (Important: this is always == 1 for legacy reason)
  ImDrawFlags_RoundCornersTopLeft =
    1 << 4,  // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only
             // (when rounding > 0.0f, we default to all corners). Was 0x01.
  ImDrawFlags_RoundCornersTopRight =
    1 << 5,  // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only
             // (when rounding > 0.0f, we default to all corners). Was 0x02.
  ImDrawFlags_RoundCornersBottomLeft =
    1 << 6,  // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only
             // (when rounding > 0.0f, we default to all corners). Was 0x04.
  ImDrawFlags_RoundCornersBottomRight =
    1 << 7,  // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only
             // (when rounding > 0.0f, we default to all corners). Wax 0x08.
  ImDrawFlags_RoundCornersNone =
    1 << 8,  // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when
             // rounding > 0.0f). This is NOT zero, NOT an implicit flag!
  ImDrawFlags_RoundCornersTop = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
  ImDrawFlags_RoundCornersBottom = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
  ImDrawFlags_RoundCornersLeft = ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft,
  ImDrawFlags_RoundCornersRight = ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight,
  ImDrawFlags_RoundCornersAll = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight |
                                ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight,
  ImDrawFlags_RoundCornersDefault_ = ImDrawFlags_RoundCornersAll,  // Default to ALL corners if none of the
                                                                   // _RoundCornersXX flags are specified.
  ImDrawFlags_RoundCornersMask_ = ImDrawFlags_RoundCornersAll | ImDrawFlags_RoundCornersNone
};

// Flags for ImDrawList instance. Those are set automatically by ANCHOR:: functions from ANCHOR_IO
// settings, and generally not manipulated directly. It is however possible to temporarily alter
// flags between calls to ImDrawList:: functions.
enum ImDrawListFlags_ {
  ImDrawListFlags_None = 0,
  ImDrawListFlags_AntiAliasedLines =
    1 << 0,  // Enable anti-aliased lines/borders (*2 the number of triangles for 1.0f wide line or
             // lines thin enough to be drawn using textures, otherwise *3 the number of triangles)
  ImDrawListFlags_AntiAliasedLinesUseTex =
    1 << 1,  // Enable anti-aliased lines/borders using textures when possible. Require backend
             // to render with bilinear filtering.
  ImDrawListFlags_AntiAliasedFill =
    1 << 2,  // Enable anti-aliased edge around filled shapes (rounded rectangles, circles).
  ImDrawListFlags_AllowVtxOffset = 1 << 3  // Can emit 'VtxOffset > 0' to allow large meshes. Set when
                                           // 'ANCHORBackendFlags_RendererHasVtxOffset' is enabled.
};

// Draw command list
// This is the low-level list of polygons that ANCHOR:: functions are filling. At the end of the
// frame, all command lists are passed to your ANCHOR_IO::RenderDrawListFn function for rendering.
// Each ANCHOR window contains its own ImDrawList. You can use ANCHOR::GetWindowDrawList() to
// access the current window draw list and draw custom primitives.
// You can interleave normal ANCHOR:: calls and adding primitives to the current draw list.
// In single viewport mode, top-left is == GetMainViewport()->Pos (generally 0,0), bottom-right is
// == GetMainViewport()->Pos+Size (generally io.DisplaySize). You are totally free to apply
// whatever transformation matrix to want to the data (depending on the use of the transformation
// you may want to apply it to ClipRect as well!) Important: Primitives are always added to the
// list and not culled (culling is done at higher-level by ANCHOR:: functions), if you use this API
// a lot consider coarse culling your drawn objects.
struct ImDrawList {
  // This is what you have to render
  AnchorVector<ImDrawCmd> CmdBuffer;   // Draw commands. Typically 1 command = 1 GPU draw call,
                                       // unless the command is a callback.
  AnchorVector<ImDrawIdx> IdxBuffer;   // Index buffer. Each command consume ImDrawCmd::ElemCount of those
  AnchorVector<ImDrawVert> VtxBuffer;  // Vertex buffer.
  ImDrawListFlags Flags;  // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.

  // [Internal, used while building lists]
  unsigned int _VtxCurrentIdx;  // [Internal] generally == VtxBuffer.Size unless we are past 64K
                                // vertices, in which case this gets reset to 0.
  const ImDrawListSharedData
    *_Data;                // Pointer to shared draw data (you can use ANCHOR::GetDrawListSharedData() to get
                           // the one from current ANCHOR context)
  const char *_OwnerName;  // Pointer to owner window's name for debugging
  ImDrawVert *_VtxWritePtr;  // [Internal] point within VtxBuffer.Data after each add command (to
                             // avoid using the AnchorVector<> operators too much)
  ImDrawIdx *_IdxWritePtr;   // [Internal] point within IdxBuffer.Data after each add command (to
                             // avoid using the AnchorVector<> operators too much)
  AnchorVector<wabi::GfVec4f> _ClipRectStack;     // [Internal]
  AnchorVector<AnchorTextureID> _TextureIdStack;  // [Internal]
  AnchorVector<wabi::GfVec2f> _Path;              // [Internal] current path building
  ImDrawCmdHeader _CmdHeader;    // [Internal] template of active commands. Fields should match those
                                 // of CmdBuffer.back().
  ImDrawListSplitter _Splitter;  // [Internal] for channels api (note: prefer using your own
                                 // persistent instance of ImDrawListSplitter!)
  float _FringeScale;            // [Internal] anti-alias fringe is scaled by this value, this helps to keep
                                 // things sharp while zooming at vertex buffer content

  // If you want to create ImDrawList instances, pass them ANCHOR::GetDrawListSharedData() or
  // create and use your own ImDrawListSharedData (so you can use ImDrawList without ANCHOR)
  ImDrawList(const ImDrawListSharedData *shared_data)
  {
    memset(this, 0, sizeof(*this));
    _Data = shared_data;
  }

  ~ImDrawList()
  {
    _ClearFreeMemory();
  }
  ANCHOR_API void PushClipRect(
    wabi::GfVec2f clip_rect_min,
    wabi::GfVec2f clip_rect_max,
    bool intersect_with_current_clip_rect =
      false);  // Render-level scissoring. This is passed down to your render function but not
               // used for CPU-side coarse clipping. Prefer using higher-level
               // ANCHOR::PushClipRect() to affect logic (hit-testing and widget culling)
  ANCHOR_API void PushClipRectFullScreen();
  ANCHOR_API void PopClipRect();
  ANCHOR_API void PushTextureID(AnchorTextureID texture_id);
  ANCHOR_API void PopTextureID();
  inline wabi::GfVec2f GetClipRectMin() const
  {
    const wabi::GfVec4f &cr = _ClipRectStack.back();
    return wabi::GfVec2f(cr[0], cr[1]);
  }
  inline wabi::GfVec2f GetClipRectMax() const
  {
    const wabi::GfVec4f &cr = _ClipRectStack.back();
    return wabi::GfVec2f(cr[2], cr[3]);
  }

  // Primitives
  // - For rectangular primitives, "p_min" and "p_max" represent the upper-left and lower-right
  // corners.
  // - For circle primitives, use "num_segments == 0" to automatically calculate tessellation
  // (preferred).
  //   In older versions (until ANCHOR 1.77) the AddCircle functions defaulted to num_segments
  //   == 12. In future versions we will use textures to provide cheaper and higher-quality
  //   circles. Use AddNgon() and AddNgonFilled() functions if you need to guaranteed a specific
  //   number of sides.
  ANCHOR_API void AddLine(const wabi::GfVec2f &p1,
                          const wabi::GfVec2f &p2,
                          AnchorU32 col,
                          float thickness = 1.0f);
  ANCHOR_API void AddRect(const wabi::GfVec2f &p_min,
                          const wabi::GfVec2f &p_max,
                          AnchorU32 col,
                          float rounding = 0.0f,
                          ImDrawFlags flags = 0,
                          float thickness = 1.0f);  // a: upper-left, b: lower-right (== upper-left + size)
  ANCHOR_API void AddRectFilled(
    const wabi::GfVec2f &p_min,
    const wabi::GfVec2f &p_max,
    AnchorU32 col,
    float rounding = 0.0f,
    ImDrawFlags flags = 0);  // a: upper-left, b: lower-right (== upper-left + size)
  ANCHOR_API void AddRectFilledMultiColor(const wabi::GfVec2f &p_min,
                                          const wabi::GfVec2f &p_max,
                                          AnchorU32 col_upr_left,
                                          AnchorU32 col_upr_right,
                                          AnchorU32 col_bot_right,
                                          AnchorU32 col_bot_left);
  ANCHOR_API void AddQuad(const wabi::GfVec2f &p1,
                          const wabi::GfVec2f &p2,
                          const wabi::GfVec2f &p3,
                          const wabi::GfVec2f &p4,
                          AnchorU32 col,
                          float thickness = 1.0f);
  ANCHOR_API void AddQuadFilled(const wabi::GfVec2f &p1,
                                const wabi::GfVec2f &p2,
                                const wabi::GfVec2f &p3,
                                const wabi::GfVec2f &p4,
                                AnchorU32 col);
  ANCHOR_API void AddTriangle(const wabi::GfVec2f &p1,
                              const wabi::GfVec2f &p2,
                              const wabi::GfVec2f &p3,
                              AnchorU32 col,
                              float thickness = 1.0f);
  ANCHOR_API void AddTriangleFilled(const wabi::GfVec2f &p1,
                                    const wabi::GfVec2f &p2,
                                    const wabi::GfVec2f &p3,
                                    AnchorU32 col);
  ANCHOR_API void AddCircle(const wabi::GfVec2f &center,
                            float radius,
                            AnchorU32 col,
                            int num_segments = 0,
                            float thickness = 1.0f);
  ANCHOR_API void AddCircleFilled(const wabi::GfVec2f &center,
                                  float radius,
                                  AnchorU32 col,
                                  int num_segments = 0);
  ANCHOR_API void AddNgon(const wabi::GfVec2f &center,
                          float radius,
                          AnchorU32 col,
                          int num_segments,
                          float thickness = 1.0f);
  ANCHOR_API void AddNgonFilled(const wabi::GfVec2f &center, float radius, AnchorU32 col, int num_segments);
  ANCHOR_API void AddText(const wabi::GfVec2f &pos,
                          AnchorU32 col,
                          const char *text_begin,
                          const char *text_end = NULL);
  ANCHOR_API void AddText(const AnchorFont *font,
                          float font_size,
                          const wabi::GfVec2f &pos,
                          AnchorU32 col,
                          const char *text_begin,
                          const char *text_end = NULL,
                          float wrap_width = 0.0f,
                          const wabi::GfVec4f *cpu_fine_clip_rect = NULL);
  ANCHOR_API void AddPolyline(const wabi::GfVec2f *points,
                              int num_points,
                              AnchorU32 col,
                              ImDrawFlags flags,
                              float thickness);
  ANCHOR_API void AddConvexPolyFilled(
    const wabi::GfVec2f *points,
    int num_points,
    AnchorU32 col);  // Note: Anti-aliased filling requires points to be in clockwise order.
  ANCHOR_API void AddBezierCubic(const wabi::GfVec2f &p1,
                                 const wabi::GfVec2f &p2,
                                 const wabi::GfVec2f &p3,
                                 const wabi::GfVec2f &p4,
                                 AnchorU32 col,
                                 float thickness,
                                 int num_segments = 0);  // Cubic Bezier (4 control points)
  ANCHOR_API void AddBezierQuadratic(const wabi::GfVec2f &p1,
                                     const wabi::GfVec2f &p2,
                                     const wabi::GfVec2f &p3,
                                     AnchorU32 col,
                                     float thickness,
                                     int num_segments = 0);  // Quadratic Bezier (3 control points)

  // Image primitives
  // - Read FAQ to understand what AnchorTextureID is.
  // - "p_min" and "p_max" represent the upper-left and lower-right corners of the rectangle.
  // - "uv_min" and "uv_max" represent the normalized texture coordinates to use for those corners.
  // Using (0,0)->(1,1) texture coordinates will generally display the entire texture.
  ANCHOR_API void AddImage(AnchorTextureID user_texture_id,
                           const wabi::GfVec2f &p_min,
                           const wabi::GfVec2f &p_max,
                           const wabi::GfVec2f &uv_min = wabi::GfVec2f(0, 0),
                           const wabi::GfVec2f &uv_max = wabi::GfVec2f(1, 1),
                           AnchorU32 col = ANCHOR_COL32_WHITE);
  ANCHOR_API void AddImageQuad(AnchorTextureID user_texture_id,
                               const wabi::GfVec2f &p1,
                               const wabi::GfVec2f &p2,
                               const wabi::GfVec2f &p3,
                               const wabi::GfVec2f &p4,
                               const wabi::GfVec2f &uv1 = wabi::GfVec2f(0, 0),
                               const wabi::GfVec2f &uv2 = wabi::GfVec2f(1, 0),
                               const wabi::GfVec2f &uv3 = wabi::GfVec2f(1, 1),
                               const wabi::GfVec2f &uv4 = wabi::GfVec2f(0, 1),
                               AnchorU32 col = ANCHOR_COL32_WHITE);
  ANCHOR_API void AddImageRounded(AnchorTextureID user_texture_id,
                                  const wabi::GfVec2f &p_min,
                                  const wabi::GfVec2f &p_max,
                                  const wabi::GfVec2f &uv_min,
                                  const wabi::GfVec2f &uv_max,
                                  AnchorU32 col,
                                  float rounding,
                                  ImDrawFlags flags = 0);

  // Stateful path API, add points then finish with PathFillConvex() or PathStroke()
  inline void PathClear()
  {
    _Path.Size = 0;
  }
  inline void PathLineTo(const wabi::GfVec2f &pos)
  {
    _Path.push_back(pos);
  }
  inline void PathLineToMergeDuplicate(const wabi::GfVec2f &pos)
  {
    if (_Path.Size == 0 || memcmp(&_Path.Data[_Path.Size - 1], &pos, 8) != 0)
      _Path.push_back(pos);
  }
  inline void PathFillConvex(AnchorU32 col)
  {
    AddConvexPolyFilled(_Path.Data, _Path.Size, col);
    _Path.Size = 0;
  }  // Note: Anti-aliased filling requires points to be in clockwise order.
  inline void PathStroke(AnchorU32 col, ImDrawFlags flags = 0, float thickness = 1.0f)
  {
    AddPolyline(_Path.Data, _Path.Size, col, flags, thickness);
    _Path.Size = 0;
  }
  ANCHOR_API void PathArcTo(const wabi::GfVec2f &center,
                            float radius,
                            float a_min,
                            float a_max,
                            int num_segments = 0);
  ANCHOR_API void PathArcToFast(const wabi::GfVec2f &center,
                                float radius,
                                int a_min_of_12,
                                int a_max_of_12);  // Use precomputed angles for a 12 steps circle
  ANCHOR_API void PathBezierCubicCurveTo(const wabi::GfVec2f &p2,
                                         const wabi::GfVec2f &p3,
                                         const wabi::GfVec2f &p4,
                                         int num_segments = 0);  // Cubic Bezier (4 control points)
  ANCHOR_API void PathBezierQuadraticCurveTo(const wabi::GfVec2f &p2,
                                             const wabi::GfVec2f &p3,
                                             int num_segments = 0);  // Quadratic Bezier (3 control points)
  ANCHOR_API void PathRect(const wabi::GfVec2f &rect_min,
                           const wabi::GfVec2f &rect_max,
                           float rounding = 0.0f,
                           ImDrawFlags flags = 0);

  // Advanced
  ANCHOR_API void AddCallback(
    ImDrawCallback callback,
    void *callback_data);        // Your rendering function must check for 'UserCallback' in ImDrawCmd
                                 // and call the function instead of rendering triangles.
  ANCHOR_API void AddDrawCmd();  // This is useful if you need to forcefully create a new draw call
                                 // (to allow for dependent rendering / blending). Otherwise
                                 // primitives are merged into the same draw-call as much as
                                 // possible
  ANCHOR_API ImDrawList *CloneOutput() const;  // Create a clone of the CmdBuffer/IdxBuffer/VtxBuffer.

  // Advanced: Channels
  // - Use to split render into layers. By switching channels to can render out-of-order (e.g.
  // submit FG primitives before BG primitives)
  // - Use to minimize draw calls (e.g. if going back-and-forth between multiple clipping
  // rectangles, prefer to append into separate channels then merge at the end)
  // - FIXME-OBSOLETE: This API shouldn't have been in ImDrawList in the first place!
  //   Prefer using your own persistent instance of ImDrawListSplitter as you can stack them.
  //   Using the ImDrawList::ChannelsXXXX you cannot stack a split over another.
  inline void ChannelsSplit(int count)
  {
    _Splitter.Split(this, count);
  }
  inline void ChannelsMerge()
  {
    _Splitter.Merge(this);
  }
  inline void ChannelsSetCurrent(int n)
  {
    _Splitter.SetCurrentChannel(this, n);
  }

  // Advanced: Primitives allocations
  // - We render triangles (three vertices)
  // - All primitives needs to be reserved via PrimReserve() beforehand.
  ANCHOR_API void PrimReserve(int idx_count, int vtx_count);
  ANCHOR_API void PrimUnreserve(int idx_count, int vtx_count);
  ANCHOR_API void PrimRect(const wabi::GfVec2f &a,
                           const wabi::GfVec2f &b,
                           AnchorU32 col);  // Axis aligned rectangle (composed of two triangles)
  ANCHOR_API void PrimRectUV(const wabi::GfVec2f &a,
                             const wabi::GfVec2f &b,
                             const wabi::GfVec2f &uv_a,
                             const wabi::GfVec2f &uv_b,
                             AnchorU32 col);
  ANCHOR_API void PrimQuadUV(const wabi::GfVec2f &a,
                             const wabi::GfVec2f &b,
                             const wabi::GfVec2f &c,
                             const wabi::GfVec2f &d,
                             const wabi::GfVec2f &uv_a,
                             const wabi::GfVec2f &uv_b,
                             const wabi::GfVec2f &uv_c,
                             const wabi::GfVec2f &uv_d,
                             AnchorU32 col);
  inline void PrimWriteVtx(const wabi::GfVec2f &pos, const wabi::GfVec2f &uv, AnchorU32 col)
  {
    _VtxWritePtr->pos = pos;
    _VtxWritePtr->uv = uv;
    _VtxWritePtr->col = col;
    _VtxWritePtr++;
    _VtxCurrentIdx++;
  }
  inline void PrimWriteIdx(ImDrawIdx idx)
  {
    *_IdxWritePtr = idx;
    _IdxWritePtr++;
  }
  inline void PrimVtx(const wabi::GfVec2f &pos, const wabi::GfVec2f &uv, AnchorU32 col)
  {
    PrimWriteIdx((ImDrawIdx)_VtxCurrentIdx);
    PrimWriteVtx(pos, uv, col);
  }  // Write vertex with unique index

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  inline void AddBezierCurve(const wabi::GfVec2f &p1,
                             const wabi::GfVec2f &p2,
                             const wabi::GfVec2f &p3,
                             const wabi::GfVec2f &p4,
                             AnchorU32 col,
                             float thickness,
                             int num_segments = 0)
  {
    AddBezierCubic(p1, p2, p3, p4, col, thickness, num_segments);
  }
  inline void PathBezierCurveTo(const wabi::GfVec2f &p2,
                                const wabi::GfVec2f &p3,
                                const wabi::GfVec2f &p4,
                                int num_segments = 0)
  {
    PathBezierCubicCurveTo(p2, p3, p4, num_segments);
  }
#  endif

  // [Internal helpers]
  ANCHOR_API void _ResetForNewFrame();
  ANCHOR_API void _ClearFreeMemory();
  ANCHOR_API void _PopUnusedDrawCmd();
  ANCHOR_API void _OnChangedClipRect();
  ANCHOR_API void _OnChangedTextureID();
  ANCHOR_API void _OnChangedVtxOffset();
  ANCHOR_API int _CalcCircleAutoSegmentCount(float radius) const;
  ANCHOR_API void _PathArcToFastEx(const wabi::GfVec2f &center,
                                   float radius,
                                   int a_min_sample,
                                   int a_max_sample,
                                   int a_step);
  ANCHOR_API void _PathArcToN(const wabi::GfVec2f &center,
                              float radius,
                              float a_min,
                              float a_max,
                              int num_segments);
};

// All draw data to render a ANCHOR frame
// (NB: the style and the naming convention here is a little inconsistent, we currently preserve
// them for backward compatibility purpose, as this is one of the oldest structure exposed by the
// library! Basically, ImDrawList == CmdList)
struct ImDrawData {
  bool Valid;                // Only valid after Render() is called and before the next NewFrame() is called.
  int CmdListsCount;         // Number of ImDrawList* to render
  int TotalIdxCount;         // For convenience, sum of all ImDrawList's IdxBuffer.Size
  int TotalVtxCount;         // For convenience, sum of all ImDrawList's VtxBuffer.Size
  ImDrawList **CmdLists;     // Array of ImDrawList* to render. The ImDrawList are owned by
                             // ANCHOR_Context and only pointed to from here.
  wabi::GfVec2f DisplayPos;  // Top-left position of the viewport to render (== top-left of the
                             // orthogonal projection matrix to use) (== GetMainViewport()->Pos for
                             // the main viewport, == (0.0) in most single-viewport applications)
  wabi::GfVec2f DisplaySize;       // Size of the viewport to render (== GetMainViewport()->Size for the main
                                   // viewport, == io.DisplaySize in most single-viewport applications)
  wabi::GfVec2f FramebufferScale;  // Amount of pixels for each unit of DisplaySize. Based on
                                   // io.DisplayFramebufferScale. Generally (1,1) on normal
                                   // display, (2,2) on OSX with Retina display.

  // Functions
  ImDrawData()
  {
    Clear();
  }
  void Clear()
  {
    memset(this, 0, sizeof(*this));
  }                                     // The ImDrawList are owned by ANCHOR_Context!
  ANCHOR_API void DeIndexAllBuffers();  // Helper to convert all buffers from indexed to
                                        // non-indexed, in case you cannot render indexed. Note:
                                        // this is slow and most likely a waste of resources.
                                        // Always prefer indexed rendering!
  ANCHOR_API void ScaleClipRects(
    const wabi::GfVec2f
      &fb_scale);  // Helper to scale the ClipRect field of each ImDrawCmd. Use if your final
                   // output buffer is at a different scale than ANCHOR expects, or if there is
                   // a difference between your window resolution and framebuffer resolution.
};

//-----------------------------------------------------------------------------
// [SECTION] Font API (AnchorFontConfig, AnchorFontGlyph, AnchorFontAtlasFlags, AnchorFontAtlas,
// AnchorFontGlyphRangesBuilder, AnchorFont)
//-----------------------------------------------------------------------------

struct AnchorFontConfig {
  void *FontData;             //          // TTF/OTF data
  int FontDataSize;           //          // TTF/OTF data size
  bool FontDataOwnedByAtlas;  // true     // TTF/OTF data ownership taken by the container
                              // AnchorFontAtlas (will delete memory itself).
  int FontNo;                 // 0        // Index of font within TTF/OTF file
  float SizePixels;           //          // Size in pixels for rasterizer (more or less maps to the
                              //          resulting font height).
  int OversampleH;            // 3        // Rasterize at higher quality for sub-pixel positioning. Note the
                              // difference between 2 and 3 is minimal so you can reduce this to 2 to save
                              // memory. Read
                              // https://github.com/nothings/stb/blob/master/tests/oversample/README.md for
                              // details.
  int OversampleV;            // 1        // Rasterize at higher quality for sub-pixel positioning. This is
                              // not really useful as we don't use sub-pixel positions on the Y axis.
  bool PixelSnapH;            // false    // Align every glyph to pixel boundary. Useful e.g. if you are
                              // merging a non-pixel aligned font with the default font. If enabled, you can
                              // set OversampleH/V to 1.
  wabi::GfVec2f GlyphExtraSpacing;  // 0, 0     // Extra spacing (in pixels) between glyphs. Only X
                                    // axis is supported for now.
  wabi::GfVec2f GlyphOffset;        // 0, 0     // Offset all glyphs from this font input.
  const AnchorWChar *GlyphRanges;   // NULL     // Pointer to a user-provided list of Unicode range (2 value
                                    // per range, values are inclusive, zero-terminated list). THE ARRAY DATA
                                    // NEEDS TO PERSIST AS LONG AS THE FONT IS ALIVE.
  float GlyphMinAdvanceX;           // 0        // Minimum AdvanceX for glyphs, set Min to align font icons,
                                    // set both Min/Max to enforce mono-space font
  float GlyphMaxAdvanceX;           // FLT_MAX  // Maximum AdvanceX for glyphs
  bool MergeMode;  // false    // Merge into previous AnchorFont, so you can combine multiple
                   // inputs font into one AnchorFont (e.g. ASCII font + icons + Japanese glyphs).
                   // You may want to use GlyphOffset[1] when merge font of different heights.
  unsigned int FontBuilderFlags;  // 0        // Settings for custom font builder. THIS IS BUILDER
                                  // IMPLEMENTATION DEPENDENT. Leave as zero if unsure.
  float RasterizerMultiply;       // 1.0f     // Brighten (>1.0f) or darken (<1.0f) font output.
                                  // Brightening small fonts may be a good workaround to make them more
                                  // readable.
  AnchorWChar EllipsisChar;       // -1       // Explicitly specify unicode codepoint of ellipsis character.
                                  // When fonts are being merged first specified ellipsis will be used.

  // [Internal]
  char Name[40];  // Name (strictly to ease debugging)
  AnchorFont *DstFont;

  ANCHOR_API AnchorFontConfig();
};

// Hold rendering data for one glyph.
// (Note: some language parsers may fail to convert the 31+1 bitfield members, in this case maybe
// drop store a single u32 or we can rework this)
struct AnchorFontGlyph {
  unsigned int Colored : 1;  // Flag to indicate glyph is colored and should generally ignore tinting (make
                             // it usable with no shift on little-endian as this is used in loops)
  unsigned int Visible : 1;  // Flag to indicate glyph has no visible pixels (e.g. space). Allow
                             // early out when rendering.
  unsigned int Codepoint : 30;  // 0x0000..0x10FFFF
  float AdvanceX;               // Distance to next character (= data from font +
                                // AnchorFontConfig::GlyphExtraSpacing[0] baked in)
  float X0, Y0, X1, Y1;         // Glyph corners
  float U0, V0, U1, V1;         // Texture coordinates
};

// Helper to build glyph ranges from text/string data. Feed your application strings/characters to
// it then call BuildRanges(). This is essentially a tightly packed of vector of 64k booleans = 8KB
// storage.
struct AnchorFontGlyphRangesBuilder {
  AnchorVector<AnchorU32> UsedChars;  // Store 1-bit per Unicode code point (0=unused, 1=used)

  AnchorFontGlyphRangesBuilder()
  {
    Clear();
  }
  inline void Clear()
  {
    int size_in_bytes = (IM_UNICODE_CODEPOINT_MAX + 1) / 8;
    UsedChars.resize(size_in_bytes / (int)sizeof(AnchorU32));
    memset(UsedChars.Data, 0, (size_t)size_in_bytes);
  }
  inline bool GetBit(size_t n) const
  {
    int off = (int)(n >> 5);
    AnchorU32 mask = 1u << (n & 31);
    return (UsedChars[off] & mask) != 0;
  }  // Get bit n in the array
  inline void SetBit(size_t n)
  {
    int off = (int)(n >> 5);
    AnchorU32 mask = 1u << (n & 31);
    UsedChars[off] |= mask;
  }  // Set bit n in the array
  inline void AddChar(AnchorWChar c)
  {
    SetBit(c);
  }  // Add character
  ANCHOR_API void AddText(
    const char *text,
    const char *text_end = NULL);  // Add string (each character of the UTF-8 string are added)
  ANCHOR_API void AddRanges(
    const AnchorWChar *ranges);  // Add ranges, e.g.
                                 // builder.AddRanges(AnchorFontAtlas::GetGlyphRangesDefault())
                                 // to force add all of ASCII/Latin+Ext
  ANCHOR_API void BuildRanges(AnchorVector<AnchorWChar> *out_ranges);  // Output new ranges
};

// See AnchorFontAtlas::AddCustomRectXXX functions.
struct AnchorFontAtlasCustomRect {
  unsigned short Width, Height;  // Input    // Desired rectangle dimension
  unsigned short X, Y;           // Output   // Packed position in Atlas
  unsigned int GlyphID;          // Input    // For custom font glyphs only (ID < 0x110000)
  float GlyphAdvanceX;           // Input    // For custom font glyphs only: glyph xadvance
  wabi::GfVec2f GlyphOffset;     // Input    // For custom font glyphs only: glyph display offset
  AnchorFont *Font;              // Input    // For custom font glyphs only: target font
  AnchorFontAtlasCustomRect()
  {
    Width = Height = 0;
    X = Y = 0xFFFF;
    GlyphID = 0;
    GlyphAdvanceX = 0.0f;
    GlyphOffset = wabi::GfVec2f(0, 0);
    Font = NULL;
  }
  bool IsPacked() const
  {
    return X != 0xFFFF;
  }
};

// Flags for AnchorFontAtlas build
enum AnchorFontAtlasFlags_ {
  AnchorFontAtlasFlags_None = 0,
  AnchorFontAtlasFlags_NoPowerOfTwoHeight = 1 << 0,  // Don't round the height to next power of two
  AnchorFontAtlasFlags_NoMouseCursors =
    1 << 1,  // Don't build software mouse cursors into the atlas (save a little texture memory)
  AnchorFontAtlasFlags_NoBakedLines =
    1 << 2  // Don't build thick line textures into the atlas (save a little texture memory). The
            // AntiAliasedLinesUseTex features uses them, otherwise they will be rendered using
            // polygons (more expensive for CPU/GPU).
};

// Load and rasterize multiple TTF/OTF fonts into a same texture. The font atlas will build a
// single texture holding:
//  - One or more fonts.
//  - Custom graphics data needed to render the shapes needed by ANCHOR.
//  - Mouse cursor shapes for software cursor rendering (unless setting 'Flags |=
//  AnchorFontAtlasFlags_NoMouseCursors' in the font atlas).
// It is the user-code responsibility to setup/build the atlas, then upload the pixel data into a
// texture accessible by your graphics api.
//  - Optionally, call any of the AddFont*** functions. If you don't call any, the default font
//  embedded in the code will be loaded for you.
//  - Call GetTexDataAsAlpha8() or GetTexDataAsRGBA32() to build and retrieve pixels data.
//  - Upload the pixels data into a texture within your graphics system (see ANCHOR_impl_xxxx.cpp
//  examples)
//  - Call SetTexID(my_tex_id); and pass the pointer/identifier to your texture in a format natural
//  to your graphics API.
//    This value will be passed back to you during rendering to identify the texture. Read FAQ
//    entry about AnchorTextureID for more details.
// Common pitfalls:
// - If you pass a 'glyph_ranges' array to AddFont*** functions, you need to make sure that your
// array persist up until the
//   atlas is build (when calling GetTexData*** or Build()). We only copy the pointer, not the
//   data.
// - Important: By default, AddFontFromMemoryTTF() takes ownership of the data. Even though we are
// not writing to it, we will free the pointer on destruction.
//   You can set font_cfg->FontDataOwnedByAtlas=false to keep ownership of your data and it won't
//   be freed,
// - Even though many functions are suffixed with "TTF", OTF data is supported just as well.
// - This is an old API and it is currently awkward for those and and various other reasons! We
// will address them in the future!
struct AnchorFontAtlas {
  ANCHOR_API AnchorFontAtlas();
  ANCHOR_API ~AnchorFontAtlas();
  ANCHOR_API AnchorFont *AddFont(const AnchorFontConfig *font_cfg);
  ANCHOR_API AnchorFont *AddFontDefault(const AnchorFontConfig *font_cfg = NULL);
  ANCHOR_API AnchorFont *AddFontFromFileTTF(const char *filename,
                                            float size_pixels,
                                            const AnchorFontConfig *font_cfg = NULL,
                                            const AnchorWChar *glyph_ranges = NULL);
  ANCHOR_API AnchorFont *AddFontFromMemoryTTF(
    void *font_data,
    int font_size,
    float size_pixels,
    const AnchorFontConfig *font_cfg = NULL,
    const AnchorWChar *glyph_ranges =
      NULL);  // Note: Transfer ownership of 'ttf_data' to AnchorFontAtlas! Will be deleted
              // after destruction of the atlas. Set font_cfg->FontDataOwnedByAtlas=false to
              // keep ownership of your data and it won't be freed.
  ANCHOR_API AnchorFont *AddFontFromMemoryCompressedTTF(
    const void *compressed_font_data,
    int compressed_font_size,
    float size_pixels,
    const AnchorFontConfig *font_cfg = NULL,
    const AnchorWChar *glyph_ranges = NULL);  // 'compressed_font_data' still owned by caller.
                                              // Compress with binary_to_compressed_c.cpp.
  ANCHOR_API AnchorFont *AddFontFromMemoryCompressedBase85TTF(
    const char *compressed_font_data_base85,
    float size_pixels,
    const AnchorFontConfig *font_cfg = NULL,
    const AnchorWChar *glyph_ranges =
      NULL);                         // 'compressed_font_data_base85' still owned by caller. Compress with
                                     // binary_to_compressed_c.cpp with -base85 parameter.
  ANCHOR_API void ClearInputData();  // Clear input data (all AnchorFontConfig structures including
                                     // sizes, TTF data, glyph ranges, etc.) = all the data used to
                                     // build the texture and fonts.
  ANCHOR_API void ClearTexData();    // Clear output texture data (CPU side). Saves RAM once the
                                     // texture has been copied to graphics memory.
  ANCHOR_API void ClearFonts();      // Clear output font data (glyphs storage, UV coordinates).
  ANCHOR_API void Clear();           // Clear all input and output.

  // Build atlas, retrieve pixel data.
  // User is in charge of copying the pixels into graphics memory (e.g. create a texture with your
  // engine). Then store your texture handle with SetTexID(). The pitch is always = Width *
  // BytesPerPixels (1 or 4) Building in RGBA32 format is provided for convenience and
  // compatibility, but note that unless you manually manipulate or copy color data into the
  // texture (e.g. when using the AddCustomRect*** api), then the RGB pixels emitted will always be
  // white (~75% of memory/bandwidth waste.
  ANCHOR_API bool Build();  // Build pixels data. This is called automatically for you by the
                            // GetTexData*** functions.
  ANCHOR_API void GetTexDataAsAlpha8(unsigned char **out_pixels,
                                     int *out_width,
                                     int *out_height,
                                     int *out_bytes_per_pixel = NULL);  // 1 byte per-pixel
  ANCHOR_API void GetTexDataAsRGBA32(unsigned char **out_pixels,
                                     int *out_width,
                                     int *out_height,
                                     int *out_bytes_per_pixel = NULL);  // 4 bytes-per-pixel
  bool IsBuilt() const
  {
    return Fonts.Size > 0 && (TexPixelsAlpha8 != NULL || TexPixelsRGBA32 != NULL);
  }
  void SetTexID(AnchorTextureID id)
  {
    TexID = id;
  }

  //-------------------------------------------
  // Glyph Ranges
  //-------------------------------------------

  // Helpers to retrieve list of common Unicode ranges (2 value per range, values are inclusive,
  // zero-terminated list) NB: Make sure that your string are UTF-8 and NOT in your local code
  // page. In C++11, you can create UTF-8 string literal using the u8"Hello world" syntax. See FAQ
  // for details. NB: Consider using AnchorFontGlyphRangesBuilder to build glyph ranges from
  // textual data.
  ANCHOR_API const AnchorWChar *GetGlyphRangesDefault();      // Basic Latin, Extended Latin
  ANCHOR_API const AnchorWChar *GetGlyphRangesKorean();       // Default + Korean characters
  ANCHOR_API const AnchorWChar *GetGlyphRangesJapanese();     // Default + Hiragana, Katakana,
                                                              // Half-Width, Selection of 2999
                                                              // Ideographs
  ANCHOR_API const AnchorWChar *GetGlyphRangesChineseFull();  // Default + Half-Width + Japanese
                                                              // Hiragana/Katakana + full set of
                                                              // about 21000 CJK Unified Ideographs
  ANCHOR_API const AnchorWChar *GetGlyphRangesChineseSimplifiedCommon();  // Default + Half-Width +
                                                                          // Japanese
                                                                          // Hiragana/Katakana +
                                                                          // set of 2500 CJK
                                                                          // Unified Ideographs for
                                                                          // common simplified
                                                                          // Chinese
  ANCHOR_API const AnchorWChar *GetGlyphRangesCyrillic();                 // Default + about 400 Cyrillic
                                                                          // characters
  ANCHOR_API const AnchorWChar *GetGlyphRangesThai();                     // Default + Thai characters
  ANCHOR_API const AnchorWChar *GetGlyphRangesVietnamese();               // Default + Vietnamese characters

  //-------------------------------------------
  // [BETA] Custom Rectangles/Glyphs API
  //-------------------------------------------

  // You can request arbitrary rectangles to be packed into the atlas, for your own purposes.
  // - After calling Build(), you can query the rectangle position and render your pixels.
  // - If you render colored output, set 'atlas->TexPixelsUseColors = true' as this may help some
  // backends decide of prefered texture format.
  // - You can also request your rectangles to be mapped as font glyph (given a font + Unicode
  // point),
  //   so you can render e.g. custom colorful icons and use them as regular glyphs.
  // - Read docs/FONTS.md for more details about using colorful icons.
  // - Note: this API may be redesigned later in order to support multi-monitor varying DPI
  // settings.
  ANCHOR_API int AddCustomRectRegular(int width, int height);
  ANCHOR_API int AddCustomRectFontGlyph(AnchorFont *font,
                                        AnchorWChar id,
                                        int width,
                                        int height,
                                        float advance_x,
                                        const wabi::GfVec2f &offset = wabi::GfVec2f(0, 0));
  AnchorFontAtlasCustomRect *GetCustomRectByIndex(int index)
  {
    ANCHOR_ASSERT(index >= 0);
    return &CustomRects[index];
  }

  // [Internal]
  ANCHOR_API void CalcCustomRectUV(const AnchorFontAtlasCustomRect *rect,
                                   wabi::GfVec2f *out_uv_min,
                                   wabi::GfVec2f *out_uv_max) const;
  ANCHOR_API bool GetMouseCursorTexData(ANCHOR_MouseCursor cursor,
                                        wabi::GfVec2f *out_offset,
                                        wabi::GfVec2f *out_size,
                                        wabi::GfVec2f out_uv_border[2],
                                        wabi::GfVec2f out_uv_fill[2]);

  //-------------------------------------------
  // Members
  //-------------------------------------------

  AnchorFontAtlasFlags Flags;  // Build flags (see AnchorFontAtlasFlags_)
  AnchorTextureID TexID;  // User data to refer to the texture once it has been uploaded to user's graphic
                          // systems. It is passed back to you during rendering via the ImDrawCmd structure.
  int TexDesiredWidth;    // Texture width desired by user before Build(). Must be a power-of-two. If
                          // have many glyphs your graphics API have texture size restrictions you
                          // may want to increase texture width to decrease height.
  int TexGlyphPadding;    // Padding between glyphs within texture in pixels. Defaults to 1. If your
                          // rendering method doesn't rely on bilinear filtering you may set this to
                          // 0.
  bool Locked;            // Marked as Locked by ANCHOR::NewFrame() so attempt to modify the atlas will
                          // assert.

  // [Internal]
  // NB: Access texture data via GetTexData*() calls! Which will setup a default font for you.
  bool TexPixelsUseColors;         // Tell whether our texture data is known to use colors (rather than
                                   // just alpha channel), in order to help backend select a format.
  unsigned char *TexPixelsAlpha8;  // 1 component per pixel, each component is unsigned 8-bit.
                                   // Total size = TexWidth * TexHeight
  unsigned int *TexPixelsRGBA32;   // 4 component per pixel, each component is unsigned 8-bit. Total
                                   // size = TexWidth * TexHeight * 4
  int TexWidth;                    // Texture width calculated during Build().
  int TexHeight;                   // Texture height calculated during Build().
  wabi::GfVec2f TexUvScale;        // = (1.0f/TexWidth, 1.0f/TexHeight)
  wabi::GfVec2f TexUvWhitePixel;   // Texture coordinates to a white pixel
  AnchorVector<AnchorFont *>
    Fonts;  // Hold all the fonts returned by AddFont*. Fonts[0] is the default font upon calling
            // ANCHOR::NewFrame(), use ANCHOR::PushFont()/PopFont() to change the current font.
  AnchorVector<AnchorFontAtlasCustomRect>
    CustomRects;                              // Rectangles for packing custom texture data into the atlas.
  AnchorVector<AnchorFontConfig> ConfigData;  // Configuration data
  wabi::GfVec4f TexUvLines[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];  // UVs for baked anti-aliased lines

  // [Internal] Font builder
  const AnchorFontBuilderIO
    *FontBuilderIO;               // Opaque interface to a font builder (default to stb_truetype, can be
                                  // changed to use FreeType by defining ANCHOR_ENABLE_FREETYPE).
  unsigned int FontBuilderFlags;  // Shared flags (for all fonts) for custom font builder. THIS IS
                                  // BUILD IMPLEMENTATION DEPENDENT. Per-font override is also
                                  // available in AnchorFontConfig.

  // [Internal] Packing data
  int PackIdMouseCursors;  // Custom texture rectangle ID for white pixel and mouse cursors
  int PackIdLines;         // Custom texture rectangle ID for baked anti-aliased lines

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
  typedef AnchorFontAtlasCustomRect CustomRect;             // OBSOLETED in 1.72+
  typedef AnchorFontGlyphRangesBuilder GlyphRangesBuilder;  // OBSOLETED in 1.67+
#  endif
};

// Font runtime data and rendering
// AnchorFontAtlas automatically loads a default embedded font for you when you call
// GetTexDataAsAlpha8() or GetTexDataAsRGBA32().
struct AnchorFont {
  // Members: Hot ~20/24 bytes (for CalcTextSize)
  AnchorVector<float> IndexAdvanceX;  // 12-16 // out //            // Sparse. Glyphs->AdvanceX in a directly
                                      // indexable way (cache-friendly for CalcTextSize functions which only
                                      // this this info, and are often bottleneck in large UI).
  float FallbackAdvanceX;             // 4     // out // = FallbackGlyph->AdvanceX
  float FontSize;  // 4     // in  //            // Height of characters/line, set during loading
                   // (don't change after loading)

  // Members: Hot ~28/40 bytes (for CalcTextSize + render loop)
  AnchorVector<AnchorWChar>
    IndexLookup;  // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
  AnchorVector<AnchorFontGlyph> Glyphs;  // 12-16 // out //            // All glyphs.
  const AnchorFontGlyph *FallbackGlyph;  // 4-8   // out // = FindGlyph(FontFallbackChar)

  // Members: Cold ~32/40 bytes
  AnchorFontAtlas *ContainerAtlas;  // 4-8   // out //            // What we has been loaded into
  const AnchorFontConfig
    *ConfigData;             // 4-8   // in  //            // Pointer within ContainerAtlas->ConfigData
  short ConfigDataCount;     // 2     // in  // ~ 1        // Number of AnchorFontConfig involved in
                             // creating this font. Bigger than 1 when merging multiple font sources
                             // into one AnchorFont.
  AnchorWChar FallbackChar;  // 2     // in  // = '?'      // Replacement character if a glyph
                             // isn't found. Only set via SetFallbackChar()
  AnchorWChar EllipsisChar;  // 2     // out // = -1       // Character used for ellipsis rendering.
  bool DirtyLookupTables;    // 1     // out //
  float Scale;            // 4     // in  // = 1.f      // Base font scale, multiplied by the per-window font
                          // scale which you can adjust with SetWindowFontScale()
  float Ascent, Descent;  // 4+4   // out //            // Ascent: distance from top to bottom of
                          // e.g. 'A' [0..FontSize]
  int MetricsTotalSurface;  // 4     // out //            // Total surface in pixels to get an idea
                            // of the font rasterization/texture cost (not exact, we approximate
                            // the cost of padding between glyphs)
  AnchorU8 Used4kPagesMap[(IM_UNICODE_CODEPOINT_MAX + 1) / 4096 /
                          8];  // 2 bytes if AnchorWChar=AnchorWChar16, 34 bytes if
                               // AnchorWChar==AnchorWChar32. Store 1-bit for each block of 4K
                               // codepoints that has one active glyph. This is mainly used to
                               // facilitate iterations across all used codepoints.

  // Methods
  ANCHOR_API AnchorFont();
  ANCHOR_API ~AnchorFont();
  ANCHOR_API const AnchorFontGlyph *FindGlyph(AnchorWChar c) const;
  ANCHOR_API const AnchorFontGlyph *FindGlyphNoFallback(AnchorWChar c) const;
  float GetCharAdvance(AnchorWChar c) const
  {
    return ((int)c < IndexAdvanceX.Size) ? IndexAdvanceX[(int)c] : FallbackAdvanceX;
  }
  bool IsLoaded() const
  {
    return ContainerAtlas != NULL;
  }
  const char *GetDebugName() const
  {
    return ConfigData ? ConfigData->Name : "<unknown>";
  }

  // 'max_width' stops rendering after a certain width (could be turned into a 2d size). FLT_MAX to
  // disable. 'wrap_width' enable automatic word-wrapping across multiple lines to fit into given
  // width. 0.0f to disable.
  ANCHOR_API wabi::GfVec2f CalcTextSizeA(float size,
                                         float max_width,
                                         float wrap_width,
                                         const char *text_begin,
                                         const char *text_end = NULL,
                                         const char **remaining = NULL) const;  // utf8
  ANCHOR_API const char *CalcWordWrapPositionA(float scale,
                                               const char *text,
                                               const char *text_end,
                                               float wrap_width) const;
  ANCHOR_API void RenderChar(ImDrawList *draw_list,
                             float size,
                             wabi::GfVec2f pos,
                             AnchorU32 col,
                             AnchorWChar c) const;
  ANCHOR_API void RenderText(ImDrawList *draw_list,
                             float size,
                             wabi::GfVec2f pos,
                             AnchorU32 col,
                             const wabi::GfVec4f &clip_rect,
                             const char *text_begin,
                             const char *text_end,
                             float wrap_width = 0.0f,
                             bool cpu_fine_clip = false) const;

  // [Internal] Don't use!
  ANCHOR_API void BuildLookupTable();
  ANCHOR_API void ClearOutputData();
  ANCHOR_API void GrowIndex(int new_size);
  ANCHOR_API void AddGlyph(const AnchorFontConfig *src_cfg,
                           AnchorWChar c,
                           float x0,
                           float y0,
                           float x1,
                           float y1,
                           float u0,
                           float v0,
                           float u1,
                           float v1,
                           float advance_x);
  ANCHOR_API void AddRemapChar(
    AnchorWChar dst,
    AnchorWChar src,
    bool overwrite_dst = true);  // Makes 'dst' character/glyph points to 'src' character/glyph.
                                 // Currently needs to be called AFTER fonts have been built.
  ANCHOR_API void SetGlyphVisible(AnchorWChar c, bool visible);
  ANCHOR_API void SetFallbackChar(AnchorWChar c);
  ANCHOR_API bool IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);
};

//-----------------------------------------------------------------------------
// [SECTION] Viewports
//-----------------------------------------------------------------------------

// Flags stored in ANCHORViewport::Flags
enum ANCHORViewportFlags_ {
  ANCHORViewportFlags_None = 0,
  ANCHORViewportFlags_IsPlatformWindow = 1 << 0,   // Represent a Platform Window
  ANCHORViewportFlags_IsPlatformMonitor = 1 << 1,  // Represent a Platform Monitor (unused yet)
  ANCHORViewportFlags_OwnedByApp = 1 << 2          // Platform Window: is created/managed by the
                                                   // application (rather than a ANCHOR backend)
};

// - Currently represents the Platform Window created by the application which is hosting our
// ANCHOR windows.
// - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple
// active viewports.
// - In the future we will extend this concept further to also represent Platform Monitor and
// support a "no main platform window" operation mode.
// - About Main Area vs Work Area:
//   - Main Area = entire viewport.
//   - Work Area = entire viewport minus sections used by main menu bars (for platform windows), or
//   by task bar (for platform monitor).
//   - Windows are generally trying to stay within the Work Area of their host viewport.
struct ANCHORViewport {
  ANCHORViewportFlags Flags;  // See ANCHORViewportFlags_
  wabi::GfVec2f Pos;          // Main Area: Position of the viewport (ANCHOR coordinates are the same as OS
                              // desktop/native coordinates)
  wabi::GfVec2f Size;         // Main Area: Size of the viewport.
  wabi::GfVec2f WorkPos;      // Work Area: Position of the viewport minus task bars, menus bars,
                              // status bars (>= Pos)
  wabi::GfVec2f WorkSize;     // Work Area: Size of the viewport minus task bars, menu bars, status
                              // bars (<= Size)

  ANCHORViewport()
  {
    memset(this, 0, sizeof(*this));
  }

  // Helpers
  wabi::GfVec2f GetCenter() const
  {
    return wabi::GfVec2f(Pos[0] + Size[0] * 0.5f, Pos[1] + Size[1] * 0.5f);
  }
  wabi::GfVec2f GetWorkCenter() const
  {
    return wabi::GfVec2f(WorkPos[0] + WorkSize[0] * 0.5f, WorkPos[1] + WorkSize[1] * 0.5f);
  }
};

//-----------------------------------------------------------------------------
// [SECTION] Obsolete functions and types
// (Will be removed! Read 'API BREAKING CHANGES' section in ANCHOR.cpp for details)
// Please keep your copy of ANCHOR up to date! Occasionally set '#define
// ANCHOR_DISABLE_OBSOLETE_FUNCTIONS' in ANCHOR_config.h to stay ahead.
//-----------------------------------------------------------------------------

#  ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
namespace ANCHOR {
// OBSOLETED in 1.81 (from February 2021)
ANCHOR_API bool ListBoxHeader(
  const char *label,
  int items_count,
  int height_in_items = -1);  // Helper to calculate size from items_count and height_in_items
static inline bool ListBoxHeader(const char *label, const wabi::GfVec2f &size = wabi::GfVec2f(0, 0))
{
  return BeginListBox(label, size);
}
static inline void ListBoxFooter()
{
  EndListBox();
}
// OBSOLETED in 1.79 (from August 2020)
static inline void OpenPopupContextItem(const char *str_id = NULL, ANCHOR_MouseButton mb = 1)
{
  OpenPopupOnItemClick(str_id, mb);
}  // Bool return value removed. Use IsWindowAppearing() in BeginPopup() instead. Renamed in 1.77,
   // renamed back in 1.79. Sorry!
// OBSOLETED in 1.78 (from June 2020)
// Old drag/sliders functions that took a 'float power = 1.0' argument instead of flags.
// For shared code, you can version check at compile-time with `#if ANCHOR_VERSION_NUM >= 17704`.
ANCHOR_API bool DragScalar(const char *label,
                           ANCHOR_DataType data_type,
                           void *p_data,
                           float v_speed,
                           const void *p_min,
                           const void *p_max,
                           const char *format,
                           float power);
ANCHOR_API bool DragScalarN(const char *label,
                            ANCHOR_DataType data_type,
                            void *p_data,
                            int components,
                            float v_speed,
                            const void *p_min,
                            const void *p_max,
                            const char *format,
                            float power);
static inline bool DragFloat(const char *label,
                             float *v,
                             float v_speed,
                             float v_min,
                             float v_max,
                             const char *format,
                             float power)
{
  return DragScalar(label, ANCHOR_DataType_Float, v, v_speed, &v_min, &v_max, format, power);
}
static inline bool DragFloat2(const char *label,
                              float v[2],
                              float v_speed,
                              float v_min,
                              float v_max,
                              const char *format,
                              float power)
{
  return DragScalarN(label, ANCHOR_DataType_Float, v, 2, v_speed, &v_min, &v_max, format, power);
}
static inline bool DragFloat3(const char *label,
                              float v[3],
                              float v_speed,
                              float v_min,
                              float v_max,
                              const char *format,
                              float power)
{
  return DragScalarN(label, ANCHOR_DataType_Float, v, 3, v_speed, &v_min, &v_max, format, power);
}
static inline bool DragFloat4(const char *label,
                              float v[4],
                              float v_speed,
                              float v_min,
                              float v_max,
                              const char *format,
                              float power)
{
  return DragScalarN(label, ANCHOR_DataType_Float, v, 4, v_speed, &v_min, &v_max, format, power);
}
ANCHOR_API bool SliderScalar(const char *label,
                             ANCHOR_DataType data_type,
                             void *p_data,
                             const void *p_min,
                             const void *p_max,
                             const char *format,
                             float power);
ANCHOR_API bool SliderScalarN(const char *label,
                              ANCHOR_DataType data_type,
                              void *p_data,
                              int components,
                              const void *p_min,
                              const void *p_max,
                              const char *format,
                              float power);
static inline bool SliderFloat(const char *label,
                               float *v,
                               float v_min,
                               float v_max,
                               const char *format,
                               float power)
{
  return SliderScalar(label, ANCHOR_DataType_Float, v, &v_min, &v_max, format, power);
}
static inline bool SliderFloat2(const char *label,
                                float v[2],
                                float v_min,
                                float v_max,
                                const char *format,
                                float power)
{
  return SliderScalarN(label, ANCHOR_DataType_Float, v, 2, &v_min, &v_max, format, power);
}
static inline bool SliderFloat3(const char *label,
                                float v[3],
                                float v_min,
                                float v_max,
                                const char *format,
                                float power)
{
  return SliderScalarN(label, ANCHOR_DataType_Float, v, 3, &v_min, &v_max, format, power);
}
static inline bool SliderFloat4(const char *label,
                                float v[4],
                                float v_min,
                                float v_max,
                                const char *format,
                                float power)
{
  return SliderScalarN(label, ANCHOR_DataType_Float, v, 4, &v_min, &v_max, format, power);
}
// OBSOLETED in 1.77 (from June 2020)
static inline bool BeginPopupContextWindow(const char *str_id, ANCHOR_MouseButton mb, bool over_items)
{
  return BeginPopupContextWindow(str_id, mb | (over_items ? 0 : ANCHORPopupFlags_NoOpenOverItems));
}
// OBSOLETED in 1.72 (from April 2019)
static inline void TreeAdvanceToLabelPos()
{
  SetCursorPosX(GetCursorPosX() + GetTreeNodeToLabelSpacing());
}
// OBSOLETED in 1.71 (from June 2019)
static inline void SetNextTreeNodeOpen(bool open, ANCHOR_Cond cond = 0)
{
  SetNextItemOpen(open, cond);
}
// OBSOLETED in 1.70 (from May 2019)
static inline float GetContentRegionAvailWidth()
{
  return GetContentRegionAvail()[0];
}
// OBSOLETED in 1.69 (from Mar 2019)
static inline ImDrawList *GetOverlayDrawList()
{
  return GetForegroundDrawList();
}
}  // namespace ANCHOR

// OBSOLETED in 1.82 (from Mars 2021): flags for AddRect(), AddRectFilled(), AddImageRounded(),
// PathRect()
typedef ImDrawFlags ImDrawCornerFlags;
enum ImDrawCornerFlags_ {
  ImDrawCornerFlags_None =
    ImDrawFlags_RoundCornersNone,  // Was == 0 prior to 1.82, this is now ==
                                   // ImDrawFlags_RoundCornersNone which is != 0 and not implicit
  ImDrawCornerFlags_TopLeft = ImDrawFlags_RoundCornersTopLeft,  // Was == 0x01 (1 << 0) prior to 1.82. Order
                                                                // matches ImDrawFlags_NoRoundCorner* flag
                                                                // (we exploit this internally).
  ImDrawCornerFlags_TopRight = ImDrawFlags_RoundCornersTopRight,     // Was == 0x02 (1 << 1) prior to 1.82.
  ImDrawCornerFlags_BotLeft = ImDrawFlags_RoundCornersBottomLeft,    // Was == 0x04 (1 << 2) prior to 1.82.
  ImDrawCornerFlags_BotRight = ImDrawFlags_RoundCornersBottomRight,  // Was == 0x08 (1 << 3) prior to 1.82.
  ImDrawCornerFlags_All = ImDrawFlags_RoundCornersAll,               // Was == 0x0F prior to 1.82
  ImDrawCornerFlags_Top = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight,
  ImDrawCornerFlags_Bot = ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight,
  ImDrawCornerFlags_Left = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotLeft,
  ImDrawCornerFlags_Right = ImDrawCornerFlags_TopRight | ImDrawCornerFlags_BotRight
};

#  endif  // #ifndef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS

//-----------------------------------------------------------------------------

#  if defined(__clang__)
#    pragma clang diagnostic pop
#  elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#  endif

#  ifdef _MSC_VER
#    pragma warning(pop)
#  endif

// Include ANCHOR_user.h at the end of ANCHOR_api.h (convenient for user to only explicitly include
// vanilla ANCHOR_api.h)
#  ifdef ANCHOR_INCLUDE_ANCHOR_USER_H
#    include "ANCHOR_user.h"
#  endif

#endif  // #ifndef ANCHOR_DISABLE

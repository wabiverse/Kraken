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

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 *
 * Anchor basic types.
 * Compliant with C.
 */

#include <stdint.h>

/**
 * Platform agnostic handles to backends. */
#define ANCHOR_DECLARE_HANDLE(name) \
  typedef struct name##__           \
  {                                 \
    int unused;                     \
  } * name

/**
 * ----- ANCHOR ENUMS ----- */

typedef enum
{
  ANCHOR_FAILURE = 0,
  ANCHOR_SUCCESS,
} eAnchorStatus;

typedef enum
{
  ANCHOR_BUTTON_MASK_NONE,
  ANCHOR_BUTTON_MASK_LEFT,
  ANCHOR_BUTTON_MASK_MIDDLE,
  ANCHOR_BUTTON_MASK_RIGHT,
  ANCHOR_BUTTON_MASK_BUTTON_4,
  ANCHOR_BUTTON_MASK_BUTTON_5,
  /**
   * Trackballs and programmable buttons */
  ANCHOR_BUTTON_MASK_BUTTON_6,
  ANCHOR_BUTTON_MASK_BUTTON_7,
  ANCHOR_BUTTON_MASK_MAX
} eAnchorButtonMask;

typedef enum
{
  ANCHOR_ModifierKeyLeftShift = 0,
  ANCHOR_ModifierKeyRightShift,
  ANCHOR_ModifierKeyLeftAlt,
  ANCHOR_ModifierKeyRightAlt,
  ANCHOR_ModifierKeyLeftControl,
  ANCHOR_ModifierKeyRightControl,
  ANCHOR_ModifierKeyOS,
  ANCHOR_ModifierKeyNumMasks
} eAnchorModifierKeyMask;

/**
 * Event Types ----------- */

typedef enum
{
  AnchorEventTypeUnknown = 0,

  AnchorEventTypeCursorMove,  /// Mouse move event
  AnchorEventTypeButtonDown,  /// Mouse button event
  AnchorEventTypeButtonUp,    /// Mouse button event
  AnchorEventTypeWheel,       /// Mouse wheel event
  AnchorEventTypeTrackpad,    /// Trackpad event

  AnchorEventTypeKeyDown,
  AnchorEventTypeKeyUp,

  AnchorEventTypeQuitRequest,

  AnchorEventTypeWindowClose,
  AnchorEventTypeWindowActivate,
  AnchorEventTypeWindowDeactivate,
  AnchorEventTypeWindowUpdate,
  AnchorEventTypeWindowSize,
  AnchorEventTypeWindowMove,
  AnchorEventTypeWindowDPIHintChanged,

  AnchorEventTypeDraggingEntered,
  AnchorEventTypeDraggingUpdated,
  AnchorEventTypeDraggingExited,
  AnchorEventTypeDraggingDropDone,

  AnchorEventTypeOpenMainFile,  // Needed for Cocoa to open double-clicked .usd(*) file at startup
  AnchorEventTypeNativeResolutionChange,  // Needed for Cocoa when window moves to other display

  AnchorEventTypeTimer,

  AnchorEventTypeImeCompositionStart,
  AnchorEventTypeImeComposition,
  AnchorEventTypeImeCompositionEnd,

  ANCHOR_NumEventTypes
} eAnchorEventType;

typedef enum
{
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
} eAnchorDrawingContextType;

typedef enum
{
  AnchorWindowStateNormal = 0,
  AnchorWindowStateMaximized,
  AnchorWindowStateMinimized,
  AnchorWindowStateFullScreen,
  AnchorWindowStateEmbedded,
} eAnchorWindowState;

typedef enum
{
  AnchorWindowOrderTop = 0,
  AnchorWindowOrderBottom,
} eAnchorWindowOrder;

/**
 * Enumeration for GetMouseCursor()
 * User code may request backend to
 * display given cursor by calling
 * SetMouseCursor() */
typedef enum
{
  ANCHOR_StandardCursorNone = -1,
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
} eAnchorStandardCursor;

/**
 * Introducing :: Tablet Support */

typedef enum
{
  AnchorTabletModeNone = 0,
  AnchorTabletModeStylus,
  AnchorTabletModeEraser,
} eAnchorTabletMode;

typedef enum
{
  AnchorTabletAutomatic = 0,
  AnchorTabletNative,
  AnchorTabletWintab,
} eAnchorTabletAPI;

typedef enum
{
  /**
   * Grab not set. */
  ANCHOR_GrabDisable = 0,
  /**
   * No cursor adjustments. */
  ANCHOR_GrabNormal,
  /**
   * Wrap the mouse location to prevent limiting screen bounds. */
  ANCHOR_GrabWrap,
  /**
   * Hide the mouse while grabbing and restore the original location on release
   * (used for number buttons and some other draggable UI elements). */
  ANCHOR_GrabHide,
} eAnchorGrabCursorMode;

typedef enum
{
  /**
   * Axis that cursor grab will wrap. */
  ANCHOR_GrabAxisNone = 0,
  ANCHOR_GrabAxisX = (1 << 0),
  ANCHOR_GrabAxisY = (1 << 1),
} eAnchorAxisFlag;

typedef enum
{
  AnchorKeyUnknown = -1,
  AnchorKeyBackSpace,
  AnchorKeyTab,
  AnchorKeyLinefeed,
  AnchorKeyClear,
  AnchorKeyEnter = 0x0D,

  AnchorKeyEsc = 0x1B,
  AnchorKeySpace = ' ',
  AnchorKeyQuote = 0x27,
  AnchorKeyComma = ',',
  AnchorKeyMinus = '-',
  AnchorKeyPlus = '+',
  AnchorKeyPeriod = '.',
  AnchorKeySlash = '/',

  /**
   * Number keys */
  AnchorKey0 = '0',
  AnchorKey1,
  AnchorKey2,
  AnchorKey3,
  AnchorKey4,
  AnchorKey5,
  AnchorKey6,
  AnchorKey7,
  AnchorKey8,
  AnchorKey9,

  AnchorKeySemicolon = ';',
  AnchorKeyEqual = '=',

  /**
   * Character keys */
  AnchorKeyA = 'A',
  AnchorKeyB,
  AnchorKeyC,
  AnchorKeyD,
  AnchorKeyE,
  AnchorKeyF,
  AnchorKeyG,
  AnchorKeyH,
  AnchorKeyI,
  AnchorKeyJ,
  AnchorKeyK,
  AnchorKeyL,
  AnchorKeyM,
  AnchorKeyN,
  AnchorKeyO,
  AnchorKeyP,
  AnchorKeyQ,
  AnchorKeyR,
  AnchorKeyS,
  AnchorKeyT,
  AnchorKeyU,
  AnchorKeyV,
  AnchorKeyW,
  AnchorKeyX,
  AnchorKeyY,
  AnchorKeyZ,

  AnchorKeyLeftBracket = '[',
  AnchorKeyRightBracket = ']',
  AnchorKeyBackslash = 0x5C,
  AnchorKeyAccentGrave = '`',

  AnchorKeyLeftShift = 0x100,
  AnchorKeyRightShift,
  AnchorKeyLeftControl,
  AnchorKeyRightControl,
  AnchorKeyLeftAlt,
  AnchorKeyRightAlt,
  AnchorKeyOS,      // Command key on Apple, Windows key(s) on Windows
  AnchorKeyGrLess,  // German PC only!
  AnchorKeyApp,     /* Also known as menu key. */

  AnchorKeyCapsLock,
  AnchorKeyNumLock,
  AnchorKeyScrollLock,

  AnchorKeyLeftArrow,
  AnchorKeyRightArrow,
  AnchorKeyUpArrow,
  AnchorKeyDownArrow,

  AnchorKeyPrintScreen,
  AnchorKeyPause,

  AnchorKeyInsert,
  AnchorKeyDelete,
  AnchorKeyHome,
  AnchorKeyEnd,
  AnchorKeyUpPage,
  AnchorKeyDownPage,

  /**
   * Numpad keys */
  AnchorKeyNumpad0,
  AnchorKeyNumpad1,
  AnchorKeyNumpad2,
  AnchorKeyNumpad3,
  AnchorKeyNumpad4,
  AnchorKeyNumpad5,
  AnchorKeyNumpad6,
  AnchorKeyNumpad7,
  AnchorKeyNumpad8,
  AnchorKeyNumpad9,
  AnchorKeyNumpadPeriod,
  AnchorKeyNumpadEnter,
  AnchorKeyNumpadPlus,
  AnchorKeyNumpadMinus,
  AnchorKeyNumpadAsterisk,
  AnchorKeyNumpadSlash,

  /**
   * Function keys */
  AnchorKeyF1,
  AnchorKeyF2,
  AnchorKeyF3,
  AnchorKeyF4,
  AnchorKeyF5,
  AnchorKeyF6,
  AnchorKeyF7,
  AnchorKeyF8,
  AnchorKeyF9,
  AnchorKeyF10,
  AnchorKeyF11,
  AnchorKeyF12,
  AnchorKeyF13,
  AnchorKeyF14,
  AnchorKeyF15,
  AnchorKeyF16,
  AnchorKeyF17,
  AnchorKeyF18,
  AnchorKeyF19,
  AnchorKeyF20,
  AnchorKeyF21,
  AnchorKeyF22,
  AnchorKeyF23,
  AnchorKeyF24,

  /**
   * Multimedia keypad buttons */
  AnchorKeyMediaPlay,
  AnchorKeyMediaStop,
  AnchorKeyMediaFirst,
  AnchorKeyMediaLast
} eAnchorKey;

typedef enum
{
  ANCHOR_UserSpecialDirDesktop,
  ANCHOR_UserSpecialDirDocuments,
  ANCHOR_UserSpecialDirDownloads,
  ANCHOR_UserSpecialDirMusic,
  ANCHOR_UserSpecialDirPictures,
  ANCHOR_UserSpecialDirVideos,
  ANCHOR_UserSpecialDirCaches,
} eAnchorUserSpecialDirTypes;

typedef enum
{
  ANCHOR_TrackpadEventUnknown = 0,
  ANCHOR_TrackpadEventScroll,
  ANCHOR_TrackpadEventRotate,
  ANCHOR_TrackpadEventSwipe, /* Reserved, not used for now */
  ANCHOR_TrackpadEventMagnify,
  ANCHOR_TrackpadEventSmartMagnify
} eAnchorTrackpadEventSubtypes;

typedef enum
{
  ANCHOR_DragnDropTypeUnknown = 0,
  /*Array of strings representing file names (full path) */
  ANCHOR_DragnDropTypeFilenames,
  /* Unformatted text UTF-8 string */
  ANCHOR_DragnDropTypeString,
  /*Bitmap image data */
  ANCHOR_DragnDropTypeBitmap
} eAnchorDragnDropTypes;

typedef enum
{
  ANCHOR_NotVisible = 0,
  ANCHOR_PartiallyVisible,
  ANCHOR_FullyVisible
} eAnchorVisibility;

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
 * Universe.
 * Set the Stage.
 */

#include "WM_api.h"

#include <wabi/usd/usd/timeCode.h>

WABI_NAMESPACE_BEGIN

/** Timer flags. */
enum eWmTimerFlags
{
  /** Do not attempt to free customdata pointer even if non-NULL. */
  WM_TIMER_NO_FREE_CUSTOM_DATA = 1 << 0,
};

enum eReportListFlags
{
  RPT_PRINT = (1 << 0),
  RPT_STORE = (1 << 1),
  RPT_FREE = (1 << 2),
  RPT_OP_HOLD = (1 << 3),
};

enum eWmCursorWrapType
{
  WM_CURSOR_WRAP_NONE = 0,
  WM_CURSOR_WRAP_X,
  WM_CURSOR_WRAP_Y,
  WM_CURSOR_WRAP_XY,
};

enum eWmOperatorContext
{
  /* if there's invoke, call it, otherwise exec */
  WM_OP_INVOKE_DEFAULT,
  WM_OP_INVOKE_REGION_WIN,
  WM_OP_INVOKE_REGION_CHANNELS,
  WM_OP_INVOKE_REGION_PREVIEW,
  WM_OP_INVOKE_AREA,
  WM_OP_INVOKE_SCREEN,
  /* only call exec */
  WM_OP_EXEC_DEFAULT,
  WM_OP_EXEC_REGION_WIN,
  WM_OP_EXEC_REGION_CHANNELS,
  WM_OP_EXEC_REGION_PREVIEW,
  WM_OP_EXEC_AREA,
  WM_OP_EXEC_SCREEN,
};

enum eWmOperatorFlag
{
  OP_IS_INVOKE = (1 << 0),
  OP_IS_REPEAT = (1 << 1),
  OP_IS_REPEAT_LAST = (1 << 1),
  OP_IS_MODAL_GRAB_CURSOR = (1 << 2),
  OP_IS_MODAL_CURSOR_REGION = (1 << 3),
};

enum eWmEventType
{
  EVENT_NONE = 0x0000,

  /* MOUSE : 0x000x, 0x001x */
  LEFTMOUSE = 0x0001,
  MIDDLEMOUSE = 0x0002,
  RIGHTMOUSE = 0x0003,
  MOUSEMOVE = 0x0004,
  /* Extra mouse buttons */
  BUTTON4MOUSE = 0x0007,
  BUTTON5MOUSE = 0x0008,
  /* More mouse buttons - can't use 9 and 10 here (wheel) */
  BUTTON6MOUSE = 0x0012,
  BUTTON7MOUSE = 0x0013,
  /* Extra trackpad gestures */
  MOUSEPAN = 0x000e,
  MOUSEZOOM = 0x000f,
  MOUSEROTATE = 0x0010,
  MOUSESMARTZOOM = 0x0017,

  /* defaults from anchor */
  WHEELUPMOUSE = 0x000a,
  WHEELDOWNMOUSE = 0x000b,
  /* mapped with userdef */
  WHEELINMOUSE = 0x000c,
  WHEELOUTMOUSE = 0x000d,
  INBETWEEN_MOUSEMOVE = 0x0011,

  WM_IME_COMPOSITE_START = 0x0014,
  WM_IME_COMPOSITE_EVENT = 0x0015,
  WM_IME_COMPOSITE_END = 0x0016,

  TABLET_STYLUS = 0x001a,
  TABLET_ERASER = 0x001b,

  EVT_ZEROKEY = 0x0030,  /* '0' (48). */
  EVT_ONEKEY = 0x0031,   /* '1' (49). */
  EVT_TWOKEY = 0x0032,   /* '2' (50). */
  EVT_THREEKEY = 0x0033, /* '3' (51). */
  EVT_FOURKEY = 0x0034,  /* '4' (52). */
  EVT_FIVEKEY = 0x0035,  /* '5' (53). */
  EVT_SIXKEY = 0x0036,   /* '6' (54). */
  EVT_SEVENKEY = 0x0037, /* '7' (55). */
  EVT_EIGHTKEY = 0x0038, /* '8' (56). */
  EVT_NINEKEY = 0x0039,  /* '9' (57). */

  EVT_AKEY = 0x0061, /* 'a' (97). */
  EVT_BKEY = 0x0062, /* 'b' (98). */
  EVT_CKEY = 0x0063, /* 'c' (99). */
  EVT_DKEY = 0x0064, /* 'd' (100). */
  EVT_EKEY = 0x0065, /* 'e' (101). */
  EVT_FKEY = 0x0066, /* 'f' (102). */
  EVT_GKEY = 0x0067, /* 'g' (103). */
  EVT_HKEY = 0x0068, /* 'h' (104). */
  EVT_IKEY = 0x0069, /* 'i' (105). */
  EVT_JKEY = 0x006a, /* 'j' (106). */
  EVT_KKEY = 0x006b, /* 'k' (107). */
  EVT_LKEY = 0x006c, /* 'l' (108). */
  EVT_MKEY = 0x006d, /* 'm' (109). */
  EVT_NKEY = 0x006e, /* 'n' (110). */
  EVT_OKEY = 0x006f, /* 'o' (111). */
  EVT_PKEY = 0x0070, /* 'p' (112). */
  EVT_QKEY = 0x0071, /* 'q' (113). */
  EVT_RKEY = 0x0072, /* 'r' (114). */
  EVT_SKEY = 0x0073, /* 's' (115). */
  EVT_TKEY = 0x0074, /* 't' (116). */
  EVT_UKEY = 0x0075, /* 'u' (117). */
  EVT_VKEY = 0x0076, /* 'v' (118). */
  EVT_WKEY = 0x0077, /* 'w' (119). */
  EVT_XKEY = 0x0078, /* 'x' (120). */
  EVT_YKEY = 0x0079, /* 'y' (121). */
  EVT_ZKEY = 0x007a, /* 'z' (122). */

  EVT_LEFTARROWKEY = 0x0089,  /* 137 */
  EVT_DOWNARROWKEY = 0x008a,  /* 138 */
  EVT_RIGHTARROWKEY = 0x008b, /* 139 */
  EVT_UPARROWKEY = 0x008c,    /* 140 */

  EVT_PAD0 = 0x0096, /* 150 */
  EVT_PAD1 = 0x0097, /* 151 */
  EVT_PAD2 = 0x0098, /* 152 */
  EVT_PAD3 = 0x0099, /* 153 */
  EVT_PAD4 = 0x009a, /* 154 */
  EVT_PAD5 = 0x009b, /* 155 */
  EVT_PAD6 = 0x009c, /* 156 */
  EVT_PAD7 = 0x009d, /* 157 */
  EVT_PAD8 = 0x009e, /* 158 */
  EVT_PAD9 = 0x009f, /* 159 */
  /* Key-pad keys. */
  EVT_PADASTERKEY = 0x00a0, /* 160 */
  EVT_PADSLASHKEY = 0x00a1, /* 161 */
  EVT_PADMINUS = 0x00a2,    /* 162 */
  EVT_PADENTER = 0x00a3,    /* 163 */
  EVT_PADPLUSKEY = 0x00a4,  /* 164 */

  EVT_PAUSEKEY = 0x00a5,    /* 165 */
  EVT_INSERTKEY = 0x00a6,   /* 166 */
  EVT_HOMEKEY = 0x00a7,     /* 167 */
  EVT_PAGEUPKEY = 0x00a8,   /* 168 */
  EVT_PAGEDOWNKEY = 0x00a9, /* 169 */
  EVT_ENDKEY = 0x00aa,      /* 170 */
  /* Note that 'PADPERIOD' is defined out-of-order. */
  EVT_UNKNOWNKEY = 0x00ab, /* 171 */
  EVT_OSKEY = 0x00ac,      /* 172 */
  EVT_GRLESSKEY = 0x00ad,  /* 173 */
  /* Media keys. */
  EVT_MEDIAPLAY = 0x00ae,  /* 174 */
  EVT_MEDIASTOP = 0x00af,  /* 175 */
  EVT_MEDIAFIRST = 0x00b0, /* 176 */
  EVT_MEDIALAST = 0x00b1,  /* 177 */
  /* Menu/App key. */
  EVT_APPKEY = 0x00b2, /* 178 */

  EVT_PADPERIOD = 0x00c7, /* 199 */

  EVT_CAPSLOCKKEY = 0x00d3, /* 211 */

  /* Modifier keys. */
  EVT_LEFTCTRLKEY = 0x00d4,   /* 212 */
  EVT_LEFTALTKEY = 0x00d5,    /* 213 */
  EVT_RIGHTALTKEY = 0x00d6,   /* 214 */
  EVT_RIGHTCTRLKEY = 0x00d7,  /* 215 */
  EVT_RIGHTSHIFTKEY = 0x00d8, /* 216 */
  EVT_LEFTSHIFTKEY = 0x00d9,  /* 217 */
  /* Special characters. */
  EVT_ESCKEY = 0x00da,          /* 218 */
  EVT_TABKEY = 0x00db,          /* 219 */
  EVT_RETKEY = 0x00dc,          /* 220 */
  EVT_SPACEKEY = 0x00dd,        /* 221 */
  EVT_LINEFEEDKEY = 0x00de,     /* 222 */
  EVT_BACKSPACEKEY = 0x00df,    /* 223 */
  EVT_DELKEY = 0x00e0,          /* 224 */
  EVT_SEMICOLONKEY = 0x00e1,    /* 225 */
  EVT_PERIODKEY = 0x00e2,       /* 226 */
  EVT_COMMAKEY = 0x00e3,        /* 227 */
  EVT_QUOTEKEY = 0x00e4,        /* 228 */
  EVT_ACCENTGRAVEKEY = 0x00e5,  /* 229 */
  EVT_MINUSKEY = 0x00e6,        /* 230 */
  EVT_PLUSKEY = 0x00e7,         /* 231 */
  EVT_SLASHKEY = 0x00e8,        /* 232 */
  EVT_BACKSLASHKEY = 0x00e9,    /* 233 */
  EVT_EQUALKEY = 0x00ea,        /* 234 */
  EVT_LEFTBRACKETKEY = 0x00eb,  /* 235 */
  EVT_RIGHTBRACKETKEY = 0x00ec, /* 236 */

  EVT_F1KEY = 0x012c,  /* 300 */
  EVT_F2KEY = 0x012d,  /* 301 */
  EVT_F3KEY = 0x012e,  /* 302 */
  EVT_F4KEY = 0x012f,  /* 303 */
  EVT_F5KEY = 0x0130,  /* 304 */
  EVT_F6KEY = 0x0131,  /* 305 */
  EVT_F7KEY = 0x0132,  /* 306 */
  EVT_F8KEY = 0x0133,  /* 307 */
  EVT_F9KEY = 0x0134,  /* 308 */
  EVT_F10KEY = 0x0135, /* 309 */
  EVT_F11KEY = 0x0136, /* 310 */
  EVT_F12KEY = 0x0137, /* 311 */
  EVT_F13KEY = 0x0138, /* 312 */
  EVT_F14KEY = 0x0139, /* 313 */
  EVT_F15KEY = 0x013a, /* 314 */
  EVT_F16KEY = 0x013b, /* 315 */
  EVT_F17KEY = 0x013c, /* 316 */
  EVT_F18KEY = 0x013d, /* 317 */
  EVT_F19KEY = 0x013e, /* 318 */
  EVT_F20KEY = 0x013f, /* 319 */
  EVT_F21KEY = 0x0140, /* 320 */
  EVT_F22KEY = 0x0141, /* 321 */
  EVT_F23KEY = 0x0142, /* 322 */
  EVT_F24KEY = 0x0143, /* 323 */

  /* XXX Those are mixed inside keyboard 'area'! */
  /* System: 0x010x */
  INPUTCHANGE = 0x0103,   /* Input connected or disconnected, (259). */
  WINDEACTIVATE = 0x0104, /* Window is deactivated, focus lost, (260). */
  /* Timer: 0x011x */
  TIMER = 0x0110,         /* Timer event, passed on to all queues (272). */
  TIMER0 = 0x0111,        /* Timer event, slot for internal use (273). */
  TIMER1 = 0x0112,        /* Timer event, slot for internal use (274). */
  TIMER2 = 0x0113,        /* Timer event, slot for internal use (275). */
  TIMERJOBS = 0x0114,     /* Timer event, jobs system (276). */
  TIMERAUTOSAVE = 0x0115, /* Timer event, autosave (277). */
  TIMERREPORT = 0x0116,   /* Timer event, reports (278). */
  TIMERREGION = 0x0117,   /* Timer event, region slide in/out (279). */
  TIMERNOTIFIER = 0x0118, /* Timer event, notifier sender (280). */
  TIMERF = 0x011F,        /* Last timer (287). */

  /* Actionzones, tweak, gestures: 0x500x, 0x501x */
  /* Keep in sync with IS_EVENT_ACTIONZONE(...). */
  EVT_ACTIONZONE_AREA = 0x5000,       /* 20480 */
  EVT_ACTIONZONE_REGION = 0x5001,     /* 20481 */
  EVT_ACTIONZONE_FULLSCREEN = 0x5011, /* 20497 */

  /* Tweak events:
   * Sent as additional event with the mouse coordinates
   * from where the initial click was placed. */

  /* tweak events for L M R mousebuttons */
  EVT_TWEAK_L = 0x5002, /* 20482 */
  EVT_TWEAK_M = 0x5003, /* 20483 */
  EVT_TWEAK_R = 0x5004, /* 20484 */
  /* 0x5010 (and lower) should be left to add other tweak types in the future. */

  /* 0x5011 is taken, see EVT_ACTIONZONE_FULLSCREEN */

  EVT_FILESELECT = 0x5020, /* 20512 */
  EVT_BUT_OPEN = 0x5021,   /* 20513 */
  EVT_MODAL_MAP = 0x5022,  /* 20514 */
  EVT_DROP = 0x5023,       /* 20515 */
  /* When value is 0, re-activate, when 1, don't re-activate the button under the cursor. */
  EVT_BUT_CANCEL = 0x5024, /* 20516 */

  /* could become gizmo callback */
  EVT_GIZMO_UPDATE = 0x5025, /* 20517 */
};

enum eWmOperatorType
{
  OPTYPE_REGISTER = (1 << 0),
  OPTYPE_UNDO = (1 << 1),
  OPTYPE_BLOCKING = (1 << 2),
  OPTYPE_MACRO = (1 << 3),

  OPTYPE_GRAB_CURSOR_XY = (1 << 4),
  OPTYPE_GRAB_CURSOR_X = (1 << 5),
  OPTYPE_GRAB_CURSOR_Y = (1 << 6),

  OPTYPE_PRESET = (1 << 7),

  OPTYPE_INTERNAL = (1 << 8),

  OPTYPE_LOCK_BYPASS = (1 << 9),
  OPTYPE_UNDO_GROUPED = (1 << 10),
};

struct wmTimer
{
  /** Set by timer user. */
  UsdTimeCode timestep;
  /** Set by timer user, goes to event system. */
  int event_type;
  /** Various flags controlling timer options, see below. */
  eWmTimerFlags flags;
  /** Set by timer user, to allow custom values. */
  void *customdata = NULL;

  /** Total running time in seconds. */
  UsdTimeCode duration;
  /** Time since previous step in seconds. */
  UsdTimeCode delta;

  /** Internal, last time timer was activated. */
  UsdTimeCode ltime;
  /** Internal, next time we want to activate the timer. */
  UsdTimeCode ntime;
  /** Internal, when the timer started. */
  UsdTimeCode stime;
  /** Internal, put timers to sleep when needed. */
  bool sleep;
};

struct ReportList
{
  /** ReportType. */
  int printlevel;
  /** ReportType. */
  int storelevel;
  int flag;
  wmTimer *reporttimer;
};

struct wmEvent
{
  /** Event code itself. */
  eWmEventType type;
  /** Press, release, scroll-value. */
  short val;
  /** Mouse pointer position, screen coord. */
  int x, y;
  int mval[2];
  /**
   * From, anchor if utf8 is enabled for the platform,
   * #CLI_str_utf8_size() must _always_ be valid, check
   * when assigning s we don't need to check on every access after.
   */
  char utf8_buf[6];
  /** From anchor, fallback if utf8 isn't set. */
  char ascii;

  /**
   * Generated by auto-repeat, note that this must only ever be set for keyboard events
   * where `ISKEYBOARD(event->type) == true`.
   *
   * See #KMI_REPEAT_IGNORE for details on how key-map handling uses this.
   */
  char is_repeat;

  /** The previous value of `type`. */
  short prevtype;
  /** The previous value of `val`. */
  short prevval;
  /** The time when the key is pressed, see #PIL_check_seconds_timer. */
  double prevclicktime;
  /** The location when the key is pressed (used to enforce drag thresholds). */
  int prevclickx, prevclicky;
  /**
   * The previous value of #wmEvent.x #wmEvent.y,
   * Unlike other previous state variables, this is set on any mouse motion.
   * Use `prevclickx` & `prevclicky` for the value at time of pressing.
   */
  int prevx, prevy;

  /** Modifier states. */
  /** 'oskey' is apple or windows-key, value denotes order of pressed. */
  short shift, ctrl, alt, oskey;
  /** Raw-key modifier (allow using any key as a modifier). */
  short keymodifier;

  /* Custom data. */
  /** Custom data type, stylus, 6dof, see wm_event_types.h */
  short custom;
  short customdatafree;
  int pad2;
  /** Ascii, unicode, mouse-coords, angles, vectors, NDOF data, drag-drop info. */
  void *customdata = NULL;

  /**
   * True if the operating system inverted the delta x/y values and resulting
   * `prevx`, `prevy` values, for natural scroll direction.
   * For absolute scroll direction, the delta must be negated again.
   */
  char is_direction_inverted;
};

WABI_NAMESPACE_END
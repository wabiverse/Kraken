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

/**
 * @file
 * Luxo.
 * The Universe Gets Animated.
 */

#include "LUXO_runtime.h"

#include "USD_api.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_default_tables.h"
#include "USD_factory.h"
#include "USD_file.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"

#include "KLI_utildefines.h"
#include "KLI_icons.h"
#include "KLI_string.h"
#include "KLI_path_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_screen.h"

#include "LUXO_runtime.h"
#include "LUXO_access.h"
#include "LUXO_define.h"

#include "LUXO_internal.h"

#include "WM_tokens.h"

#include <wabi/base/tf/token.h>

KRAKEN_NAMESPACE_BEGIN

const EnumPropertyItem luxo_enum_event_type_items[] = {
  /* - Note we abuse 'tooltip' message here to store a 'compact' form of some (too) long names.
  * - Intentionally excluded: #CAPSLOCKKEY, #UNKNOWNKEY.
  */
  {0,                         WM_ID_(NONE),                 0, "",                          ""            },
  {LEFTMOUSE,                 WM_ID_(LEFTMOUSE),            0, "Left Mouse",                "LMB"         },
  {MIDDLEMOUSE,               WM_ID_(MIDDLEMOUSE),          0, "Middle Mouse",              "MMB"         },
  {RIGHTMOUSE,                WM_ID_(RIGHTMOUSE),           0, "Right Mouse",               "RMB"         },
  {BUTTON4MOUSE,              WM_ID_(BUTTON4MOUSE),         0, "Button4 Mouse",             "MB4"         },
  {BUTTON5MOUSE,              WM_ID_(BUTTON5MOUSE),         0, "Button5 Mouse",             "MB5"         },
  {BUTTON6MOUSE,              WM_ID_(BUTTON6MOUSE),         0, "Button6 Mouse",             "MB6"         },
  {BUTTON7MOUSE,              WM_ID_(BUTTON7MOUSE),         0, "Button7 Mouse",             "MB7"         },
  LUXO_ENUM_ITEM_SEPR,
  {TABLET_STYLUS,             WM_ID_(PEN),                  0, "Pen",                       ""            },
  {TABLET_ERASER,             WM_ID_(ERASER),               0, "Eraser",                    ""            },
  LUXO_ENUM_ITEM_SEPR,
  {MOUSEMOVE,                 WM_ID_(MOUSEMOVE),            0, "Mouse Move",                "MsMov"       },
  {INBETWEEN_MOUSEMOVE,       WM_ID_(INBETWEEN_MOUSEMOVE),  0, "In-between Move",           "MsSubMov"    },
  {MOUSEPAN,                  WM_ID_(TRACKPADPAN),          0, "Mouse/Trackpad Pan",        "MsPan"       },
  {MOUSEZOOM,                 WM_ID_(TRACKPADZOOM),         0, "Mouse/Trackpad Zoom",       "MsZoom"      },
  {MOUSEROTATE,               WM_ID_(MOUSEROTATE),          0, "Mouse/Trackpad Rotate",     "MsRot"       },
  {MOUSESMARTZOOM,            WM_ID_(MOUSESMARTZOOM),       0, "Mouse/Trackpad Smart Zoom", "MsSmartZoom" },
  LUXO_ENUM_ITEM_SEPR,
  {WHEELUPMOUSE,              WM_ID_(WHEELUPMOUSE),         0, "Wheel Up",                  "WhUp"        },
  {WHEELDOWNMOUSE,            WM_ID_(WHEELDOWNMOUSE),       0, "Wheel Down",                "WhDown"      },
  {WHEELINMOUSE,              WM_ID_(WHEELINMOUSE),         0, "Wheel In",                  "WhIn"        },
  {WHEELOUTMOUSE,             WM_ID_(WHEELOUTMOUSE),        0, "Wheel Out",                 "WhOut"       },
  LUXO_ENUM_ITEM_SEPR,
  {EVT_AKEY,                  WM_ID_(A),                    0, "A",                         ""            },
  {EVT_BKEY,                  WM_ID_(B),                    0, "B",                         ""            },
  {EVT_CKEY,                  WM_ID_(C),                    0, "C",                         ""            },
  {EVT_DKEY,                  WM_ID_(D),                    0, "D",                         ""            },
  {EVT_EKEY,                  WM_ID_(E),                    0, "E",                         ""            },
  {EVT_FKEY,                  WM_ID_(F),                    0, "F",                         ""            },
  {EVT_GKEY,                  WM_ID_(G),                    0, "G",                         ""            },
  {EVT_HKEY,                  WM_ID_(H),                    0, "H",                         ""            },
  {EVT_IKEY,                  WM_ID_(I),                    0, "I",                         ""            },
  {EVT_JKEY,                  WM_ID_(J),                    0, "J",                         ""            },
  {EVT_KKEY,                  WM_ID_(K),                    0, "K",                         ""            },
  {EVT_LKEY,                  WM_ID_(L),                    0, "L",                         ""            },
  {EVT_MKEY,                  WM_ID_(M),                    0, "M",                         ""            },
  {EVT_NKEY,                  WM_ID_(N),                    0, "N",                         ""            },
  {EVT_OKEY,                  WM_ID_(O),                    0, "O",                         ""            },
  {EVT_PKEY,                  WM_ID_(P),                    0, "P",                         ""            },
  {EVT_QKEY,                  WM_ID_(Q),                    0, "Q",                         ""            },
  {EVT_RKEY,                  WM_ID_(R),                    0, "R",                         ""            },
  {EVT_SKEY,                  WM_ID_(S),                    0, "S",                         ""            },
  {EVT_TKEY,                  WM_ID_(T),                    0, "T",                         ""            },
  {EVT_UKEY,                  WM_ID_(U),                    0, "U",                         ""            },
  {EVT_VKEY,                  WM_ID_(V),                    0, "V",                         ""            },
  {EVT_WKEY,                  WM_ID_(W),                    0, "W",                         ""            },
  {EVT_XKEY,                  WM_ID_(X),                    0, "X",                         ""            },
  {EVT_YKEY,                  WM_ID_(Y),                    0, "Y",                         ""            },
  {EVT_ZKEY,                  WM_ID_(Z),                    0, "Z",                         ""            },
  LUXO_ENUM_ITEM_SEPR,
  {EVT_ZEROKEY,               WM_ID_(ZERO),                 0, "0",                         ""            },
  {EVT_ONEKEY,                WM_ID_(ONE),                  0, "1",                         ""            },
  {EVT_TWOKEY,                WM_ID_(TWO),                  0, "2",                         ""            },
  {EVT_THREEKEY,              WM_ID_(THREE),                0, "3",                         ""            },
  {EVT_FOURKEY,               WM_ID_(FOUR),                 0, "4",                         ""            },
  {EVT_FIVEKEY,               WM_ID_(FIVE),                 0, "5",                         ""            },
  {EVT_SIXKEY,                WM_ID_(SIX),                  0, "6",                         ""            },
  {EVT_SEVENKEY,              WM_ID_(SEVEN),                0, "7",                         ""            },
  {EVT_EIGHTKEY,              WM_ID_(EIGHT),                0, "8",                         ""            },
  {EVT_NINEKEY,               WM_ID_(NINE),                 0, "9",                         ""            },
  LUXO_ENUM_ITEM_SEPR,
  {EVT_LEFTCTRLKEY,           WM_ID_(LEFT_CTRL),            0, "Left Ctrl",                 "CtrlL"       },
  {EVT_LEFTALTKEY,            WM_ID_(LEFT_ALT),             0, "Left Alt",                  "AltL"        },
  {EVT_LEFTSHIFTKEY,          WM_ID_(LEFT_SHIFT),           0, "Left Shift",                "ShiftL"      },
  {EVT_RIGHTALTKEY,           WM_ID_(RIGHT_ALT),            0, "Right Alt",                 "AltR"        },
  {EVT_RIGHTCTRLKEY,          WM_ID_(RIGHT_CTRL),           0, "Right Ctrl",                "CtrlR"       },
  {EVT_RIGHTSHIFTKEY,         WM_ID_(RIGHT_SHIFT),          0, "Right Shift",               "ShiftR"      },
  LUXO_ENUM_ITEM_SEPR,
  {EVT_OSKEY,                 WM_ID_(OSKEY),                0, "OS Key",                    "Cmd"         },
  {EVT_APPKEY,                WM_ID_(APP),                  0, "Application",               "App"         },
  {EVT_GRLESSKEY,             WM_ID_(GRLESS),               0, "Grless",                    ""            },
  {EVT_ESCKEY,                WM_ID_(ESC),                  0, "Esc",                       ""            },
  {EVT_TABKEY,                WM_ID_(TAB),                  0, "Tab",                       ""            },
  {EVT_RETKEY,                WM_ID_(RET),                  0, "Return",                    "Enter"       },
  {EVT_SPACEKEY,              WM_ID_(SPACE),                0, "Spacebar",                  "Space"       },
  {EVT_LINEFEEDKEY,           WM_ID_(LINE_FEED),            0, "Line Feed",                 ""            },
  {EVT_BACKSPACEKEY,          WM_ID_(BACK_SPACE),           0, "Backspace",                 "BkSpace"     },
  {EVT_DELKEY,                WM_ID_(DEL),                  0, "Delete",                    "Del"         },
  {EVT_SEMICOLONKEY,          WM_ID_(SEMI_COLON),           0, ";",                         ""            },
  {EVT_PERIODKEY,             WM_ID_(PERIOD),               0, ".",                         ""            },
  {EVT_COMMAKEY,              WM_ID_(COMMA),                0, ",",                         ""            },
  {EVT_QUOTEKEY,              WM_ID_(QUOTE),                0, "\"",                        ""            },
  {EVT_ACCENTGRAVEKEY,        WM_ID_(ACCENT_GRAVE),         0, "`",                         ""            },
  {EVT_MINUSKEY,              WM_ID_(MINUS),                0, "-",                         ""            },
  {EVT_PLUSKEY,               WM_ID_(PLUS),                 0, "+",                         ""            },
  {EVT_SLASHKEY,              WM_ID_(SLASH),                0, "/",                         ""            },
  {EVT_BACKSLASHKEY,          WM_ID_(BACK_SLASH),           0, "\\",                        ""            },
  {EVT_EQUALKEY,              WM_ID_(EQUAL),                0, "=",                         ""            },
  {EVT_LEFTBRACKETKEY,        WM_ID_(LEFT_BRACKET),         0, "[",                         ""            },
  {EVT_RIGHTBRACKETKEY,       WM_ID_(RIGHT_BRACKET),        0, "]",                         ""            },
  {EVT_LEFTARROWKEY,          WM_ID_(LEFT_ARROW),           0, "Left Arrow",                "←"         },
  {EVT_DOWNARROWKEY,          WM_ID_(DOWN_ARROW),           0, "Down Arrow",                "↓"         },
  {EVT_RIGHTARROWKEY,         WM_ID_(RIGHT_ARROW),          0, "Right Arrow",               "→"         },
  {EVT_UPARROWKEY,            WM_ID_(UP_ARROW),             0, "Up Arrow",                  "↑"         },
  {EVT_PAD2,                  WM_ID_(NUMPAD_2),             0, "Numpad 2",                  "Pad2"        },
  {EVT_PAD4,                  WM_ID_(NUMPAD_4),             0, "Numpad 4",                  "Pad4"        },
  {EVT_PAD6,                  WM_ID_(NUMPAD_6),             0, "Numpad 6",                  "Pad6"        },
  {EVT_PAD8,                  WM_ID_(NUMPAD_8),             0, "Numpad 8",                  "Pad8"        },
  {EVT_PAD1,                  WM_ID_(NUMPAD_1),             0, "Numpad 1",                  "Pad1"        },
  {EVT_PAD3,                  WM_ID_(NUMPAD_3),             0, "Numpad 3",                  "Pad3"        },
  {EVT_PAD5,                  WM_ID_(NUMPAD_5),             0, "Numpad 5",                  "Pad5"        },
  {EVT_PAD7,                  WM_ID_(NUMPAD_7),             0, "Numpad 7",                  "Pad7"        },
  {EVT_PAD9,                  WM_ID_(NUMPAD_9),             0, "Numpad 9",                  "Pad9"        },
  {EVT_PADPERIOD,             WM_ID_(NUMPAD_PERIOD),        0, "Numpad .",                  "Pad."        },
  {EVT_PADSLASHKEY,           WM_ID_(NUMPAD_SLASH),         0, "Numpad /",                  "Pad/"        },
  {EVT_PADASTERKEY,           WM_ID_(NUMPAD_ASTERIX),       0, "Numpad *",                  "Pad*"        },
  {EVT_PAD0,                  WM_ID_(NUMPAD_0),             0, "Numpad 0",                  "Pad0"        },
  {EVT_PADMINUS,              WM_ID_(NUMPAD_MINUS),         0, "Numpad -",                  "Pad-"        },
  {EVT_PADENTER,              WM_ID_(NUMPAD_ENTER),         0, "Numpad Enter",              "PadEnter"    },
  {EVT_PADPLUSKEY,            WM_ID_(NUMPAD_PLUS),          0, "Numpad +",                  "Pad+"        },
  {EVT_F1KEY,                 WM_ID_(F1),                   0, "F1",                        ""            },
  {EVT_F2KEY,                 WM_ID_(F2),                   0, "F2",                        ""            },
  {EVT_F3KEY,                 WM_ID_(F3),                   0, "F3",                        ""            },
  {EVT_F4KEY,                 WM_ID_(F4),                   0, "F4",                        ""            },
  {EVT_F5KEY,                 WM_ID_(F5),                   0, "F5",                        ""            },
  {EVT_F6KEY,                 WM_ID_(F6),                   0, "F6",                        ""            },
  {EVT_F7KEY,                 WM_ID_(F7),                   0, "F7",                        ""            },
  {EVT_F8KEY,                 WM_ID_(F8),                   0, "F8",                        ""            },
  {EVT_F9KEY,                 WM_ID_(F9),                   0, "F9",                        ""            },
  {EVT_F10KEY,                WM_ID_(F10),                  0, "F10",                       ""            },
  {EVT_F11KEY,                WM_ID_(F11),                  0, "F11",                       ""            },
  {EVT_F12KEY,                WM_ID_(F12),                  0, "F12",                       ""            },
  {EVT_F13KEY,                WM_ID_(F13),                  0, "F13",                       ""            },
  {EVT_F14KEY,                WM_ID_(F14),                  0, "F14",                       ""            },
  {EVT_F15KEY,                WM_ID_(F15),                  0, "F15",                       ""            },
  {EVT_F16KEY,                WM_ID_(F16),                  0, "F16",                       ""            },
  {EVT_F17KEY,                WM_ID_(F17),                  0, "F17",                       ""            },
  {EVT_F18KEY,                WM_ID_(F18),                  0, "F18",                       ""            },
  {EVT_F19KEY,                WM_ID_(F19),                  0, "F19",                       ""            },
  {EVT_F20KEY,                WM_ID_(F20),                  0, "F20",                       ""            },
  {EVT_F21KEY,                WM_ID_(F21),                  0, "F21",                       ""            },
  {EVT_F22KEY,                WM_ID_(F22),                  0, "F22",                       ""            },
  {EVT_F23KEY,                WM_ID_(F23),                  0, "F23",                       ""            },
  {EVT_F24KEY,                WM_ID_(F24),                  0, "F24",                       ""            },
  {EVT_PAUSEKEY,              WM_ID_(PAUSE),                0, "Pause",                     ""            },
  {EVT_INSERTKEY,             WM_ID_(INSERT),               0, "Insert",                    "Ins"         },
  {EVT_HOMEKEY,               WM_ID_(HOME),                 0, "Home",                      ""            },
  {EVT_PAGEUPKEY,             WM_ID_(PAGE_UP),              0, "Page Up",                   "PgUp"        },
  {EVT_PAGEDOWNKEY,           WM_ID_(PAGE_DOWN),            0, "Page Down",                 "PgDown"      },
  {EVT_ENDKEY,                WM_ID_(END),                  0, "End",                       ""            },
  LUXO_ENUM_ITEM_SEPR,
  {EVT_MEDIAPLAY,             WM_ID_(MEDIA_PLAY),           0, "Media Play/Pause",          ">/||"        },
  {EVT_MEDIASTOP,             WM_ID_(MEDIA_STOP),           0, "Media Stop",                "Stop"        },
  {EVT_MEDIAFIRST,            WM_ID_(MEDIA_FIRST),          0, "Media First",               "|<<"         },
  {EVT_MEDIALAST,             WM_ID_(MEDIA_LAST),           0, "Media Last",                ">>|"         },
  LUXO_ENUM_ITEM_SEPR,
  {KM_TEXTINPUT,              WM_ID_(TEXTINPUT),            0, "Text Input",                "TxtIn"       },
  LUXO_ENUM_ITEM_SEPR,
  {WINDEACTIVATE,             WM_ID_(WINDOW_DEACTIVATE),    0, "Window Deactivate",         ""            },
  {TIMER,                     WM_ID_(TIMER),                0, "Timer",                     "Tmr"         },
  {TIMER0,                    WM_ID_(TIMER0),               0, "Timer 0",                   "Tmr0"        },
  {TIMER1,                    WM_ID_(TIMER1),               0, "Timer 1",                   "Tmr1"        },
  {TIMER2,                    WM_ID_(TIMER2),               0, "Timer 2",                   "Tmr2"        },
  {TIMERJOBS,                 WM_ID_(TIMER_JOBS),           0, "Timer Jobs",                "TmrJob"      },
  {TIMERAUTOSAVE,             WM_ID_(TIMER_AUTOSAVE),       0, "Timer Autosave",            "TmrSave"     },
  {TIMERREPORT,               WM_ID_(TIMER_REPORT),         0, "Timer Report",              "TmrReport"   },
  {TIMERREGION,               WM_ID_(TIMERREGION),          0, "Timer Region",              "TmrReg"      },
  LUXO_ENUM_ITEM_SEPR,
  {NDOF_MOTION,               WM_ID_(NDOF_MOTION),          0, "NDOF Motion",               "NdofMov"     },
 /* buttons on all 3dconnexion devices */
  {NDOF_BUTTON_MENU,          WM_ID_(NDOF_BUTTON_MENU),     0, "NDOF Menu",                 "NdofMenu"    },
  {NDOF_BUTTON_FIT,           WM_ID_(NDOF_BUTTON_FIT),      0, "NDOF Fit",                  "NdofFit"     },
 /* view buttons */
  {NDOF_BUTTON_TOP,           WM_ID_(NDOF_BUTTON_TOP),      0, "NDOF Top",                  "Ndof↑"     },
  {NDOF_BUTTON_BOTTOM,        WM_ID_(NDOF_BUTTON_BOTTOM),   0, "NDOF Bottom",               "Ndof↓"     },
  {NDOF_BUTTON_LEFT,          WM_ID_(NDOF_BUTTON_LEFT),     0, "NDOF Left",                 "Ndof←"     },
  {NDOF_BUTTON_RIGHT,         WM_ID_(NDOF_BUTTON_RIGHT),    0, "NDOF Right",                "Ndof→"     },
  {NDOF_BUTTON_FRONT,         WM_ID_(NDOF_BUTTON_FRONT),    0, "NDOF Front",                "NdofFront"   },
  {NDOF_BUTTON_BACK,          WM_ID_(NDOF_BUTTON_BACK),     0, "NDOF Back",                 "NdofBack"    },
 /* more views */
  {NDOF_BUTTON_ISO1,          WM_ID_(NDOF_BUTTON_ISO1),     0, "NDOF Isometric 1",          "NdofIso1"    },
  {NDOF_BUTTON_ISO2,          WM_ID_(NDOF_BUTTON_ISO2),     0, "NDOF Isometric 2",          "NdofIso2"    },
 /* 90 degree rotations */
  {NDOF_BUTTON_ROLL_CW,       WM_ID_(NDOF_BUTTON_ROLL_CW),  0, "NDOF Roll CW",              "NdofRCW"     },
  {NDOF_BUTTON_ROLL_CCW,      WM_ID_(NDOF_BUTTON_ROLL_CCW), 0, "NDOF Roll CCW",             "NdofRCCW"    },
  {NDOF_BUTTON_SPIN_CW,       WM_ID_(NDOF_BUTTON_SPIN_CW),  0, "NDOF Spin CW",              "NdofSCW"     },
  {NDOF_BUTTON_SPIN_CCW,      WM_ID_(NDOF_BUTTON_SPIN_CCW), 0, "NDOF Spin CCW",             "NdofSCCW"    },
  {NDOF_BUTTON_TILT_CW,       WM_ID_(NDOF_BUTTON_TILT_CW),  0, "NDOF Tilt CW",              "NdofTCW"     },
  {NDOF_BUTTON_TILT_CCW,      WM_ID_(NDOF_BUTTON_TILT_CCW), 0, "NDOF Tilt CCW",             "NdofTCCW"    },
 /* device control */
  {NDOF_BUTTON_ROTATE,        WM_ID_(NDOF_BUTTON_ROTATE),   0, "NDOF Rotate",               "NdofRot"     },
  {NDOF_BUTTON_PANZOOM,       WM_ID_(NDOF_BUTTON_PANZOOM),  0, "NDOF Pan/Zoom",             "NdofPanZoom" },
  {NDOF_BUTTON_DOMINANT,      WM_ID_(NDOF_BUTTON_DOMINANT), 0, "NDOF Dominant",             "NdofDom"     },
  {NDOF_BUTTON_PLUS,          WM_ID_(NDOF_BUTTON_PLUS),     0, "NDOF Plus",                 "Ndof+"       },
  {NDOF_BUTTON_MINUS,         WM_ID_(NDOF_BUTTON_MINUS),    0, "NDOF Minus",                "Ndof-"       },
 /* general-purpose buttons */
  {NDOF_BUTTON_1,             WM_ID_(NDOF_BUTTON_1),        0, "NDOF Button 1",             "NdofB1"      },
  {NDOF_BUTTON_2,             WM_ID_(NDOF_BUTTON_2),        0, "NDOF Button 2",             "NdofB2"      },
  {NDOF_BUTTON_3,             WM_ID_(NDOF_BUTTON_3),        0, "NDOF Button 3",             "NdofB3"      },
  {NDOF_BUTTON_4,             WM_ID_(NDOF_BUTTON_4),        0, "NDOF Button 4",             "NdofB4"      },
  {NDOF_BUTTON_5,             WM_ID_(NDOF_BUTTON_5),        0, "NDOF Button 5",             "NdofB5"      },
  {NDOF_BUTTON_6,             WM_ID_(NDOF_BUTTON_6),        0, "NDOF Button 6",             "NdofB6"      },
  {NDOF_BUTTON_7,             WM_ID_(NDOF_BUTTON_7),        0, "NDOF Button 7",             "NdofB7"      },
  {NDOF_BUTTON_8,             WM_ID_(NDOF_BUTTON_8),        0, "NDOF Button 8",             "NdofB8"      },
  {NDOF_BUTTON_9,             WM_ID_(NDOF_BUTTON_9),        0, "NDOF Button 9",             "NdofB9"      },
  {NDOF_BUTTON_10,            WM_ID_(NDOF_BUTTON_10),       0, "NDOF Button 10",            "NdofB10"     },
  {NDOF_BUTTON_A,             WM_ID_(NDOF_BUTTON_A),        0, "NDOF Button A",             "NdofBA"      },
  {NDOF_BUTTON_B,             WM_ID_(NDOF_BUTTON_B),        0, "NDOF Button B",             "NdofBB"      },
  {NDOF_BUTTON_C,             WM_ID_(NDOF_BUTTON_C),        0, "NDOF Button C",             "NdofBC"      },
 /* Action Zones. */
  {EVT_ACTIONZONE_AREA,       WM_ID_(ACTIONZONE_AREA),      0, "ActionZone Area",           "AZone Area"  },
  {EVT_ACTIONZONE_REGION,     WM_ID_(ACTIONZONE_REGION),    0, "ActionZone Region",         "AZone Region"},
  {EVT_ACTIONZONE_FULLSCREEN,
   WM_ID_(ACTIONZONE_FULLSCREEN),
   0,                                                          "ActionZone Fullscreen",
   "AZone FullScr"                                                                                        },
 /* xr */
  {EVT_XR_ACTION,             WM_ID_(XR_ACTION),            0, "XR Action",                 ""            },
  {0,                         wabi::TfToken(),              0, NULL,                        NULL          },
};

static void prim_def_operator(KrakenPRIM *wm)
{
  KrakenPRIM *op;

  op = PRIM_def_struct(wm, SdfPath("Operator"));
  PRIM_def_struct_ui_text(op, "Operator", "Storage of an operator being executed, or registered after execution");
  // PRIM_def_struct_refine_func(kstage, "rna_Operator_refine");
  // PRIM_def_struct_flag(kstage, STRUCT_PUBLIC_NAMESPACE_INHERIT);
}

void PRIM_def_wm(const KrakenSTAGE &kstage)
{
  KrakenPRIM *wm;

  wm = PRIM_def_struct_ptr(kstage, SdfPath("WindowManager"));
  
  /* all types that make up wm... */
  
  prim_def_operator(wm);

  /* ... */
}

KRAKEN_NAMESPACE_END
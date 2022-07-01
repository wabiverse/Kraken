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
 * Luxo.
 * The Universe Gets Animated.
 */

#include "LUXO_runtime.h"

#include "KLI_icons.h"
#include "KLI_string_utils.h"
#include "KLI_path_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_screen.h"

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

#include "LUXO_internal.h"
#include "LUXO_runtime.h"
#include "LUXO_define.h"

WABI_NAMESPACE_BEGIN

KRAKEN_REGISTER_LUXO_RUNTIME_TYPES(TfEnum)
{
  LUXO_ADD_ENUM_PROP(LEFTMOUSE, "LEFTMOUSE", ICON_NONE, "Left Mouse", "LMB");
  LUXO_ADD_ENUM_PROP(MIDDLEMOUSE, "MIDDLEMOUSE", ICON_NONE, "Middle Mouse", "MMB");
  LUXO_ADD_ENUM_PROP(RIGHTMOUSE, "RIGHTMOUSE", ICON_NONE, "Right Mouse", "RMB");
  LUXO_ADD_ENUM_PROP(BUTTON4MOUSE, "BUTTON4MOUSE", ICON_NONE, "Button4 Mouse", "MB4");
  LUXO_ADD_ENUM_PROP(BUTTON5MOUSE, "BUTTON5MOUSE", ICON_NONE, "Button5 Mouse", "MB5");
  LUXO_ADD_ENUM_PROP(BUTTON6MOUSE, "BUTTON6MOUSE", ICON_NONE, "Button6 Mouse", "MB6");
  LUXO_ADD_ENUM_PROP(BUTTON7MOUSE, "BUTTON7MOUSE", ICON_NONE, "Button7 Mouse", "MB7");
  LUXO_ADD_ENUM_PROP(TABLET_STYLUS, "PEN", ICON_NONE, "Pen", "");
  LUXO_ADD_ENUM_PROP(TABLET_ERASER, "ERASER", ICON_NONE, "Eraser", "");
  LUXO_ADD_ENUM_PROP(MOUSEMOVE, "MOUSEMOVE", ICON_NONE, "Mouse Move", "MsMov");
  LUXO_ADD_ENUM_PROP(INBETWEEN_MOUSEMOVE,
                     "INBETWEEN_MOUSEMOVE",
                     ICON_NONE,
                     "In-between Move",
                     "MsSubMov");
  LUXO_ADD_ENUM_PROP(MOUSEPAN, "TRACKPADPAN", ICON_NONE, "Mouse/Trackpad Pan", "MsPan");
  LUXO_ADD_ENUM_PROP(MOUSEZOOM, "TRACKPADZOOM", ICON_NONE, "Mouse/Trackpad Zoom", "MsZoom");
  LUXO_ADD_ENUM_PROP(MOUSEROTATE, "MOUSEROTATE", ICON_NONE, "Mouse/Trackpad Rotate", "MsRot");
  LUXO_ADD_ENUM_PROP(MOUSESMARTZOOM,
                     "MOUSESMARTZOOM",
                     ICON_NONE,
                     "Mouse/Trackpad Smart Zoom",
                     "MsSmartZoom");
  LUXO_ADD_ENUM_PROP(WHEELUPMOUSE, "WHEELUPMOUSE", ICON_NONE, "Wheel Up", "WhUp");
  LUXO_ADD_ENUM_PROP(WHEELDOWNMOUSE, "WHEELDOWNMOUSE", ICON_NONE, "Wheel Down", "WhDown");
  LUXO_ADD_ENUM_PROP(WHEELINMOUSE, "WHEELINMOUSE", ICON_NONE, "Wheel In", "WhIn");
  LUXO_ADD_ENUM_PROP(WHEELOUTMOUSE, "WHEELOUTMOUSE", ICON_NONE, "Wheel Out", "WhOut");
  LUXO_ADD_ENUM_PROP(EVT_TWEAK_L, "EVT_TWEAK_L", ICON_NONE, "Tweak Left", "TwkL");
  LUXO_ADD_ENUM_PROP(EVT_TWEAK_M, "EVT_TWEAK_M", ICON_NONE, "Tweak Middle", "TwkM");
  LUXO_ADD_ENUM_PROP(EVT_TWEAK_R, "EVT_TWEAK_R", ICON_NONE, "Tweak Right", "TwkR");
  LUXO_ADD_ENUM_PROP(EVT_AKEY, "A", ICON_NONE, "A", "");
  LUXO_ADD_ENUM_PROP(EVT_BKEY, "B", ICON_NONE, "B", "");
  LUXO_ADD_ENUM_PROP(EVT_CKEY, "C", ICON_NONE, "C", "");
  LUXO_ADD_ENUM_PROP(EVT_DKEY, "D", ICON_NONE, "D", "");
  LUXO_ADD_ENUM_PROP(EVT_EKEY, "E", ICON_NONE, "E", "");
  LUXO_ADD_ENUM_PROP(EVT_FKEY, "F", ICON_NONE, "F", "");
  LUXO_ADD_ENUM_PROP(EVT_GKEY, "G", ICON_NONE, "G", "");
  LUXO_ADD_ENUM_PROP(EVT_HKEY, "H", ICON_NONE, "H", "");
  LUXO_ADD_ENUM_PROP(EVT_IKEY, "I", ICON_NONE, "I", "");
  LUXO_ADD_ENUM_PROP(EVT_JKEY, "J", ICON_NONE, "J", "");
  LUXO_ADD_ENUM_PROP(EVT_KKEY, "K", ICON_NONE, "K", "");
  LUXO_ADD_ENUM_PROP(EVT_LKEY, "L", ICON_NONE, "L", "");
  LUXO_ADD_ENUM_PROP(EVT_MKEY, "M", ICON_NONE, "M", "");
  LUXO_ADD_ENUM_PROP(EVT_NKEY, "N", ICON_NONE, "N", "");
  LUXO_ADD_ENUM_PROP(EVT_OKEY, "O", ICON_NONE, "O", "");
  LUXO_ADD_ENUM_PROP(EVT_PKEY, "P", ICON_NONE, "P", "");
  LUXO_ADD_ENUM_PROP(EVT_QKEY, "Q", ICON_NONE, "Q", "");
  LUXO_ADD_ENUM_PROP(EVT_RKEY, "R", ICON_NONE, "R", "");
  LUXO_ADD_ENUM_PROP(EVT_SKEY, "S", ICON_NONE, "S", "");
  LUXO_ADD_ENUM_PROP(EVT_TKEY, "T", ICON_NONE, "T", "");
  LUXO_ADD_ENUM_PROP(EVT_UKEY, "U", ICON_NONE, "U", "");
  LUXO_ADD_ENUM_PROP(EVT_VKEY, "V", ICON_NONE, "V", "");
  LUXO_ADD_ENUM_PROP(EVT_WKEY, "W", ICON_NONE, "W", "");
  LUXO_ADD_ENUM_PROP(EVT_XKEY, "X", ICON_NONE, "X", "");
  LUXO_ADD_ENUM_PROP(EVT_YKEY, "Y", ICON_NONE, "Y", "");
  LUXO_ADD_ENUM_PROP(EVT_ZKEY, "Z", ICON_NONE, "Z", "");
  LUXO_ADD_ENUM_PROP(EVT_ZEROKEY, "ZERO", ICON_NONE, "0", "");
  LUXO_ADD_ENUM_PROP(EVT_ONEKEY, "ONE", ICON_NONE, "1", "");
  LUXO_ADD_ENUM_PROP(EVT_TWOKEY, "TWO", ICON_NONE, "2", "");
  LUXO_ADD_ENUM_PROP(EVT_THREEKEY, "THREE", ICON_NONE, "3", "");
  LUXO_ADD_ENUM_PROP(EVT_FOURKEY, "FOUR", ICON_NONE, "4", "");
  LUXO_ADD_ENUM_PROP(EVT_FIVEKEY, "FIVE", ICON_NONE, "5", "");
  LUXO_ADD_ENUM_PROP(EVT_SIXKEY, "SIX", ICON_NONE, "6", "");
  LUXO_ADD_ENUM_PROP(EVT_SEVENKEY, "SEVEN", ICON_NONE, "7", "");
  LUXO_ADD_ENUM_PROP(EVT_EIGHTKEY, "EIGHT", ICON_NONE, "8", "");
  LUXO_ADD_ENUM_PROP(EVT_NINEKEY, "NINE", ICON_NONE, "9", "");
  LUXO_ADD_ENUM_PROP(EVT_LEFTCTRLKEY, "LEFT_CTRL", ICON_NONE, "Left Ctrl", "CtrlL");
  LUXO_ADD_ENUM_PROP(EVT_LEFTALTKEY, "LEFT_ALT", ICON_NONE, "Left Alt", "AltL");
  LUXO_ADD_ENUM_PROP(EVT_LEFTSHIFTKEY, "LEFT_SHIFT", ICON_NONE, "Left Shift", "ShiftL");
  LUXO_ADD_ENUM_PROP(EVT_RIGHTALTKEY, "RIGHT_ALT", ICON_NONE, "Right Alt", "AltR");
  LUXO_ADD_ENUM_PROP(EVT_RIGHTCTRLKEY, "RIGHT_CTRL", ICON_NONE, "Right Ctrl", "CtrlR");
  LUXO_ADD_ENUM_PROP(EVT_RIGHTSHIFTKEY, "RIGHT_SHIFT", ICON_NONE, "Right Shift", "ShiftR");
  LUXO_ADD_ENUM_PROP(EVT_OSKEY, "OSKEY", ICON_NONE, "OS Key", "Cmd");
  LUXO_ADD_ENUM_PROP(EVT_APPKEY, "APP", ICON_NONE, "Application", "App");
  LUXO_ADD_ENUM_PROP(EVT_GRLESSKEY, "GRLESS", ICON_NONE, "Grless", "");
  LUXO_ADD_ENUM_PROP(EVT_ESCKEY, "ESC", ICON_NONE, "Esc", "");
  LUXO_ADD_ENUM_PROP(EVT_TABKEY, "TAB", ICON_NONE, "Tab", "");
  LUXO_ADD_ENUM_PROP(EVT_RETKEY, "RET", ICON_NONE, "Return", "Enter");
  LUXO_ADD_ENUM_PROP(EVT_SPACEKEY, "SPACE", ICON_NONE, "Spacebar", "Space");
  LUXO_ADD_ENUM_PROP(EVT_LINEFEEDKEY, "LINE_FEED", ICON_NONE, "Line Feed", "");
  LUXO_ADD_ENUM_PROP(EVT_BACKSPACEKEY, "BACK_SPACE", ICON_NONE, "Backspace", "BkSpace");
  LUXO_ADD_ENUM_PROP(EVT_DELKEY, "DEL", ICON_NONE, "Delete", "Del");
  LUXO_ADD_ENUM_PROP(EVT_SEMICOLONKEY, "SEMI_COLON", ICON_NONE, ";", "");
  LUXO_ADD_ENUM_PROP(EVT_PERIODKEY, "PERIOD", ICON_NONE, ".", "");
  LUXO_ADD_ENUM_PROP(EVT_COMMAKEY, "COMMA", ICON_NONE, ",", "");
  LUXO_ADD_ENUM_PROP(EVT_QUOTEKEY, "QUOTE", ICON_NONE, "\"", "");
  LUXO_ADD_ENUM_PROP(EVT_ACCENTGRAVEKEY, "ACCENT_GRAVE", ICON_NONE, "`", "");
  LUXO_ADD_ENUM_PROP(EVT_MINUSKEY, "MINUS", ICON_NONE, "-", "");
  LUXO_ADD_ENUM_PROP(EVT_PLUSKEY, "PLUS", ICON_NONE, "+", "");
  LUXO_ADD_ENUM_PROP(EVT_SLASHKEY, "SLASH", ICON_NONE, "/", "");
  LUXO_ADD_ENUM_PROP(EVT_BACKSLASHKEY, "BACK_SLASH", ICON_NONE, "\\", "");
  LUXO_ADD_ENUM_PROP(EVT_EQUALKEY, "EQUAL", ICON_NONE, "=", "");
  LUXO_ADD_ENUM_PROP(EVT_LEFTBRACKETKEY, "LEFT_BRACKET", ICON_NONE, "[", "");
  LUXO_ADD_ENUM_PROP(EVT_RIGHTBRACKETKEY, "RIGHT_BRACKET", ICON_NONE, "]", "");
  LUXO_ADD_ENUM_PROP(EVT_LEFTARROWKEY, "LEFT_ARROW", ICON_NONE, "Left Arrow", "←");
  LUXO_ADD_ENUM_PROP(EVT_DOWNARROWKEY, "DOWN_ARROW", ICON_NONE, "Down Arrow", "↓");
  LUXO_ADD_ENUM_PROP(EVT_RIGHTARROWKEY, "RIGHT_ARROW", ICON_NONE, "Right Arrow", "→");
  LUXO_ADD_ENUM_PROP(EVT_UPARROWKEY, "UP_ARROW", ICON_NONE, "Up Arrow", "↑");
  LUXO_ADD_ENUM_PROP(EVT_PAD2, "NUMPAD_2", ICON_NONE, "Numpad 2", "Pad2");
  LUXO_ADD_ENUM_PROP(EVT_PAD4, "NUMPAD_4", ICON_NONE, "Numpad 4", "Pad4");
  LUXO_ADD_ENUM_PROP(EVT_PAD6, "NUMPAD_6", ICON_NONE, "Numpad 6", "Pad6");
  LUXO_ADD_ENUM_PROP(EVT_PAD8, "NUMPAD_8", ICON_NONE, "Numpad 8", "Pad8");
  LUXO_ADD_ENUM_PROP(EVT_PAD1, "NUMPAD_1", ICON_NONE, "Numpad 1", "Pad1");
  LUXO_ADD_ENUM_PROP(EVT_PAD3, "NUMPAD_3", ICON_NONE, "Numpad 3", "Pad3");
  LUXO_ADD_ENUM_PROP(EVT_PAD5, "NUMPAD_5", ICON_NONE, "Numpad 5", "Pad5");
  LUXO_ADD_ENUM_PROP(EVT_PAD7, "NUMPAD_7", ICON_NONE, "Numpad 7", "Pad7");
  LUXO_ADD_ENUM_PROP(EVT_PAD9, "NUMPAD_9", ICON_NONE, "Numpad 9", "Pad9");
  LUXO_ADD_ENUM_PROP(EVT_PADPERIOD, "NUMPAD_PERIOD", ICON_NONE, "Numpad .", "Pad.");
  LUXO_ADD_ENUM_PROP(EVT_PADSLASHKEY, "NUMPAD_SLASH", ICON_NONE, "Numpad /", "Pad/");
  LUXO_ADD_ENUM_PROP(EVT_PADASTERKEY, "NUMPAD_ASTERIX", ICON_NONE, "Numpad *", "Pad*");
  LUXO_ADD_ENUM_PROP(EVT_PAD0, "NUMPAD_0", ICON_NONE, "Numpad 0", "Pad0");
  LUXO_ADD_ENUM_PROP(EVT_PADMINUS, "NUMPAD_MINUS", ICON_NONE, "Numpad -", "Pad-");
  LUXO_ADD_ENUM_PROP(EVT_PADENTER, "NUMPAD_ENTER", ICON_NONE, "Numpad Enter", "PadEnter");
  LUXO_ADD_ENUM_PROP(EVT_PADPLUSKEY, "NUMPAD_PLUS", ICON_NONE, "Numpad +", "Pad+");
  LUXO_ADD_ENUM_PROP(EVT_F1KEY, "F1", ICON_NONE, "F1", "");
  LUXO_ADD_ENUM_PROP(EVT_F2KEY, "F2", ICON_NONE, "F2", "");
  LUXO_ADD_ENUM_PROP(EVT_F3KEY, "F3", ICON_NONE, "F3", "");
  LUXO_ADD_ENUM_PROP(EVT_F4KEY, "F4", ICON_NONE, "F4", "");
  LUXO_ADD_ENUM_PROP(EVT_F5KEY, "F5", ICON_NONE, "F5", "");
  LUXO_ADD_ENUM_PROP(EVT_F6KEY, "F6", ICON_NONE, "F6", "");
  LUXO_ADD_ENUM_PROP(EVT_F7KEY, "F7", ICON_NONE, "F7", "");
  LUXO_ADD_ENUM_PROP(EVT_F8KEY, "F8", ICON_NONE, "F8", "");
  LUXO_ADD_ENUM_PROP(EVT_F9KEY, "F9", ICON_NONE, "F9", "");
  LUXO_ADD_ENUM_PROP(EVT_F10KEY, "F10", ICON_NONE, "F10", "");
  LUXO_ADD_ENUM_PROP(EVT_F11KEY, "F11", ICON_NONE, "F11", "");
  LUXO_ADD_ENUM_PROP(EVT_F12KEY, "F12", ICON_NONE, "F12", "");
  LUXO_ADD_ENUM_PROP(EVT_F13KEY, "F13", ICON_NONE, "F13", "");
  LUXO_ADD_ENUM_PROP(EVT_F14KEY, "F14", ICON_NONE, "F14", "");
  LUXO_ADD_ENUM_PROP(EVT_F15KEY, "F15", ICON_NONE, "F15", "");
  LUXO_ADD_ENUM_PROP(EVT_F16KEY, "F16", ICON_NONE, "F16", "");
  LUXO_ADD_ENUM_PROP(EVT_F17KEY, "F17", ICON_NONE, "F17", "");
  LUXO_ADD_ENUM_PROP(EVT_F18KEY, "F18", ICON_NONE, "F18", "");
  LUXO_ADD_ENUM_PROP(EVT_F19KEY, "F19", ICON_NONE, "F19", "");
  LUXO_ADD_ENUM_PROP(EVT_F20KEY, "F20", ICON_NONE, "F20", "");
  LUXO_ADD_ENUM_PROP(EVT_F21KEY, "F21", ICON_NONE, "F21", "");
  LUXO_ADD_ENUM_PROP(EVT_F22KEY, "F22", ICON_NONE, "F22", "");
  LUXO_ADD_ENUM_PROP(EVT_F23KEY, "F23", ICON_NONE, "F23", "");
  LUXO_ADD_ENUM_PROP(EVT_F24KEY, "F24", ICON_NONE, "F24", "");
  LUXO_ADD_ENUM_PROP(EVT_PAUSEKEY, "PAUSE", ICON_NONE, "Pause", "");
  LUXO_ADD_ENUM_PROP(EVT_INSERTKEY, "INSERT", ICON_NONE, "Insert", "Ins");
  LUXO_ADD_ENUM_PROP(EVT_HOMEKEY, "HOME", ICON_NONE, "Home", "");
  LUXO_ADD_ENUM_PROP(EVT_PAGEUPKEY, "PAGE_UP", ICON_NONE, "Page Up", "PgUp");
  LUXO_ADD_ENUM_PROP(EVT_PAGEDOWNKEY, "PAGE_DOWN", ICON_NONE, "Page Down", "PgDown");
  LUXO_ADD_ENUM_PROP(EVT_ENDKEY, "END", ICON_NONE, "End", "");
  LUXO_ADD_ENUM_PROP(EVT_MEDIAPLAY, "MEDIA_PLAY", ICON_NONE, "Media Play/Pause", ">/||");
  LUXO_ADD_ENUM_PROP(EVT_MEDIASTOP, "MEDIA_STOP", ICON_NONE, "Media Stop", "Stop");
  LUXO_ADD_ENUM_PROP(EVT_MEDIAFIRST, "MEDIA_FIRST", ICON_NONE, "Media First", "|<<");
  LUXO_ADD_ENUM_PROP(EVT_MEDIALAST, "MEDIA_LAST", ICON_NONE, "Media Last", ">>|");
  LUXO_ADD_ENUM_PROP(KM_TEXTINPUT, "TEXTINPUT", ICON_NONE, "Text Input", "TxtIn");
  LUXO_ADD_ENUM_PROP(WINDEACTIVATE, "WINDOW_DEACTIVATE", ICON_NONE, "Window Deactivate", "");
  LUXO_ADD_ENUM_PROP(TIMER, "TIMER", ICON_NONE, "Timer", "Tmr");
  LUXO_ADD_ENUM_PROP(TIMER0, "TIMER0", ICON_NONE, "Timer 0", "Tmr0");
  LUXO_ADD_ENUM_PROP(TIMER1, "TIMER1", ICON_NONE, "Timer 1", "Tmr1");
  LUXO_ADD_ENUM_PROP(TIMER2, "TIMER2", ICON_NONE, "Timer 2", "Tmr2");
  LUXO_ADD_ENUM_PROP(TIMERJOBS, "TIMER_JOBS", ICON_NONE, "Timer Jobs", "TmrJob");
  LUXO_ADD_ENUM_PROP(TIMERAUTOSAVE, "TIMER_AUTOSAVE", ICON_NONE, "Timer Autosave", "TmrSave");
  LUXO_ADD_ENUM_PROP(TIMERREPORT, "TIMER_REPORT", ICON_NONE, "Timer Report", "TmrReport");
  LUXO_ADD_ENUM_PROP(TIMERREGION, "TIMERREGION", ICON_NONE, "Timer Region", "TmrReg");
  /* Action Zones. */
  LUXO_ADD_ENUM_PROP(EVT_ACTIONZONE_AREA,
                     "ACTIONZONE_AREA",
                     ICON_NONE,
                     "ActionZone Area",
                     "AZone Area");
  LUXO_ADD_ENUM_PROP(EVT_ACTIONZONE_REGION,
                     "ACTIONZONE_REGION",
                     ICON_NONE,
                     "ActionZone Region",
                     "AZone Region");
  LUXO_ADD_ENUM_PROP(EVT_ACTIONZONE_FULLSCREEN,
                     "ACTIONZONE_FULLSCREEN",
                     ICON_NONE,
                     "ActionZone Fullscreen",
                     "AZone FullScr");
}

static void prim_def_operator(KrakenSTAGE kstage)
{
  KrakenPRIM *kprim;
  KrakenPROP *prop;

  kprim = PRIM_def_struct(kstage, SdfPath("Operator"));
}

void PRIM_def_wm(KrakenSTAGE kstage)
{
  prim_def_operator(kstage);
  // PRIM_def_struct_ui_text(kstage, "Operator", "Storage of an operator being executed, or
  // registered after execution"); PRIM_def_struct_sdna(kstage, "wmOperator");
  // PRIM_def_struct_refine_func(kstage, "rna_Operator_refine");
#ifdef WITH_PYTHON
  // PRIM_def_struct_register_funcs(kstage, "rna_Operator_register", "rna_Operator_unregister",
  // "rna_Operator_instance");
#endif
  // PRIM_def_struct_flag(kstage, STRUCT_PUBLIC_NAMESPACE_INHERIT);
}

WABI_NAMESPACE_END
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
 * Editors.
 * Tools for Artists.
 */

#ifndef KRAKEN_EDITORS_UI_INTERFACE_H
#define KRAKEN_EDITORS_UI_INTERFACE_H

#include "kraken/kraken.h"

#include "UI_tokens.h"

/* Defines */

/* char for splitting strings, aligning shortcuts in menus, users never see */
#define UI_SEP_CHAR '|'
#define UI_SEP_CHAR_S "|"

/* Separator for text in search menus (right pointing arrow).
 * keep in sync with `string_search.cpp`. */
#define UI_MENU_ARROW_SEP "\xe2\x96\xb8"

/* names */
#define UI_MAX_DRAW_STR 400
#define UI_MAX_NAME_STR 128
#define UI_MAX_SHORTCUT_STR 64

/* How long before a tool-tip shows. */
#define UI_TOOLTIP_DELAY 0.5
#define UI_TOOLTIP_DELAY_LABEL 0.2

/* Float precision helpers */
#define UI_PRECISION_FLOAT_MAX 6
/* For float buttons the 'step' (or a1), is scaled */
#define UI_PRECISION_FLOAT_SCALE 0.01f

KRAKEN_NAMESPACE_BEGIN

/* Layout
 *
 * More automated layout of buttons. Has three levels:
 * - Layout: contains a number templates, within a bounded width or height.
 * - Template: predefined layouts for buttons with a number of slots, each
 *   slot can contain multiple items.
 * - Item: item to put in a template slot, being either an RNA property,
 *   operator, label or menu. Also regular buttons can be used when setting
 *   uiBlockCurLayout. */

/* layout */
enum
{
  UI_LAYOUT_HORIZONTAL = 0,
  UI_LAYOUT_VERTICAL = 1,
};

enum
{
  UI_LAYOUT_PANEL = 0,
  UI_LAYOUT_HEADER = 1,
  UI_LAYOUT_MENU = 2,
  UI_LAYOUT_TOOLBAR = 3,
  UI_LAYOUT_PIEMENU = 4,
  UI_LAYOUT_VERT_BAR = 5,
};

// #define UI_UNIT_X ((void)0, U.widget_unit)
// #define UI_UNIT_Y ((void)0, U.widget_unit)

enum
{
  UI_LAYOUT_ALIGN_EXPAND = 0,
  UI_LAYOUT_ALIGN_LEFT = 1,
  UI_LAYOUT_ALIGN_CENTER = 2,
  UI_LAYOUT_ALIGN_RIGHT = 3,
};

enum
{
  /* UI_ITEM_O_RETURN_PROPS = 1 << 0, */ /* UNUSED */
  UI_ITEM_R_EXPAND = 1 << 1,
  UI_ITEM_R_SLIDER = 1 << 2,
  /**
   * Use for booleans, causes the button to draw with an outline (emboss),
   * instead of text with a checkbox.
   * This is implied when toggle buttons have an icon
   * unless #UI_ITEM_R_ICON_NEVER flag is set.
   */
  UI_ITEM_R_TOGGLE = 1 << 3,
  /**
   * Don't attempt to use an icon when the icon is set to #ICON_NONE.
   *
   * Use for boolean's, causes the buttons to always show as a checkbox
   * even when there is an icon (which would normally show the button as a toggle).
   */
  UI_ITEM_R_ICON_NEVER = 1 << 4,
  UI_ITEM_R_ICON_ONLY = 1 << 5,
  UI_ITEM_R_EVENT = 1 << 6,
  UI_ITEM_R_FULL_EVENT = 1 << 7,
  UI_ITEM_R_NO_BG = 1 << 8,
  UI_ITEM_R_IMMEDIATE = 1 << 9,
  UI_ITEM_O_DEPRESS = 1 << 10,
  UI_ITEM_R_COMPACT = 1 << 11,
  UI_ITEM_R_CHECKBOX_INVERT = 1 << 12,
  /** Don't add a real decorator item, just blank space. */
  UI_ITEM_R_FORCE_BLANK_DECORATE = 1 << 13,
  /* Even create the property split layout if there's no name to show there. */
  UI_ITEM_R_SPLIT_EMPTY_NAME = 1 << 14,
};

#define UI_HEADER_OFFSET ((void)0, 0.4f * UI_UNIT_X)

/* uiLayoutOperatorButs flags */
enum
{
  UI_TEMPLATE_OP_PROPS_SHOW_TITLE = 1 << 0,
  UI_TEMPLATE_OP_PROPS_SHOW_EMPTY = 1 << 1,
  UI_TEMPLATE_OP_PROPS_COMPACT = 1 << 2,
  UI_TEMPLATE_OP_PROPS_HIDE_ADVANCED = 1 << 3,
  /* Disable property split for the default layout (custom ui callbacks still have full control
   * over the layout and can enable it). */
  UI_TEMPLATE_OP_PROPS_NO_SPLIT_LAYOUT = 1 << 4,
};

/* used for transp checkers */
#define UI_ALPHA_CHECKER_DARK 100
#define UI_ALPHA_CHECKER_LIGHT 160

void UI_tooltip_free(kContext *C, kScreen *screen, ARegion *region);

/**
 * This is a bit of a hack but best keep it in one place at least.
 */
struct PanelType *UI_but_paneltype_get(struct uiBut *but);

struct uiBut *UI_context_active_but_get(const struct kContext *C);

void UI_menutype_draw(struct kContext *C, struct MenuType *mt, struct uiLayout *layout);
/**
 * Used for popup panels only. */
void UI_paneltype_draw(struct kContext *C, struct PanelType *pt, struct uiLayout *layout);

/** #uiBlock.emboss and #uiBut.emboss */
enum eUIEmbossType
{
  UI_EMBOSS = 0,          /* use widget style for drawing */
  UI_EMBOSS_NONE = 1,     /* Nothing, only icon and/or text */
  UI_EMBOSS_PULLDOWN = 2, /* Pull-down menu style */
  UI_EMBOSS_RADIAL = 3,   /* Pie Menu */
  /**
   * The same as #UI_EMBOSS_NONE, unless the button has
   * a coloring status like an animation state or red alert.
   */
  UI_EMBOSS_NONE_OR_STATUS = 4,
  /* For layout engine, use emboss from block. */
  UI_EMBOSS_UNDEFINED = 255
};

/** #uiBut.flag general state flags. */
enum
{
  /* WARNING: the first 8 flags are internal (see #UI_SELECT definition). */
  UI_BUT_ICON_SUBMENU = 1 << 8,
  UI_BUT_ICON_PREVIEW = 1 << 9,

  UI_BUT_NODE_LINK = 1 << 10,
  UI_BUT_NODE_ACTIVE = 1 << 11,
  UI_BUT_DRAG_LOCK = 1 << 12,
  /** Grayed out and un-editable. */
  UI_BUT_DISABLED = 1 << 13,

  UI_BUT_ANIMATED = 1 << 14,
  UI_BUT_ANIMATED_KEY = 1 << 15,
  UI_BUT_DRIVEN = 1 << 16,
  UI_BUT_REDALERT = 1 << 17,
  /** Grayed out but still editable. */
  UI_BUT_INACTIVE = 1 << 18,
  UI_BUT_LAST_ACTIVE = 1 << 19,
  UI_BUT_UNDO = 1 << 20,
  /* UNUSED = 1 << 21, */
  UI_BUT_NO_UTF8 = 1 << 22,

  /** For popups, pressing return activates this button, overriding the highlighted button.
   * For non-popups this is just used as a display hint for the user to let them
   * know the action which is activated when pressing return (file selector for eg). */
  UI_BUT_ACTIVE_DEFAULT = 1 << 23,

  /** This but is "inside" a list item (currently used to change theme colors). */
  UI_BUT_LIST_ITEM = 1 << 24,
  /** edit this button as well as the active button (not just dragging) */
  UI_BUT_DRAG_MULTI = 1 << 25,
  /** Use for popups to start editing the button on initialization. */
  UI_BUT_ACTIVATE_ON_INIT = 1 << 26,

  /** #uiBut.str contains #UI_SEP_CHAR, used for key shortcuts */
  UI_BUT_HAS_SEP_CHAR = 1 << 27,
  /** Don't run updates while dragging (needed in rare cases). */
  UI_BUT_UPDATE_DELAY = 1 << 28,
  /** When widget is in text-edit mode, update value on each char stroke. */
  UI_BUT_TEXTEDIT_UPDATE = 1 << 29,
  /** Show 'x' icon to clear/unlink value of text or search button. */
  UI_BUT_VALUE_CLEAR = 1 << 30,

  /** RNA property of the button is overridden from linked reference data. */
  UI_BUT_OVERRIDDEN = 1u << 31u,
};

/** #uiBlock.flag (controls) */
enum
{
  UI_BLOCK_LOOP = 1 << 0,
  UI_BLOCK_IS_FLIP = 1 << 1,
  UI_BLOCK_NO_FLIP = 1 << 2,
  UI_BLOCK_NUMSELECT = 1 << 3,
  /** Don't apply window clipping. */
  UI_BLOCK_NO_WIN_CLIP = 1 << 4,
  UI_BLOCK_CLIPBOTTOM = 1 << 5,
  UI_BLOCK_CLIPTOP = 1 << 6,
  UI_BLOCK_MOVEMOUSE_QUIT = 1 << 7,
  UI_BLOCK_KEEP_OPEN = 1 << 8,
  UI_BLOCK_POPUP = 1 << 9,
  UI_BLOCK_OUT_1 = 1 << 10,
  UI_BLOCK_SEARCH_MENU = 1 << 11,
  UI_BLOCK_POPUP_MEMORY = 1 << 12,
  /* Stop handling mouse events. */
  UI_BLOCK_CLIP_EVENTS = 1 << 13,

  /* block->flag bits 14-17 are identical to but->drawflag bits */

  UI_BLOCK_POPUP_HOLD = 1 << 18,
  UI_BLOCK_LIST_ITEM = 1 << 19,
  UI_BLOCK_RADIAL = 1 << 20,
  UI_BLOCK_POPOVER = 1 << 21,
  UI_BLOCK_POPOVER_ONCE = 1 << 22,
  /** Always show key-maps, even for non-menus. */
  UI_BLOCK_SHOW_SHORTCUT_ALWAYS = 1 << 23,
  /** Don't show library override state for buttons in this block. */
  UI_BLOCK_NO_DRAW_OVERRIDDEN_STATE = 1 << 24,
  /** The block is only used during the search process and will not be drawn.
   * Currently just for the case of a closed panel's sub-panel (and its sub-panels). */
  UI_BLOCK_SEARCH_ONLY = 1 << 25,
  /** Hack for quick setup (splash screen) to draw text centered. */
  UI_BLOCK_QUICK_SETUP = 1 << 26,
};

/** #uiPopupBlockHandle.menuretval */
enum
{
  /** Cancel all menus cascading. */
  UI_RETURN_CANCEL = 1 << 0,
  /** Choice made. */
  UI_RETURN_OK = 1 << 1,
  /** Left the menu. */
  UI_RETURN_OUT = 1 << 2,
  /** Let the parent handle this event. */
  UI_RETURN_OUT_PARENT = 1 << 3,
  /** Update the button that opened. */
  UI_RETURN_UPDATE = 1 << 4,
  /** Popup is ok to be handled. */
  UI_RETURN_POPUP_OK = 1 << 5,
};

/* block bounds/position calculation */
enum eBlockBoundsCalc
{
  UI_BLOCK_BOUNDS_NONE = 0,
  UI_BLOCK_BOUNDS = 1,
  UI_BLOCK_BOUNDS_TEXT,
  UI_BLOCK_BOUNDS_POPUP_MOUSE,
  UI_BLOCK_BOUNDS_POPUP_MENU,
  UI_BLOCK_BOUNDS_POPUP_CENTER,
  UI_BLOCK_BOUNDS_PIE_CENTER,
};

/** Gradient types, for color picker #UI_BTYPE_HSVCUBE etc. */
enum eButGradientType
{
  UI_GRAD_SV = 0,
  UI_GRAD_HV = 1,
  UI_GRAD_HS = 2,
  UI_GRAD_H = 3,
  UI_GRAD_S = 4,
  UI_GRAD_V = 5,

  UI_GRAD_V_ALT = 9,
  UI_GRAD_L_ALT = 10,
};

enum
{
  /** Text and icon alignment (by default, they are centered). */
  UI_BUT_TEXT_LEFT = 1 << 1,
  UI_BUT_ICON_LEFT = 1 << 2,
  UI_BUT_TEXT_RIGHT = 1 << 3,
  /** Prevent the button to show any tooltip. */
  UI_BUT_NO_TOOLTIP = 1 << 4,
  /** Do not add the usual horizontal padding for text drawing. */
  UI_BUT_NO_TEXT_PADDING = 1 << 5,

  /* Button align flag, for drawing groups together.
   * Used in 'uiBlock.flag', take care! */
  UI_BUT_ALIGN_TOP = 1 << 14,
  UI_BUT_ALIGN_LEFT = 1 << 15,
  UI_BUT_ALIGN_RIGHT = 1 << 16,
  UI_BUT_ALIGN_DOWN = 1 << 17,
  UI_BUT_ALIGN = UI_BUT_ALIGN_TOP | UI_BUT_ALIGN_LEFT | UI_BUT_ALIGN_RIGHT | UI_BUT_ALIGN_DOWN,
  /* end bits shared with 'uiBlock.flag' */

  /**
   * Warning - HACK!
   * Needed for buttons which are not TOP/LEFT aligned,
   * but have some top/left corner stitched to some other TOP/LEFT-aligned button,
   * because of 'corrective' hack in widget_roundbox_set()... */
  UI_BUT_ALIGN_STITCH_TOP = 1 << 18,
  UI_BUT_ALIGN_STITCH_LEFT = 1 << 19,
  UI_BUT_ALIGN_ALL = UI_BUT_ALIGN | UI_BUT_ALIGN_STITCH_TOP | UI_BUT_ALIGN_STITCH_LEFT,

  /** This but is "inside" a box item (currently used to change theme colors). */
  UI_BUT_BOX_ITEM = 1 << 20,

  /** Active left part of number button */
  UI_BUT_ACTIVE_LEFT = 1 << 21,
  /** Active right part of number button */
  UI_BUT_ACTIVE_RIGHT = 1 << 22,

  /** Reverse order of consecutive off/on icons */
  UI_BUT_ICON_REVERSE = 1 << 23,

  /** Value is animated, but the current value differs from the animated one. */
  UI_BUT_ANIMATED_CHANGED = 1 << 24,

  /* Draw the checkbox buttons inverted. */
  UI_BUT_CHECKBOX_INVERT = 1 << 25,
};

uiBut *UI_region_active_but_get(const struct ARegion *region);

/* -------------------------------------------------------------------- */
/** \name Main UI API.
 * \{ */

struct uiLayout *uiLayoutRow(struct uiLayout *layout, bool align);
void uiItemL(struct uiLayout *layout, const char *name, int icon); /* label */
void uiItemL_ex(struct uiLayout *layout, const char *name, int icon, bool highlight, bool redalert);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Custom Interaction
 *
 * Sometimes it's useful to create data that remains available
 * while the user interacts with a button.
 *
 * A common case is dragging a number button or slider
 * however this could be used in other cases too.
 * \{ */

struct uiBlockInteraction_Params
{
  /**
   * When true, this interaction is not modal
   * (user clicking on a number button arrows or pasting a value for example).
   */
  bool is_click;
  /**
   * Array of unique event ID's (values from #uiBut.retval).
   * There may be more than one for multi-button editing (see #UI_BUT_DRAG_MULTI).
   */
  int *unique_retval_ids;
  uint unique_retval_ids_len;
};

typedef void (*uiButHandleFunc)(struct kContext *C, void *arg1, void *arg2);
typedef void (*uiButHandleRenameFunc)(struct kContext *C, void *arg, char *origstr);
typedef void (*uiButHandleNFunc)(struct kContext *C, void *argN, void *arg2);
typedef void (*uiButHandleHoldFunc)(struct kContext *C,
                                    struct ARegion *butregion,
                                    struct uiBut *but);
typedef int (*uiButCompleteFunc)(struct kContext *C, char *str, void *arg);

/** Function to compare the identity of two buttons over redraws, to check if they represent the
 * same data, and thus should be considered the same button over redraws. */
typedef bool (*uiButIdentityCompareFunc)(const struct uiBut *a, const struct uiBut *b);

/* Search types. */
typedef struct ARegion *(*uiButSearchCreateFn)(struct kContext *C,
                                               struct ARegion *butregion,
                                               struct uiButSearch *search_but);
/**
 * `is_first` is typically used to ignore search filtering when the menu is first opened in order
 * to display the full list of options. The value will be false after the button's text is edited
 * (for every call except the first).
 */
typedef void (*uiButSearchUpdateFn)(const struct kContext *C,
                                    void *arg,
                                    const char *str,
                                    struct uiSearchItems *items,
                                    bool is_first);
typedef bool (*uiButSearchContextMenuFn)(struct kContext *C,
                                         void *arg,
                                         void *active,
                                         const struct wmEvent *event);
typedef struct ARegion *(*uiButSearchTooltipFn)(struct kContext *C,
                                                struct ARegion *region,
                                                const struct wabi::GfVec4f *item_rect,
                                                void *arg,
                                                void *active);
typedef void (*uiButSearchListenFn)(const struct wmRegionListenerParams *params, void *arg);

/* Must return allocated string. */
typedef char *(*uiButToolTipFunc)(struct kContext *C, void *argN, const char *tip);
typedef int (*uiButPushedStateFunc)(struct uiBut *but, const void *arg);

typedef void (*uiBlockHandleFunc)(struct kContext *C, void *arg, int event);

/** Returns 'user_data', freed by #uiBlockInteractionEndFn. */
typedef void *(*uiBlockInteractionBeginFn)(struct kContext *C,
                                           const struct uiBlockInteraction_Params *params,
                                           void *arg1);
typedef void (*uiBlockInteractionEndFn)(struct kContext *C,
                                        const struct uiBlockInteraction_Params *params,
                                        void *arg1,
                                        void *user_data);
typedef void (*uiBlockInteractionUpdateFn)(struct kContext *C,
                                           const struct uiBlockInteraction_Params *params,
                                           void *arg1,
                                           void *user_data);

struct uiBlockInteraction_CallbackData
{
  uiBlockInteractionBeginFn begin_fn;
  uiBlockInteractionEndFn end_fn;
  uiBlockInteractionUpdateFn update_fn;
  void *arg1;
};

void UI_block_interaction_set(uiBlock *block, uiBlockInteraction_CallbackData *callbacks);

/** \} */

bool UI_popup_block_name_exists(const struct kScreen *screen, const TfToken &name);

typedef bool (*uiMenuStepFunc)(struct kContext *C, int direction, void *arg1);

typedef void *(*uiCopyArgFunc)(const void *arg);
typedef void (*uiFreeArgFunc)(void *arg);

bool UI_but_has_tooltip_label(const uiBut *but);
bool UI_but_is_tool(const uiBut *but);

typedef uiBlock *(*uiBlockCreateFunc)(struct kContext *C, struct ARegion *region, void *arg1);
typedef void (*uiBlockCancelFunc)(struct kContext *C, void *arg1);

void UI_popup_block_invoke(struct kContext *C,
                           uiBlockCreateFunc func,
                           void *arg,
                           uiFreeArgFunc arg_free);
void UI_popup_block_invoke_ex(struct kContext *C,
                              uiBlockCreateFunc func,
                              void *arg,
                              uiFreeArgFunc arg_free,
                              bool can_refresh);

void UI_popup_handlers_add(struct kContext *C,
                           std::vector<struct wmEventHandler *> handlers,
                           struct uiPopupBlockHandle *popup,
                           char flag);
KRAKEN_NAMESPACE_END

#endif /* KRAKEN_EDITORS_UI_INTERFACE_H */
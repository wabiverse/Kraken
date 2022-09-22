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

#ifndef KRAKEN_EDITORS_INTERFACE_INTERN_H
#define KRAKEN_EDITORS_INTERFACE_INTERN_H

#include "kraken/kraken.h"

#include "USD_api.h"
#include "USD_color_types.h"
#include "USD_curveprofile_types.h"
#include "USD_scene.h"
#include "USD_texture_types.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include <wabi/base/tf/token.h>

KRAKEN_NAMESPACE_BEGIN

struct ARegion;
struct AnimationEvalContext;
struct CurveMapping;
struct CurveProfile;
struct ID;
struct ImBuf;
struct Main;
struct Scene;
struct kContext;
struct kContextStore;
struct uiHandleButtonData;
struct uiLayout;
struct uiStyle;
struct uiUndoStack_Text;
struct uiWidgetColors;
struct wmEvent;
struct wmKeyConfig;
struct wmOperatorType;
struct wmTimer;

/* bit button defines */
/* Bit operations */
#define UI_BITBUT_TEST(a, b) (((a) & (1 << (b))) != 0)
#define UI_BITBUT_VALUE_TOGGLED(a, b) ((a) ^ (1 << (b)))
#define UI_BITBUT_VALUE_ENABLED(a, b) ((a) | (1 << (b)))
#define UI_BITBUT_VALUE_DISABLED(a, b) ((a) & ~(1 << (b)))

#define UI_TEXT_MARGIN_X 0.4f
#define UI_POPUP_MARGIN (UI_DPI_FAC * 12)
/**
 * Margin at top of screen for popups.
 * Note this value must be sufficient to draw a popover arrow to avoid cropping it.
 */
#define UI_POPUP_MENU_TOP (int)(10 * UI_DPI_FAC)

#define UI_PIXEL_AA_JITTER 8
extern const float ui_pixel_jitter[UI_PIXEL_AA_JITTER][2];

/** #uiBut.flag */
enum
{
  /** Use when the button is pressed. */
  UI_SELECT = (1 << 0),
  /** Temporarily hidden (scrolled out of the view). */
  UI_SCROLLED = (1 << 1),
  UI_ACTIVE = (1 << 2),
  UI_HAS_ICON = (1 << 3),
  UI_HIDDEN = (1 << 4),
  /** Display selected, doesn't impact interaction. */
  UI_SELECT_DRAW = (1 << 5),
  /** Property search filter is active and the button does not match. */
  UI_SEARCH_FILTER_NO_MATCH = (1 << 6),

  /** Temporarily override the active button for lookups in context, regions, etc. (everything
   * using #ui_context_button_active()). For example, so that operators normally acting on the
   * active button can be polled on non-active buttons to (e.g. for disabling). */
  UI_BUT_ACTIVE_OVERRIDE = (1 << 7),

  /* WARNING: rest of #uiBut.flag in UI_interface.h */
};

/* #uiButtonGroup.flag. */
enum uiButtonGroupFlag
{
  /** While this flag is set, don't create new button groups for layout item calls. */
  UI_BUTTON_GROUP_LOCK = (1 << 0),
  /** The buttons in this group are inside a panel header. */
  UI_BUTTON_GROUP_PANEL_HEADER = (1 << 1),
};
ENUM_OPERATORS(uiButtonGroupFlag, UI_BUTTON_GROUP_PANEL_HEADER);

/** #uiBut.pie_dir */
enum RadialDirection
{
  UI_RADIAL_NONE = -1,
  UI_RADIAL_N = 0,
  UI_RADIAL_NE = 1,
  UI_RADIAL_E = 2,
  UI_RADIAL_SE = 3,
  UI_RADIAL_S = 4,
  UI_RADIAL_SW = 5,
  UI_RADIAL_W = 6,
  UI_RADIAL_NW = 7,
};

struct uiBut
{
  /** Pointer back to the layout item holding this button. */
  uiLayout *layout;
  int flag, drawflag;
  eButType type;
  eButPointerType pointype;
  short bit, bitnr, retval, strwidth, alignnr;
  short ofs, pos, selsta, selend;

  char *str;
  char strdata[UI_MAX_NAME_STR];
  char drawstr[UI_MAX_DRAW_STR];

  wabi::GfVec4f rect; /* block relative coords */

  char *poin;
  float hardmin, hardmax, softmin, softmax;

  /* both these values use depends on the button type
   * (polymorphic struct or union would be nicer for this stuff) */

  /**
   * For #uiBut.type:
   * - UI_BTYPE_LABEL:        Use `(a1 == 1.0f)` to use a2 as a blending factor (imaginative!).
   * - UI_BTYPE_SCROLL:       Use as scroll size.
   * - UI_BTYPE_SEARCH_MENU:  Use as number or rows.
   */
  float a1;

  /**
   * For #uiBut.type:
   * - UI_BTYPE_HSVCIRCLE:    Use to store the luminosity.
   * - UI_BTYPE_LABEL:        If `(a1 == 1.0f)` use a2 as a blending factor.
   * - UI_BTYPE_SEARCH_MENU:  Use as number or columns.
   */
  float a2;

  uchar col[4];

  /** See \ref UI_but_func_identity_compare_set(). */
  uiButIdentityCompareFunc identity_cmp_func;

  uiButHandleFunc func;
  void *func_arg1;
  void *func_arg2;

  uiButHandleNFunc funcN;
  void *func_argN;

  uiButCompleteFunc autocomplete_func;
  void *autofunc_arg;

  uiButHandleRenameFunc rename_func;
  void *rename_arg1;
  void *rename_orig;

  /** Run an action when holding the button down. */
  uiButHandleHoldFunc hold_func;
  void *hold_argN;

  const char *tip;
  uiButToolTipFunc tip_func;
  void *tip_arg;
  uiFreeArgFunc tip_arg_free;

  /** info on why button is disabled, displayed in tooltip */
  const char *disabled_info;

  KIFIconID icon;

  /** Copied from the #uiBlock.emboss */
  eUIEmbossType emboss;

  RadialDirection pie_dir;

  /** could be made into a single flag */
  bool changed;
  /** so buttons can support unit systems which are not RNA */
  uchar unit_type;
  short iconadd;

  /** #UI_BTYPE_BLOCK data */
  uiBlockCreateFunc block_create_func;

  /** #UI_BTYPE_PULLDOWN / #UI_BTYPE_MENU data */
  uiMenuCreateFunc menu_create_func;

  uiMenuStepFunc menu_step_func;

  /* USD data */
  KrakenPRIM stagepoin;
  KrakenPROP *stageprop;
  int stageindex;

  /* Operator data */
  struct wmOperatorType *optype;
  KrakenPRIM *opptr;
  eWmOperatorContext opcontext;

  /** When non-zero, this is the key used to activate a menu items (`a-z` always lower case). */
  uchar menu_key;

  std::vector<struct uiButExtraOpIcon *> extra_op_icons;

  /* Drag-able data, type is WM_DRAG_... */
  char dragtype;
  short dragflag;
  void *dragpoin;
  // struct ImBuf *imb;
  float imb_scale;

  struct uiHandleButtonData *active;

  /** Custom button data (borrowed, not owned). */
  void *custom_data;

  char *editstr;
  double *editval;
  float *editvec;

  uiButPushedStateFunc pushed_state_func;
  const void *pushed_state_arg;

  struct uiBlock *block;
};

/** Derived struct for #UI_BTYPE_NUM */
struct uiButNumber
{
  struct uiBut but;

  float step_size;
  float precision;
};

/** Derived struct for #UI_BTYPE_COLOR */
struct uiButColor
{
  struct uiBut but;

  bool is_pallete_color;
  int palette_color_index;
};

/** Derived struct for #UI_BTYPE_TAB */
struct uiButTab
{
  struct uiBut but;
  // struct MenuType *menu;
};

/** Derived struct for #UI_BTYPE_SEARCH_MENU */
struct uiButSearch
{
  struct uiBut but;

  uiButSearchCreateFn popup_create_fn;
  uiButSearchUpdateFn items_update_fn;
  uiButSearchListenFn listen_fn;

  void *item_active;

  void *arg;
  uiFreeArgFunc arg_free_fn;

  uiButSearchContextMenuFn item_context_menu_fn;
  uiButSearchTooltipFn item_tooltip_fn;

  const char *item_sep_string;

  KrakenPRIM stagesearchpoin;
  KrakenPROP *stagesearchprop;

  /**
   * The search box only provides suggestions, it does not force
   * the string to match one of the search items when applying.
   */
  bool results_are_suggestions;
};

/**
 * Additional, superimposed icon for a button, invoking an operator.
 */
struct uiButExtraOpIcon
{
  struct uiButExtraOpIcon *next, *prev;

  KIFIconID icon;
  struct wmOperatorCallParams *optype_params;

  bool highlighted;
  bool disabled;
};

struct ColorPicker
{
  /** Color in HSV or HSL, in color picking color space. Used for HSV cube,
   * circle and slider widgets. The color picking space is perceptually
   * linear for intuitive editing. */
  float hsv_perceptual[3];
  /** Initial color data (to detect changes). */
  float hsv_perceptual_init[3];
  bool is_init;

  /** HSV or HSL color in scene linear color space value used for number
   * buttons. This is scene linear so that there is a clear correspondence
   * to the scene linear RGB values. */
  float hsv_scene_linear[3];

  /** Cubic saturation for the color wheel. */
  bool use_color_cubic;
  bool use_color_lock;
  bool use_luminosity_lock;
  float luminosity_lock_value;
};

struct ColorPickerData
{
  std::vector<ColorPicker> list;
};

struct PieMenuData
{
  /** store title and icon to allow access when pie levels are created */
  const char *title;
  int icon;

  float pie_dir[2];
  float pie_center_init[2];
  float pie_center_spawned[2];
  float last_pos[2];
  double duration_gesture;
  int flags;
  /** Initial event used to fire the pie menu, store here so we can query for release */
  short event_type;
  float alphafac;
};

/** Derived struct for #UI_BTYPE_COLORBAND. */
struct uiButColorBand
{
  struct uiBut but;

  struct ColorBand *edit_coba;
};

/** Derived struct for #UI_BTYPE_CURVEPROFILE. */
struct uiButCurveProfile
{
  struct uiBut but;

  struct CurveProfile *edit_profile;
};

/** Derived struct for #UI_BTYPE_CURVE. */
struct uiButCurveMapping
{
  struct uiBut but;

  struct CurveMapping *edit_cumap;
  eButGradientType gradient_type;
};

/** Derived struct for #UI_BTYPE_DECORATOR */
struct uiButDecorator
{
  uiBut but;

  struct KrakenPRIM rnapoin;
  struct KrakenPROP *rnaprop;
  int rnaindex;
};

/** Derived struct for #UI_BTYPE_PROGRESS_BAR. */
struct uiButProgressbar
{
  uiBut but;

  /* 0..1 range */
  float progress;
};

struct uiButViewItem
{
  uiBut but;

  /* C-Handle to the view item this button was created for. */
  struct uiViewItemHandle *view_item;
};

/** Derived struct for #UI_BTYPE_HSVCUBE. */
struct uiButHSVCube
{
  uiBut but;

  eButGradientType gradient_type;
};

/** Derived struct for #UI_BTYPE_HOTKEY_EVENT. */
struct uiButHotkeyEvent
{
  uiBut but;

  short modifier_key;
};

/* Menu Callbacks */

typedef void (*uiMenuCreateFunc)(struct kContext *C, struct uiLayout *layout, void *arg1);
typedef void (*uiMenuHandleFunc)(struct kContext *C, void *arg, int event);

/* menu callback */
void ui_item_menutype_func(struct kContext *C, struct uiLayout *layout, void *arg_mt);
void ui_item_paneltype_func(struct kContext *C, struct uiLayout *layout, void *arg_pt);

void UI_block_new_button_group(uiBlock *block, uiButtonGroupFlag flag);

/**
 * A group of button references, used by property search to keep track of sets of buttons that
 * should be searched together. For example, in property split layouts number buttons and their
 * labels (and even their decorators) are separate buttons, but they must be searched and
 * highlighted together.
 */
struct uiButtonGroup
{
  std::vector<uiBut *> buttons;
  short flag;
};

struct uiBlock
{
  TfToken name;

  std::vector<struct uiBut *> buttons;
  struct Panel *panel;
  struct uiBlock *oldblock;

  /** Used for `UI_butstore_*` runtime function. */
  std::vector<struct uiBut *> butstore;

  std::vector<struct uiButtonGroup *> button_groups; /* #uiButtonGroup. */

  std::vector<struct uiLayoutRoot *> layouts;
  struct uiLayout *curlayout;

  /** Custom interaction data. */
  uiBlockInteraction_CallbackData custom_interaction_callbacks;

  int flag;

  float winmat[4][4];
  wabi::GfVec4f rect;
  float aspect;

  /** Unique hash used to implement popup menu memory. */
  uint puphash;

  uiButHandleFunc func;
  void *func_arg1;
  void *func_arg2;

  uiButHandleNFunc funcN;
  void *func_argN;

  uiMenuHandleFunc butm_func;
  void *butm_func_arg;

  uiBlockHandleFunc handle_func;
  void *handle_func_arg;

  /** Custom interaction data. */
  uiBlockInteraction_CallbackData custom_interaction_callbacks;

  /** Custom extra event handling. */
  int (*block_event_func)(const struct kContext *C, struct uiBlock *, const struct wmEvent *);

  /** Custom extra draw function for custom blocks. */
  void (*drawextra)(const struct kContext *C, void *idv, void *arg1, void *arg2, rcti *rect);
  void *drawextra_arg1;
  void *drawextra_arg2;

  int flag;
  short alignnr;
  /** Hints about the buttons of this block. Used to avoid iterating over
   * buttons to find out if some criteria is met by any. Instead, check this
   * criteria when adding the button and set a flag here if it's met. */
  short content_hints; /* #eBlockContentHints */

  char direction;
  /** UI_BLOCK_THEME_STYLE_* */
  char theme_style;
  /** Copied to #uiBut.emboss */
  eUIEmbossType emboss;
  bool auto_open;
  char _pad[5];
  double auto_open_last;

  const char *lockstr;

  bool lock;
  /** To keep blocks while drawing and free them afterwards. */
  bool active;
  /** To avoid tool-tip after click. */
  bool tooltipdisabled;
  /** True when #UI_block_end has been called. */
  bool endblock;

  /** for doing delayed */
  eBlockBoundsCalc bounds_type;
  /** Offset to use when calculating bounds (in pixels). */
  int bounds_offset[2];
  /** for doing delayed */
  int bounds, minbounds;

  /** Pull-downs, to detect outside, can differ per case how it is created. */
  wabi::GfVec4f safety;
  /** #uiSafetyRct list */
  std::vector<wabi::GfVec4f> saferct;

  uiPopupBlockHandle *handle;

  /** use so presets can find the operator,
   * across menus and from nested popups which fail for operator context. */
  struct wmOperator *ui_operator;

  /** XXX hack for dynamic operator enums */
  void *evil_C;

  /** unit system, used a lot for numeric buttons so include here
   * rather than fetching through the scene every time. */
  struct UnitSettings *unit;
  /** \note only accessed by color picker templates. */
  ColorPickerData color_pickers;

  /** Block for color picker with gamma baked in. */
  bool is_color_gamma_picker;

  /**
   * Display device name used to display this block,
   * used by color widgets to transform colors from/to scene linear.
   */
  char display_device[64];

  PieMenuData pie_data;
};

void ui_block_to_region_fl(const struct ARegion *region, uiBlock *block, float *r_x, float *r_y);
void ui_block_to_window_fl(const struct ARegion *region, uiBlock *block, float *x, float *y);

typedef uiBlock *(*uiBlockHandleCreateFunc)(struct kContext *C,
                                            struct uiPopupBlockHandle *handle,
                                            void *arg1);

struct uiPopupBlockCreate
{
  uiBlockCreateFunc create_func;
  uiBlockHandleCreateFunc handle_create_func;
  void *arg;
  uiFreeArgFunc arg_free;

  int event_xy[2];

  /** Set when popup is initialized from a button. */
  struct ARegion *butregion;
  uiBut *but;
};

struct uiPopupBlockHandle
{
  /* internal */
  struct ARegion *region;

  /** Use only for #UI_BLOCK_MOVEMOUSE_QUIT popups. */
  float towards_xy[2];
  double towardstime;
  bool dotowards;

  bool popup;
  uiBlockHandleFunc popup_func;
  uiBlockCancelFunc cancel_func;
  void *popup_arg;

  /** Store data for refreshing popups. */
  struct uiPopupBlockCreate popup_create_vars;
  /** True if we can re-create the popup using #uiPopupBlockHandle.popup_create_vars. */
  bool can_refresh;
  bool refresh;

  /* for operator popups */
  struct wmOperator *popup_op;
  struct ScrArea *ctx_area;
  struct ARegion *ctx_region;

  /* return values */
  int butretval;
  int menuretval;
  int retvalue;
  float retvec[4];
};

uiPopupBlockHandle *ui_popup_block_create(struct kContext *C,
                                          struct ARegion *butregion,
                                          uiBut *but,
                                          uiBlockCreateFunc create_func,
                                          uiBlockHandleCreateFunc handle_create_func,
                                          void *arg,
                                          uiFreeArgFunc arg_free);

/* -------------------------------------------------------------------- */
/** \name Interface Queries
 * \{ */

size_t UI_but_drawstr_len_without_sep_char(const uiBut *but);
size_t UI_but_drawstr_without_sep_char(const uiBut *but, char *str, size_t str_maxlen);
uiBut *UI_region_find_active_but(struct ARegion *region);
uiBut *UI_block_active_but_get(const uiBlock *block);
bool UI_block_is_menu(const uiBlock *block) ATTR_WARN_UNUSED_RESULT;
bool UI_block_is_popover(const uiBlock *block) ATTR_WARN_UNUSED_RESULT;

/** \} */

void UI_window_to_block_fl(const ARegion *region, uiBlock *block, float *r_x, float *r_y);
void UI_window_to_block(const ARegion *region, uiBlock *block, int *r_x, int *r_y);

template<typename T> T UI_but_value_get(uiBut *but);

template<typename T> void UI_but_value_set(uiBut *but, T value);

template<typename T> void UI_but_update(uiBut *but);

template<typename T> void UI_but_update_edited(uiBut *but);

void UI_but_string_get_ex(uiBut *but,
                          char *str,
                          const size_t maxlen,
                          const int float_precision,
                          const bool use_exp_float,
                          bool *r_use_exp_float);
void UI_but_string_get(uiBut *but, char *str, const size_t maxlen);

bool UI_but_is_editing(const uiBut *but);

bool ui_but_can_align(const uiBut *but) ATTR_WARN_UNUSED_RESULT;

KRAKEN_NAMESPACE_END

#endif /* KRAKEN_EDITORS_INTERFACE_INTERN_H */
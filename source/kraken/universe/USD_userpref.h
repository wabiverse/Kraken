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
 * Universe.
 * Set the Stage.
 */

#include "USD_context.h"
#include "USD_screen.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"

#include "KKE_context.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usdUI/userPref.h>

KRAKEN_NAMESPACE_BEGIN

struct ColorBand;

/* ************************ style definitions ******************** */

#define MAX_STYLE_NAME 64

/**
 * Default offered by Kraken.
 * #uiFont.uifont_id
 */
enum eUIFont_ID
{
  UIFONT_DEFAULT = 0,
  /*  UIFONT_BITMAP   = 1 */ /* UNUSED */

  /* free slots */
  UIFONT_CUSTOM1 = 2,
  /* UIFONT_CUSTOM2 = 3, */ /* UNUSED */
};

/** #UserDef.factor_display_type */
enum eUserpref_FactorDisplay
{
  USER_FACTOR_AS_FACTOR = 0,
  USER_FACTOR_AS_PERCENTAGE = 1,
};

enum eUserprefUIFlag
{
  USER_UIFLAG_UNUSED_0 = (1 << 0),
  USER_UIFLAG_UNUSED_1 = (1 << 1),
  USER_WHEELZOOMDIR = (1 << 2),
  USER_FILTERFILEEXTS = (1 << 3),
  USER_DRAWVIEWINFO = (1 << 4),
  USER_PLAINMENUS = (1 << 5),
  USER_LOCK_CURSOR_ADJUST = (1 << 6),
  USER_HEADER_BOTTOM = (1 << 7),
  USER_HEADER_FROM_PREF = (1 << 8),
  USER_MENUOPENAUTO = (1 << 9),
  USER_DEPTH_CURSOR = (1 << 10),
  USER_AUTOPERSP = (1 << 11),
  USER_UIFLAG_UNUSED_12 = (1 << 12),
  USER_GLOBALUNDO = (1 << 13),
  USER_ORBIT_SELECTION = (1 << 14),
  USER_DEPTH_NAVIGATE = (1 << 15),
  USER_HIDE_DOT = (1 << 16),
  USER_SHOW_GIZMO_NAVIGATE = (1 << 17),
  USER_SHOW_VIEWPORTNAME = (1 << 18),
  USER_UIFLAG_UNUSED_3 = (1 << 19),
  USER_ZOOM_TO_MOUSEPOS = (1 << 20),
  USER_SHOW_FPS = (1 << 21),
  USER_UIFLAG_UNUSED_22 = (1 << 22),
  USER_MENUFIXEDORDER = (1 << 23),
  USER_CONTINUOUS_MOUSE = (1 << 24),
  USER_ZOOM_INVERT = (1 << 25),
  USER_ZOOM_HORIZ = (1 << 26),
  USER_SPLASH_DISABLE = (1 << 27),
  USER_HIDE_RECENT = (1 << 28),
  USER_SAVE_PROMPT = (1 << 30),
  USER_HIDE_SYSTEM_BOOKMARKS = (1u << 31),
};

enum eUserPrefFlag
{
  USER_AUTOSAVE = (1 << 0),
  USER_FLAG_NUMINPUT_ADVANCED = (1 << 1),
  USER_FLAG_UNUSED_2 = (1 << 2), /* cleared */
  USER_FLAG_UNUSED_3 = (1 << 3), /* cleared */
  USER_FLAG_UNUSED_4 = (1 << 4), /* cleared */
  USER_TRACKBALL = (1 << 5),
  USER_FLAG_UNUSED_6 = (1 << 6), /* cleared */
  USER_FLAG_UNUSED_7 = (1 << 7), /* cleared */
  USER_MAT_ON_OB = (1 << 8),
  USER_FLAG_UNUSED_9 = (1 << 9), /* cleared */
  USER_DEVELOPER_UI = (1 << 10),
  USER_TOOLTIPS = (1 << 11),
  USER_TWOBUTTONMOUSE = (1 << 12),
  USER_NONUMPAD = (1 << 13),
  USER_ADD_CURSORALIGNED = (1 << 14),
  USER_FILECOMPRESS = (1 << 15),
  USER_SAVE_PREVIEWS = (1 << 16),
  USER_CUSTOM_RANGE = (1 << 17),
  USER_ADD_EDITMODE = (1 << 18),
  USER_ADD_VIEWALIGNED = (1 << 19),
  USER_RELPATHS = (1 << 20),
  USER_RELEASECONFIRM = (1 << 21),
  USER_SCRIPT_AUTOEXEC_DISABLE = (1 << 22),
  USER_FILENOUI = (1 << 23),
  USER_NONEGFRAMES = (1 << 24),
  USER_TXT_TABSTOSPACES_DISABLE = (1 << 25),
  USER_TOOLTIPS_PYTHON = (1 << 26),
  USER_FLAG_UNUSED_27 = (1 << 27), /* dirty */
};

struct uiFont
{
  char filepath[1024];
  /** From blfont lib. */
  short blf_id;
  /** Own id (eUIFont_ID). */
  short uifont_id;
};

struct uiFontStyle
{
  /** Saved in file, 0 is default. */
  short uifont_id;
  /** Actual size depends on 'global' DPI. */
  float points;
  /** Style hint. */
  short italic, bold;
  /** Value is amount of pixels blur. */
  short shadow;
  /** Shadow offset in pixels. */
  short shadx, shady;
  /** Total alpha. */
  float shadowalpha;
  /** 1 value, typically white or black anyway. */
  float shadowcolor;
};

struct uiStyle
{
  /** MAX_STYLE_NAME. */
  char name[64];

  uiFontStyle paneltitle;
  uiFontStyle grouplabel;
  uiFontStyle widgetlabel;
  uiFontStyle widget;

  float panelzoom;

  /** In characters. */
  short minlabelchars;
  /** In characters. */
  short minwidgetchars;

  short columnspace;
  short templatespace;
  short boxspace;
  short buttonspacex;
  short buttonspacey;
  short panelspace;
  short panelouter;
};

struct uiWidgetColors
{
  unsigned char outline[4];
  unsigned char inner[4];
  unsigned char inner_sel[4];
  unsigned char item[4];
  unsigned char text[4];
  unsigned char text_sel[4];
  unsigned char shaded;
  short shadetop, shadedown;
  float roundness;
};

struct uiWidgetStateColors
{
  unsigned char inner_anim[4];
  unsigned char inner_anim_sel[4];
  unsigned char inner_key[4];
  unsigned char inner_key_sel[4];
  unsigned char inner_driven[4];
  unsigned char inner_driven_sel[4];
  unsigned char inner_overridden[4];
  unsigned char inner_overridden_sel[4];
  unsigned char inner_changed[4];
  unsigned char inner_changed_sel[4];
  float blend;
};

struct uiPanelColors
{
  unsigned char header[4];
  unsigned char back[4];
  unsigned char sub_back[4];
};

struct ThemeUI
{
  /* Interface Elements (buttons, menus, icons) */
  uiWidgetColors wcol_regular, wcol_tool, wcol_toolbar_item, wcol_text;
  uiWidgetColors wcol_radio, wcol_option, wcol_toggle;
  uiWidgetColors wcol_num, wcol_numslider, wcol_tab;
  uiWidgetColors wcol_menu, wcol_pulldown, wcol_menu_back, wcol_menu_item, wcol_tooltip;
  uiWidgetColors wcol_box, wcol_scroll, wcol_progress, wcol_list_item, wcol_pie_menu;
  uiWidgetColors wcol_view_item;

  uiWidgetStateColors wcol_state;

  unsigned char widget_emboss[4];

  /* fac: 0 - 1 for blend factor, width in pixels */
  float menu_shadow_fac;
  short menu_shadow_width;

  unsigned char editor_outline[4];

  /* Transparent Grid */
  unsigned char transparent_checker_primary[4], transparent_checker_secondary[4];
  unsigned char transparent_checker_size;

  float icon_alpha;
  float icon_saturation;
  unsigned char widget_text_cursor[4];

  /* Axis Colors */
  unsigned char xaxis[4], yaxis[4], zaxis[4];

  /* Gizmo Colors. */
  unsigned char gizmo_hi[4];
  unsigned char gizmo_primary[4];
  unsigned char gizmo_secondary[4];
  unsigned char gizmo_view_align[4];
  unsigned char gizmo_a[4];
  unsigned char gizmo_b[4];

  /* Icon Colors. */
  /** Scene items. */
  unsigned char icon_scene[4];
  /** Collection items. */
  unsigned char icon_collection[4];
  /** Object items. */
  unsigned char icon_object[4];
  /** Object data items. */
  unsigned char icon_object_data[4];
  /** Modifier and constraint items. */
  unsigned char icon_modifier[4];
  /** Shading related items. */
  unsigned char icon_shading[4];
  /** File folders. */
  unsigned char icon_folder[4];
  /** Intensity of the border icons. >0 will render an border around themed
   * icons. */
  float icon_border_intensity;
  float panel_roundness;
};

/* try to put them all in one, if needed a special struct can be created as well
 * for example later on, when we introduce wire colors for ob types or so...
 */
struct ThemeSpace
{
  /* main window colors */
  unsigned char back[4];
  unsigned char back_grad[4];

  char background_type;

  /** Panel title. */
  unsigned char title[4];
  unsigned char text[4];
  unsigned char text_hi[4];

  /* header colors */
  /** Region background. */
  unsigned char header[4];
  /** Unused. */
  unsigned char header_title[4];
  unsigned char header_text[4];
  unsigned char header_text_hi[4];

  /* region tabs */
  unsigned char tab_active[4];
  unsigned char tab_inactive[4];
  unsigned char tab_back[4];
  unsigned char tab_outline[4];

  /* button/tool regions */
  /** Region background. */
  unsigned char button[4];
  /** Panel title. */
  unsigned char button_title[4];
  unsigned char button_text[4];
  unsigned char button_text_hi[4];

  /* listview regions */
  /** Region background. */
  unsigned char list[4];
  /** Panel title. */
  unsigned char list_title[4];
  unsigned char list_text[4];
  unsigned char list_text_hi[4];

  /* navigation bar regions */
  /** Region background. */
  unsigned char navigation_bar[4];
  /** Region background. */
  unsigned char execution_buts[4];

  /* NOTE: cannot use name 'panel' because of DNA mapping old files. */
  uiPanelColors panelcolors;

  unsigned char shade1[4];
  unsigned char shade2[4];

  unsigned char hilite[4];
  unsigned char grid[4];

  unsigned char view_overlay[4];

  unsigned char wire[4], wire_edit[4], select[4];
  unsigned char lamp[4], speaker[4], empty[4], camera[4];
  unsigned char active[4], group[4], group_active[4], transform[4];
  unsigned char vertex[4], vertex_select[4], vertex_active[4], vertex_bevel[4],
    vertex_unreferenced[4];
  unsigned char edge[4], edge_select[4];
  unsigned char edge_seam[4], edge_sharp[4], edge_facesel[4], edge_crease[4], edge_bevel[4];
  /** Solid faces. */
  unsigned char face[4], face_select[4], face_back[4], face_front[4];
  /** Selected color. */
  unsigned char face_dot[4];
  unsigned char extra_edge_len[4], extra_edge_angle[4], extra_face_angle[4], extra_face_area[4];
  unsigned char normal[4];
  unsigned char vertex_normal[4];
  unsigned char loop_normal[4];
  unsigned char bone_solid[4], bone_pose[4], bone_pose_active[4], bone_locked_weight[4];
  unsigned char strip[4], strip_select[4];
  unsigned char cframe[4];
  unsigned char time_keyframe[4], time_gp_keyframe[4];
  unsigned char freestyle_edge_mark[4], freestyle_face_mark[4];
  unsigned char time_scrub_background[4];
  unsigned char time_marker_line[4], time_marker_line_selected[4];

  unsigned char nurb_uline[4], nurb_vline[4];
  unsigned char act_spline[4], nurb_sel_uline[4], nurb_sel_vline[4], lastsel_point[4];

  unsigned char handle_free[4], handle_auto[4], handle_vect[4], handle_align[4],
    handle_auto_clamped[4];
  unsigned char handle_sel_free[4], handle_sel_auto[4], handle_sel_vect[4], handle_sel_align[4],
    handle_sel_auto_clamped[4];

  /** Dopesheet. */
  unsigned char ds_channel[4], ds_subchannel[4], ds_ipoline[4];
  /** Keytypes. */
  unsigned char keytype_keyframe[4], keytype_extreme[4], keytype_breakdown[4], keytype_jitter[4],
    keytype_movehold[4];
  /** Keytypes. */
  unsigned char keytype_keyframe_select[4], keytype_extreme_select[4], keytype_breakdown_select[4],
    keytype_jitter_select[4], keytype_movehold_select[4];
  unsigned char keyborder[4], keyborder_select[4];

  unsigned char console_output[4], console_input[4], console_info[4], console_error[4];
  unsigned char console_cursor[4], console_select[4];

  unsigned char vertex_size, outline_width, obcenter_dia, facedot_size;
  unsigned char noodle_curving;
  unsigned char grid_levels;
  float dash_alpha;

  /* Syntax for text-window and nodes. */
  unsigned char syntaxl[4], syntaxs[4]; /* in nodespace used for backdrop matte */
  unsigned char syntaxb[4], syntaxn[4]; /* in nodespace used for color input */
  unsigned char syntaxv[4], syntaxc[4]; /* in nodespace used for converter group */
  unsigned char syntaxd[4], syntaxr[4]; /* in nodespace used for distort */

  unsigned char line_numbers[4];

  unsigned char nodeclass_output[4], nodeclass_filter[4];
  unsigned char nodeclass_vector[4], nodeclass_texture[4];
  unsigned char nodeclass_shader[4], nodeclass_script[4];
  unsigned char nodeclass_pattern[4], nodeclass_layout[4];
  unsigned char nodeclass_geometry[4], nodeclass_attribute[4];

  /** For sequence editor. */
  unsigned char movie[4], movieclip[4], mask[4], image[4], scene[4], audio[4];
  unsigned char effect[4], transition[4], meta[4], text_strip[4], color_strip[4];
  unsigned char active_strip[4], selected_strip[4];

  /** For dopesheet - scale factor for size of keyframes (i.e. height of channels). */
  float keyframe_scale_fac;

  unsigned char editmesh_active[4];

  unsigned char handle_vertex[4];
  unsigned char handle_vertex_select[4];

  unsigned char handle_vertex_size;

  unsigned char clipping_border_3d[4];

  unsigned char marker_outline[4], marker[4], act_marker[4], sel_marker[4], dis_marker[4],
    lock_marker[4];
  unsigned char bundle_solid[4];
  unsigned char path_before[4], path_after[4];
  unsigned char path_keyframe_before[4], path_keyframe_after[4];
  unsigned char camera_path[4];

  unsigned char gp_vertex_size;
  unsigned char gp_vertex[4], gp_vertex_select[4];

  unsigned char preview_back[4];
  unsigned char preview_stitch_face[4];
  unsigned char preview_stitch_edge[4];
  unsigned char preview_stitch_vert[4];
  unsigned char preview_stitch_stitchable[4];
  unsigned char preview_stitch_unstitchable[4];
  unsigned char preview_stitch_active[4];

  /** Two uses, for uvs with modifier applied on mesh and uvs during painting. */
  unsigned char uv_shadow[4];

  /** Search filter match, used for property search and in the outliner. */
  unsigned char match[4];
  /** Outliner - selected item. */
  unsigned char selected_highlight[4];
  /** Outliner - selected object. */
  unsigned char selected_object[4];
  /** Outliner - active object. */
  unsigned char active_object[4];
  /** Outliner - edited object. */
  unsigned char edited_object[4];
  /** Outliner - row color difference. */
  unsigned char row_alternate[4];

  /** Skin modifier root color. */
  unsigned char skin_root[4];

  /* NLA */
  /** Active Action + Summary Channel. */
  unsigned char anim_active[4];
  /** Active Action = NULL. */
  unsigned char anim_non_active[4];
  /** Preview range overlay. */
  unsigned char anim_preview_range[4];

  /** NLA 'Tweaking' action/strip. */
  unsigned char nla_tweaking[4];
  /** NLA - warning color for duplicate instances of tweaking strip. */
  unsigned char nla_tweakdupli[4];

  /** NLA "Track" */
  unsigned char nla_track[4];
  /** NLA "Transition" strips. */
  unsigned char nla_transition[4], nla_transition_sel[4];
  /** NLA "Meta" strips. */
  unsigned char nla_meta[4], nla_meta_sel[4];
  /** NLA "Sound" strips. */
  unsigned char nla_sound[4], nla_sound_sel[4];

  /* info */
  unsigned char info_selected[4], info_selected_text[4];
  unsigned char info_error[4], info_error_text[4];
  unsigned char info_warning[4], info_warning_text[4];
  unsigned char info_info[4], info_info_text[4];
  unsigned char info_debug[4], info_debug_text[4];
  unsigned char info_property[4], info_property_text[4];
  unsigned char info_operator[4], info_operator_text[4];

  unsigned char paint_curve_pivot[4];
  unsigned char paint_curve_handle[4];

  unsigned char metadatabg[4];
  unsigned char metadatatext[4];
};

/* Viewport Background Gradient Types. */

enum eBackgroundGradientTypes
{
  TH_BACKGROUND_SINGLE_COLOR = 0,
  TH_BACKGROUND_GRADIENT_LINEAR = 1,
  TH_BACKGROUND_GRADIENT_RADIAL = 2,
};

/** Set of colors for use as a custom color set for Objects/Bones wire drawing. */
struct ThemeWireColor
{
  unsigned char solid[4];
  unsigned char select[4];
  unsigned char active[4];

  /** #eWireColor_Flags. */
  short flag;
};

/** #ThemeWireColor.flag */
enum eWireColor_Flags
{
  TH_WIRECOLOR_CONSTCOLS = (1 << 0),
  /* TH_WIRECOLOR_TEXTCOLS = (1 << 1), */ /* UNUSED */
};

struct ThemeCollectionColor
{
  unsigned char color[4];
};

struct ThemeStripColor
{
  unsigned char color[4];
};

/**
 * A theme.
 *
 * \note Currently only a single theme is ever used at once.
 * Different theme presets are stored as external files now.
 */
struct kTheme
{
  char name[32];

  ThemeUI tui;

  /**
   * Individual Spacetypes:
   * \note Ensure #UI_THEMESPACE_END is updated when adding.
   */
  ThemeSpace space_properties;
  ThemeSpace space_view3d;
  ThemeSpace space_file;
  ThemeSpace space_graph;
  ThemeSpace space_info;
  ThemeSpace space_action;
  ThemeSpace space_nla;
  ThemeSpace space_sequencer;
  ThemeSpace space_image;
  ThemeSpace space_text;
  ThemeSpace space_outliner;
  ThemeSpace space_node;
  ThemeSpace space_preferences;
  ThemeSpace space_console;
  ThemeSpace space_clip;
  ThemeSpace space_topbar;
  ThemeSpace space_statusbar;
  ThemeSpace space_spreadsheet;

  /* 20 sets of bone colors for this theme */
  ThemeWireColor tarm[20];
  // ThemeWireColor tobj[20];

  /* See COLLECTION_COLOR_TOT for the number of collection colors. */
  ThemeCollectionColor collection_color[8];

  /* See SEQUENCE_COLOR_TOT for the total number of strip colors. */
  ThemeStripColor strip_color[9];

  int active_theme_area;
};

#define UI_THEMESPACE_START(ktheme) \
  (CHECK_TYPE_INLINE(ktheme, kTheme *), &((ktheme)->space_properties))
#define UI_THEMESPACE_END(ktheme) \
  (CHECK_TYPE_INLINE(ktheme, kTheme *), (&((ktheme)->space_spreadsheet) + 1))

struct kAddon
{
  struct kAddon *next, *prev;
  char module[64];
};

struct kPathCompare
{
  char path[768];
  char flag;
};

/** May be part of #kUserMenu or other list. */
struct kUserMenuItem
{
  char ui_name[64];
  char type;
};

struct kUserMenu
{
  char space_type;
  char context[64];
  std::vector<kUserMenuItem *> items;
};

struct kUserAssetLibrary
{
  char name[64];   /* MAX_NAME */
  char path[1024]; /* FILE_MAX */
};

struct UserDef : public wabi::UsdUIUserPref
{

  wabi::SdfPath path;

  wabi::UsdAttribute showsave;
  wabi::UsdAttribute dpifac;

  int uiflag;

  std::vector<struct kTheme *> themes;
  std::vector<struct uiFont *> uifonts;
  std::vector<struct uiStyle *> uistyles;
  std::vector<struct wmKeyMap *> user_keymaps;

  std::vector<struct wmKeyConfigPref *> user_keyconfig_prefs;
  std::vector<struct kAddon *> addons;
  std::vector<struct kPathCompare *> autoexec_paths;

  std::vector<struct kUserMenu *> user_menus;

  std::vector<struct kUserAssetLibrary *> asset_libraries;

  inline UserDef(kContext *C,
                 const wabi::SdfPath &stagepath = SdfPath(KRAKEN_PATH_DEFAULTS::KRAKEN_USERPREFS));
};

UserDef::UserDef(kContext *C, const wabi::SdfPath &stagepath)
  : UsdUIUserPref(KRAKEN_STAGE_CREATE(C)),
    path(GetPath()),
    showsave(CreateShowSavePromptAttr()),
    dpifac(CreateDpifacAttr()),
    uiflag(VALUE_ZERO)
{}

KRAKEN_NAMESPACE_END
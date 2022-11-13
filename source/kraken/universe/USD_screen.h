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

#include "KKE_context.h"

#include "USD_api.h"
#include "USD_listBase.h"
#include "USD_types.h"

#include <wabi/base/gf/vec2i.h>
#include <wabi/usd/usdUI/screen.h>

/* ------ */

struct wmMsgBus;

/* ------ */


#define AREAGRID 4
#define AREAMINX 29
#define HEADER_PADDING_Y 6
#define HEADERY (20 + HEADER_PADDING_Y)

#define AREAMAP_FROM_SCREEN(screen) ((ScrAreaMap *)&(screen)->verts)

enum eScreenRedrawsFlag
{
  TIME_REGION = (1 << 0),
  TIME_ALL_3D_WIN = (1 << 1),
  TIME_ALL_ANIM_WIN = (1 << 2),
  TIME_ALL_BUTS_WIN = (1 << 3),
  TIME_SEQ = (1 << 4),
  TIME_ALL_IMAGE_WIN = (1 << 5),
  TIME_NODES = (1 << 6),
  TIME_CLIPS = (1 << 7),

  TIME_FOLLOW = (1 << 15),
};

/** #Panel.flag */
enum
{
  PNL_SELECT = (1 << 0),
  PNL_UNUSED_1 = (1 << 1), /* Cleared */
  PNL_CLOSED = (1 << 2),
  // PNL_TABBED = (1 << 3),  /* UNUSED */
  // PNL_OVERLAP = (1 << 4), /* UNUSED */
  PNL_PIN = (1 << 5),
  PNL_POPOVER = (1 << 6),
  /** The panel has been drag-drop reordered and the instanced panel list needs to be rebuilt. */
  PNL_INSTANCED_LIST_ORDER_CHANGED = (1 << 7),
};

/** #ARegion.do_draw */
enum
{
  /* Region must be fully redrawn. */
  RGN_DRAW = 1,
  /* Redraw only part of region, for sculpting and painting to get smoother
   * stroke painting on heavy meshes. */
  RGN_DRAW_PARTIAL = 2,
  /* For outliner, to do faster redraw without rebuilding outliner tree.
   * For 3D viewport, to display a new progressive render sample without
   * while other buffers and overlays remain unchanged. */
  RGN_DRAW_NO_REBUILD = 4,

  /* Set while region is being drawn. */
  RGN_DRAWING = 8,
  /* For popups, to refresh UI layout along with drawing. */
  RGN_REFRESH_UI = 16,

  /* Only editor overlays (currently gizmos only!) should be redrawn. */
  RGN_DRAW_EDITOR_OVERLAYS = 32,
};

/** Function mapping a context member name to its value. */
typedef int /*eContextResult*/ (*kContextDataCallback)(const kContext *C,
                                                       const char *member,
                                                       kContextDataResult *result);

struct kScreen : public wabi::UsdUIScreen
{
  wabi::SdfPath path;

  wabi::UsdAttribute align;
  wabi::UsdRelationship areas_rel;

  ListBase verts;
  ListBase edges;
  ListBase areas;

  /** Screen level regions (menus), runtime only. */
  ListBase regions;
  short alignment;

  /** Active region that has mouse focus. */
  ARegion *active_region;

  /** If set, screen has timer handler added in window. */
  struct wmTimer *animtimer;
  /** Context callback. */
  kContextDataCallback context;

  /** Runtime */
  short redraws_flag;

  char temp;
  int winid;
  char do_refresh;

  /** Notifier for drawing edges. */
  char do_draw;
  /** Notifier for gesture draw. */
  char do_draw_gesture;
  /** Notifier for paint cursor draw. */
  char do_draw_paintcursor;
  /** Notifier for dragging draw. */
  char do_draw_drag;

  /** Runtime. */
  struct wmTooltipState *tool_tip;

  inline kScreen(kContext *C, const wabi::SdfPath &stagepath);
};

kScreen::kScreen(kContext *C, const wabi::SdfPath &stagepath)
  : wabi::UsdUIScreen(KRAKEN_STAGE_CREATE(C)),
    path(GetPath()),
    align(CreateAlignmentAttr(DEFAULT_VALUE(UsdUITokens->none))),
    areas_rel(CreateAreasRel()),
    active_region(POINTER_ZERO),
    animtimer(POINTER_ZERO),
    context(POINTER_ZERO),
    redraws_flag(VALUE_ZERO),
    temp(VALUE_ZERO),
    winid(VALUE_ZERO),
    do_refresh(VALUE_ZERO),
    do_draw(VALUE_ZERO),
    tool_tip(POINTER_ZERO)
{
  verts = {NULL, NULL};
  edges = {NULL, NULL};
  areas = {NULL, NULL};
  regions = {NULL, NULL};
}

typedef struct ScrVert
{
  struct ScrVert *next, *prev, *newv;

  wabi::GfVec2h vec;
  /* first one used internally, second one for tools */
  short flag, editflag;
} ScrVert;

typedef struct ScrEdge
{
  struct ScrEdge *next, *prev;

  ScrVert *v1, *v2;
  /** 1 when at edge of screen. */
  short border;
  short flag;
} ScrEdge;

typedef struct ScrAreaMap
{
  /** ScrVert. */
  ListBase verts;
  /** ScrEdge. */
  ListBase edges;
  /** ScrArea. */
  ListBase areas;
} ScrAreaMap;

typedef struct Panel_Runtime
{
  /* Applied to Panel.ofsx, but saved separately so we can track changes between redraws. */
  int region_ofsx;

  /**
   * Pointer for storing which data the panel corresponds to.
   * Useful when there can be multiple instances of the same panel type.
   *
   * @note A panel and its sub-panels share the same custom data pointer.
   * This avoids freeing the same pointer twice when panels are removed.
   */
  KrakenPRIM *custom_data_ptr;

  /* Pointer to the panel's block. Useful when changes to panel #uiBlocks
   * need some context from traversal of the panel "tree". */
  uiBlock *block;

  /* Non-owning pointer. The context is stored in the block. */
  kContextStore *context;
} Panel_Runtime;

typedef struct Panel
{
  struct Panel *next;
  /** Runtime. */
  struct PanelType *type;
  /** Runtime for drawing. */
  struct uiLayout *layout;

  /** Defined as UI_MAX_NAME_STR. */
  char panelname[64];
  /** Panel name is identifier for restoring location. */
  char drawname[64];
  /** Offset within the region. */
  int ofsx, ofsy;
  /** Panel size including children. */
  int sizex, sizey;
  /** Panel size excluding children. */
  int blocksizex, blocksizey;
  short labelofs;
  short flag, runtime_flag;
  /** Panels are aligned according to increasing sort-order. */
  int sortorder;
  /** Runtime for panel manipulation. */
  void *activedata;
  /** Sub panels. */
  ListBase children;

  Panel_Runtime runtime;
} Panel;

/**
 * Used for passing expansion between instanced panel data and the panels themselves.
 * There are 16 defines because the expansion data is typically stored in a short.
 *
 * @note Expansion for instanced panels is stored in depth first order. For example, the value of
 * UI_SUBPANEL_DATA_EXPAND_2 correspond to mean the expansion of the second subpanel or the first
 * subpanel's first subpanel.
 */
typedef enum uiPanelDataExpansion
{
  UI_PANEL_DATA_EXPAND_ROOT = (1 << 0),
  UI_SUBPANEL_DATA_EXPAND_1 = (1 << 1),
  UI_SUBPANEL_DATA_EXPAND_2 = (1 << 2),
  UI_SUBPANEL_DATA_EXPAND_3 = (1 << 3),
  UI_SUBPANEL_DATA_EXPAND_4 = (1 << 4),
  UI_SUBPANEL_DATA_EXPAND_5 = (1 << 5),
  UI_SUBPANEL_DATA_EXPAND_6 = (1 << 6),
  UI_SUBPANEL_DATA_EXPAND_7 = (1 << 7),
  UI_SUBPANEL_DATA_EXPAND_8 = (1 << 8),
  UI_SUBPANEL_DATA_EXPAND_9 = (1 << 9),
  UI_SUBPANEL_DATA_EXPAND_10 = (1 << 10),
  UI_SUBPANEL_DATA_EXPAND_11 = (1 << 11),
  UI_SUBPANEL_DATA_EXPAND_12 = (1 << 12),
  UI_SUBPANEL_DATA_EXPAND_13 = (1 << 13),
  UI_SUBPANEL_DATA_EXPAND_14 = (1 << 14),
  UI_SUBPANEL_DATA_EXPAND_15 = (1 << 15),
  UI_SUBPANEL_DATA_EXPAND_16 = (1 << 16),
} uiPanelDataExpansion;

/**
 * Notes on Panel Categories:
 *
 * - #ARegion.panels_category (#PanelCategoryDyn)
 *   is a runtime only list of categories collected during draw.
 *
 * - #ARegion.panels_category_active (#PanelCategoryStack)
 *   is basically a list of strings (category id's).
 *
 * Clicking on a tab moves it to the front of region->panels_category_active,
 * If the context changes so this tab is no longer displayed,
 * then the first-most tab in #ARegion.panels_category_active is used.
 *
 * This way you can change modes and always have the tab you last clicked on.
 */

/* region level tabs */
typedef struct PanelCategoryDyn
{
  struct PanelCategoryDyn *next, *prev;
  char idname[64];
  rcti rect;
} PanelCategoryDyn;

/** Region stack of active tabs. */
typedef struct PanelCategoryStack
{
  struct PanelCategoryStack *next, *prev;
  char idname[64];
} PanelCategoryStack;

typedef void (*uiListFreeRuntimeDataFunc)(struct uiList *ui_list);

typedef struct uiListDyn
{
  /** Callback to free UI data when freeing UI-Lists in BKE. */
  uiListFreeRuntimeDataFunc free_runtime_data_fn;

  /** Number of rows needed to draw all elements. */
  int height;
  /** Actual visual height of the list (in rows). */
  int visual_height;
  /** Minimal visual height of the list (in rows). */
  int visual_height_min;

  /** Number of columns drawn for grid layouts. */
  int columns;

  /** Number of items in collection. */
  int items_len;
  /** Number of items actually visible after filtering. */
  int items_shown;

  /* Those are temp data used during drag-resize with GRIP button
   * (they are in pixels, the meaningful data is the
   * difference between resize_prev and resize)...
   */
  int resize;
  int resize_prev;

  /** Allocated custom data. Freed together with the #uiList (and when re-assigning). */
  void *customdata;

  /* Filtering data. */
  /** Items_len length. */
  int *items_filter_flags;
  /** Org_idx -> new_idx, items_len length. */
  int *items_filter_neworder;

  struct wmOperatorType *custom_drag_optype;
  struct KrakenPRIM *custom_drag_opptr;
  struct wmOperatorType *custom_activate_optype;
  struct KrakenPRIM *custom_activate_opptr;
} uiListDyn;

/* some list UI data need to be saved in file */
typedef struct uiList
{
  struct uiList *next, *prev;

  /** Runtime. */
  struct uiListType *type;

  /** Defined as UI_MAX_NAME_STR. */
  char list_id[64];

  /** How items are laid out in the list. */
  int layout_type;
  int flag;

  int list_scroll;
  int list_grip;
  int list_last_len;
  int list_last_activei;

  /* Filtering data. */
  /** Defined as UI_MAX_NAME_STR. */
  char filter_byname[64];
  int filter_flag;
  int filter_sort_flag;

  /* Custom sub-classes properties. */
  IDProperty *properties;

  /* Dynamic data (runtime). */
  uiListDyn *dyn_data;
} uiList;

struct wmRegionMessageSubscribeParams
{
  const kContext *context;
  wmMsgBus *message_bus;
  WorkSpace *workspace;
  kScene *scene;
  kScreen *screen;
  ScrArea *area;
  ARegion *region;

  wmRegionMessageSubscribeParams()
    : context(POINTER_ZERO),
      message_bus(POINTER_ZERO),
      workspace(POINTER_ZERO),
      scene(POINTER_ZERO),
      screen(POINTER_ZERO),
      area(POINTER_ZERO),
      region(POINTER_ZERO)
  {}
};

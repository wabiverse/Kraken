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

#include "USD_area.h"
#include "USD_context.h"
#include "USD_region.h"

#include "KKE_context.h"
#include "KKE_robinhood.h"
#include "KKE_utils.h"

#include <wabi/base/gf/vec2i.h>
#include <wabi/usd/usdUI/screen.h>

KRAKEN_NAMESPACE_BEGIN

#define AREAMAP_FROM_SCREEN(screen) ((ScrAreaMap *)&(screen)->verts)

#define KKE_ST_MAXNAME 64

struct Menu
{
  struct MenuType *type;   /* runtime */
  struct uiLayout *layout; /* runtime for drawing */
};

struct MenuType
{
  char idname[KKE_ST_MAXNAME]; /* unique name */
  char label[KKE_ST_MAXNAME];  /* for button text */
  char translation_context[KKE_ST_MAXNAME];
  char owner_id[KKE_ST_MAXNAME]; /* optional, see: #wmOwnerID */
  const char *description;

  /* verify if the menu should draw or not */
  bool (*poll)(const struct kContext *C, struct MenuType *mt);
  /* draw entirely, view changes should be handled here */
  void (*draw)(const struct kContext *C, struct Menu *menu);
};

struct PanelType
{
  char idname[KKE_ST_MAXNAME]; /* unique name */
  char label[KKE_ST_MAXNAME];  /* for panel header */
  char *description;           /* for panel tooltip */
  char translation_context[KKE_ST_MAXNAME];
  char context[KKE_ST_MAXNAME];   /* for buttons window */
  char category[KKE_ST_MAXNAME];  /* for category tabs */
  char owner_id[KKE_ST_MAXNAME];  /* for work-spaces to selectively show. */
  char parent_id[KKE_ST_MAXNAME]; /* parent idname for sub-panels */
  /** Boolean property identifier of the panel custom data. Used to draw a highlighted border. */
  char active_property[KKE_ST_MAXNAME];
  short space_type;
  short region_type;
  /* For popovers, 0 for default. */
  int ui_units_x;
  int order;

  int flag;

  /* verify if the panel should draw or not */
  bool (*poll)(const struct kContext *C, struct PanelType *pt);
  /* draw header (optional) */
  void (*draw_header)(const struct kContext *C, struct Panel *panel);
  /* draw header preset (optional) */
  void (*draw_header_preset)(const struct kContext *C, struct Panel *panel);
  /* draw entirely, view changes should be handled here */
  void (*draw)(const struct kContext *C, struct Panel *panel);

  /* For instanced panels corresponding to a list: */

  /** Reorder function, called when drag and drop finishes. */
  void (*reorder)(struct kContext *C, struct Panel *pa, int new_index);
  /**
   * Get the panel and sub-panel's expansion state from the expansion flag in the corresponding
   * data item. Called on draw updates.
   * \note Sub-panels are indexed in depth first order,
   * the visual order you would see if all panels were expanded.
   */
  short (*get_list_data_expand_flag)(const struct kContext *C, struct Panel *pa);
  /**
   * Set the expansion bit-field from the closed / open state of this panel and its sub-panels.
   * Called when the expansion state of the panel changes with user input.
   * \note Sub-panels are indexed in depth first order,
   * the visual order you would see if all panels were expanded.
   */
  void (*set_list_data_expand_flag)(const struct kContext *C, struct Panel *pa, short expand_flag);

  /* sub panels */
  struct PanelType *parent;
  std::vector<struct PanelType *> children;
};

struct Panel
{
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
  char _pad[6];
  /** Panels are aligned according to increasing sort-order. */
  int sortorder;
  /** Runtime for panel manipulation. */
  void *activedata;
  /** Sub panels. */
  std::vector<struct Panel *> children;

  // Panel_Runtime runtime;
};

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

struct ScrVert
{
  GfVec2h vec;
  /* first one used internally, second one for tools */
  short flag, editflag;

  ScrVert() : vec(VALUE_ZERO), flag(VALUE_ZERO), editflag(VALUE_ZERO) {}
};

struct ScrEdge
{
  ScrVert *v1, *v2;
  /** 1 when at edge of screen. */
  short border;
  short flag;

  ScrEdge() : v1(POINTER_ZERO), v2(POINTER_ZERO), border(VALUE_ZERO), flag(VALUE_ZERO) {}
};

struct kScreen : public wabi::UsdUIScreen
{
  wabi::SdfPath path;

  wabi::UsdAttribute align;
  wabi::UsdRelationship areas_rel;

  std::vector<ScrVert *> verts;
  std::vector<ScrEdge *> edges;
  std::vector<ScrArea *> areas;

  /** Screen level regions (menus), runtime only. */
  std::vector<ARegion *> regions;
  short alignment;

  ARegion *active_region;

  /** Runtime */
  short redraws_flag;

  char temp;
  int winid;
  char do_refresh;

  /** Notifier for drawing edges. */
  char do_draw;

  /** Runtime. */
  struct wmTooltipState *tool_tip;

  inline kScreen(kContext *C, const wabi::SdfPath &stagepath);
};

kScreen::kScreen(kContext *C, const wabi::SdfPath &stagepath)
  : wabi::UsdUIScreen(KRAKEN_STAGE_CREATE(C)),
    path(GetPath()),
    align(CreateAlignmentAttr(DEFAULT_VALUE(UsdUITokens->none))),
    areas_rel(CreateAreasRel()),
    verts(EMPTY),
    areas(EMPTY),
    regions(EMPTY),
    active_region(POINTER_ZERO),
    redraws_flag(VALUE_ZERO),
    temp(VALUE_ZERO),
    winid(VALUE_ZERO),
    do_refresh(VALUE_ZERO),
    do_draw(VALUE_ZERO),
    tool_tip(POINTER_ZERO)
{}


struct wmRegionMessageSubscribeParams
{
  const struct kContext *context;
  struct wmMsgBus *message_bus;
  struct WorkSpace *workspace;
  struct Scene *scene;
  struct kScreen *screen;
  struct ScrArea *area;
  struct ARegion *region;

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


KRAKEN_NAMESPACE_END
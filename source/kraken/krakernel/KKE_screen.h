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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KKE_api.h"
#include "KKE_context.h"
#include "KKE_utils.h"

#include "USD_listBase.h"
#include "USD_wm_types.h"
#include "USD_object.h"
#include "USD_region.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "LUXO_types.h"

#include "wabi/usd/usdUI/window.h"
#include "wabi/usd/usd/prim.h"

#define KKE_ST_MAXNAME 64

struct kContext;

typedef struct wmRegionListenerParams
{
  struct wmWindow *window;
  struct ScrArea *area;
  struct ARegion *region;
  struct wmNotifier *notifier;
  const struct kScene *scene;
} wmRegionListenerParams;


typedef struct ARegionType
{
  struct ARegionType *next, *prev;

  int regionid;

  void (*init)(wmWindowManager *wm, ARegion *region);
  void (*exit)(wmWindowManager *wm, ARegion *region);
  void (*draw)(const kContext *C, ARegion *region);

  void (*draw_overlay)(const kContext *C, ARegion *region);
  void (*layout)(const kContext *C, ARegion *region);
  int (*snap_size)(const ARegion *region, int size, int axis);
  void (*listener)(const wmRegionListenerParams *params);
  void (*message_subscribe)(const wmRegionMessageSubscribeParams *params);

  void (*free)(ARegion *);

  void *(*duplicate)(void *poin);

  void (*operatortypes)(void);
  void (*keymap)(wmKeyConfig *keyconf);
  void (*cursor)(wmWindow *win, struct ScrArea *area, ARegion *region);

  /* return context data */
  kContextDataCallback context;

  void (*on_view2d_changed)(const kContext *C, ARegion *region);

  ListBase paneltypes;

  int minsizex, minsizey;
  int prefsizex, prefsizey;
  int keymapflag;
  short do_lock, lock;
  bool clip_gizmo_events_by_ui;
  short event_cursor;
} ARegionType;

typedef struct PanelType
{
  wabi::TfToken idname;       /* unique name */
  char label[KKE_ST_MAXNAME]; /* for panel header */
  char *description;          /* for panel tooltip */
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
   * @note Sub-panels are indexed in depth first order,
   * the visual order you would see if all panels were expanded.
   */
  short (*get_list_data_expand_flag)(const struct kContext *C, struct Panel *pa);
  /**
   * Set the expansion bit-field from the closed / open state of this panel and its sub-panels.
   * Called when the expansion state of the panel changes with user input.
   * @note Sub-panels are indexed in depth first order,
   * the visual order you would see if all panels were expanded.
   */
  void (*set_list_data_expand_flag)(const struct kContext *C, struct Panel *pa, short expand_flag);

  /* sub panels */
  struct PanelType *parent;
  ListBase children;
} PanelType;

/* #PanelType.flag */
enum
{
  PANEL_TYPE_DEFAULT_CLOSED = (1 << 0),
  PANEL_TYPE_NO_HEADER = (1 << 1),
  /** Makes buttons in the header shrink/stretch to fill full layout width. */
  PANEL_TYPE_HEADER_EXPAND = (1 << 2),
  PANEL_TYPE_LAYOUT_VERT_BAR = (1 << 3),
  /** This panel type represents data external to the UI. */
  PANEL_TYPE_INSTANCED = (1 << 4),
  /** Don't search panels with this type during property search. */
  PANEL_TYPE_NO_SEARCH = (1 << 7),
};

/* Draw an item in the uiList */
typedef void (*uiListDrawItemFunc)(struct uiList *ui_list,
                                   const struct kContext *C,
                                   struct uiLayout *layout,
                                   struct KrakenPRIM *dataptr,
                                   struct KrakenPRIM *itemptr,
                                   int icon,
                                   struct KrakenPRIM *active_dataptr,
                                   const char *active_propname,
                                   int index,
                                   int flt_flag);

/* Draw the filtering part of an uiList */
typedef void (*uiListDrawFilterFunc)(struct uiList *ui_list,
                                     const struct kContext *C,
                                     struct uiLayout *layout);

/* Filter items of an uiList */
typedef void (*uiListFilterItemsFunc)(struct uiList *ui_list,
                                      const struct kContext *C,
                                      struct KrakenPRIM *,
                                      const char *propname);

/* Listen to notifiers. Only for lists defined in C. */
typedef void (*uiListListener)(struct uiList *ui_list, wmRegionListenerParams *params);

typedef struct uiListType
{
  struct uiListType *next, *prev;

  char idname[KKE_ST_MAXNAME]; /* unique name */

  uiListDrawItemFunc draw_item;
  uiListDrawFilterFunc draw_filter;
  uiListFilterItemsFunc filter_items;

  /* For lists defined in C only. */
  uiListListener listener;

  /* RNA integration */
  ExtensionPRIM rna_ext;
} uiListType;

typedef struct MenuType
{
  wabi::TfToken idname;       /* unique name */
  char label[KKE_ST_MAXNAME]; /* for button text */
  char translation_context[KKE_ST_MAXNAME];
  char owner_id[KKE_ST_MAXNAME]; /* optional, see: #wmOwnerID */
  const char *description;

  /* verify if the menu should draw or not */
  bool (*poll)(const struct kContext *C, struct MenuType *mt);
  /* draw entirely, view changes should be handled here */
  void (*draw)(const struct kContext *C, struct Menu *menu);
} MenuType;

typedef struct Menu
{
  struct MenuType *type;   /* runtime */
  struct uiLayout *layout; /* runtime for drawing */
} Menu;

typedef struct SpaceType
{
  wabi::TfToken name; /* for menus */
  int spaceid;        /* unique space identifier */
  int iconid;         /* icon lookup for menus */

  struct SpaceLink *(*create)(const struct ScrArea *area, const struct kScene *scene);

  void (*free)(struct SpaceLink *sl);


  void (*init)(struct wmWindowManager *wm, struct ScrArea *area);
  void (*exit)(struct wmWindowManager *wm, struct ScrArea *area);
  void (*listener)(const struct wmSpaceTypeListenerParams *params);


  void (*deactivate)(struct ScrArea *area);

  void (*refresh)(const struct kContext *C, struct ScrArea *area);

  struct SpaceLink *(*duplicate)(struct SpaceLink *sl);

  void (*operatortypes)(void);
  void (*keymap)(struct wmKeyConfig *keyconf);
  void (*dropboxes)(void);

  void (*gizmos)(void);

  /* return context data */
  kContextDataCallback context;

  /* Used when we want to replace an ID by another (or NULL). */
  void (*id_remap)(struct ScrArea *area,
                   struct SpaceLink *sl,
                   wabi::TfToken old_id,
                   wabi::TfToken new_id);

  int (*space_subtype_get)(struct ScrArea *area);
  void (*space_subtype_set)(struct ScrArea *area, int value);
  // void (*space_subtype_item_extend)(kContext *C, EnumPROP **item, int *totitem);

  /* region type definitions */
  ListBase regiontypes;

  int keymapflag;

} SpaceType;


wabi::SdfPath make_screenpath(const char *layout_name, int id);
int find_free_screenid(struct kContext *C);

struct ARegion *KKE_area_find_region_type(const struct ScrArea *area, int type);
struct ARegion *KKE_area_find_region_active_win(struct ScrArea *area);

struct SpaceType *KKE_spacetype_from_id(int spaceid);
struct ARegionType *KKE_regiontype_from_id_or_first(struct SpaceType *st, int regionid);
struct ARegionType *KKE_regiontype_from_id(struct SpaceType *st, int regionid);

bool KKE_screen_is_used(const kScreen *screen);
void KKE_screen_sort_scrvert(struct ScrVert **v1, struct ScrVert **v2);
struct ScrArea *KKE_screen_find_big_area(kScreen *screen, const int spacetype, const short min);


void KKE_area_region_free(struct SpaceType *st, struct ARegion *region);
void KKE_screen_area_map_free(struct ScrAreaMap *area_map);
void KKE_screen_area_free(struct ScrArea *area);

void KKE_area_region_panels_free(ListBase *panels);

struct ScrEdge *KKE_screen_find_edge(const struct kScreen *screen,
                                     struct ScrVert *v1,
                                     struct ScrVert *v2);

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
 * COVAH Kernel.
 * Purple Underground.
 */

#pragma once

#include "CKE_api.h"
#include "CKE_context.h"
#include "CKE_utils.h"

#include "UNI_area.h"
#include "UNI_object.h"
#include "UNI_region.h"
#include "UNI_scene.h"
#include "UNI_screen.h"
#include "UNI_space_types.h"
#include "UNI_window.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

WABI_NAMESPACE_BEGIN


struct wmRegionListenerParams
{
  struct wmWindow *window;
  struct ScrArea *area;
  struct ARegion *region;
  struct wmNotifier *notifier;
  const struct Scene *scene;

  wmRegionListenerParams()
    : window(POINTER_ZERO),
      area(POINTER_ZERO),
      region(POINTER_ZERO),
      notifier(POINTER_ZERO),
      scene(POINTER_ZERO)
  {}
};


struct ARegionType
{
  int regionid;

  void (*init)(wmWindowManager *wm, ARegion *region);
  void (*exit)(wmWindowManager *wm, ARegion *region);
  void (*draw)(const cContext *C, ARegion *region);

  void (*draw_overlay)(const cContext *C, ARegion *region);
  void (*layout)(const cContext *C, ARegion *region);
  int (*snap_size)(const ARegion *region, int size, int axis);
  void (*listener)(const wmRegionListenerParams *params);
  void (*message_subscribe)(const struct wmRegionMessageSubscribeParams *params);

  void (*free)(ARegion *);

  void *(*duplicate)(void *poin);

  void (*operatortypes)(void);
  void (*keymap)(wmKeyConfig *keyconf);
  void (*cursor)(wmWindow *win, ScrArea *area, ARegion *region);

  /* return context data */
  cContextDataCallback context;

  void (*on_view2d_changed)(const cContext *C, ARegion *region);

  int minsizex, minsizey;
  int prefsizex, prefsizey;
  int keymapflag;
  short do_lock, lock;
  bool clip_gizmo_events_by_ui;
  short event_cursor;

  ARegionType()
    : regionid(VALUE_ZERO),
      context(VALUE_ZERO),
      minsizex(VALUE_ZERO),
      minsizey(VALUE_ZERO),
      prefsizex(VALUE_ZERO),
      prefsizey(VALUE_ZERO),
      keymapflag(VALUE_ZERO),
      do_lock(VALUE_ZERO),
      lock(VALUE_ZERO),
      clip_gizmo_events_by_ui(VALUE_ZERO),
      event_cursor(VALUE_ZERO)
  {}
};


struct SpaceType
{
  TfToken name; /* for menus */
  int spaceid;  /* unique space identifier */
  int iconid;   /* icon lookup for menus */

  struct SpaceLink *(*create)(const ScrArea *area, const Scene *scene);

  void (*free)(struct SpaceLink *sl);


  void (*init)(wmWindowManager *wm, ScrArea *area);
  void (*exit)(wmWindowManager *wm, ScrArea *area);
  void (*listener)(const struct wmSpaceTypeListenerParams *params);


  void (*deactivate)(ScrArea *area);

  void (*refresh)(const cContext *C, ScrArea *area);

  struct SpaceLink *(*duplicate)(struct SpaceLink *sl);

  void (*operatortypes)(void);
  void (*keymap)(wmKeyConfig *keyconf);
  void (*dropboxes)(void);

  void (*gizmos)(void);

  /* return context data */
  cContextDataCallback context;

  /* Used when we want to replace an ID by another (or NULL). */
  void (*id_remap)(ScrArea *area,
                   struct SpaceLink *sl,
                   TfToken old_id,
                   TfToken new_id);

  int (*space_subtype_get)(ScrArea *area);
  void (*space_subtype_set)(ScrArea *area, int value);
  // void (*space_subtype_item_extend)(bContext *C, EnumPropertyItem **item, int *totitem);

  /* region type definitions */
  std::vector<ARegionType *> regiontypes;

  int keymapflag;

  SpaceType()
    : name(EMPTY),
      spaceid(VALUE_ZERO),
      iconid(VALUE_ZERO),
      context(VALUE_ZERO),
      regiontypes(EMPTY),
      keymapflag(VALUE_ZERO)
  {}
};


int find_free_screenid(Main *cmain);
SdfPath make_screenpath(int id, const char *name);

SpaceType *CKE_spacetype_from_id(int spaceid);
bool CKE_screen_is_used(const cScreen *screen);
void CKE_screen_sort_scrvert(struct ScrVert **v1, struct ScrVert **v2);
ScrArea *CKE_screen_find_big_area(cScreen *screen, const int spacetype, const short min);

WABI_NAMESPACE_END
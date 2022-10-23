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
 * Editors.
 * Tools for Artists.
 */

#include <stdbool.h>

#include "KLI_sys_types.h"

struct ARegion;
struct kContext;
struct kScreen;
struct ScrArea;
struct wmWindow;
struct WorkSpace;
struct wmMsgSubscribeKey;
struct wmMsgSubscribeValue;
struct WorkSpaceLayout;
struct ScrVert;
struct ScrEdge;
struct ScrAreaMap;
struct GfRect2i;

#ifdef __cplusplus
#  include "USD_area.h"
#  include "USD_context.h"
#  include "USD_region.h"
#  include "USD_screen.h"
#  include "USD_window.h"
#  include "USD_workspace.h"

#  include "KKE_context.h"

#  include "ED_defines.h"
#endif /* __cplusplus */

typedef enum eScreenAxis
{
  /** Horizontal. */
  SCREEN_AXIS_H = 'h',
  /** Vertical. */
  SCREEN_AXIS_V = 'v',
} eScreenAxis;

struct WorkSpace *ED_workspace_add(struct kContext *C, const char *name);

void ED_area_init(struct wmWindowManager *wm, struct wmWindow *win, struct ScrArea *area);
void ED_area_exit(struct kContext *C, struct ScrArea *area);
void ED_area_do_refresh(struct kContext *C, struct ScrArea *area);
bool ED_area_is_global(const struct ScrArea *area);
int ED_area_global_size_y(const struct ScrArea *area);

bool ED_screen_change(struct kContext *C, struct kScreen *screen);
void ED_screen_exit(struct kContext *C, struct wmWindow *window, struct kScreen *screen);

void ED_region_exit(struct kContext *C, struct ARegion *region);

void ED_area_tag_refresh(struct ScrArea *area);
void ED_region_tag_redraw(struct ARegion *region);
void ED_region_tag_redraw_no_rebuild(struct ARegion *region);
void ED_region_tag_refresh_ui(struct ARegion *region);

int ED_area_headersize(void);

#ifdef __cplusplus
void ED_area_newspace(struct kContext *C,
                      struct ScrArea *area,
                      TfToken type,
                      bool skip_region_exit);
#endif

/* message_bus callbacks */
void ED_region_do_msg_notify_tag_redraw(struct kContext *C,
                                        struct wmMsgSubscribeKey *msg_key,
                                        struct wmMsgSubscribeValue *msg_val);
void ED_area_do_msg_notify_tag_refresh(struct kContext *C,
                                       struct wmMsgSubscribeKey *msg_key,
                                       struct wmMsgSubscribeValue *msg_val);

struct WorkSpaceLayout *ED_workspace_layout_add(struct kContext *C,
                                                struct WorkSpace *workspace,
                                                struct wmWindow *win,
                                                const char *name) ATTR_NONNULL();

struct WorkSpaceLayout *ED_workspace_screen_change_ensure_unused_layout(
  struct Main *kmain,
  struct WorkSpace *workspace,
  struct WorkSpaceLayout *layout_new,
  struct WorkSpaceLayout *layout_fallback_base,
  struct wmWindow *win);

struct kScreen *screen_add(struct kContext *C, const char *name, const rcti *rect);
struct ScrVert *screen_geom_vertex_add_ex(struct ScrAreaMap *area_map, short x, short y);
struct ScrVert *screen_geom_vertex_add(struct kScreen *screen, short x, short y);
struct ScrEdge *screen_geom_edge_add(struct kScreen *screen,
                                     struct ScrVert *v1,
                                     struct ScrVert *v2);
struct ScrEdge *screen_geom_edge_add_ex(struct ScrAreaMap *area_map,
                                        struct ScrVert *v1,
                                        struct ScrVert *v2);
void screen_geom_select_connected_edge(const struct wmWindow *win, struct ScrEdge *edge);

void ED_screen_global_areas_refresh(struct wmWindow *win);
void ED_screen_refresh(struct wmWindowManager *wm, struct wmWindow *win);
void ED_screen_ensure_updated(struct wmWindowManager *wm, struct wmWindow *win, struct kScreen *screen);

struct ScrArea *ED_screen_areas_iter_first(const struct wmWindow *win,
                                           const struct kScreen *screen);
struct ScrArea *ED_screen_areas_iter_next(const struct kScreen *screen,
                                          const struct ScrArea *area);

#define ED_screen_areas_iter(win, screen, area_name)                                    \
  for (ScrArea *area_name = ED_screen_areas_iter_first(win, screen); area_name != NULL; \
       area_name = ED_screen_areas_iter_next(screen, area_name))
#define ED_screen_verts_iter(win, screen, vert_name)                          \
  for (ScrVert *vert_name = (ScrVert *)(win)->global_areas.verts.first ?      \
                              (ScrVert *)(win)->global_areas.verts.first :    \
                              (ScrVert *)(screen)->verts.first;               \
       vert_name != NULL;                                                     \
       vert_name = (vert_name == (ScrVert *)(win)->global_areas.verts.last) ? \
                     (ScrVert *)(screen)->verts.first :                       \
                     vert_name->next)

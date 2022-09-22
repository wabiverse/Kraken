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
#include "USD_screen.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "KKE_context.h"

#include "ED_defines.h"

KRAKEN_NAMESPACE_BEGIN

WorkSpace *ED_workspace_add(kContext *C, const char *name);

void ED_area_exit(kContext *C, ScrArea *area);
void ED_area_do_refresh(kContext *C, ScrArea *area);
bool ED_area_is_global(const ScrArea *area);
int ED_area_global_size_y(const ScrArea *area);
void ED_area_newspace(kContext *C,
                      ScrArea *area,
                      const TfToken &type,
                      const bool skip_region_exit);

bool ED_screen_change(kContext *C, kScreen *screen);
void ED_screen_exit(kContext *C, wmWindow *window, kScreen *screen);

void ED_region_exit(kContext *C, ARegion *region);

void ED_region_tag_redraw_no_rebuild(struct ARegion *region);
void ED_region_tag_refresh_ui(struct ARegion *region);

WorkSpaceLayout *ED_workspace_screen_change_ensure_unused_layout(
  Main *kmain,
  WorkSpace *workspace,
  WorkSpaceLayout *layout_new,
  const WorkSpaceLayout *layout_fallback_base,
  wmWindow *win);
WorkSpaceLayout *ED_workspace_layout_add(kContext *C,
                                         WorkSpace *workspace,
                                         wmWindow *win,
                                         const char *name) ATTR_NONNULL();

kScreen *screen_add(kContext *C, const char *name, const GfRect2i *rect);
ScrVert *screen_geom_vertex_add_ex(ScrAreaMap *area_map, short x, short y);
ScrVert *screen_geom_vertex_add(kScreen *screen, short x, short y);
ScrEdge *screen_geom_edge_add(kScreen *screen, ScrVert *v1, ScrVert *v2);
ScrEdge *screen_geom_edge_add_ex(ScrAreaMap *area_map, ScrVert *v1, ScrVert *v2);

KRAKEN_NAMESPACE_END
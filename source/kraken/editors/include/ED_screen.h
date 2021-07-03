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

#pragma once

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_window.h"
#include "UNI_workspace.h"

#include "KKE_context.h"

#include "ED_defines.h"

WABI_NAMESPACE_BEGIN

WorkSpace *ED_workspace_add(cContext *C, const char *name);

void ED_area_exit(cContext *C, ScrArea *area);
bool ED_screen_change(cContext *C, cScreen *screen);
bool ED_area_is_global(const ScrArea *area);
int ED_area_global_size_y(const ScrArea *area);
void ED_area_newspace(cContext *C, ScrArea *area, const TfToken &type, const bool skip_region_exit);

cScreen *screen_add(cContext *C, const char *name, const GfRect2i *rect);
ScrVert *screen_geom_vertex_add_ex(ScrAreaMap *area_map, short x, short y);
ScrVert *screen_geom_vertex_add(cScreen *screen, short x, short y);
ScrEdge *screen_geom_edge_add(cScreen *screen, ScrVert *v1, ScrVert *v2);
ScrEdge *screen_geom_edge_add_ex(ScrAreaMap *area_map, ScrVert *v1, ScrVert *v2);

WorkSpaceLayout *ED_workspace_screen_change_ensure_unused_layout(Main *cmain,
                                                                 WorkSpace *workspace,
                                                                 WorkSpaceLayout *layout_new,
                                                                 const WorkSpaceLayout *layout_fallback_base,
                                                                 wmWindow *win);
WorkSpaceLayout *ED_workspace_layout_add(cContext *C,
                                         WorkSpace *workspace,
                                         wmWindow *win,
                                         const char *name) ATTR_NONNULL();

WABI_NAMESPACE_END
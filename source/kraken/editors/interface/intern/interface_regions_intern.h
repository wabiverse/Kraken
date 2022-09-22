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

#ifndef KRAKEN_EDITORS_INTERFACE_REGIONS_INTERN_H
#define KRAKEN_EDITORS_INTERFACE_REGIONS_INTERN_H

#include "kraken/kraken.h"

#include "USD_api.h"
#include "USD_color_types.h"
#include "USD_curveprofile_types.h"
#include "USD_scene.h"
#include "USD_area.h"
#include "USD_screen.h"
#include "USD_texture_types.h"

#include "KKE_context.h"

#include "UI_interface.h"

KRAKEN_NAMESPACE_BEGIN

void ui_region_temp_remove(struct kContext *C, kScreen *screen, ARegion *region);

KRAKEN_NAMESPACE_END

#endif /* KRAKEN_EDITORS_INTERFACE_REGIONS_INTERN_H */
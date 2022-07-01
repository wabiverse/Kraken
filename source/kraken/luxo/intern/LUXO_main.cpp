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
 * Luxo.
 * The Universe Gets Animated.
 */

#include "LUXO_main.h"
#include "LUXO_internal.h"
#include "LUXO_access.h"

WABI_NAMESPACE_BEGIN

static void luxo_preprocess(KrakenSTAGE kstage)
{
  PRIM_def_wm(kstage);
  PRIM_def_info(kstage);
}

void LUXO_main(KrakenSTAGE kstage)
{
  luxo_preprocess(kstage);
}

WABI_NAMESPACE_END
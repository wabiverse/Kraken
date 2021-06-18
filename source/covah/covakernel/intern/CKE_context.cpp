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

#include "CKE_context.h"
#include "CKE_main.h"
#include "CKE_version.h"

#include "UNI_object.h"
#include "UNI_system.h"
#include "UNI_window.h"

#include <wabi/base/tf/mallocTag.h>
#include <wabi/usd/usd/attribute.h>

WABI_NAMESPACE_BEGIN

/**
 * Main CTX Creation. */
cContext CTX_create(void)
{
  TfAutoMallocTag2 tag("cContext", "CTX_create");

  return TfCreateRefPtr(new CovahContext());
}

/**
 * Main CTX Deletion. */
void CTX_free(const cContext &C)
{
  TfAutoMallocTag2 tag("cContext", "CTX_free");

  /**
   * CTX out - */

  C.~TfRefPtr();
}

/**
 * Getters. */

Main CTX_data_main(const cContext &C)
{
  return C->data.main;
}

wmWindowManager CTX_wm_manager(const cContext &C)
{
  return C->wm.manager;
}

wmWindow CTX_wm_window(const cContext &C)
{
  return C->wm.window;
}

Scene CTX_data_scene(const cContext &C)
{
  return C->data.scene;
}

Stage CTX_data_stage(const cContext &C)
{
  return C->data.stage;
}

/**
 * Setters. */

void CTX_data_main_set(const cContext &C, const Main &cmain)
{
  C->data.main = cmain;
}

void CTX_wm_manager_set(const cContext &C, const wmWindowManager &wm)
{
  C->wm.manager = wm;
}

void CTX_wm_window_set(const cContext &C, const wmWindow &win)
{
  C->wm.window = win;
}

void CTX_data_scene_set(const cContext &C, const Scene &cscene)
{
  C->data.scene = cscene;
  C->data.stage = cscene->stage;
}

WABI_NAMESPACE_END
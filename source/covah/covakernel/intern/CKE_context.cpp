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

#include "WM_windowmanager.h"

#include <wabi/base/tf/mallocTag.h>

WABI_NAMESPACE_USING

struct cContext {

  cContext() = default;

  int thread;

  /* windowmanager context */
  struct {
    wmWindowManager *manager;
    const char *operator_poll_msg;
  } wm;

  /* data context */
  struct {
    struct Main *main;
    struct Scene *scene;
  } data;
};

cContext *CTX_create(void)
{
  TfAutoMallocTag2 tag("cContext", "CTX_create");

  cContext *C = new cContext();

  return C;
}

void CTX_free(cContext *C)
{
  TfAutoMallocTag2 tag("cContext", "CTX_free");

  delete C;
}

wmWindowManager *CTX_wm_manager(const cContext *C)
{
  return C->wm.manager;
}

void CTX_wm_manager_set(cContext *C, wmWindowManager *wm)
{
  C->wm.manager = wm;
}

Main *CTX_data_main(const cContext *C)
{
  return C->data.main;
}

Scene *CTX_data_scene(const cContext *C)
{
  return C->data.scene;
}
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

struct cContext;
struct Main;
struct Scene;
struct wmWindowManager;

/**
 * Covah Context:
 *  - Creation.
 *  - Destruction. */

cContext *CTX_create(void);
void CTX_free(cContext *C);

/**
 * Covah Context Getters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data. */

Main *CTX_data_main(const cContext *C);
wmWindowManager *CTX_wm_manager(const cContext *C);
Scene *CTX_data_scene(const cContext *C);

/**
 * Covah Context Setters:
 *  - System Main.
 *  - WindowManager.
 *  - Scene data. */

void CTX_data_main_set(cContext *C, Main *cmain);
void CTX_wm_manager_set(cContext *C, wmWindowManager *wm);
void CTX_data_scene_set(cContext *C, Scene *cscene);
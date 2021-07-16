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
 * KRAKEN Python.
 * It Bites.
 */

#include "KPY_api.h"

/* For 'FILE'. */
#include <stdio.h>

WABI_NAMESPACE_BEGIN

struct kContext;

/**
 * Functionality relating to Python setup & teardown. */

/* wpy_interface.cpp */
void KPY_python_start(struct kContext *C, int argc, const char **argv);

void KPY_python_reset(struct kContext *C);

WABI_NAMESPACE_END
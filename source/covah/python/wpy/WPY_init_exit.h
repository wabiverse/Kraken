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
 * COVAH Python.
 * It Bites.
 */

#pragma once

#include "WPY_api.h"

#include "CKE_main.h"

/**
 *  -----  The Covah Python Module. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  Python Init & Exit. ----- */


COVAH_PYTHON_API
void WPY_python_init(cContext *C);

COVAH_PYTHON_API
void WPY_python_exit(void);


/* ------ */


WABI_NAMESPACE_END
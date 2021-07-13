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
 * KRAKEN Python.
 * It Bites.
 */

#include "KPY_api.h"

#include "kpy_interface.h"
#include "kpy_uni.h"

WABI_NAMESPACE_BEGIN

void KPY_context_set(kContext *C)
{
  kpy_context_module->ptr.data = (void *)C;
}

kContext *KPY_context_get(void)
{
  return (kContext *)kpy_context_module->ptr.data;
}

/* call KPY_context_set first */
void KPY_python_start(kContext *C, int argc, const char **argv)
{

}

WABI_NAMESPACE_END
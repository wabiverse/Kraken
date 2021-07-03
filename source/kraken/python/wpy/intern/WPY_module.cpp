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

#include "UNI_context.h"
#include "UNI_scene.h"

WABI_NAMESPACE_BEGIN

#if 0
PYBIND11_MODULE(wpy, m)
{
  m.attr("__name__") = "wpy";
  m.doc() = "Kraken python module";
}

PYBIND11_MODULE(context, m)
{
  m.attr("__name__") = "wpy.context";
  m.doc() = "Main 'context' instanced by a <filepath>, from which all data is derived";
}
#endif

WABI_NAMESPACE_END
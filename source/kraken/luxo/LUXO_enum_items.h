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
 * Luxo.
 * The Universe Gets Animated.
 */

/* NOTE: this is included multiple times with different #defines for DEF_ENUM. */

/* use in cases where only dynamic types are used */
DEF_ENUM(DummyPRIM_NULL_items)
DEF_ENUM(DummyPRIM_DEFAULT_items)

/* all others should follow 'prim_enum_*_items' naming */
DEF_ENUM(prim_enum_id_type_items)

DEF_ENUM(prim_enum_event_type_items)

DEF_ENUM(prim_enum_prop_type_items)

#undef DEF_ENUM

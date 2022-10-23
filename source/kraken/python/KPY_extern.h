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

#pragma once

/**
 * @file
 * KRAKEN Python.
 * It Bites.
 */

struct ARegionType;
struct AnimationEvalContext;
struct ChannelDriver;
struct ID;
struct ListBase;
struct Object;
struct PathResolvedRNA;
struct Text;
struct kConstraint;
struct kConstraintOb;
struct kConstraintTarget;
struct kContext;
struct kContextDataResult;
struct kPythonConstraint;
struct wmWindowManager;

#include "KLI_utildefines.h"

#ifdef __cplusplus
extern "C" {
#endif

void KPY_context_set(struct kContext *C);
/**
 * Use for updating while a python script runs - in case of file load.
 */
void KPY_context_update(struct kContext *C);


#define KPY_context_dict_clear_members(C, ...)                          \
  KPY_context_dict_clear_members_array(&((C)->data.py_context),         \
                                       (C)->data.py_context_orig,       \
                                       ((const char *[]){__VA_ARGS__}), \
                                       VA_NARGS_COUNT(__VA_ARGS__))
/**
 * Use for `CTX_*_set(..)` functions need to set values which are later read back as expected.
 * In this case we don't want the Python context to override the values as it causes problems
 * see T66256.
 *
 * \param dict_p: A pointer to #kContext.data.py_context so we can assign a new value.
 * \param dict_orig: The value of #kContext.data.py_context_orig to check if we need to copy.
 *
 * \note Typically accessed via #KPY_context_dict_clear_members macro.
 */
void KPY_context_dict_clear_members_array(void **dict_p,
                                          void *dict_orig,
                                          const char *context_members[],
                                          uint context_members_len);

#ifdef __cplusplus
}
#endif

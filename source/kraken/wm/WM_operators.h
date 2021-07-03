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
 * Window Manager.
 * Making GUI Fly.
 */

#pragma once

#include "WM_api.h"

#include "WM_init_exit.h"

#include "CKE_robinhood.h"

#include "UNI_object.h"
#include "UNI_operator.h"
#include "UNI_path_defaults.h"
#include "UNI_wm_types.h"

#include <wabi/base/tf/hash.h>
#include <wabi/usd/usd/attribute.h>
#include <wabi/usd/usd/prim.h>

WABI_NAMESPACE_BEGIN

enum
{
  OPERATOR_RUNNING_MODAL = (1 << 0),
  OPERATOR_CANCELLED = (1 << 1),
  OPERATOR_FINISHED = (1 << 2),
  OPERATOR_PASS_THROUGH = (1 << 3),
  OPERATOR_HANDLED = (1 << 4),
  OPERATOR_INTERFACE = (1 << 5),
};

struct wmOperatorType
{
  /** Text for UI, undo. */
  const char *name;
  /** Unique identifier. */
  TfToken idname;
  /** Use for tool-tips and Python docs. */
  const char *description;

  /** Signal changes, allow for Pub/Sub. */
  const TfNotice notice;

  /** Properties on this operator. */
  UsdAttributeVector uprops;

  eWmOperatorType flag;

  int (*exec)(cContext *C, wmOperator *op) ATTR_WARN_UNUSED_RESULT;

  int (*invoke)(cContext *C, wmOperator *op, wmEvent *event) ATTR_WARN_UNUSED_RESULT;

  bool (*poll)(cContext *C) ATTR_WARN_UNUSED_RESULT;
};

typedef robin_hood::unordered_map<TfToken, wmOperatorType *, TfHash> RHashOp;

void WM_operatortype_append(void (*opfunc)(wmOperatorType *));
void WM_operators_init(cContext *C);
void WM_operators_register(cContext *C);

void WM_operator_properties_create_ptr(PointerUNI *ptr, wmOperatorType *ot);
void WM_operator_properties_free(PointerUNI *ptr);

wmOperatorType *WM_operatortype_find(const TfToken &idname);

WABI_NAMESPACE_END
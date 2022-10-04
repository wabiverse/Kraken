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
 * Window Manager.
 * Making GUI Fly.
 */

#pragma once

#include "WM_api.h"

#include "WM_init_exit.h"

#include "KKE_robinhood.h"

#include "USD_wm_types.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_path_defaults.h"

#include <wabi/base/tf/hash.h>
#include <wabi/usd/usd/attribute.h>
#include <wabi/usd/usd/prim.h>



enum
{
  OPERATOR_RUNNING_MODAL = (1 << 0),
  OPERATOR_CANCELLED = (1 << 1),
  OPERATOR_FINISHED = (1 << 2),
  OPERATOR_PASS_THROUGH = (1 << 3),
  OPERATOR_HANDLED = (1 << 4),
  OPERATOR_INTERFACE = (1 << 5),
};

typedef struct wmOperatorType
{
  /** Text for UI, undo. */
  const char *name;
  /** Unique identifier. */
  TfToken idname;
  /** Use for tool-tips and Python docs. */
  const char *description;

  KrakenPRIM *prim;

  std::vector<wmOperatorTypeMacro *> macro;

  /** Signal changes, allow for Pub/Sub. */
  // const TfNotice notice;

  eWmOperatorType flag;

  /** Cursor to use when waiting for cursor input, see: #OPTYPE_DEPENDS_ON_CURSOR. */
  int cursor_pending;

  int (*exec)(kContext *C, wmOperator *op) ATTR_WARN_UNUSED_RESULT;

  int (*invoke)(kContext *C, wmOperator *op, wmEvent *event) ATTR_WARN_UNUSED_RESULT;

  bool (*poll)(kContext *C) ATTR_WARN_UNUSED_RESULT;
} wmOperatorType;

void WM_operatortype_append(void (*opfunc)(wmOperatorType *));
void WM_operators_init(kContext *C);
void WM_operators_register(kContext *C);

void WM_operator_properties_create_ptr(KrakenPRIM *ptr, wmOperatorType *ot);
void WM_operator_properties_free(KrakenPRIM *ptr);

wmOperatorType *WM_operatortype_find(const TfToken &idname);
bool WM_operator_properties_default(KrakenPRIM *ptr, bool do_update);
const char *WM_operatortype_name(struct wmOperatorType *ot, KrakenPRIM *properties);

size_t WM_operator_py_idname(char *dst, const char *src);
char *WM_operator_pystring_ex(kContext *C,
                              wmOperator *op,
                              const bool all_args,
                              const bool macro_args,
                              wmOperatorType *ot,
                              KrakenPRIM *opptr);


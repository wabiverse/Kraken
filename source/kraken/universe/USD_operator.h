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
 * Universe.
 * Set the Stage.
 */

#include "USD_context.h"
#include "USD_object.h"

#include <wabi/usd/usd/attribute.h>

WABI_NAMESPACE_BEGIN

struct wmOperator
{
  /* saved */
  /** Used to retrieve type pointer. */
  TfToken idname;
  /** Saved, user-settable properties. */
  KrakenPRIM *properties;

  /* runtime */
  /** Operator type definition from idname. */
  struct wmOperatorType *type;
  /** Custom storage, only while operator runs. */
  void *customdata;

  /** Errors and warnings storage. */
  struct ReportList *reports;

  /** List of operators. */
  std::vector<wmOperator> macro;
  /** Current running macro, not saved. */
  struct wmOperator *opm;
  /** Runtime for drawing. */
  struct uiLayout *layout;
  short flag;
  char _pad[6];
};

WABI_NAMESPACE_END
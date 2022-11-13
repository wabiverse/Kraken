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

#include <stdlib.h>

#include <CLG_log.h>

#include "USD_ID.h"

#include "KLI_utildefines.h"

#include "LUXO_access.h"
#include "LUXO_define.h"
#include "LUXO_enum_types.h"
#include "LUXO_types.h"

#include "luxo_internal.h"

#include <wabi/base/tf/token.h>

/* -------------------------------------------------------------------- */
/** @name Generic Enum's
 * @{ */

/* Reuse for dynamic types. */
const EnumPROP DummyPRIM_NULL_items[] = {
  {0, wabi::TfToken(), 0, NULL, NULL},
};

/* Reuse for dynamic types with default value */
const EnumPROP DummyPRIM_DEFAULT_items[] = {
  {0, wabi::TfToken("DEFAULT"), 0, "Default", ""  },
  {0, wabi::TfToken(),          0, NULL,      NULL},
};

/** @} */

/* -------------------------------------------------------------------- */
/** @name RNA Enum's
 * @{ */

const EnumPROP prim_enum_prop_type_items[] = {
  {PROP_BOOLEAN,    wabi::TfToken("BOOLEAN"),    0, "Boolean",     ""  },
  {PROP_INT,        wabi::TfToken("INT"),        0, "Integer",     ""  },
  {PROP_FLOAT,      wabi::TfToken("FLOAT"),      0, "Float",       ""  },
  {PROP_STRING,     wabi::TfToken("STRING"),     0, "String",      ""  },
  {PROP_ENUM,       wabi::TfToken("ENUM"),       0, "Enumeration", ""  },
  {PROP_POINTER,    wabi::TfToken("POINTER"),    0, "Pointer",     ""  },
  {PROP_COLLECTION, wabi::TfToken("COLLECTION"), 0, "Collection",  ""  },
  {0,               wabi::TfToken(),             0, NULL,          NULL},
};

/** @} */

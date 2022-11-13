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

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "KLI_utildefines.h"

#include "atomic_ops.h"
#include "MEM_guardedalloc.h"

#include "USD_api.h"
#include "USD_types.h"
#include "USD_object.h"

#include "KKE_context.h"
#include "KKE_utils.h"
#include "KLI_listbase.h"

#include "UI_resources.h"
#include "UI_interface.h"

#include "LUXO_define.h"

#include "LUXO_internal.h"

#include <wabi/base/tf/token.h>

#include "CLG_log.h"

static CLG_LogRef LOG = {"luxo.define"};

using std::string;

WABI_NAMESPACE_USING


KrakenPRIM *PRIM_def_struct_ptr(const KrakenSTAGE &kstage,
                                const wabi::SdfPath &identifier,
                                KrakenPRIM *primfrom)
{
  KrakenPRIM *kprim;
  // KrakenPROP *prop;

  kprim = MEM_new<KrakenPRIM>(K_BEDROCK.GetText(), kstage->GetPrimAtPath(K_BEDROCK));

  if (primfrom && primfrom->IsValid()) {
    /**
     * Ensure if a type, such as "Operators" is passed, we deduce
     * down to the actual type, "Operator". This is a lame check
     * and is only meant to be done with Kraken stage data and
     * not on user stage data. */
    const std::string type_maybe_plural = primfrom->GetName().GetString();
    size_t final_s = type_maybe_plural.find_last_of("s");

    std::string type_no_plural;
    if (final_s != std::string::npos) {
      type_no_plural = type_maybe_plural.substr(0, final_s);
    } else {
      type_no_plural = type_maybe_plural;
    }

    const TfToken type = TfToken(type_no_plural);
    const TfToken type_ctx = primfrom->GetName();

    const SdfPath path = kprim->GetPath();
    const SdfPath path_ctx = path.AppendPath(SdfPath(type_ctx)).AppendPath(identifier);

    /* Get or create prim at ctx. */
    kprim = MEM_new<KrakenPRIM>(path_ctx.GetText(), kstage->DefinePrim(path_ctx, type));
    kprim->py_type = NULL;
    kprim->base = primfrom;
  } else {
    const SdfPath path_ctx = SdfPath(identifier.GetString());

    /**
     * Handle case in which we just want a pointer to
     * top level Kraken prim hierarchy, which is already
     * set above, nothing to do.
     */
    if (!identifier.GetString().contains("Struct")) {
      kprim = MEM_new<KrakenPRIM>(K_BEDROCK.AppendPath(path_ctx).GetText(),
                                  kstage->DefinePrim(K_BEDROCK.AppendPath(path_ctx)));
    }

    kprim->py_type = NULL;
    kprim->base = MEM_new<KrakenPRIM>(__func__, kprim->GetParent());
  }

  kprim->identifier = kprim->GetName();
  kprim->ClearDocumentation();

  if (!primfrom || !primfrom->IsValid()) {
    kprim->icon = ICON_DOT;
    kprim->flag |= STRUCT_UNDO;
  }

  return kprim;
}

KrakenPRIM *PRIM_def_struct(KrakenPRIM *kprim,
                            const wabi::SdfPath &identifier,
                            const wabi::TfToken &from)
{
  KrakenPRIM *prim;
  UsdStageWeakPtr stage;
  SdfPath path_ctx;

  /**
   * -- *** Pixar Style *** --
   * find prim to derive from (optional) */
  stage = kprim->GetStage();
  path_ctx = kprim->GetPath().AppendPath(identifier);

  prim = MEM_new<KrakenPRIM>(path_ctx.GetText(), stage->DefinePrim(path_ctx, from));

  if (prim == nullptr || !prim->IsValid()) {
    if (from.IsEmpty()) {
      CLOG_ERROR(&LOG, "prim %s could not be created.", identifier.GetText());
    } else {
      CLOG_ERROR(&LOG, "prim %s not found to define %s.", from.GetText(), identifier.GetText());
    }

    /* construct invalid prim on error. */
    prim = MEM_new<KrakenPRIM>(identifier.GetText(), UsdPrim());
  }

  return prim;
}

void PRIM_def_struct_identifier(const KrakenSTAGE &kstage,
                                KrakenPRIM *prim,
                                const TfToken &identifier)
{
  if (identifier != prim->identifier) {
    prim = PRIM_def_struct_ptr(kstage, SdfPath(identifier), prim);
  }

  prim->identifier = identifier;
}

void PRIM_def_struct_ui_text(KrakenPRIM *prim,
                             const std::string &ui_name,
                             const std::string &ui_description)
{
  if (ui_name.length()) {
    wabi::UsdAttribute name = prim->CreateAttribute(wabi::TfToken("ui:name"),
                                                    wabi::SdfValueTypeNames->Token,
                                                    wabi::SdfVariability::SdfVariabilityUniform);
    name.Set(wabi::TfToken(ui_name));
  }

  if (ui_description.length()) {
    prim->SetDocumentation(ui_description);
  }
}

void PRIM_def_boolean(KrakenPRIM *prim,
                      const std::string &identifier,
                      bool default_value,
                      const std::string &ui_name,
                      const std::string &ui_description)
{
  wabi::UsdAttribute attr = prim->CreateAttribute(wabi::TfToken(identifier),
                                                  wabi::SdfValueTypeNames->Bool,
                                                  wabi::SdfVariability::SdfVariabilityVarying);
  attr.Set(default_value);

  if (ui_name.length()) {
    attr.SetDisplayName(ui_name);
  }

  if (ui_description.length()) {
    attr.SetDocumentation(ui_description);
  }
}

void PRIM_def_asset(KrakenPRIM *prim,
                    const std::string &identifier,
                    const std::string &default_value,
                    const std::string &ui_name,
                    const std::string &ui_description)
{
  wabi::UsdAttribute attr = prim->CreateAttribute(wabi::TfToken(identifier),
                                                  wabi::SdfValueTypeNames->Asset,
                                                  wabi::SdfVariability::SdfVariabilityVarying);
  attr.Set(wabi::SdfAssetPath(default_value));

  if (ui_name.length()) {
    attr.SetDisplayName(ui_name);
  }

  if (ui_description.length()) {
    attr.SetDocumentation(ui_description);
  }
}

void PRIM_def_py_data(KrakenPROP *prop, void *py_data)
{
  prop->py_data = py_data;
}

static void (*g_py_data_clear_fn)(KrakenPROP *prop) = NULL;

/**
 * Set the callback used to decrement the user count of a property.
 *
 * This function is called when freeing each dynamically defined property.
 */
void PRIM_def_property_free_pointers_set_py_data_callback(
  void (*py_data_clear_fn)(KrakenPROP *prop))
{
  g_py_data_clear_fn = py_data_clear_fn;
}

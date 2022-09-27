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

#include "KLI_utildefines.h"
#include "MEM_guardedalloc.h"

#include "USD_api.h"
#include "USD_object.h"
#include "USD_types.h"

#include "KKE_utils.h"
#include "KLI_listbase.h"

#include "UI_resources.h"
#include "UI_interface.h"

#include "LUXO_define.h"

#include "LUXO_internal.h"

#include <wabi/base/tf/token.h>

using std::string;

WABI_NAMESPACE_USING

KRAKEN_NAMESPACE_BEGIN

KrakenPRIM *PRIM_def_struct_ptr(const KrakenSTAGE &kstage,
                                const wabi::SdfPath &identifier,
                                KrakenPRIM *primfrom)
{
  KrakenPRIM *kprim;
  KrakenPROP *prop;

  kprim = &KrakenPRIM(kstage->GetPrimAtPath(K_BEDROCK));

  if (primfrom && primfrom->IsValid()) {
    const TfToken type = primfrom->GetName();
    const TfToken type_ctx = TfToken(type.GetString() + "s");

    const SdfPath path = kprim->GetPath();
    const SdfPath path_ctx = path.AppendPath(SdfPath(type_ctx)).AppendPath(identifier);

    /* Get or create prim at ctx. */
    kprim = &KrakenPRIM(kstage->DefinePrim(path_ctx, type));
    kprim->py_type = NULL;
    kprim->base = primfrom;
  } else {
    const SdfPath path_ctx = SdfPath(identifier.GetString());

    kprim = &KrakenPRIM(kstage->DefinePrim(K_BEDROCK.AppendPath(path_ctx)));
    kprim->py_type = NULL;
    kprim->base = kprim;
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
  KrakenSTAGE stage;
  SdfPath path_ctx;

  /**
   * -- *** Pixar Style *** --
   * find struct to derive from (optional) */
  stage = KrakenSTAGE(kprim->GetStage());
  path_ctx = kprim->GetPath().AppendPath(identifier);
  prim = &KrakenPRIM(stage->DefinePrim(path_ctx, from));

  if (!prim->IsValid()) {
    wabi::TF_WARN("prim %s could not be created.", identifier.GetText());
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

KRAKEN_NAMESPACE_END
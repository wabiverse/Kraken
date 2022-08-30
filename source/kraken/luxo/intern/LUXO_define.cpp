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

#include "LUXO_runtime.h"

#include "KLI_icons.h"
#include "KLI_string_utils.h"
#include "KLI_path_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_screen.h"

#include "USD_api.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_default_tables.h"
#include "USD_factory.h"
#include "USD_file.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_types.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"

#include "LUXO_access.h"
#include "LUXO_define.h"

#include <wabi/base/tf/token.h>

using std::string;

WABI_NAMESPACE_USING

KRAKEN_NAMESPACE_BEGIN

void PRIM_def_struct_ptr(const Stage &kstage,
                         const wabi::SdfPath &identifier,
                         KrakenPRIM *r_ptr,
                         const wabi::TfToken &from)
{
  // KrakenPRIM kprim;
  // KrakenPROP prop;
  // KrakenPRIMDEF *kpdef = NULL, *kpdefrom = NULL;

  *r_ptr = KrakenPRIM(kstage->GetPrimAtPath(wabi::SdfPath("/WabiAnimationStudios/Structs").AppendPath(identifier)));

  if (!r_ptr->IsValid()) {
    /* This prim doesn't exist yet. */
    *r_ptr = KrakenPRIM(kstage->DefinePrim(wabi::SdfPath("/WabiAnimationStudios/Structs").AppendPath(identifier), from));
  }

  r_ptr->identifier = identifier.GetAsString().c_str();
  // kprim.SetDocumentation(kprim.description);
}

void PRIM_def_struct(const Stage &kstage,
                     const wabi::SdfPath &identifier,
                     KrakenPRIM *r_ptr,
                     const wabi::TfToken &from)
{
  /* only use PRIM_def_struct() while pre-processing, otherwise use PRIM_def_struct_ptr() */
  // KLI_assert(DefPRIM.preprocess);

  /**
   * -- *** Pixar Style *** --
   * find struct to derive from (optional) */
  *r_ptr = KrakenPRIM(kstage->DefinePrim(wabi::SdfPath("/WabiAnimationStudios/Structs").AppendPath(identifier), from));

  if (!r_ptr->IsValid()) {

    wabi::TF_WARN("struct %s could not be created.", identifier.GetText());
    // DefPRIM.error = true;
  }

  PRIM_def_struct_ptr(kstage, identifier, r_ptr, from);
}

void PRIM_def_begin(wabi::UsdPrim &prim, 
                    const wabi::TfToken &identifier,
                    const wabi::TfToken &type,
                    const std::string &ui_name,
                    const std::string &ui_description)
{
  prim = LUXO_get_stage()->DefinePrim(wabi::SdfPath("/WabiAnimationStudios/Structs").AppendPath(wabi::SdfPath(type)).AppendPath(wabi::SdfPath(identifier)), type);
  if (ui_name.length()) {
    wabi::UsdAttribute name = prim.CreateAttribute(wabi::TfToken("ui:operator:name"), wabi::SdfValueTypeNames->Token, wabi::SdfVariability::SdfVariabilityUniform);
    name.Set(wabi::TfToken(ui_name));
  }

  if (ui_description.length()) {
    prim.SetDocumentation(ui_description);
  }
}

void PRIM_def_boolean(wabi::UsdPrim &prim, 
                      const std::string &identifier, 
                      bool default_value, 
                      const std::string &ui_name, 
                      const std::string &ui_description)
{ 
  wabi::UsdAttribute attr = prim.CreateAttribute(wabi::TfToken(identifier), wabi::SdfValueTypeNames->Bool, wabi::SdfVariability::SdfVariabilityVarying);
  attr.Set(default_value);

  if (ui_name.length()) {
    attr.SetDisplayName(ui_name);
  }

  if (ui_description.length()) {
    attr.SetDocumentation(ui_description);
  }
}

void PRIM_def_asset(wabi::UsdPrim &prim, 
                    const std::string &identifier, 
                    const std::string &default_value, 
                    const std::string &ui_name, 
                    const std::string &ui_description)
{ 
  wabi::UsdAttribute attr = prim.CreateAttribute(wabi::TfToken(identifier), wabi::SdfValueTypeNames->Asset, wabi::SdfVariability::SdfVariabilityVarying);
  attr.Set(wabi::SdfAssetPath(default_value));

  if (ui_name.length()) {
    attr.SetDisplayName(ui_name);
  }

  if (ui_description.length()) {
    attr.SetDocumentation(ui_description);
  }
}

KRAKEN_NAMESPACE_END
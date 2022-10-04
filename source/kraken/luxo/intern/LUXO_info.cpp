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

#include "kraken/kraken.h"

#include "KKE_global.h"

#include "LUXO_main.h"
#include "LUXO_internal.h"
#include "LUXO_access.h"

#include <wabi/usd/usdGeom/tokens.h>
#include <wabi/imaging/hdx/tokens.h>



/* not technically a prim, but meh. */
static void prim_def_config(const KrakenSTAGE &kstage)
{
  kstage->GetRootLayer()->SetDocumentation(KRAKEN_FILE_VERSION_HEADER);
  kstage->SetColorConfiguration(wabi::SdfAssetPath(G.main->ocio_cfg));
  kstage->SetColorManagementSystem(wabi::HdxColorCorrectionTokens->openColorIO);
  kstage->SetMetadata(wabi::UsdGeomTokens->upAxis, wabi::UsdGeomTokens->z);
}

void PRIM_def_info(const KrakenSTAGE &kstage)
{
  prim_def_config(kstage);
}



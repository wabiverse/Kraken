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
 * Universe.
 * Set the Stage.
 */

#include "UNI_diagnostics.h"

#include <wabi/wabi.h>

#include <wabi/base/plug/debugCodes.h>
#include <wabi/base/tf/debugCodes.h>

#include <wabi/usd/ar/debugCodes.h>
#include <wabi/usd/ndr/debugCodes.h>
#include <wabi/usd/pcp/debugCodes.h>
#include <wabi/usd/sdf/debugCodes.h>
#include <wabi/usd/usd/debugCodes.h>
#include <wabi/usd/usdGeom/debugCodes.h>
#include <wabi/usd/usdSkel/debugCodes.h>
#include <wabi/usd/usdUtils/debugCodes.h>

#include <wabi/usd/plugin/usdMtlx/debugCodes.h>

#include <wabi/imaging/glf/debugCodes.h>
#include <wabi/imaging/hd/debugCodes.h>
#include <wabi/imaging/hdPh/debugCodes.h>
#include <wabi/imaging/hdx/debugCodes.h>
#include <wabi/imaging/hio/debugCodes.h>

#include <wabi/imaging/plugin/hioOpenVDB/debugCodes.h>

#include <wabi/usdImaging/usdImaging/debugCodes.h>
#include <wabi/usdImaging/usdviewq/debugCodes.h>

WABI_NAMESPACE_BEGIN

void UNI_enable_all_debug_codes()
{
  /** Asset Resolver Debug Codes. */
  TfDebug::Enable(AR_RESOLVER_INIT);

  /** Tools Foundations Debug Codes. */
  TfDebug::Enable(TF_DISCOVERY_TERSE);
  TfDebug::Enable(TF_DISCOVERY_DETAILED);
  TfDebug::Enable(TF_DEBUG_REGISTRY);
  TfDebug::Enable(TF_DLOPEN);
  TfDebug::Enable(TF_DLCLOSE);
  TfDebug::Enable(TF_SCRIPT_MODULE_LOADER);
  TfDebug::Enable(TF_TYPE_REGISTRY);
  TfDebug::Enable(TF_ATTACH_DEBUGGER_ON_ERROR);
  TfDebug::Enable(TF_ATTACH_DEBUGGER_ON_FATAL_ERROR);
  TfDebug::Enable(TF_ATTACH_DEBUGGER_ON_WARNING);

  /** Plugin Debug Codes. */
  TfDebug::Enable(PLUG_LOAD);
  TfDebug::Enable(PLUG_REGISTRATION);
  TfDebug::Enable(PLUG_LOAD_IN_SECONDARY_THREAD);
  TfDebug::Enable(PLUG_INFO_SEARCH);

  /** Node Definition Registry Debug Codes. */
  TfDebug::Enable(NDR_DISCOVERY);
  TfDebug::Enable(NDR_PARSING);
  TfDebug::Enable(NDR_INFO);
  TfDebug::Enable(NDR_STATS);
  TfDebug::Enable(NDR_DEBUG);

  /** Prim Cache Population (Composition) Codes. */
  TfDebug::Enable(PCP_CHANGES);
  TfDebug::Enable(PCP_DEPENDENCIES);
  TfDebug::Enable(PCP_PRIM_INDEX);
  TfDebug::Enable(PCP_PRIM_INDEX_GRAPHS);
  TfDebug::Enable(PCP_NAMESPACE_EDIT);

  /** Scene Description Foundations Debug Codes. */
  TfDebug::Enable(SDF_LAYER);
  TfDebug::Enable(SDF_CHANGES);
  TfDebug::Enable(SDF_ASSET);
  TfDebug::Enable(SDF_ASSET_TRACE_INVALID_CONTEXT);
  TfDebug::Enable(SDF_FILE_FORMAT);

  /** Universal Scene Description Debug Codes. */
  TfDebug::Enable(USD_AUTO_APPLY_API_SCHEMAS);
  TfDebug::Enable(USD_CHANGES);
  TfDebug::Enable(USD_CLIPS);
  TfDebug::Enable(USD_COMPOSITION);
  TfDebug::Enable(USD_DATA_BD);
  TfDebug::Enable(USD_DATA_BD_TRY);
  TfDebug::Enable(USD_INSTANCING);
  TfDebug::Enable(USD_PATH_RESOLUTION);
  TfDebug::Enable(USD_PAYLOADS);
  TfDebug::Enable(USD_PRIM_LIFETIMES);
  TfDebug::Enable(USD_SCHEMA_REGISTRATION);
  TfDebug::Enable(USD_STAGE_CACHE);
  TfDebug::Enable(USD_STAGE_LIFETIMES);
  TfDebug::Enable(USD_STAGE_OPEN);
  TfDebug::Enable(USD_STAGE_INSTANTIATION_TIME);
  TfDebug::Enable(USD_VALUE_RESOLUTION);
  TfDebug::Enable(USD_VALIDATE_VARIABILITY);

  /** Universal Scene Description Utilities Debug Codes. */
  TfDebug::Enable(USDUTILS_CREATE_USDZ_PACKAGE);

  /** Universal Scene Description Geometry Debug Codes. */
  TfDebug::Enable(USDGEOM_BBOX);
  TfDebug::Enable(USDGEOM_XFORMCOMMONAPI);

  /** Universal Scene Description Skeleton Debug Codes. */
  TfDebug::Enable(USDSKEL_CACHE);
  TfDebug::Enable(USDSKEL_BAKESKINNING);

  /** MaterialX Debug Codes. */
  TfDebug::Enable(USDMTLX_READER);

  /** Imaging Debug Codes. */
  TfDebug::Enable(USDIMAGING_COLLECTIONS);
  TfDebug::Enable(USDIMAGING_COORDSYS);
  TfDebug::Enable(USDIMAGING_CHANGES);
  TfDebug::Enable(USDIMAGING_COMPUTATIONS);
  TfDebug::Enable(USDIMAGING_INSTANCER);
  TfDebug::Enable(USDIMAGING_PLUGINS);
  TfDebug::Enable(USDIMAGING_POINT_INSTANCER_PROTO_CREATED);
  TfDebug::Enable(USDIMAGING_POINT_INSTANCER_PROTO_CULLING);
  TfDebug::Enable(USDIMAGING_SELECTION);
  TfDebug::Enable(USDIMAGING_SHADERS);
  TfDebug::Enable(USDIMAGING_TEXTURES);
  TfDebug::Enable(USDIMAGING_UPDATES);

  /** Hydra Debug Codes. */
  TfDebug::Enable(HD_BPRIM_ADDED);
  TfDebug::Enable(HD_BPRIM_REMOVED);
  TfDebug::Enable(HD_BUFFER_ARRAY_INFO);
  TfDebug::Enable(HD_BUFFER_ARRAY_RANGE_CLEANED);
  TfDebug::Enable(HD_CACHE_HITS);
  TfDebug::Enable(HD_CACHE_MISSES);
  TfDebug::Enable(HD_COUNTER_CHANGED);
  TfDebug::Enable(HD_DIRTY_ALL_COLLECTIONS);
  TfDebug::Enable(HD_DIRTY_LIST);
  TfDebug::Enable(HD_DISABLE_MULTITHREADED_RPRIM_SYNC);
  TfDebug::Enable(HD_DRAWITEMS_CULLED);
  TfDebug::Enable(HD_ENGINE_PHASE_INFO);
  TfDebug::Enable(HD_EXT_COMPUTATION_ADDED);
  TfDebug::Enable(HD_EXT_COMPUTATION_REMOVED);
  TfDebug::Enable(HD_EXT_COMPUTATION_UPDATED);
  TfDebug::Enable(HD_EXT_COMPUTATION_EXECUTION);
  TfDebug::Enable(HD_FREEZE_CULL_FRUSTUM);
  TfDebug::Enable(HD_INSTANCER_ADDED);
  TfDebug::Enable(HD_INSTANCER_CLEANED);
  TfDebug::Enable(HD_INSTANCER_REMOVED);
  TfDebug::Enable(HD_INSTANCER_UPDATED);
  TfDebug::Enable(HD_MDI);
  TfDebug::Enable(HD_RENDER_SETTINGS);
  TfDebug::Enable(HD_RPRIM_ADDED);
  TfDebug::Enable(HD_RPRIM_CLEANED);
  TfDebug::Enable(HD_RPRIM_REMOVED);
  TfDebug::Enable(HD_RPRIM_UPDATED);
  TfDebug::Enable(HD_SAFE_MODE);
  TfDebug::Enable(HD_SELECTION_UPDATE);
  TfDebug::Enable(HD_SHARED_EXT_COMPUTATION_DATA);
  TfDebug::Enable(HD_SPRIM_ADDED);
  TfDebug::Enable(HD_SPRIM_REMOVED);
  TfDebug::Enable(HD_TASK_ADDED);
  TfDebug::Enable(HD_TASK_REMOVED);
  TfDebug::Enable(HD_VARYING_STATE);

  /** Hydra Extensions Debug Codes. */
  TfDebug::Enable(HDX_DISABLE_ALPHA_TO_COVERAGE);
  TfDebug::Enable(HDX_INTERSECT);
  TfDebug::Enable(HDX_SELECTION_SETUP);

  /** Hydra Resource I/O Debug Codes. */
  TfDebug::Enable(HIO_DEBUG_GLSLFX);
  TfDebug::Enable(HIO_DEBUG_TEXTURE_IMAGE_PLUGINS);
  TfDebug::Enable(HIO_DEBUG_FIELD_TEXTURE_DATA_PLUGINS);

  /** OpenVDB Hydra Resource I/O Debug Codes. */
  TfDebug::Enable(HIOOPENVDB_DEBUG_TEXTURE);

  /** Phoenix Render Engine Debug Codes. */
  TfDebug::Enable(HDPH_DRAW_BATCH);
  TfDebug::Enable(HDPH_FORCE_DRAW_BATCH_REBUILD);
  TfDebug::Enable(HDPH_DRAW_ITEM_GATHER);
  TfDebug::Enable(HDPH_DISABLE_FRUSTUM_CULLING);
  TfDebug::Enable(HDPH_DISABLE_MULTITHREADED_CULLING);
  TfDebug::Enable(HDPH_DUMP_GLSLFX_CONFIG);
  TfDebug::Enable(HDPH_DUMP_FAILING_SHADER_SOURCE);
  TfDebug::Enable(HDPH_DUMP_FAILING_SHADER_SOURCEFILE);
  TfDebug::Enable(HDPH_DUMP_SHADER_SOURCE);
  TfDebug::Enable(HDPH_DUMP_SHADER_SOURCEFILE);
  TfDebug::Enable(HDPH_MATERIAL_ADDED);
  TfDebug::Enable(HDPH_MATERIAL_REMOVED);

  /** OpenGL Foundations Debug Codes. */
  TfDebug::Enable(GLF_DEBUG_CONTEXT_CAPS);
  TfDebug::Enable(GLF_DEBUG_ERROR_STACKTRACE);
  TfDebug::Enable(GLF_DEBUG_SHADOW_TEXTURES);
  TfDebug::Enable(GLF_DEBUG_DUMP_SHADOW_TEXTURES);
  TfDebug::Enable(GLF_DEBUG_POST_SURFACE_LIGHTING);

  /** USDView Qt GUI elements and python application Debug Codes. */
  TfDebug::Enable(USDVIEWQ_DEBUG_CLIPPING);
}

WABI_NAMESPACE_END
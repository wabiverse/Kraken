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
#include "UNI_api.h"
#include "UNI_context.h"
#include "UNI_scene.h"

#include <wabi/wabi.h>

#include <wabi/base/plug/registry.h>
#include <wabi/base/tf/debug.h>
#include <wabi/base/trace/trace.h>

#include <wabi/usd/sdf/schema.h>
#include <wabi/usd/usd/primRange.h>
#include <wabi/usd/usd/stage.h>

#include <wabi/usd/usdUI/backdrop.h>
#include <wabi/usd/usdUI/window.h>

#include <wabi/usd/usdGeom/capsule.h>
#include <wabi/usd/usdGeom/cone.h>
#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdGeom/curves.h>
#include <wabi/usd/usdGeom/cylinder.h>
#include <wabi/usd/usdGeom/gprim.h>
#include <wabi/usd/usdGeom/imageable.h>
#include <wabi/usd/usdGeom/mesh.h>
#include <wabi/usd/usdGeom/modelAPI.h>
#include <wabi/usd/usdGeom/motionAPI.h>
#include <wabi/usd/usdGeom/nurbsCurves.h>
#include <wabi/usd/usdGeom/pointBased.h>
#include <wabi/usd/usdGeom/primvarsAPI.h>
#include <wabi/usd/usdGeom/sphere.h>
#include <wabi/usd/usdGeom/xform.h>
#include <wabi/usd/usdGeom/xformCommonAPI.h>
#include <wabi/usd/usdGeom/xformOp.h>
#include <wabi/usd/usdGeom/xformable.h>

#include <wabi/usd/usdLux/light.h>
#include <wabi/usd/usdLux/listAPI.h>
#include <wabi/usd/usdLux/shadowAPI.h>
#include <wabi/usd/usdLux/shapingAPI.h>

#include <wabi/usd/usdGeom/camera.h>

#include <wabi/usd/usdShade/connectableAPI.h>
#include <wabi/usd/usdShade/connectableAPIBehavior.h>
#include <wabi/usd/usdShade/coordSysAPI.h>
#include <wabi/usd/usdShade/material.h>
#include <wabi/usd/usdShade/materialBindingAPI.h>
#include <wabi/usd/usdShade/nodeDefAPI.h>

#include <wabi/usd/usdSkel/bindingAPI.h>
#include <wabi/usd/usdSkel/skeleton.h>

#include <wabi/usd/usdVol/volume.h>

#include <wabi/usd/usdMedia/spatialAudio.h>

#include <wabi/usd/usdRender/settings.h>
#include <wabi/usd/usdRender/settingsAPI.h>

#include <wabi/imaging/garch/glApi.h>
#include <wabi/imaging/glf/contextCaps.h>
#include <wabi/imaging/glf/diagnostic.h>
#include <wabi/imaging/glf/glContext.h>

#include <wabi/usdImaging/usdImagingGL/engine.h>

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

WABI_NAMESPACE_USING

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

static void plugin_diagnostics()
{
  printf("::: PLUGIN SYSTEM :::\n");
  const std::string pixar_plugs = TfStringCatPaths(UNI.system.paths.datafiles_path, "covahverse");
  const std::string other_plugs = TfStringCatPaths(UNI.system.paths.datafiles_path, "plugin/covahverse");
  PlugPluginPtrVector pixars = PlugRegistry::GetInstance().RegisterPlugins(pixar_plugs);
  PlugPluginPtrVector others = PlugRegistry::GetInstance().RegisterPlugins(other_plugs);
}

static void sdf_diagnostics()
{
  printf("::: SCENE DESCRIPTION FOUNDATIONS :::\n");
  const SdfSchema &schema = SdfSchema::GetInstance();

  printf("Registered Attribute Specifications:\n");
  TfTokenVector attrs = schema.GetFields(SdfSpecType::SdfSpecTypeAttribute);
  TfTokenVector::iterator attrs_it = attrs.begin();
  while (attrs_it != attrs.end()) {
    printf(":: %s\n", attrs_it->GetText());
    attrs_it++;
  }
  printf("\n");

  printf("Registered Connection Specifications:\n");
  TfTokenVector connections = schema.GetFields(SdfSpecType::SdfSpecTypeConnection);
  TfTokenVector::iterator connections_it = connections.begin();
  while (connections_it != connections.end()) {
    printf(":: %s\n", connections_it->GetText());
    connections_it++;
  }
  printf("\n");

#ifdef PIXAR_HOLDS_BROKEN_FEATURESET
  /**
   * Accessing the following schema fields results in a crash, verify why
   * this is happening, no sane public facing API should provide calls out
   * to non-implemented design.
   *
   *  :: Either implement completeley -- or remove dead code. */

  printf("Registered Expression Specifications:\n");
  TfTokenVector expressions = schema.GetFields(SdfSpecType::SdfSpecTypeExpression);
  TfTokenVector::iterator expressions_it = expressions.begin();
  while (expressions_it != expressions.end()) {
    printf(":: %s\n", expressions_it->GetText());
    expressions_it++;
  }
  printf("\n");

  printf("Registered Mapper Specifications:\n");
  TfTokenVector mappers = schema.GetFields(SdfSpecType::SdfSpecTypeMapper);
  TfTokenVector::iterator mappers_it = mappers.begin();
  while (mappers_it != mappers.end()) {
    printf(":: %s\n", mappers_it->GetText());
    mappers_it++;
  }
  printf("\n");

  printf("Registered Mapper Argument Specifications:\n");
  TfTokenVector mapargs = schema.GetFields(SdfSpecType::SdfSpecTypeMapperArg);
  TfTokenVector::iterator mapargs_it = mapargs.begin();
  while (mapargs_it != mapargs.end()) {
    printf(":: %s\n", mapargs_it->GetText());
    mapargs_it++;
  }
  printf("\n");
#endif /* PIXAR_HOLDS_BROKEN_FEATURESET */

  printf("Registered Prim Specifications:\n");
  TfTokenVector prims = schema.GetFields(SdfSpecType::SdfSpecTypePrim);
  TfTokenVector::iterator prims_it = prims.begin();
  while (prims_it != prims.end()) {
    printf(":: %s\n", prims_it->GetText());
    prims_it++;
  }
  printf("\n");

  printf("Registered Pseuedo Root Specifications:\n");
  TfTokenVector proots = schema.GetFields(SdfSpecType::SdfSpecTypePseudoRoot);
  TfTokenVector::iterator proots_it = proots.begin();
  while (proots_it != proots.end()) {
    printf(":: %s\n", proots_it->GetText());
    proots_it++;
  }
  printf("\n");

  printf("Registered Relationship Specifications:\n");
  TfTokenVector rels = schema.GetFields(SdfSpecType::SdfSpecTypeRelationship);
  TfTokenVector::iterator rels_it = rels.begin();
  while (rels_it != rels.end()) {
    printf(":: %s\n", rels_it->GetText());
    rels_it++;
  }
  printf("\n");

  printf("Registered Relationship Target Specifications:\n");
  TfTokenVector reltargs = schema.GetFields(SdfSpecType::SdfSpecTypeRelationshipTarget);
  TfTokenVector::iterator reltargs_it = reltargs.begin();
  while (reltargs_it != reltargs.end()) {
    printf(":: %s\n", reltargs_it->GetText());
    reltargs_it++;
  }
  printf("\n");

  printf("Registered Variant Specifications:\n");
  TfTokenVector variants = schema.GetFields(SdfSpecType::SdfSpecTypeVariant);
  TfTokenVector::iterator variants_it = variants.begin();
  while (variants_it != variants.end()) {
    printf(":: %s\n", variants_it->GetText());
    variants_it++;
  }
  printf("\n");

  printf("Registered Variant Set Specifications:\n");
  TfTokenVector variantsets = schema.GetFields(SdfSpecType::SdfSpecTypeVariantSet);
  TfTokenVector::iterator variantsets_it = variantsets.begin();
  while (variantsets_it != variantsets.end()) {
    printf(":: %s\n", variantsets_it->GetText());
    variantsets_it++;
  }
  printf("\n");

  typedef typename std::vector<SdfValueTypeName> SdfValueTypeNameVector;
  printf("Registered Schema Type Names:\n");
  SdfValueTypeNameVector schema_types = schema.GetAllTypes();
  SdfValueTypeNameVector::iterator schema_types_it = schema_types.begin();
  while (schema_types_it != schema_types.end()) {
    printf(":: %s\n", schema_types_it->GetAsToken().GetText());
    schema_types_it++;
  }
  printf("\n");
}

static void gui_diagnostics()
{
  printf("::: GUI COMPONENTS :::\n");
  UsdUIBackdrop backdrop = UsdUIBackdrop::Define(UNI.stage, SdfPath("/Backdrop"));

  UsdUIWindow window = UsdUIWindow::Define(UNI.stage, SdfPath("/Window"));
  window.CreateWindowTitleAttr(VtValue(TfToken("Covah")));

  printf("\n");
}

static void geometry_diagnostics()
{
  printf("::: GEOMETRY COMPONENTS :::\n");

  /** AUTHOR A CUBE */
  UsdGeomCube cube = UsdGeomCube::Define(UNI.stage, SdfPath("/Cube"));
  cube.CreateVisibilityAttr(VtValue(UsdGeomTokens->inherited));
  cube.CreateDisplayColorAttr(VtValue(VtVec3fArray({GfVec3f(1.0f, 1.0f, 1.0f)})));
  cube.CreateDisplayOpacityAttr(VtValue(VtFloatArray({1.0f})));
  cube.CreateDoubleSidedAttr(VtValue(true));
  cube.CreateOrientationAttr(VtValue(UsdGeomTokens->rightHanded));

  /** POSITION IT */
  UsdGeomXformOp location = cube.AddTranslateOp();
  location.Set(VtValue(GfVec3d(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp rotation = cube.AddRotateXYZOp();
  rotation.Set(VtValue(GfVec3f(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp scale = cube.AddScaleOp();
  scale.Set(VtValue(GfVec3f(1.0f, 1.0f, 1.0f)));

  /** AUTHOR A CAMERA */
  UsdGeomCamera camera = UsdGeomCamera::Define(UNI.stage, SdfPath("/Camera"));
  camera.CreateExposureAttr();
  camera.CreateFocalLengthAttr();
  camera.CreateProjectionAttr();
  camera.CreateVerticalApertureAttr();
  camera.CreateVisibilityAttr();

  /** AUTHOR A SPHERE */
  UsdGeomSphere sphere = UsdGeomSphere::Define(UNI.stage, SdfPath("/Sphere"));
  sphere.CreateOrientationAttr();
  sphere.CreateVisibilityAttr();
  sphere.CreateDisplayOpacityPrimvar();
  sphere.CreateRadiusAttr();
  sphere.CreatePurposeAttr();

  /** AUTHOR A CAPSULE */
  UsdGeomCapsule capsule = UsdGeomCapsule::Define(UNI.stage, SdfPath("/Capsule"));
  capsule.CreateRadiusAttr();
  capsule.CreateXformOpOrderAttr();
  capsule.CreateHeightAttr();

  /** AUTHOR A CYLINDER */
  UsdGeomCylinder cylinder = UsdGeomCylinder::Define(UNI.stage, SdfPath("/Cylinder"));
  cylinder.CreateDisplayOpacityAttr();
  cylinder.CreateOrientationAttr();
  cylinder.CreateDoubleSidedAttr();
  cylinder.CreateAxisAttr();

  /** AUTHOR A MESH */
  UsdGeomMesh mesh = UsdGeomMesh::Define(UNI.stage, SdfPath("/Mesh"));
  mesh.CreateDoubleSidedAttr();
  mesh.CreateInterpolateBoundaryAttr();
  mesh.CreateNormalsAttr();
  mesh.CreateVelocitiesAttr();

  /** AUTHOR A NURBS */
  UsdGeomNurbsCurves nurbs = UsdGeomNurbsCurves::Define(UNI.stage, SdfPath("/Nurbs"));
  nurbs.CreateWidthsAttr();
  nurbs.CreateOrientationAttr();
  nurbs.CreateRangesAttr();

  /** AUTHOR A XFORM */
  UsdGeomXform xform = UsdGeomXform::Define(UNI.stage, SdfPath("/Xform"));
  xform.CreatePurposeAttr();
  xform.CreateVisibilityAttr();

  printf("\n");
}

void UNI_diagnostics_hydra_test()
{
  printf("::: HYDRA IMAGING :::\n");

  /** Initiate a OpenGL Context. */
  // GarchGLApiLoad();
  // GlfSharedGLContextScopeHolder sharedContext;
  // GlfRegisterDefaultDebugOutputMessageCallback();

  // const auto &caps = GlfContextCaps::GetInstance();
  // GlfGLContextSharedPtr context = GlfGLContext::GetCurrentGLContext();

  // std::cout << glGetString(GL_VENDOR) << "\n";
  // std::cout << glGetString(GL_RENDERER) << "\n";
  // std::cout << glGetString(GL_VERSION) << "\n";

  // if (!context->IsValid()) {
  //   printf("Critical Error. Could not activate OpenGL context.\n");
  //   printf("Hydra will not be able to function.\n");
  //   printf("\n");
  //   return;
  // }

  printf("Successfully activated Vulkan context.\n");

  EnginePtr engine;
  SdfPathVector excluded_paths;

  printf("Hydra Imaging Engine, reset.\n");
  /** Initiate Hydra Imaging Engine and activate Phoenix Render Engine. */
  engine.reset(new UsdImagingGLEngine(UNI_stage_root(), excluded_paths));
  printf("Phoenix Render Engine, aquired.\n");
  engine->SetRendererPlugin(TfToken("HdPhoenixRendererPlugin"));

  printf("Hydra Imaging Engine, reset.\n");
  /** Initiate Hydra Imaging Engine and activate Pixar Renderman. */
  engine.reset(new UsdImagingGLEngine(UNI_stage_root(), excluded_paths));
  printf("Pixar Renderman, aquired.\n");
  engine->SetRendererPlugin(TfToken("HdPrmanLoaderRendererPlugin"));

  printf("Hydra Imaging Engine, reset.\n");
  /** Initiate Hydra Imaging Engine and activate Intel Embree. */
  engine.reset(new UsdImagingGLEngine(UNI_stage_root(), excluded_paths));
  printf("Intel Embree, aquired.\n");
  engine->SetRendererPlugin(TfToken("HdEmbreeRendererPlugin"));

  printf("Hydra Imaging Engine, reset.\n");
  /** Initiate Hydra Imaging Engine and activate Arnold Render Engine. */
  engine.reset(new UsdImagingGLEngine(UNI_stage_root(), excluded_paths));
  printf("Arnold Renderer, aquired.\n");
  engine->SetRendererPlugin(TfToken("HdArnoldRendererPlugin"));

  printf("\n");
}

void UNI_diagnostics_check()
{
  printf("::: RUNNING PIXAR DEBUGGING LOGISTICS :::\n\n");
  printf("PIXAR SYSTEM VERSION: v%s\n\n", UNI.system.version.pixar_version);

  /** Enable all debug codes. */
  // enable_debug_codes();

  /** Run respective tests. */
  plugin_diagnostics();
  gui_diagnostics();
  geometry_diagnostics();
  sdf_diagnostics();

  /** Any warnings are errors. */
  printf("::: PIXAR DIAGNOSTICS CHECK COMPLETE :::\n\n");
  UNI_save_stage();
}

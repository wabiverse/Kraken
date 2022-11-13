/* SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup RNA
 */

#include <stdio.h>
#include <stdlib.h>

#include "USD_wm_types.h"
#include "USD_ID.h"
#include "USD_object.h"
#include "USD_types.h"

#include "KLI_utildefines.h"

#include "KKE_icons.h"
#include "KKE_lib_id.h"

#include "LUXO_access.h"
#include "LUXO_define.h"
#include "LUXO_enum_types.h"

#include "LUXO_internal.h"


short PRIM_type_to_ID_code(const KrakenPRIM *type)
{
  const TfToken base_type = type->GetTypeName();
  if (UNLIKELY(base_type.IsEmpty())) {
    return 0;
  }
  if (base_type == TfToken("Operator")) {
    return ID_AC;
  }
  if (base_type == TfToken("SkelRoot") || base_type == TfToken("Skeleton") ||
      base_type == TfToken("SkelAnimation")) {
    return ID_AR;
  }
  if (base_type == TfToken("Brush")) {
    return ID_BR;
  }
  if (base_type == TfToken("Cache")) {
    return ID_CF;
  }
  if (base_type == TfToken("Camera")) {
    return ID_CA;
  }
  if (base_type == TfToken("GreasePencil")) {
    return ID_GD;
  }
  if (base_type == TfToken("Collection")) {
    return ID_GR;
  }
  if (base_type == TfToken("Image")) {
    return ID_IM;
  }
  if (base_type == TfToken("Key")) {
    return ID_KE;
  }
  if (base_type == TfToken("LightAPI") || base_type == TfToken("VolumeLightAPI") ||
      base_type == TfToken("LightListAPI") || base_type == TfToken("ShadowAPI") ||
      base_type == TfToken("LightFilter") || base_type == TfToken("DistantLight") ||
      base_type == TfToken("DiskLight") || base_type == TfToken("RectLight") ||
      base_type == TfToken("SphereLight") || base_type == TfToken("CylinderLight") ||
      base_type == TfToken("GeometryLight") || base_type == TfToken("DomeLight") ||
      base_type == TfToken("PortalLight") || base_type == TfToken("PluginLight") ||
      base_type == TfToken("PluginLightFilter")) {
    return ID_LA;
  }
  if (base_type == TfToken("Library")) {
    return ID_LI;
  }
  if (base_type == TfToken("FreeStyleLineStyle")) {
    return ID_LS;
  }
  if (base_type == TfToken("NurbsPatch") || base_type == TfToken("Curves") ||
      base_type == TfToken("BasisCurves") || base_type == TfToken("NurbsCurves")) {
    return ID_CV;
  }
  if (base_type == TfToken("Lattice")) {
    return ID_LT;
  }
  if (base_type == TfToken("Material") || base_type == TfToken("Shader") ||
      base_type == TfToken("MaterialBindingAPI")) {
    return ID_MA;
  }
  if (base_type == TfToken("MetaBall")) {
    return ID_MB;
  }
  if (base_type == TfToken("MovieClip")) {
    return ID_MC;
  }
  if (base_type == TfToken("Boundable")) {
    return ID_ME;
  }
  if (base_type == TfToken("Mask")) {
    return ID_MSK;
  }
  if (base_type == TfToken("NodeGraph") || base_type == TfToken("NodeDefAPI") ||
      base_type == TfToken("ConnectableAPI") || base_type == TfToken("CoordSysAPI")) {
    return ID_NT;
  }
  if (base_type == TfToken("Object")) {
    return ID_OB;
  }
  if (base_type == TfToken("ParticleSettings")) {
    return ID_PA;
  }
  if (base_type == TfToken("Palette")) {
    return ID_PAL;
  }
  if (base_type == TfToken("PaintCurve")) {
    return ID_PC;
  }
  if (base_type == TfToken("PointCloud")) {
    return ID_PT;
  }
  if (base_type == TfToken("LightProbe")) {
    return ID_LP;
  }
  if (base_type == TfToken("Scene")) {
    return ID_SCE;
  }
  if (base_type == TfToken("Screen")) {
    return ID_SCR;
  }
  if (base_type == TfToken("PhysicsScene") || base_type == TfToken("PhysicsRigidBodyAPI") ||
      base_type == TfToken("PhysicsMassAPI") || base_type == TfToken("PhysicsCollisionAPI") ||
      base_type == TfToken("PhysicsFilteredPairsAPI") ||
      base_type == TfToken("PhysicsMaterialAPI") ||
      base_type == TfToken("PhysicsCollisionGroup") ||
      base_type == TfToken("PhysicsFilteredPairsAPI") || base_type == TfToken("PhysicsJoint") ||
      base_type == TfToken("PhysicsRevoluteJoint") ||
      base_type == TfToken("PhysicsPrismaticJoint") ||
      base_type == TfToken("PhysicsSphericalJoint") ||
      base_type == TfToken("PhysicsDistanceJoint") || base_type == TfToken("PhysicsFixedJoint") ||
      base_type == TfToken("PhysicsLimitAPI") || base_type == TfToken("PhysicsDriveAPI") ||
      base_type == TfToken("PhysicsArticulationRootAPI")) {
    return ID_SIM;
  }
  if (base_type == TfToken("Sound")) {
    return ID_SO;
  }
  if (base_type == TfToken("Speaker")) {
    return ID_SPK;
  }
  if (base_type == TfToken("Texture")) {
    return ID_TE;
  }
  if (base_type == TfToken("Text")) {
    return ID_TXT;
  }
  if (base_type == TfToken("VectorFont")) {
    return ID_VF;
  }
  if (base_type == TfToken("Volume")) {
    return ID_VO;
  }
  if (base_type == TfToken("Workspace")) {
    return ID_WS;
  }
  if (base_type == TfToken("World")) {
    return ID_WO;
  }
  if (base_type == TfToken("Window")) {
    return ID_WM;
  }

  return 0;
}

KrakenPRIM *ID_code_to_PRIM_type(short idcode)
{
  /* NOTE: this switch doesn't use a 'default',
   * so adding new ID's causes a warning. */
  switch ((ID_Type)idcode) {
    case ID_AC:
      return &PRIM_Action;
    case ID_AR:
      return &PRIM_Armature;
    case ID_BR:
      return &PRIM_Brush;
    case ID_CA:
      return &PRIM_Camera;
    case ID_CF:
      return &PRIM_CacheFile;
    case ID_CV:
      return &PRIM_Curve;
    case ID_GD:
      return &PRIM_GreasePencil;
    case ID_GR:
      return &PRIM_Collection;
    case ID_IM:
      return &PRIM_Image;
    case ID_KE:
      return &PRIM_Key;
    case ID_LA:
      return &PRIM_Light;
    case ID_LI:
      return &PRIM_Library;
    case ID_LS:
      return &PRIM_FreestyleLineStyle;
    case ID_LT:
      return &PRIM_Lattice;
    case ID_MA:
      return &PRIM_Material;
    case ID_MB:
      return &PRIM_MetaBall;
    case ID_MC:
      return &PRIM_MovieClip;
    case ID_ME:
      return &PRIM_Mesh;
    case ID_MSK:
      return &PRIM_Mask;
    case ID_NT:
      return &PRIM_NodeTree;
    case ID_OB:
      return &PRIM_Object;
    case ID_PA:
      return &PRIM_ParticleSettings;
    case ID_PAL:
      return &PRIM_Palette;
    case ID_PC:
      return &PRIM_PaintCurve;
    case ID_PT:
      return &PRIM_PointCloud;
    case ID_LP:
      return &PRIM_LightProbe;
    case ID_SCE:
      return &PRIM_Scene;
    case ID_SCR:
      return &PRIM_Screen;
    case ID_SIM:
      return &PRIM_Simulation;
    case ID_SO:
      return &PRIM_Sound;
    case ID_SPK:
      return &PRIM_Speaker;
    case ID_TE:
      return &PRIM_Texture;
    case ID_TXT:
      return &PRIM_Text;
    case ID_VF:
      return &PRIM_VectorFont;
    case ID_VO:
      return &PRIM_Volume;
    case ID_WM:
      return &PRIM_WindowManager;
    case ID_WO:
      return &PRIM_World;
    case ID_WS:
      return &PRIM_WorkSpace;
  }

  return &PRIM_ID;
}

KrakenPRIM *prim_ID_refine(KrakenPRIM *ptr)
{
  ID *id = (ID *)ptr->data;

  return ID_code_to_PRIM_type(GS(id->name));
}

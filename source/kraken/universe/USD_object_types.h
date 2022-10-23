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
 * @file Universe
 * Set the Stage.
 *
 * @brief Object is a sort of wrapper for general info.
 */

#include "USD_ID.h"
#include "USD_color_types.h"
#include "USD_customdata_types.h"
#include "USD_defs.h"
#include "USD_listBase.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AnimData;
struct BoundBox;
struct Curve;
struct FluidsimSettings;
struct GeometrySet;
struct Ipo;
struct LightgroupMembership;
struct Material;
struct Mesh;
struct Object;
struct PartDeflect;
struct Path;
struct RigidBodyOb;
struct SculptSession;
struct SoftBody;
struct kGPdata;

/** #Object.mode */
typedef enum eObjectMode
{
  OB_MODE_OBJECT = 0,
  OB_MODE_EDIT = 1 << 0,
  OB_MODE_SCULPT = 1 << 1,
  OB_MODE_VERTEX_PAINT = 1 << 2,
  OB_MODE_WEIGHT_PAINT = 1 << 3,
  OB_MODE_TEXTURE_PAINT = 1 << 4,
  OB_MODE_PARTICLE_EDIT = 1 << 5,
  OB_MODE_POSE = 1 << 6,
  OB_MODE_EDIT_GPENCIL = 1 << 7,
  OB_MODE_PAINT_GPENCIL = 1 << 8,
  OB_MODE_SCULPT_GPENCIL = 1 << 9,
  OB_MODE_WEIGHT_GPENCIL = 1 << 10,
  OB_MODE_VERTEX_GPENCIL = 1 << 11,
  OB_MODE_SCULPT_CURVES = 1 << 12,
} eObjectMode;

/** #Object.dt, #View3DShading.type */
typedef enum eDrawType
{
  OB_BOUNDBOX = 1,
  OB_WIRE = 2,
  OB_SOLID = 3,
  OB_MATERIAL = 4,
  OB_TEXTURE = 5,
  OB_RENDER = 6,
} eDrawType;

/** Any mode where the brush system is used. */
#define OB_MODE_ALL_PAINT \
  (OB_MODE_SCULPT | OB_MODE_VERTEX_PAINT | OB_MODE_WEIGHT_PAINT | OB_MODE_TEXTURE_PAINT)

#define OB_MODE_ALL_PAINT_GPENCIL                                            \
  (OB_MODE_PAINT_GPENCIL | OB_MODE_SCULPT_GPENCIL | OB_MODE_WEIGHT_GPENCIL | \
   OB_MODE_VERTEX_GPENCIL)

/** Any mode that uses Object.sculpt. */
#define OB_MODE_ALL_SCULPT (OB_MODE_SCULPT | OB_MODE_VERTEX_PAINT | OB_MODE_WEIGHT_PAINT)

/** Any mode that uses weightpaint. */
#define OB_MODE_ALL_WEIGHT_PAINT (OB_MODE_WEIGHT_PAINT | OB_MODE_WEIGHT_GPENCIL)

/**
 * Any mode that has data or for Grease Pencil modes, we need to free when switching modes,
 * see: #ED_object_mode_generic_exit
 */
#define OB_MODE_ALL_MODE_DATA                                                                   \
  (OB_MODE_EDIT | OB_MODE_VERTEX_PAINT | OB_MODE_WEIGHT_PAINT | OB_MODE_SCULPT | OB_MODE_POSE | \
   OB_MODE_PAINT_GPENCIL | OB_MODE_EDIT_GPENCIL | OB_MODE_SCULPT_GPENCIL |                      \
   OB_MODE_WEIGHT_GPENCIL | OB_MODE_VERTEX_GPENCIL | OB_MODE_SCULPT_CURVES)

/** Vertex Groups - Name Info */
typedef struct kDeformGroup
{
  struct kDeformGroup *next, *prev;
  /** MAX_VGROUP_NAME. */
  char name[64];
  /* need this flag for locking weights */
  char flag, _pad0[7];
} kDeformGroup;

/** Face Maps. */
typedef struct kFaceMap
{
  struct kFaceMap *next, *prev;
  /** MAX_VGROUP_NAME. */
  char name[64];
  char flag;
  char _pad0[7];
} kFaceMap;

#define MAX_VGROUP_NAME 64

/* kDeformGroup->flag */
#define DG_LOCK_WEIGHT 1

/**
 * The following illustrates the orientation of the
 * bounding box in local space
 *
 * <pre>
 *
 * Z  Y
 * | /
 * |/
 * .-----X
 *     2----------6
 *    /|         /|
 *   / |        / |
 *  1----------5  |
 *  |  |       |  |
 *  |  3-------|--7
 *  | /        | /
 *  |/         |/
 *  0----------4
 * </pre>
 */
typedef struct BoundBox
{
  float vec[8][3];
  int flag;
  char _pad0[4];
} BoundBox;

/** #BoundBox.flag */
enum
{
  /* BOUNDBOX_DISABLED = (1 << 0), */ /* UNUSED */
  BOUNDBOX_DIRTY = (1 << 1),
};

struct CustomData_MeshMasks;

/** Not saved in file! */
typedef struct Object_Runtime
{
  /**
   * The custom data layer mask that was last used
   * to calculate data_eval and mesh_deform_eval.
   */
  CustomData_MeshMasks last_data_mask;

  /** Did last modifier stack generation need mapping support? */
  char last_need_mapping;

  /** Opaque data reserved for management of objects in collection context.
   *  E.g. used currently to check for potential duplicates of objects in a collection, after
   * remapping process. */
  char collection_management;

  char _pad0[2];

  /** Only used for drawing the parent/child help-line. */
  float parent_display_origin[3];

  /**
   * Selection id of this object. It might differ between an evaluated and its original object,
   * when the object is being instanced.
   */
  int select_id;
  char _pad1[3];

  /**
   * Denotes whether the evaluated data is owned by this object or is referenced and owned by
   * somebody else.
   */
  char is_data_eval_owned;

  /** Start time of the mode transfer overlay animation. */
  double overlay_mode_transfer_start_time;

  /** Axis aligned bound-box (in local-space). */
  struct BoundBox *bb;

  /**
   * Original data pointer, before object->data was changed to point
   * to data_eval.
   * Is assigned by dependency graph's copy-on-write evaluation.
   */
  struct ID *data_orig;
  /**
   * Object data structure created during object evaluation. It has all modifiers applied.
   * The type is determined by the type of the original object.
   */
  struct ID *data_eval;

  /**
   * Objects can evaluate to a geometry set instead of a single ID. In those cases, the evaluated
   * geometry set will be stored here. An ID of the correct type is still stored in #data_eval.
   * #geometry_set_eval might reference the ID pointed to by #data_eval as well, but does not own
   * the data.
   */
  // struct GeometrySet *geometry_set_eval;

  /**
   * Mesh structure created during object evaluation.
   * It has deformation only modifiers applied on it.
   */
  // struct Mesh *mesh_deform_eval;

  /* Evaluated mesh cage in edit mode. */
  // struct Mesh *editmesh_eval_cage;

  /** Cached cage bounding box of `editmesh_eval_cage` for selection. */
  struct BoundBox *editmesh_bb_cage;

  /**
   * Original grease pencil bGPdata pointer, before object->data was changed to point
   * to gpd_eval.
   * Is assigned by dependency graph's copy-on-write evaluation.
   */
  // struct kGPdata *gpd_orig;
  /**
   * bGPdata structure created during object evaluation.
   * It has all modifiers applied.
   */
  // struct kGPdata *gpd_eval;

  /**
   * This is a mesh representation of corresponding object.
   * It created when Python calls `object.to_mesh()`.
   */
  // struct Mesh *object_as_temp_mesh;

  /**
   * This is a curve representation of corresponding object.
   * It created when Python calls `object.to_curve()`.
   */
  // struct Curve *object_as_temp_curve;

  /** Runtime evaluated curve-specific data, not stored in the file. */
  // struct CurveCache *curve_cache;

  unsigned short local_collections_bits;
  short _pad2[3];

  float (*crazyspace_deform_imats)[3][3];
  float (*crazyspace_deform_cos)[3];
  int crazyspace_verts_num;

  int _pad3[3];
} Object_Runtime;

typedef struct ObjectLineArt
{
  short usage;
  short flags;

  /** if OBJECT_LRT_OWN_CREASE is set */
  float crease_threshold;

  unsigned char intersection_priority;

  char _pad[7];
} ObjectLineArt;

/**
 * @warning while the values seem to be flags, they aren't treated as flags.
 */
enum eObjectLineArt_Usage
{
  OBJECT_LRT_INHERIT = 0,
  OBJECT_LRT_INCLUDE = (1 << 0),
  OBJECT_LRT_OCCLUSION_ONLY = (1 << 1),
  OBJECT_LRT_EXCLUDE = (1 << 2),
  OBJECT_LRT_INTERSECTION_ONLY = (1 << 3),
  OBJECT_LRT_NO_INTERSECTION = (1 << 4),
  OBJECT_LRT_FORCE_INTERSECTION = (1 << 5),
};

enum eObjectLineArt_Flags
{
  OBJECT_LRT_OWN_CREASE = (1 << 0),
  OBJECT_LRT_OWN_INTERSECTION_PRIORITY = (1 << 1),
};

typedef struct Object
{
  ID id;

  short type, partype;
  /** Can be vertexnrs. */
  int par1, par2, par3;
  /** String describing subobject info, MAX_ID_NAME-2. */
  char parsubstr[64];
  struct Object *parent, *track;
  /** Pointer to objects data - an 'ID' or NULL. */
  void *data;

  /** List of ModifierData structures. */
  ListBase modifiers;
  /** List of GpencilModifierData structures. */
  ListBase greasepencil_modifiers;
  /** List of facemaps. */
  ListBase fmaps;
  /** List of viewport effects. Actually only used by grease pencil. */
  ListBase shader_fx;

  /** Local object mode. */
  int mode;
  int restore_mode;

  /* materials */
  /** Material slots. */
  struct Material **mat;
  /** A boolean field, with each byte 1 if corresponding material is linked to object. */
  char *matbits;
  /** Copy of mesh, curve & meta struct member of same name (keep in sync). */
  int totcol;
  /** Currently selected material in the UI. */
  int actcol;

  /* rot en drot have to be together! (transform('r' en 's')) */
  float loc[3], dloc[3];
  /** Scale (can be negative). */
  float scale[3];
  /** Ack!, changing. */
  float dscale[3];
  /** Euler rotation. */
  float rot[3], drot[3];
  /** Quaternion rotation. */
  float quat[4], dquat[4];
  /** Axis angle rotation - axis part. */
  float rotAxis[3], drotAxis[3];
  /** Axis angle rotation - angle part. */
  float rotAngle, drotAngle;
  /** Final world-space matrix with constraints & animsys applied. */
  float obmat[4][4];
  /** Inverse result of parent, so that object doesn't 'stick' to parent. */
  float parentinv[4][4];
  /** Inverse result of constraints.
   * doesn't include effect of parent or object local transform. */
  float constinv[4][4];
  /**
   * Inverse matrix of 'obmat' for any other use than rendering!
   *
   * @note this isn't assured to be valid as with 'obmat',
   * before using this value you should do: `invert_m4_m4(ob->imat, ob->obmat)`
   */
  float imat[4][4];

  /** Copy of Base. */
  short flag;

  /** Transformation settings and transform locks. */
  short transflag, protectflag;
  short trackflag, upflag;
  /** Used for DopeSheet filtering settings (expanded/collapsed). */
  short nlaflag;

  char _pad1;
  char duplicator_visibility_flag;

  /* Hydra */
  /** Used by hydra, flushed from base. */
  short base_flag;
  /** Used by viewport, synced from base. */
  unsigned short base_local_view_bits;

  /** Collision mask settings */
  unsigned short col_group, col_mask;

  /** Rotation mode - uses defines set out in USD_action_types.h for PoseChannel rotations.... */
  short rotmode;

  /** Bounding box use for drawing. */
  char boundtype;
  /** Bounding box type used for collision. */
  char collision_boundtype;

  /** Viewport draw extra settings. */
  short dtx;
  /** Viewport draw type. */
  char dt;
  char empty_drawtype;
  float empty_drawsize;
  /** Dupliface scale. */
  float instance_faces_scale;

  /** Custom index, for render-passes. */
  short index;
  /** Current face map, NOTE: index starts at 1. */
  unsigned short actfmap;
  char _pad2[2];
  /** Object color (in most cases the material color is used for drawing). */
  float color[4];

  /** Softbody settings. */
  short softflag;

  /** For restricting view, select, render etc. accessible in outliner. */
  short visibility_flag;

  /** Current shape key for menu or pinned. */
  short shapenr;
  /** Flag for pinning. */
  char shapeflag;

  char _pad3[1];

  /** Object constraints. */
  ListBase constraints;
  /** Particle systems. */
  ListBase particlesystem;

  ListBase pc_ids;

  /** Offset for image empties. */
  float ima_ofs[2];
  /** Must be non-null when object is an empty image. */
  ImageUser *iuser;
  char empty_image_visibility_flag;
  char empty_image_depth;
  char empty_image_flag;

  /** ObjectModifierFlag */
  uint8_t modifier_flag;
  char _pad8[4];

  struct PreviewImage *preview;

  ObjectLineArt lineart;

  /** Runtime evaluation data (keep last). */
  Object_Runtime runtime;
} Object;

/* **************** OBJECT ********************* */

/**
 * This is used as a flag for many kinds of data that use selections, examples include:
 * - #BezTriple.f1, #BezTriple.f2, #BezTriple.f3
 * - #bNote.flag
 * - #MovieTrackingTrack.flag
 * And more, ideally this would have a generic location.
 */
#define SELECT 1

/** #Object.type */
enum
{
  OB_EMPTY = 0,
  OB_MESH = 1,
  OB_CURVES = 2,
  OB_SURF = 3,
  OB_FONT = 4,
  OB_MBALL = 5,

  OB_LAMP = 10,
  OB_CAMERA = 11,

  OB_SPEAKER = 12,
  OB_LIGHTPROBE = 13,

  OB_LATTICE = 22,

  OB_ARMATURE = 25,

  OB_GPENCIL = 27,

  OB_POINTCLOUD = 28,

  OB_VOLUME = 29,

  /* Keep last. */
  OB_TYPE_MAX,
};

/* check if the object type supports materials */
#define OB_TYPE_SUPPORT_MATERIAL(_type) \
  (((_type) >= OB_MESH && (_type) <= OB_MBALL) || ((_type) >= OB_GPENCIL && (_type) <= OB_VOLUME))
/** Does the object have some render-able geometry (unlike empties, cameras, etc.). */
#define OB_TYPE_IS_GEOMETRY(_type) \
  (ELEM(_type,                     \
        OB_MESH,                   \
        OB_SURF,                   \
        OB_FONT,                   \
        OB_MBALL,                  \
        OB_GPENCIL,                \
        OB_CURVES,                 \
        OB_POINTCLOUD,             \
        OB_VOLUME))
#define OB_TYPE_SUPPORT_VGROUP(_type) (ELEM(_type, OB_MESH, OB_LATTICE, OB_GPENCIL))
#define OB_TYPE_SUPPORT_EDITMODE(_type) \
  (ELEM(_type, OB_MESH, OB_FONT, OB_CURVES, OB_SURF, OB_MBALL, OB_LATTICE, OB_ARMATURE))
#define OB_TYPE_SUPPORT_PARVERT(_type) (ELEM(_type, OB_MESH, OB_SURF, OB_CURVES, OB_LATTICE))

/** Matches #OB_TYPE_SUPPORT_EDITMODE. */
#define OB_DATA_SUPPORT_EDITMODE(_type)              \
  (ELEM(_type, ID_ME, ID_CV, ID_MB, ID_LT, ID_AR) || \
   (U.experimental.use_new_curves_tools && (_type) == ID_CV))

/* is this ID type used as object data */
#define OB_DATA_SUPPORT_ID(_id_type) \
  (ELEM(_id_type,                    \
        ID_ME,                       \
        ID_CV,                       \
        ID_MB,                       \
        ID_LA,                       \
        ID_SPK,                      \
        ID_LP,                       \
        ID_CA,                       \
        ID_LT,                       \
        ID_GD,                       \
        ID_AR,                       \
        ID_PT,                       \
        ID_VO))

#define OB_DATA_SUPPORT_ID_CASE \
  ID_ME:                        \
  case ID_CV:                   \
  case ID_MB:                   \
  case ID_LA:                   \
  case ID_SPK:                  \
  case ID_LP:                   \
  case ID_CA:                   \
  case ID_LT:                   \
  case ID_GD:                   \
  case ID_AR:                   \
  case ID_PT:                   \
  case ID_VO

/** #Object.partype: first 4 bits: type. */
enum
{
  PARTYPE = (1 << 4) - 1,
  PAROBJECT = 0,
  PARSKEL = 4,
  PARVERT1 = 5,
  PARVERT3 = 6,
  PARBONE = 7,

};

/** #Object.transflag (short) */
enum
{
  OB_TRANSFORM_ADJUST_ROOT_PARENT_FOR_VIEW_LOCK = 1 << 0,
  OB_TRANSFLAG_UNUSED_1 = 1 << 1, /* cleared */
  OB_NEG_SCALE = 1 << 2,
  OB_TRANSFLAG_UNUSED_3 = 1 << 3, /* cleared */
  OB_DUPLIVERTS = 1 << 4,
  OB_DUPLIROT = 1 << 5,
  OB_TRANSFLAG_UNUSED_6 = 1 << 6, /* cleared */
  /* runtime, calculate derivedmesh for dupli before it's used */
  OB_TRANSFLAG_UNUSED_7 = 1 << 7, /* dirty */
  OB_DUPLICOLLECTION = 1 << 8,
  OB_DUPLIFACES = 1 << 9,
  OB_DUPLIFACES_SCALE = 1 << 10,
  OB_DUPLIPARTS = 1 << 11,
  OB_TRANSFLAG_UNUSED_12 = 1 << 12, /* cleared */
  /* runtime constraints disable */
  OB_NO_CONSTRAINTS = 1 << 13,

  OB_DUPLI = OB_DUPLIVERTS | OB_DUPLICOLLECTION | OB_DUPLIFACES | OB_DUPLIPARTS,
};

/** #Object.trackflag / #Object.upflag (short) */
enum
{
  OB_POSX = 0,
  OB_POSY = 1,
  OB_POSZ = 2,
  OB_NEGX = 3,
  OB_NEGY = 4,
  OB_NEGZ = 5,
};

/** #Object.dtx draw type extra flags (short) */
enum
{
  OB_DRAWBOUNDOX = 1 << 0,
  OB_AXIS = 1 << 1,
  OB_TEXSPACE = 1 << 2,
  OB_DRAWNAME = 1 << 3,
  /* OB_DRAWIMAGE = 1 << 4, */ /* UNUSED */
  /* for solid+wire display */
  OB_DRAWWIRE = 1 << 5,
  /* For overdrawing. */
  OB_DRAW_IN_FRONT = 1 << 6,
  /* Enable transparent draw. */
  OB_DRAWTRANSP = 1 << 7,
  OB_DRAW_ALL_EDGES = 1 << 8, /* only for meshes currently */
  OB_DRAW_NO_SHADOW_CAST = 1 << 9,
  /* Enable lights for grease pencil. */
  OB_USE_GPENCIL_LIGHTS = 1 << 10,
};

/** #Object.empty_drawtype: no flags */
enum
{
  OB_ARROWS = 1,
  OB_PLAINAXES = 2,
  OB_CIRCLE = 3,
  OB_SINGLE_ARROW = 4,
  OB_CUBE = 5,
  OB_EMPTY_SPHERE = 6,
  OB_EMPTY_CONE = 7,
  OB_EMPTY_IMAGE = 8,
};

/**
 * Grease-pencil add types.
 * TODO: doesn't need to be DNA, local to `OBJECT_OT_gpencil_add`.
 */
enum
{
  GP_EMPTY = 0,
  GP_STROKE = 1,
  GP_MONKEY = 2,
  GP_LRT_SCENE = 3,
  GP_LRT_OBJECT = 4,
  GP_LRT_COLLECTION = 5,
};

/** #Object.boundtype */
enum
{
  OB_BOUND_BOX = 0,
  OB_BOUND_SPHERE = 1,
  OB_BOUND_CYLINDER = 2,
  OB_BOUND_CONE = 3,
  // OB_BOUND_TRIANGLE_MESH = 4, /* UNUSED */
  // OB_BOUND_CONVEX_HULL = 5,   /* UNUSED */
  // OB_BOUND_DYN_MESH = 6,      /* UNUSED */
  OB_BOUND_CAPSULE = 7,
};

/* **************** BASE ********************* */

/** #Base.flag_legacy */
enum
{
  BA_WAS_SEL = (1 << 1),
  /* NOTE: BA_HAS_RECALC_DATA can be re-used later if freed in readfile.c. */
  // BA_HAS_RECALC_OB = (1 << 2),  /* DEPRECATED */
  // BA_HAS_RECALC_DATA =  (1 << 3),  /* DEPRECATED */
  /** DEPRECATED, was runtime only, but was reusing an older flag. */
  BA_SNAP_FIX_DEPS_FIASCO = (1 << 2),
};

/* NOTE: this was used as a proper setting in past, so nullify before using */
#define BA_TEMP_TAG (1 << 5)

/**
 * Even if this is tagged for transform, this flag means it's being locked in place.
 * Use for #SCE_XFORM_SKIP_CHILDREN.
 */
#define BA_TRANSFORM_LOCKED_IN_PLACE (1 << 7)

#define BA_TRANSFORM_CHILD (1 << 8)   /* child of a transformed object */
#define BA_TRANSFORM_PARENT (1 << 13) /* parent of a transformed object */

#define OB_FROMDUPLI (1 << 9)
#define OB_DONE (1 << 10) /* unknown state, clear before use */
#ifdef DNA_DEPRECATED_ALLOW
#  define OB_FLAG_UNUSED_11 (1 << 11) /* cleared */
#  define OB_FLAG_UNUSED_12 (1 << 12) /* cleared */
#endif

/** #Object.visibility_flag */
enum
{
  OB_HIDE_VIEWPORT = 1 << 0,
  OB_HIDE_SELECT = 1 << 1,
  OB_HIDE_RENDER = 1 << 2,
  OB_HIDE_CAMERA = 1 << 3,
  OB_HIDE_DIFFUSE = 1 << 4,
  OB_HIDE_GLOSSY = 1 << 5,
  OB_HIDE_TRANSMISSION = 1 << 6,
  OB_HIDE_VOLUME_SCATTER = 1 << 7,
  OB_HIDE_SHADOW = 1 << 8,
  OB_HOLDOUT = 1 << 9,
  OB_SHADOW_CATCHER = 1 << 10
};

/** #Object.shapeflag */
enum
{
  OB_SHAPE_LOCK = 1 << 0,
#ifdef DNA_DEPRECATED_ALLOW
  OB_SHAPE_FLAG_UNUSED_1 = 1 << 1, /* cleared */
#endif
  OB_SHAPE_EDIT_MODE = 1 << 2,
};

/** #Object.nlaflag */
enum
{
  OB_ADS_UNUSED_1 = 1 << 0, /* cleared */
  OB_ADS_UNUSED_2 = 1 << 1, /* cleared */
  /* object-channel expanded status */
  OB_ADS_COLLAPSED = 1 << 10,
  /* object's ipo-block */
  /* OB_ADS_SHOWIPO = 1 << 11, */ /* UNUSED */
  /* object's constraint channels */
  /* OB_ADS_SHOWCONS = 1 << 12, */ /* UNUSED */
  /* object's material channels */
  /* OB_ADS_SHOWMATS = 1 << 13, */ /* UNUSED */
  /* object's particle channels */
  /* OB_ADS_SHOWPARTS = 1 << 14, */ /* UNUSED */
};

/** #Object.protectflag */
enum
{
  OB_LOCK_LOCX = 1 << 0,
  OB_LOCK_LOCY = 1 << 1,
  OB_LOCK_LOCZ = 1 << 2,
  OB_LOCK_LOC = OB_LOCK_LOCX | OB_LOCK_LOCY | OB_LOCK_LOCZ,
  OB_LOCK_ROTX = 1 << 3,
  OB_LOCK_ROTY = 1 << 4,
  OB_LOCK_ROTZ = 1 << 5,
  OB_LOCK_ROT = OB_LOCK_ROTX | OB_LOCK_ROTY | OB_LOCK_ROTZ,
  OB_LOCK_SCALEX = 1 << 6,
  OB_LOCK_SCALEY = 1 << 7,
  OB_LOCK_SCALEZ = 1 << 8,
  OB_LOCK_SCALE = OB_LOCK_SCALEX | OB_LOCK_SCALEY | OB_LOCK_SCALEZ,
  OB_LOCK_ROTW = 1 << 9,
  OB_LOCK_ROT4D = 1 << 10,
};

/** #Object.duplicator_visibility_flag */
enum
{
  OB_DUPLI_FLAG_VIEWPORT = 1 << 0,
  OB_DUPLI_FLAG_RENDER = 1 << 1,
};

/** #Object.empty_image_depth */
#define OB_EMPTY_IMAGE_DEPTH_DEFAULT 0
#define OB_EMPTY_IMAGE_DEPTH_FRONT 1
#define OB_EMPTY_IMAGE_DEPTH_BACK 2

/** #Object.empty_image_visibility_flag */
enum
{
  OB_EMPTY_IMAGE_HIDE_PERSPECTIVE = 1 << 0,
  OB_EMPTY_IMAGE_HIDE_ORTHOGRAPHIC = 1 << 1,
  OB_EMPTY_IMAGE_HIDE_BACK = 1 << 2,
  OB_EMPTY_IMAGE_HIDE_FRONT = 1 << 3,
  OB_EMPTY_IMAGE_HIDE_NON_AXIS_ALIGNED = 1 << 4,
};

/** #Object.empty_image_flag */
enum
{
  OB_EMPTY_IMAGE_USE_ALPHA_BLEND = 1 << 0,
};

typedef enum ObjectModifierFlag
{
  OB_MODIFIER_FLAG_ADD_REST_POSITION = 1 << 0,
} ObjectModifierFlag;

#define MAX_DUPLI_RECUR 8

#ifdef __cplusplus
}
#endif

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

#include "MEM_guardedalloc.h"

#include "USD_ID.h"
#include "USD_color_types.h"
#include "USD_customdata_types.h"
#include "USD_defs.h"
#include "USD_listBase.h"

#ifdef __cplusplus
#  include "KLI_array.hh"
#  include "KLI_span.hh"
#  include <mutex>
#endif /* __cplusplus */

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

typedef enum eMeshBatchDirtyMode
{
  KKE_MESH_BATCH_DIRTY_ALL = 0,
  KKE_MESH_BATCH_DIRTY_SELECT,
  KKE_MESH_BATCH_DIRTY_SELECT_PAINT,
  KKE_MESH_BATCH_DIRTY_SHADING,
  KKE_MESH_BATCH_DIRTY_UVEDIT_ALL,
  KKE_MESH_BATCH_DIRTY_UVEDIT_SELECT,
} eMeshBatchDirtyMode;

/** #MeshRuntime.wrapper_type */
typedef enum eMeshWrapperType
{
  /** Use mesh data (#Mesh.mvert, #Mesh.medge, #Mesh.mloop, #Mesh.mpoly). */
  ME_WRAPPER_TYPE_MDATA = 0,
  /** Use edit-mesh data (#Mesh.edit_mesh, #MeshRuntime.edit_data). */
  ME_WRAPPER_TYPE_BMESH = 1,
  /** Use subdivision mesh data (#MeshRuntime.mesh_eval). */
  ME_WRAPPER_TYPE_SUBD = 2,
} eMeshWrapperType;

/**
 * Mesh Vertices.
 *
 * Typically accessed from #Mesh.verts()
 */
typedef struct MVert
{
  float co[3];
  /**
   * Deprecated flag for storing hide status and selection, which are now stored in separate
   * generic attributes. Kept for file read and write.
   */
  char flag_legacy;
  char _pad[2];
} MVert;

/**
 * Mesh Edges.
 *
 * Typically accessed with #Mesh.edges()
 */
typedef struct MEdge
{
  /** Un-ordered vertex indices (cannot match). */
  unsigned int v1, v2;
  /** Deprecated edge crease, now located in #CD_CREASE, except for file read and write. */
  char crease_legacy;
  short flag;
} MEdge;

/** #MEdge.flag */
enum
{
  /** Deprecated selection status. Now stored in ".select_edge" attribute. */
  /*  SELECT = (1 << 0), */
  ME_EDGEDRAW = (1 << 1),
  ME_SEAM = (1 << 2),
  /** Deprecated hide status. Now stored in ".hide_edge" attribute. */
  /*  ME_HIDE = (1 << 4), */
  ME_EDGERENDER = (1 << 5),
  ME_LOOSEEDGE = (1 << 7),
  ME_SHARP = (1 << 9), /* only reason this flag remains a 'short' */
};

/**
 * Mesh Faces.
 * This only stores the polygon size & flags, the vertex & edge indices are stored in the #MLoop.
 *
 * Typically accessed with #Mesh.polys().
 */
typedef struct MPoly
{
  /** Offset into loop array and number of loops in the face. */
  int loopstart;
  /** Keep signed since we need to subtract when getting the previous loop. */
  int totloop;
  char flag, _pad;
} MPoly;

/**
 * Mesh Face Corners.
 * "Loop" is an internal name for the corner of a polygon (#MPoly).
 *
 * Typically accessed with #Mesh.loops().
 */
typedef struct MLoop
{
  /** Vertex index into an #MVert array. */
  unsigned int v;
  /** Edge index into an #MEdge array. */
  unsigned int e;
} MLoop;

/**
 * Optionally store the order of selected elements.
 * This won't always be set since only some selection operations have an order.
 *
 * Typically accessed from #Mesh.mselect
 */
typedef struct MSelect
{
  /** Index in the vertex, edge or polygon array. */
  int index;
  /** #ME_VSEL, #ME_ESEL, #ME_FSEL. */
  int type;
} MSelect;

typedef struct MLoopTri
{
  unsigned int tri[3];
  unsigned int poly;
} MLoopTri;

typedef struct MVertTri
{
  unsigned int tri[3];
} MVertTri;

typedef struct CharInfo
{
  short kern;
  /** Index start at 1, unlike mesh & nurbs. */
  short mat_nr;
  char flag;
  char _pad[3];
} CharInfo;

typedef struct TextBox
{
  float x, y, w, h;
} TextBox;

typedef struct PackedFile
{
  int size;
  int seek;
  void *data;
} PackedFile;

typedef struct VFontData
{
  struct RHash *characters;
  char name[128];
  float scale;
  /* Calculated from the font. */
  float em_height;
  float ascender;
} VFontData;

typedef struct VChar
{
  ListBase nurbsbase;
  unsigned int index;
  float width;
} VChar;

typedef struct VFont
{
  ID id;

  /** 1024 = FILE_MAX. */
  char filepath[1024];

  struct VFontData *data;
  struct PackedFile *packedfile;

  /* runtime only, holds memory for freetype to read from
   * TODO: replace this with #krf_font_new() style loading. */
  struct PackedFile *temp_pf;
} VFont;

/**
 * @note #KPoint.tilt location in struct is abused by Key system.
 */
typedef struct KPoint
{
  float vec[4];
  /** Tilt in 3D View. */
  float tilt;
  /** Used for softbody goal weight. */
  float weight;
  /** F1: selection status,  hide: is point hidden or not. */
  uint8_t f1;
  char _pad1[1];
  short hide;
  /** User-set radius per point for beveling etc. */
  float radius;
  char _pad[4];
} KPoint;

/**
 * Vertex group index and weight for #MDeformVert.dw
 */
typedef struct MDeformWeight
{
  /** The index for the vertex group, must *always* be unique when in an array. */
  unsigned int def_nr;
  /** Weight between 0.0 and 1.0. */
  float weight;
} MDeformWeight;

/**
 * Stores all of an element's vertex groups, and their weight values.
 */
typedef struct MDeformVert
{
  /**
   * Array of weight indices and values.
   * - There must not be any duplicate #def_nr indices.
   * - Groups in the array are unordered.
   * - Indices outside the usable range of groups are ignored.
   */
  struct MDeformWeight *dw;
  /**
   * The length of the #dw array.
   * @note This is not necessarily the same length as the total number of vertex groups.
   * However, generally it isn't larger.
   */
  int totweight;
  /** Flag is only in use as a run-time tag at the moment. */
  int flag;
} MDeformVert;

typedef struct EditMeshData
{
  /** when set, @a vertexNos, polyNos are lazy initialized */
  const float (*vertexCos)[3];

  /** lazy initialize (when @a vertexCos is set) */
  float const (*vertexNos)[3];
  float const (*polyNos)[3];
  /** also lazy init but don't depend on @a vertexCos */
  const float (*polyCos)[3];
} EditMeshData;

typedef struct EditNurb
{
  USD_DEFINE_CXX_METHODS(EditNurb)

  /* base of nurbs' list (old Curve->editnurb) */
  ListBase nurbs;

  /* index data for shape keys */
  struct RHash *keyindex;

  /* shape key being edited */
  int shapenr;

  /**
   * ID data is older than edit-mode data.
   * Set #Main.is_memfile_undo_flush_needed when enabling.
   */
  char needs_flush_to_id;

} EditNurb;

typedef struct Curve
{
  USD_DEFINE_CXX_METHODS(Curve)

  ID id;
  /** Animation data (must be immediately after id for utilities to use it). */
  struct AnimData *adt;

  /** Actual data, called splines in rna. */
  ListBase nurb;

  /** Edited data, not in file, use pointer so we can check for it. */
  EditNurb *editnurb;

  struct Object *bevobj, *taperobj, *textoncurve;

  struct Key *key;
  struct Material **mat;

  struct CurveProfile *bevel_profile;

  /* texture space, copied as one block in editobject.c */
  float loc[3];
  float size[3];

  /** Creation-time type of curve datablock. */
  short type;

  char texflag;
  char _pad0[7];
  short twist_mode;
  float twist_smooth, smallcaps_scale;

  int pathlen;
  short bevresol, totcol;
  int flag;
  float offset, extrude, bevel_radius;

  /* default */
  short resolu, resolv;
  short resolu_ren, resolv_ren;

  /* edit, index in nurb list */
  int actnu;
  /* edit, index in active nurb (BPoint or BezTriple) */
  int actvert;

  char overflow;
  char spacemode, align_y;
  char bevel_mode;
  /**
   * Determine how the effective radius of the bevel point is computed when a taper object is
   * specified. The effective radius is a function of the bevel point radius and the taper radius.
   */
  char taper_radius_mode;
  char _pad;

  /* font part */
  short lines;
  float spacing, linedist, shear, fsize, wordspace, ulpos, ulheight;
  float xof, yof;
  float linewidth;

  /* copy of EditFont vars (wchar_t aligned),
   * warning! don't use in editmode (storage only) */
  int pos;
  int selstart, selend;

  /* text data */
  /**
   * Number of characters (unicode code-points)
   * This is the length of #Curve.strinfo and the result of `BLI_strlen_utf8(cu->str)`.
   */
  int len_char32;
  /** Number of bytes: `strlen(Curve.str)`. */
  int len;
  char *str;
  struct EditFont *editfont;

  char family[64];
  struct VFont *vfont;
  struct VFont *vfontb;
  struct VFont *vfonti;
  struct VFont *vfontbi;

  struct TextBox *tb;
  int totbox, actbox;

  struct CharInfo *strinfo;
  struct CharInfo curinfo;
  /* font part end */

  /** Current evaltime - for use by Objects parented to curves. */
  float ctime;
  float bevfac1, bevfac2;
  char bevfac1_mapping, bevfac2_mapping;

  char _pad2[6];
  float fsize_realtime;

  /**
   * A pointer to curve data from evaluation. Owned by the object's #geometry_set_eval, either as a
   * geometry instance or the data of the evaluated #CurveComponent. The curve may also contain
   * data in the #nurb list, but for evaluated curves this is the proper place to retrieve data,
   * since it also contains the result of geometry nodes evaluation, and isn't just a copy of the
   * original object data.
   */
  const struct Curves *curve_eval;
  /**
   * If non-zero, the #editfont and #editnurb pointers are not owned by this #Curve. That means
   * this curve is a container for the result of object geometry evaluation. This only works
   * because evaluated object data never outlives original data.
   */
  char edit_data_from_original;
  char _pad3[7];

  void *batch_cache;
} Curve;

#ifdef __cplusplus
namespace kraken::kke
{

  /**
   * @warning Typical access is done via #Mesh::looptris().
   */
  struct MLoopTri_Store
  {
    /* WARNING! swapping between array (ready-to-be-used data) and array_wip
     * (where data is actually computed)
     * shall always be protected by same lock as one used for looptris computing. */
    MLoopTri *array = nullptr;
    MLoopTri *array_wip = nullptr;
    int len = 0;
    int len_alloc = 0;
  };

  struct MeshRuntime
  {
    /* Evaluated mesh for objects which do not have effective modifiers.
     * This mesh is used as a result of modifier stack evaluation.
     * Since modifier stack evaluation is threaded on object level we need some synchronization. */
    Mesh *mesh_eval = nullptr;
    std::mutex eval_mutex;

    /* A separate mutex is needed for normal calculation, because sometimes
     * the normals are needed while #eval_mutex is already locked. */
    std::mutex normals_mutex;

    /** Needed to ensure some thread-safety during render data pre-processing. */
    std::mutex render_mutex;

    /** Lazily initialized SoA data from the #edit_mesh field in #Mesh. */
    EditMeshData *edit_data = nullptr;

    /**
     * Data used to efficiently draw the mesh in the viewport, especially useful when
     * the same mesh is used in many objects or instances. See `draw_cache_impl_mesh.cc`.
     */
    void *batch_cache = nullptr;

    /** Cache for derived triangulation of the mesh. */
    MLoopTri_Store looptris;

    /** Cache for BVH trees generated for the mesh. Defined in 'BKE_bvhutil.c' */
    // BVHCache *bvh_cache = nullptr;

    /** Cache of non-manifold boundary data for Shrink-wrap Target Project. */
    // ShrinkwrapBoundaryData *shrinkwrap_data = nullptr;

    /** Needed in case we need to lazily initialize the mesh. */
    CustomData_MeshMasks cd_mask_extra = {};

    // SubdivCCG *subdiv_ccg = nullptr;
    int subdiv_ccg_tot_level = 0;

    /** Set by modifier stack if only deformed from original. */
    bool deformed_only = false;
    /**
     * Copied from edit-mesh (hint, draw with edit-mesh data when true).
     *
     * Modifiers that edit the mesh data in-place must set this to false
     * (most #eModifierTypeType_NonGeometrical modifiers). Otherwise the edit-mesh
     * data will be used for drawing, missing changes from modifiers. See T79517.
     */
    bool is_original_bmesh = false;

    /** #eMeshWrapperType and others. */
    eMeshWrapperType wrapper_type = ME_WRAPPER_TYPE_MDATA;
    /**
     * A type mask from wrapper_type,
     * in case there are differences in finalizing logic between types.
     */
    eMeshWrapperType wrapper_type_finalize = ME_WRAPPER_TYPE_MDATA;

    /**
     * Settings for lazily evaluating the subdivision on the CPU if needed. These are
     * set in the modifier when GPU subdivision can be performed, and owned by the by
     * the modifier in the object.
     */
    // SubsurfRuntimeData *subsurf_runtime_data = nullptr;

    /**
     * Caches for lazily computed vertex and polygon normals. These are stored here rather than in
     * #CustomData because they can be calculated on a `const` mesh, and adding custom data layers
     * on a `const` mesh is not thread-safe.
     */
    bool vert_normals_dirty = false;
    bool poly_normals_dirty = false;
    float (*vert_normals)[3] = nullptr;
    float (*poly_normals)[3] = nullptr;

    /**
     * A #BLI_bitmap containing tags for the center vertices of subdivided polygons, set by the
     * subdivision surface modifier and used by drawing code instead of polygon center face dots.
     */
    uint32_t *subsurf_face_dot_tags = nullptr;

    MeshRuntime() = default;
    /** \warning This does not free all data currently. See #BKE_mesh_runtime_free_data. */
    ~MeshRuntime() = default;

    MEM_CXX_CLASS_ALLOC_FUNCS("MeshRuntime")
  };

}  // namespace kraken::kke
#endif /* __cplusplus */

typedef struct Mesh
{
  USD_DEFINE_CXX_METHODS(Mesh)

  ID id;
  /** Animation data (must be immediately after id for utilities to use it). */
  struct AnimData *adt;

  /** Old animation system, deprecated for 2.5. */
  struct Key *key;

  /**
   * An array of materials, with length #totcol. These can be overridden by material slots
   * on #Object. Indices in the "material_index" attribute control which material is used for every
   * face.
   */
  struct Material **mat;

  /** The number of vertices (#MVert) in the mesh, and the size of #vdata. */
  int totvert;
  /** The number of edges (#MEdge) in the mesh, and the size of #edata. */
  int totedge;
  /** The number of polygons/faces (#MPoly) in the mesh, and the size of #pdata. */
  int totpoly;
  /** The number of face corners (#MLoop) in the mesh, and the size of #ldata. */
  int totloop;

  CustomData vdata, edata, pdata, ldata;

  /**
   * List of vertex group (#bDeformGroup) names and flags only. Actual weights are stored in dvert.
   * \note This pointer is for convenient access to the #CD_MDEFORMVERT layer in #vdata.
   */
  ListBase vertex_group_names;
  /** The active index in the #vertex_group_names list. */
  int vertex_group_active_index;

  /**
   * The index of the active attribute in the UI. The attribute list is a combination of the
   * generic type attributes from vertex, edge, face, and corner custom data.
   */
  int attributes_active_index;

  /**
   * Runtime storage of the edit mode mesh. If it exists, it generally has the most up-to-date
   * information about the mesh.
   * \note When the object is available, the preferred access method is #BKE_editmesh_from_object.
   */
  struct KMEditMesh *edit_mesh;

  /**
   * This array represents the selection order when the user manually picks elements in edit-mode,
   * some tools take advantage of this information. All elements in this array are expected to be
   * selected, see #BKE_mesh_mselect_validate which ensures this. For procedurally created meshes,
   * this is generally empty (selections are stored as boolean attributes in the corresponding
   * custom data).
   */
  struct MSelect *mselect;

  /** The length of the #mselect array. */
  int totselect;

  /**
   * In most cases the last selected element (see #mselect) represents the active element.
   * For faces we make an exception and store the active face separately so it can be active
   * even when no faces are selected. This is done to prevent flickering in the material properties
   * and UV Editor which base the content they display on the current material which is controlled
   * by the active face.
   *
   * \note This is mainly stored for use in edit-mode.
   */
  int act_face;

  /**
   * An optional mesh owned elsewhere (by #Main) that can be used to override
   * the texture space #loc and #size.
   * \note Vertex indices should be aligned for this to work usefully.
   */
  struct Mesh *texcomesh;

  /** Texture space location and size, used for procedural coordinates when rendering. */
  float loc[3];
  float size[3];
  char texflag;

  /** Various flags used when editing the mesh. */
  char editflag;
  /** Mostly more flags used when editing or displaying the mesh. */
  uint16_t flag;

  /**
   * The angle for auto smooth in radians. `M_PI` (180 degrees) causes all edges to be smooth.
   */
  float smoothresh;

  /**
   * User-defined symmetry flag (#eMeshSymmetryType) that causes editing operations to maintain
   * symmetrical geometry. Supported by operations such as transform and weight-painting.
   */
  char symmetry;

  /** Choice between different remesh methods in the UI. */
  char remesh_mode;

  /** The length of the #mat array. */
  short totcol;

  /** Per-mesh settings for voxel remesh. */
  float remesh_voxel_size;
  float remesh_voxel_adaptivity;

  int face_sets_color_seed;
  /* Stores the initial Face Set to be rendered white. This way the overlay can be enabled by
   * default and Face Sets can be used without affecting the color of the mesh. */
  int face_sets_color_default;

  char _pad1[4];

  /**
   * Data that isn't saved in files, including caches of derived data, temporary data to improve
   * the editing experience, etc. Runtime data is created when reading files and can be accessed
   * without null checks, with the exception of some temporary meshes which should allocate and
   * free the data if they are passed to functions that expect run-time data.
   */
  // MeshRuntimeHandle *runtime;
#ifdef __cplusplus
  /**
   * Array of vertex positions (and various other data). Edges and faces are defined by indices
   * into this array.
   */
  kraken::Span<MVert> verts() const;
  /** Write access to vertex data. */
  kraken::MutableSpan<MVert> verts_for_write();
  /**
   * Array of edges, containing vertex indices. For simple triangle or quad meshes, edges could be
   * calculated from the #MPoly and #MLoop arrays, however, edges need to be stored explicitly to
   * edge domain attributes and to support loose edges that aren't connected to faces.
   */
  kraken::Span<MEdge> edges() const;
  /** Write access to edge data. */
  kraken::MutableSpan<MEdge> edges_for_write();
  /**
   * Face topology storage of the size and offset of each face's section of the face corners.
   */
  kraken::Span<MPoly> polys() const;
  /** Write access to polygon data. */
  kraken::MutableSpan<MPoly> polys_for_write();
  /**
   * Mesh face corners that "loop" around each face, storing the vertex index and the index of the
   * subsequent edge.
   */
  kraken::Span<MLoop> loops() const;
  /** Write access to loop data. */
  kraken::MutableSpan<MLoop> loops_for_write();

  // kraken::kke::AttributeAccessor attributes() const;
  // kraken::kke::MutableAttributeAccessor attributes_for_write();

  /**
   * Vertex group data, encoded as an array of indices and weights for every vertex.
   * \warning: May be empty.
   */
  kraken::Span<MDeformVert> deform_verts() const;
  /** Write access to vertex group data. */
  kraken::MutableSpan<MDeformVert> deform_verts_for_write();

  /**
   * Cached triangulation of the mesh.
   */
  kraken::Span<MLoopTri> looptris() const;
#endif
} Mesh;

typedef struct EditLatt
{
  USD_DEFINE_CXX_METHODS(EditLatt)

  struct Lattice *latt;

  int shapenr;

  /**
   * ID data is older than edit-mode data.
   * Set #Main.is_memfile_undo_flush_needed when enabling.
   */
  char needs_flush_to_id;
} EditLatt;

typedef struct Lattice
{
  USD_DEFINE_CXX_METHODS(Lattice)

  ID id;
  struct AnimData *adt;

  short pntsu, pntsv, pntsw, flag;
  short opntsu, opntsv, opntsw;
  char _pad2[3];
  char typeu, typev, typew;
  /** Active element index, unset with LT_ACTBP_NONE. */
  int actbp;

  float fu, fv, fw, du, dv, dw;

  struct KPoint *def;

  struct Key *key;

  struct MDeformVert *dvert;
  /** Multiply the influence, MAX_VGROUP_NAME. */
  char vgroup[64];
  /** List of bDeformGroup names and flag only. */
  ListBase vertex_group_names;
  int vertex_group_active_index;

  char _pad0[4];

  struct EditLatt *editlatt;
  void *batch_cache;
} Lattice;

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

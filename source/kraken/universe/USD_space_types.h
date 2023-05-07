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
 * @file
 * Universe.
 * Set the Stage.
 */

#include "KLI_utildefines.h"

#include "USD_ID.h"

#include "USD_listBase.h"
#include "USD_scene_types.h"
#include "USD_color_types.h" /* for Histogram */
#include "USD_defs.h"
#include "USD_vec_types.h"
// #include "USD_view2d.h"

/** Workaround to forward-declare C++ type in C header. */
#ifdef __cplusplus
namespace kraken
{
  template<typename T> class Span;
  class StringRef;
  class StringRefNull;
}  // namespace kraken
namespace kraken::nodes
{
  class NodeDeclaration;
  class SocketDeclaration;
}  // namespace kraken::nodes
namespace kraken::kke
{
  class kNodeTreeRuntime;
  class kNodeRuntime;
  class kNodeSocketRuntime;
}  // namespace kraken::kke
using NodeDeclarationHandle = kraken::nodes::NodeDeclaration;
using SocketDeclarationHandle = kraken::nodes::SocketDeclaration;
using kNodeTreeRuntimeHandle = kraken::kke::kNodeTreeRuntime;
using kNodeRuntimeHandle = kraken::kke::kNodeRuntime;
using kNodeSocketRuntimeHandle = kraken::kke::kNodeSocketRuntime;
#else
typedef struct NodeDeclarationHandle NodeDeclarationHandle;
typedef struct SocketDeclarationHandle SocketDeclarationHandle;
typedef struct kNodeTreeRuntimeHandle kNodeTreeRuntimeHandle;
typedef struct kNodeRuntimeHandle kNodeRuntimeHandle;
typedef struct kNodeSocketRuntimeHandle kNodeSocketRuntimeHandle;
#endif

struct AnimData;
struct Collection;
struct ID;
struct Image;
struct ListBase;
struct Material;
struct PreviewImage;
struct Tex;
struct kGPdata;
struct kNodeInstanceHash;
struct kNodeLink;
struct kNodePreview;
struct kNodeTreeExec;
struct kNodeType;
struct kNode;
struct uiBlock;

#define NODE_MAXSTR 64

/* ns->datatype, shadetree only */
#define NS_OSA_VECTORS 1
#define NS_OSA_VALUES 2

/* node socket/node socket type -b conversion rules */
#define NS_CR_CENTER 0
#define NS_CR_NONE 1
#define NS_CR_FIT_WIDTH 2
#define NS_CR_FIT_HEIGHT 3
#define NS_CR_FIT 4
#define NS_CR_STRETCH 5

typedef struct kNodeSocket
{
  struct kNodeSocket *next, *prev;

  /** User-defined properties. */
  IDProperty *prop;

  /** Unique identifier for mapping. */
  char identifier[64];

  /** MAX_NAME. */
  char name[64];

  /** Only used for the Image and OutputFile nodes, should be removed at some point. */
  void *storage;

  /**
   * The socket's data type. #eNodeSocketDatatype.
   */
  short type;
  /** #eNodeSocketFlag */
  short flag;
  /**
   * Maximum number of links that can connect to the socket. Read via #nodeSocketLinkLimit, because
   * the limit might be defined on the socket type, in which case this value does not have any
   * effect. It is necessary to store this in the socket because it is exposed as an RNA property
   * for custom nodes.
   */
  short limit;
  /** Input/output type. */
  short in_out;
  /** Runtime type information. */
  struct kNodeSocketType *typeinfo;
  /** Runtime type identifier. */
  char idname[64];

  /**
   * The location of the sockets, in the view-space of the node editor.
   * \note These are runtime data-- only calculated when drawing, and could be removed from DNA.
   */
  float locx, locy;

  /** Default input value used for unlinked sockets. */
  void *default_value;

  /** Local stack index for "node_exec". */
  short stack_index;
  char display_shape;

  /* #eAttrDomain used when the geometry nodes modifier creates an attribute for a group
   * output. */
  char attribute_domain;

  char _pad[2];

  /* Runtime-only cache of the number of input links, for multi-input sockets. */
  short total_inputs;

  /** Custom dynamic defined label, MAX_NAME. */
  char label[64];
  char description[64];

  /**
   * The default attribute name to use for geometry nodes modifier output attribute sockets.
   * \note Storing this pointer in every single socket exposes the bad design of using sockets
   * to describe group inputs and outputs. In the future, it should be stored in socket
   * declarations.
   */
  char *default_attribute_name;

  /** Cached data from execution. */
  void *cache;

  /** A link pointer, set in #BKE_ntree_update_main. */
  struct kNodeLink *link;

  kNodeSocketRuntimeHandle *runtime;

#ifdef __cplusplus
  bool is_available() const;
  bool is_multi_input() const;
  bool is_input() const;
  bool is_output() const;

  /** Utility to access the value of the socket. */
  template<typename T> const T *default_value_typed() const;

  /* The following methods are only available when #kNodeTree.ensure_topology_cache has been
   * called. */

  /** Zero based index for every input and output socket. */
  int index() const;
  /** Socket index in the entire node tree. Inputs and outputs share the same index space. */
  int index_in_tree() const;
  /** Node this socket belongs to. */
  kNode &owner_node();
  const kNode &owner_node() const;
  /** Node tree this socket belongs to. */
  const kNodeTree &owner_tree() const;

  /** Links which are incident to this socket. */
  kraken::Span<kNodeLink *> directly_linked_links();
  kraken::Span<const kNodeLink *> directly_linked_links() const;
  /** Sockets which are connected to this socket with a link. */
  kraken::Span<kNodeSocket *> directly_linked_sockets();
  kraken::Span<const kNodeSocket *> directly_linked_sockets() const;
  bool is_directly_linked() const;
  /**
   * Sockets which are connected to this socket when reroutes and muted nodes are taken into
   * account.
   */
  kraken::Span<const kNodeSocket *> logically_linked_sockets() const;
  bool is_logically_linked() const;

  /**
   * For output sockets, this is the corresponding input socket the value of which should be
   * forwarded when the node is muted.
   */
  const kNodeSocket *internal_link_input() const;

#endif
} kNodeSocket;

/** #kNodeSocket.type & #kNodeSocketType.type */
typedef enum eNodeSocketDatatype
{
  SOCK_CUSTOM = -1, /* socket has no integer type */
  SOCK_FLOAT = 0,
  SOCK_VECTOR = 1,
  SOCK_RGBA = 2,
  SOCK_SHADER = 3,
  SOCK_BOOLEAN = 4,
  __SOCK_MESH = 5, /* deprecated */
  SOCK_INT = 6,
  SOCK_STRING = 7,
  SOCK_OBJECT = 8,
  SOCK_IMAGE = 9,
  SOCK_GEOMETRY = 10,
  SOCK_COLLECTION = 11,
  SOCK_TEXTURE = 12,
  SOCK_MATERIAL = 13,
} eNodeSocketDatatype;

/** Socket shape. */
typedef enum eNodeSocketDisplayShape
{
  SOCK_DISPLAY_SHAPE_CIRCLE = 0,
  SOCK_DISPLAY_SHAPE_SQUARE = 1,
  SOCK_DISPLAY_SHAPE_DIAMOND = 2,
  SOCK_DISPLAY_SHAPE_CIRCLE_DOT = 3,
  SOCK_DISPLAY_SHAPE_SQUARE_DOT = 4,
  SOCK_DISPLAY_SHAPE_DIAMOND_DOT = 5,
} eNodeSocketDisplayShape;

/** Socket side (input/output). */
typedef enum eNodeSocketInOut
{
  SOCK_IN = 1 << 0,
  SOCK_OUT = 1 << 1,
} eNodeSocketInOut;

/** #kNodeSocket.flag, first bit is selection. */
typedef enum eNodeSocketFlag
{
  /** Hidden is user defined, to hide unused sockets. */
  SOCK_HIDDEN = (1 << 1),
  /** For quick check if socket is linked. */
  SOCK_IN_USE = (1 << 2),
  /** Unavailable is for dynamic sockets. */
  SOCK_UNAVAIL = (1 << 3),
  // /** DEPRECATED  dynamic socket (can be modified by user) */
  // SOCK_DYNAMIC = (1 << 4),
  // /** DEPRECATED  group socket should not be exposed */
  // SOCK_INTERNAL = (1 << 5),
  /** Socket collapsed in UI. */
  SOCK_COLLAPSED = (1 << 6),
  /** Hide socket value, if it gets auto default. */
  SOCK_HIDE_VALUE = (1 << 7),
  /** Socket hidden automatically, to distinguish from manually hidden. */
  SOCK_AUTO_HIDDEN__DEPRECATED = (1 << 8),
  SOCK_NO_INTERNAL_LINK = (1 << 9),
  /** Draw socket in a more compact form. */
  SOCK_COMPACT = (1 << 10),
  /** Make the input socket accept multiple incoming links in the UI. */
  SOCK_MULTI_INPUT = (1 << 11),
  /**
   * Don't show the socket's label in the interface, for situations where the
   * type is obvious and the name takes up too much space.
   */
  SOCK_HIDE_LABEL = (1 << 12),
} eNodeSocketFlag;

/** TODO: Limit data in #kNode to what we want to see saved. */
typedef struct kNode
{
  struct kNode *next, *prev;

  /** User-defined properties. */
  IDProperty *prop;

  /** Runtime type information. */
  struct kNodeType *typeinfo;
  /** Runtime type identifier. */
  char idname[64];

  /** MAX_NAME. */
  char name[64];
  int flag;
  short type;
  /** Both for dependency and sorting. */
  short done, level;

  /** Used as a boolean for execution. */
  uint8_t need_exec;
  char _pad2[1];

  /** Custom user-defined color. */
  float color[3];

  ListBase inputs, outputs;
  /** Parent node. */
  struct kNode *parent;
  /** Optional link to libdata. */
  struct ID *id;
  /** Custom data, must be struct, for storage in file. */
  void *storage;
  /** The original node in the tree (for localized tree). */
  struct kNode *original;
  /** List of cached internal links (input to output), for muted nodes and operators. */
  ListBase internal_links;

  /** Root offset for drawing (parent space). */
  float locx, locy;
  /** Node custom width and height. */
  float width, height;
  /** Node width if hidden. */
  float miniwidth;
  /** Additional offset from loc. */
  float offsetx, offsety;
  /** Initial locx for insert offset animation. */
  float anim_init_locx;
  /** Offset that will be added to locx for insert offset animation. */
  float anim_ofsx;

  /** Update flags. */
  int update;

  /** Custom user-defined label, MAX_NAME. */
  char label[64];
  /** To be abused for buttons. */
  short custom1, custom2;
  float custom3, custom4;

  char _pad1[4];

  /** Entire bound-box (world-space). */
  rctf totr;
  /** Optional preview area. */
  rctf prvr;
  /**
   * XXX TODO
   * Node totr size depends on the prvr size, which in turn is determined from preview size.
   * In earlier versions kNodePreview was stored directly in nodes, but since now there can be
   * multiple instances using different preview images it is possible that required node size
   * varies between instances. preview_xsize, preview_ysize defines a common reserved size for
   * preview rect for now, could be replaced by more accurate node instance drawing,
   * but that requires removing totr from DNA and replacing all uses with per-instance data.
   */
  /** Reserved size of the preview rect. */
  short preview_xsize, preview_ysize;
  /** Used at runtime when going through the tree. Initialize before use. */
  short tmp_flag;

  char _pad0;
  /** Used at runtime when iterating over node branches. */
  char iter_flag;

  kNodeRuntimeHandle *runtime;

#ifdef __cplusplus
  kraken::StringRefNull label_or_name() const;
  bool is_muted() const;
  bool is_reroute() const;
  bool is_frame() const;
  bool is_group() const;
  bool is_group_input() const;
  bool is_group_output() const;
  const kraken::nodes::NodeDeclaration *declaration() const;

  /* The following methods are only available when #kNodeTree.ensure_topology_cache has been
   * called. */

  /** A span containing all input sockets of the node (including unavailable sockets). */
  kraken::Span<kNodeSocket *> input_sockets();
  kraken::Span<const kNodeSocket *> input_sockets() const;
  /** A span containing all output sockets of the node (including unavailable sockets). */
  kraken::Span<kNodeSocket *> output_sockets();
  kraken::Span<const kNodeSocket *> output_sockets() const;
  /** Utility to get an input socket by its index. */
  kNodeSocket &input_socket(int index);
  const kNodeSocket &input_socket(int index) const;
  /** Utility to get an output socket by its index. */
  kNodeSocket &output_socket(int index);
  const kNodeSocket &output_socket(int index) const;
  /** A span containing all internal links when the node is muted. */
  kraken::Span<const kNodeLink *> internal_links_span() const;
  /** Lookup socket of this node by its identifier. */
  const kNodeSocket &input_by_identifier(kraken::StringRef identifier) const;
  const kNodeSocket &output_by_identifier(kraken::StringRef identifier) const;
  /** Node tree this node belongs to. */
  const kNodeTree &owner_tree() const;
#endif
} kNode;

/* node->flag */
#define NODE_SELECT 1
#define NODE_OPTIONS 2
#define NODE_PREVIEW 4
#define NODE_HIDDEN 8
#define NODE_ACTIVE 16
// #define NODE_ACTIVE_ID 32 /* deprecated */
/* Used to indicate which group output node is used and which viewer node is active. */
#define NODE_DO_OUTPUT 64
#define __NODE_GROUP_EDIT 128 /* DEPRECATED */
/* free test flag, undefined */
#define NODE_TEST 256
/* node is disabled */
#define NODE_MUTED 512
// #define NODE_CUSTOM_NAME 1024    /* deprecated! */
// #define NODE_CONST_OUTPUT (1 << 11) /* deprecated */
/* node is always behind others */
#define NODE_BACKGROUND (1 << 12)
/* automatic flag for nodes included in transforms */
#define NODE_TRANSFORM (1 << 13)
/* node is active texture */

/* NOTE: take care with this flag since its possible it gets
 * `stuck` inside/outside the active group - which makes buttons
 * window texture not update, we try to avoid it by clearing the
 * flag when toggling group editing - Campbell */
#define NODE_ACTIVE_TEXTURE (1 << 14)
/* use a custom color for the node */
#define NODE_CUSTOM_COLOR (1 << 15)
/* Node has been initialized
 * This flag indicates the node->typeinfo->init function has been called.
 * In case of undefined type at creation time this can be delayed until
 * until the node type is registered.
 */
#define NODE_INIT (1 << 16)

/* do recalc of output, used to skip recalculation of unwanted
 * composite out nodes when editing tree
 */
#define NODE_DO_OUTPUT_RECALC (1 << 17)
/* A preview for the data in this node can be displayed in the spreadsheet editor. */
#define __NODE_ACTIVE_PREVIEW (1 << 18) /* deprecated */
/* Active node that is used to paint on. */
#define NODE_ACTIVE_PAINT_CANVAS (1 << 19)

/* node->update */
#define NODE_UPDATE_ID 1       /* associated id data block has changed */
#define NODE_UPDATE_OPERATOR 2 /* node update triggered from update operator */

/* Unique hash key for identifying node instances
 * Defined as a struct because DNA does not support other typedefs.
 */
typedef struct kNodeInstanceKey
{
  unsigned int value;
} kNodeInstanceKey;

/* the basis for a Node tree, all links and nodes reside internal here */
/* only re-usable node trees are in the library though,
 * materials and textures allocate own tree struct */
typedef struct kNodeTree
{
  ID id;
  /** Animation data (must be immediately after id for utilities to use it). */
  struct AnimData *adt;

  /** The ID owning this node tree, in case it is an embedded one. */
  ID *owner_id;

  /** Runtime type information. */
  struct kNodeTreeType *typeinfo;
  /** Runtime type identifier. */
  char idname[64];

  /** Runtime RNA type of the group interface. */
  struct KrakenPRIM *interface_type;

  /** Grease pencil data. */
  struct bGPdata *gpd;
  /** Node tree stores own offset for consistent editor view. */
  float view_center[2];

  ListBase nodes, links;

  int type;

  /**
   * Sockets in groups have unique identifiers, adding new sockets always
   * will increase this counter.
   */
  int cur_index;
  int flag;
  /** Flag to prevent re-entrant update calls. */
  short is_updating;
  /** Generic temporary flag for recursion check (DFS/BFS). */
  short done;

  /** Quality setting when editing. */
  short edit_quality;
  /** Quality setting when rendering. */
  short render_quality;
  /** Tile size for compositor engine. */
  int chunksize;
  /** Execution mode to use for compositor engine. */
  int execution_mode;

  rctf viewer_border;

  /* Lists of kNodeSocket to hold default values and own_index.
   * Warning! Don't make links to these sockets, input/output nodes are used for that.
   * These sockets are used only for generating external interfaces.
   */
  ListBase inputs, outputs;

  /* Node preview hash table
   * Only available in base node trees (e.g. scene->node_tree)
   */
  struct kNodeInstanceHash *previews;
  /* Defines the node tree instance to use for the "active" context,
   * in case multiple different editors are used and make context ambiguous.
   */
  kNodeInstanceKey active_viewer_key;

  char _pad[4];

  /** Execution data.
   *
   * XXX It would be preferable to completely move this data out of the underlying node tree,
   * so node tree execution could finally run independent of the tree itself.
   * This would allow node trees to be merely linked by other data (materials, textures, etc.),
   * as ID data is supposed to.
   * Execution data is generated from the tree once at execution start and can then be used
   * as long as necessary, even while the tree is being modified.
   */
  struct kNodeTreeExec *execdata;

  /* Callbacks. */
  void (*progress)(void *, float progress);
  /** \warning may be called by different threads */
  void (*stats_draw)(void *, const char *str);
  int (*test_break)(void *);
  void (*update_draw)(void *);
  void *tbh, *prh, *sdh, *udh;

  /** Image representing what the node group does. */
  struct PreviewImage *preview;

  kNodeTreeRuntimeHandle *runtime;

#ifdef __cplusplus
  /**
   * Update a run-time cache for the node tree based on it's current state. This makes many methods
   * available which allow efficient lookup for topology information (like neighboring sockets).
   */
  void ensure_topology_cache() const;

  /* The following methods are only available when #kNodeTree.ensure_topology_cache has been
   * called. */

  /** A span containing all nodes in the node tree. */
  kraken::Span<kNode *> all_nodes();
  kraken::Span<const kNode *> all_nodes() const;
  /** A span containing all group nodes in the node tree. */
  kraken::Span<kNode *> group_nodes();
  kraken::Span<const kNode *> group_nodes() const;
  /** A span containing all input sockets in the node tree. */
  kraken::Span<kNodeSocket *> all_input_sockets();
  kraken::Span<const kNodeSocket *> all_input_sockets() const;
  /** A span containing all output sockets in the node tree. */
  kraken::Span<kNodeSocket *> all_output_sockets();
  kraken::Span<const kNodeSocket *> all_output_sockets() const;
  /** A span containing all sockets in the node tree. */
  kraken::Span<kNodeSocket *> all_sockets();
  kraken::Span<const kNodeSocket *> all_sockets() const;
  /** Efficient lookup of all nodes with a specific type. */
  kraken::Span<kNode *> nodes_by_type(kraken::StringRefNull type_idname);
  kraken::Span<const kNode *> nodes_by_type(kraken::StringRefNull type_idname) const;
  /**
   * Cached toposort of all nodes. If there are cycles, the returned array is not actually a
   * toposort. However, if a connected component does not contain a cycle, this component is sorted
   * correctly. Use #has_available_link_cycle to check for cycles.
   */
  kraken::Span<const kNode *> toposort_left_to_right() const;
  kraken::Span<const kNode *> toposort_right_to_left() const;
  /** True when there are any cycles in the node tree. */
  bool has_available_link_cycle() const;
  /**
   * True when there are nodes or sockets in the node tree that don't use a known type. This can
   * happen when nodes don't exist in the current kraken version that existed in the version where
   * this node tree was saved.
   */
  bool has_undefined_nodes_or_sockets() const;
  /** Get the active group output node. */
  const kNode *group_output_node() const;
#endif
} kNodeTree;

typedef struct kNodeTreePath
{
  struct kNodeTreePath *next, *prev;

  struct kNodeTree *nodetree;
  /** Base key for nodes in this tree instance. */
  // kNodeInstanceKey parent_key;
  char _pad[4];
  /** V2d center point, so node trees can have different offsets in editors. */
  float view_center[2];

  /** MAX_NAME. */
  char node_name[64];
  char display_name[64];
} kNodeTreePath;

#ifdef __cplusplus
extern "C" {
#endif

struct KLI_mempool;
struct FileLayout;
struct FileList;
struct FileSelectParams;
struct Histogram;
struct ID;
struct Image;
struct Mask;
struct MovieClip;
struct MovieClipScopes;
struct Scopes;
struct Script;
struct SpaceGraph;
struct Text;
struct kDopeSheet;
struct kGPdata;
struct kNodeTree;
struct wmOperator;
struct wmTimer;


/** Defined in `buttons_intern.h`. */
typedef struct SpaceProperties_Runtime SpaceProperties_Runtime;

#ifdef __cplusplus
namespace kraken::ed::space_node
{
  struct SpaceNode_Runtime;
}  // namespace kraken::ed::space_node
using SpaceNode_Runtime = kraken::ed::space_node::SpaceNode_Runtime;

namespace kraken::ed::outliner
{
  struct SpaceOutliner_Runtime;
}  // namespace kraken::ed::outliner
using SpaceOutliner_Runtime = kraken::ed::outliner::SpaceOutliner_Runtime;
#else
typedef struct SpaceNode_Runtime SpaceNode_Runtime;
typedef struct SpaceOutliner_Runtime SpaceOutliner_Runtime;
#endif

/** Defined in `file_intern.h`. */
typedef struct SpaceFile_Runtime SpaceFile_Runtime;

/** Defined in `spreadsheet_intern.hh`. */
typedef struct SpaceSpreadsheet_Runtime SpaceSpreadsheet_Runtime;

/** #Sequence.color_tag. */
typedef enum SequenceColorTag
{
  SEQUENCE_COLOR_NONE = -1,
  SEQUENCE_COLOR_01,
  SEQUENCE_COLOR_02,
  SEQUENCE_COLOR_03,
  SEQUENCE_COLOR_04,
  SEQUENCE_COLOR_05,
  SEQUENCE_COLOR_06,
  SEQUENCE_COLOR_07,
  SEQUENCE_COLOR_08,
  SEQUENCE_COLOR_09,

  SEQUENCE_COLOR_TOT,
} SequenceColorTag;

typedef enum eRegionType
{
  RGN_TYPE_WINDOW = 0,
  RGN_TYPE_HEADER = 1,
  RGN_TYPE_CHANNELS = 2,
  RGN_TYPE_TEMPORARY = 3,
  RGN_TYPE_UI = 4,
  RGN_TYPE_TOOLS = 5,
  RGN_TYPE_TOOL_PROPS = 6,
  RGN_TYPE_PREVIEW = 7,
  RGN_TYPE_HUD = 8,
  /* Region to navigate the main region from (RGN_TYPE_WINDOW). */
  RGN_TYPE_NAV_BAR = 9,
  /* A place for buttons to trigger execution of something that was set up in other regions. */
  RGN_TYPE_EXECUTE = 10,
  RGN_TYPE_FOOTER = 11,
  RGN_TYPE_TOOL_HEADER = 12,
  RGN_TYPE_XR = 13,

#define RGN_TYPE_NUM (RGN_TYPE_XR + 1)
} eRegionType;

#define RGN_TYPE_ANY -1

/* Region supports panel tabs (categories). */
#define RGN_TYPE_HAS_CATEGORY_MASK (1 << RGN_TYPE_UI)

/* Check for any kind of header region. */
#define RGN_TYPE_IS_HEADER_ANY(regiontype) \
  (((1 << (regiontype)) &                  \
    ((1 << RGN_TYPE_HEADER) | 1 << (RGN_TYPE_TOOL_HEADER) | (1 << RGN_TYPE_FOOTER))) != 0)


#define RGN_ALIGN_ENUM_FROM_MASK(align) ((align) & ((1 << 4) - 1))
#define RGN_ALIGN_FLAG_FROM_MASK(align) ((align) & ~((1 << 4) - 1))

enum
{
  RGN_FLAG_HIDDEN = (1 << 0),
  RGN_FLAG_TOO_SMALL = (1 << 1),
  RGN_FLAG_DYNAMIC_SIZE = (1 << 2),
  RGN_FLAG_TEMP_REGIONDATA = (1 << 3),
  RGN_FLAG_PREFSIZE_OR_HIDDEN = (1 << 4),
  RGN_FLAG_SIZE_CLAMP_X = (1 << 5),
  RGN_FLAG_SIZE_CLAMP_Y = (1 << 6),
  RGN_FLAG_HIDDEN_BY_USER = (1 << 7),
  RGN_FLAG_SEARCH_FILTER_ACTIVE = (1 << 8),
  RGN_FLAG_SEARCH_FILTER_UPDATE = (1 << 9),
};


/** #ARegion.alignment */
enum
{
  RGN_ALIGN_NONE = 0,
  RGN_ALIGN_TOP = 1,
  RGN_ALIGN_BOTTOM = 2,
  RGN_ALIGN_LEFT = 3,
  RGN_ALIGN_RIGHT = 4,
  RGN_ALIGN_HSPLIT = 5,
  RGN_ALIGN_VSPLIT = 6,
  RGN_ALIGN_FLOAT = 7,
  RGN_ALIGN_QSPLIT = 8,
  /* Maximum 15. */

  /* Flags start here. */
  RGN_SPLIT_PREV = 32,
};


/* -------------------------------------------------------------------- */
/** \name SpaceLink (Base)
 * \{ */

/**
 * The base structure all the other spaces
 * are derived (implicitly) from. Would be
 * good to make this explicit.
 */
typedef struct SpaceLink
{
  struct SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
} SpaceLink;

/** #SpaceLink.link_flag */
enum
{
  /**
   * The space is not a regular one opened through the editor menu (for example) but spawned by an
   * operator to fulfill some task and then disappear again.
   * Can typically be cancelled using Escape, but that is handled on the editor level. */
  SPACE_FLAG_TYPE_TEMPORARY = (1 << 0),
  /**
   * Used to mark a space as active but "overlapped" by temporary full-screen spaces. Without this
   * we wouldn't be able to restore the correct active space after closing temp full-screens
   * reliably if the same space type is opened twice in a full-screen stack (see T19296). We don't
   * actually open the same space twice, we have to pretend it is by managing area order carefully.
   */
  SPACE_FLAG_TYPE_WAS_ACTIVE = (1 << 1),
};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Space Info
 * \{ */

/** Info Header. */
typedef struct SpaceInfo
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  char rpt_mask;
  char _pad[7];
} SpaceInfo;

/** #SpaceInfo.rpt_mask */
typedef enum eSpaceInfo_RptMask
{
  INFO_RPT_DEBUG = (1 << 0),
  INFO_RPT_INFO = (1 << 1),
  INFO_RPT_OP = (1 << 2),
  INFO_RPT_WARN = (1 << 3),
  INFO_RPT_ERR = (1 << 4),
} eSpaceInfo_RptMask;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Properties Editor
 * \{ */

/** Properties Editor. */
typedef struct SpaceProperties
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /* For different kinds of property editors (exposed in the space type selector). */
  short space_subtype;

  /** Context tabs. */
  short mainb, mainbo, mainbuser;
  /** Preview is signal to refresh. */
  short preview;
  char _pad[4];
  char flag;

  /* eSpaceButtons_OutlinerSync */
  char outliner_sync;

  /** Runtime. */
  void *path;
  /** Runtime. */
  int pathflag, dataicon;
  ID *pinid;

  void *texuser;

  /* Doesn't necessarily need to be a pointer, but runtime structs are still written to files. */
  struct SpaceProperties_Runtime *runtime;
} SpaceProperties;

/** #SpaceProperties.mainb new */
typedef enum eSpaceButtons_Context
{
  KCONTEXT_RENDER = 0,
  KCONTEXT_SCENE = 1,
  KCONTEXT_WORLD = 2,
  KCONTEXT_OBJECT = 3,
  KCONTEXT_DATA = 4,
  KCONTEXT_MATERIAL = 5,
  KCONTEXT_TEXTURE = 6,
  KCONTEXT_PARTICLE = 7,
  KCONTEXT_PHYSICS = 8,
  KCONTEXT_BONE = 9,
  KCONTEXT_MODIFIER = 10,
  KCONTEXT_CONSTRAINT = 11,
  KCONTEXT_BONE_CONSTRAINT = 12,
  KCONTEXT_VIEW_LAYER = 13,
  KCONTEXT_TOOL = 14,
  KCONTEXT_SHADERFX = 15,
  KCONTEXT_OUTPUT = 16,
  KCONTEXT_COLLECTION = 17,

  /* Keep last. */
  KCONTEXT_TOT,
} eSpaceButtons_Context;

/** #SpaceProperties.flag */
typedef enum eSpaceButtons_Flag
{
  /* SB_PRV_OSA = (1 << 0), */ /* UNUSED */
  SB_PIN_CONTEXT = (1 << 1),
  SB_FLAG_UNUSED_2 = (1 << 2),
  SB_FLAG_UNUSED_3 = (1 << 3),
  /** Do not add materials, particles, etc. in TemplateTextureUser list. */
  SB_TEX_USER_LIMITED = (1 << 3),
  SB_SHADING_CONTEXT = (1 << 4),
} eSpaceButtons_Flag;

/** #SpaceProperties.outliner_sync */
typedef enum eSpaceButtons_OutlinerSync
{
  PROPERTIES_SYNC_AUTO = 0,
  PROPERTIES_SYNC_NEVER = 1,
  PROPERTIES_SYNC_ALWAYS = 2,
} eSpaceButtons_OutlinerSync;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Outliner
 * \{ */

/** Outliner */
typedef struct SpaceOutliner
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  ListBase tree;

  /* treestore is an ordered list of TreeStoreElem's from outliner tree;
   * Note that treestore may contain duplicate elements if element
   * is used multiple times in outliner tree (e. g. linked objects)
   * Also note that KLI_mempool can not be read/written in USD directly,
   * therefore `readfile.c/writefile.c` linearize treestore into TreeStore structure
   */
  struct KLI_mempool *treestore;

  char search_string[64];

  short flag;
  short outlinevis;
  short lib_override_view_mode;
  short storeflag;
  char search_flags;
  char _pad[6];

  /** Selection syncing flag (#WM_OUTLINER_SYNC_SELECT_FROM_OBJECT and similar flags). */
  char sync_select_dirty;

  int filter;
  char filter_state;
  char show_restrict_flags;
  short filter_id_type;

  SpaceOutliner_Runtime *runtime;
} SpaceOutliner;

/** #SpaceOutliner.flag */
typedef enum eSpaceOutliner_Flag
{
  /* SO_TESTBLOCKS = (1 << 0), */         /* UNUSED */
  /* SO_NEWSELECTED = (1 << 1), */        /* UNUSED */
  SO_FLAG_UNUSED_1 = (1 << 2),            /* cleared */
  /* SO_HIDE_KEYINGSETINFO = (1 << 3), */ /* UNUSED */
  SO_SKIP_SORT_ALPHA = (1 << 4),
  SO_SYNC_SELECT = (1 << 5),
  SO_MODE_COLUMN = (1 << 6),
} eSpaceOutliner_Flag;

/** #SpaceOutliner.filter */
typedef enum eSpaceOutliner_Filter
{
  SO_FILTER_SEARCH = (1 << 0), /* Run-time flag. */
  SO_FILTER_CLEARED_1 = (1 << 1),
  SO_FILTER_NO_OBJECT = (1 << 2),
  SO_FILTER_NO_OB_CONTENT = (1 << 3), /* Not only mesh, but modifiers, constraints, ... */
  SO_FILTER_NO_CHILDREN = (1 << 4),

  SO_FILTER_UNUSED_5 = (1 << 5), /* cleared */
  /** Show overrides that are defined/controlled by kraken. */
  SO_FILTER_SHOW_SYSTEM_OVERRIDES = SO_FILTER_UNUSED_5, /* re-use */
  SO_FILTER_NO_OB_MESH = (1 << 6),
  SO_FILTER_NO_OB_ARMATURE = (1 << 7),
  SO_FILTER_NO_OB_EMPTY = (1 << 8),
  SO_FILTER_NO_OB_LAMP = (1 << 9),
  SO_FILTER_NO_OB_CAMERA = (1 << 10),
  SO_FILTER_NO_OB_OTHERS = (1 << 11),

  SO_FILTER_OB_STATE_SELECTABLE = (1 << 12), /* Not set via USD. */
  SO_FILTER_OB_STATE_VISIBLE = (1 << 13),    /* Not set via USD. */
  SO_FILTER_OB_STATE_INVERSE = (1 << 14),
  SO_FILTER_OB_STATE_SELECTED = (1 << 15), /* Not set via USD. */
  SO_FILTER_OB_STATE_ACTIVE = (1 << 16),   /* Not set via USD. */
  SO_FILTER_NO_COLLECTION = (1 << 17),
  SO_FILTER_NO_VIEW_LAYERS = (1 << 18),

  SO_FILTER_ID_TYPE = (1 << 19),
} eSpaceOutliner_Filter;

#define SO_FILTER_OB_TYPE                                                    \
  (SO_FILTER_NO_OB_MESH | SO_FILTER_NO_OB_ARMATURE | SO_FILTER_NO_OB_EMPTY | \
   SO_FILTER_NO_OB_LAMP | SO_FILTER_NO_OB_CAMERA | SO_FILTER_NO_OB_OTHERS)

#define SO_FILTER_OB_STATE                                                                \
  (SO_FILTER_OB_STATE_VISIBLE | SO_FILTER_OB_STATE_SELECTED | SO_FILTER_OB_STATE_ACTIVE | \
   SO_FILTER_OB_STATE_SELECTABLE)

#define SO_FILTER_ANY                                                                         \
  (SO_FILTER_NO_OB_CONTENT | SO_FILTER_NO_CHILDREN | SO_FILTER_OB_TYPE | SO_FILTER_OB_STATE | \
   SO_FILTER_NO_COLLECTION | SO_FILTER_NO_VIEW_LAYERS)

/** #SpaceOutliner.filter_state */
typedef enum eSpaceOutliner_StateFilter
{
  SO_FILTER_OB_ALL = 0,
  SO_FILTER_OB_VISIBLE = 1,
  SO_FILTER_OB_HIDDEN = 2, /* deprecated */
  SO_FILTER_OB_SELECTED = 3,
  SO_FILTER_OB_ACTIVE = 4,
  SO_FILTER_OB_SELECTABLE = 5,
} eSpaceOutliner_StateFilter;

/** #SpaceOutliner.show_restrict_flags */
typedef enum eSpaceOutliner_ShowRestrictFlag
{
  SO_RESTRICT_ENABLE = (1 << 0),
  SO_RESTRICT_SELECT = (1 << 1),
  SO_RESTRICT_HIDE = (1 << 2),
  SO_RESTRICT_VIEWPORT = (1 << 3),
  SO_RESTRICT_RENDER = (1 << 4),
  SO_RESTRICT_HOLDOUT = (1 << 5),
  SO_RESTRICT_INDIRECT_ONLY = (1 << 6),
} eSpaceOutliner_Restrict;

/** #SpaceOutliner.outlinevis */
typedef enum eSpaceOutliner_Mode
{
  SO_SCENES = 0,
  /* SO_CUR_SCENE      = 1, */ /* deprecated! */
  /* SO_VISIBLE        = 2, */ /* deprecated! */
  /* SO_SELECTED       = 3, */ /* deprecated! */
  /* SO_ACTIVE         = 4, */ /* deprecated! */
  /* SO_SAME_TYPE      = 5, */ /* deprecated! */
  /* SO_GROUPS         = 6, */ /* deprecated! */
  SO_LIBRARIES = 7,
  /* SO_VERSE_SESSION  = 8, */ /* deprecated! */
  /* SO_VERSE_MS       = 9, */ /* deprecated! */
  SO_SEQUENCE = 10,
  SO_DATA_API = 11,
  /* SO_USERDEF        = 12, */ /* deprecated! */
  /* SO_KEYMAP         = 13, */ /* deprecated! */
  SO_ID_ORPHANS = 14,
  SO_VIEW_LAYER = 15,
  SO_OVERRIDES_LIBRARY = 16,
} eSpaceOutliner_Mode;

/** #SpaceOutliner.outlinevis */
typedef enum eSpaceOutliner_LibOverrideViewMode
{
  /** View all overrides with RNA buttons to edit the overridden values. */
  SO_LIB_OVERRIDE_VIEW_PROPERTIES = 0,
  /** View entire override hierarchies (relationships between overridden data-blocks). */
  SO_LIB_OVERRIDE_VIEW_HIERARCHIES = 1,
} eSpaceOutliner_LibOverrideViewMode;

/** #SpaceOutliner.storeflag */
typedef enum eSpaceOutliner_StoreFlag
{
  /* cleanup tree */
  SO_TREESTORE_CLEANUP = (1 << 0),
  SO_TREESTORE_UNUSED_1 = (1 << 1), /* cleared */
  /** Rebuild the tree, similar to cleanup, but defer a call to
   * kke::outliner::treehash::rebuild_from_treestore instead. */
  SO_TREESTORE_REBUILD = (1 << 2),
} eSpaceOutliner_StoreFlag;

/** Outliner search flags (#SpaceOutliner.search_flags) */
typedef enum eSpaceOutliner_Search_Flags
{
  SO_FIND_CASE_SENSITIVE = (1 << 0),
  SO_FIND_COMPLETE = (1 << 1),
  SO_SEARCH_RECURSIVE = (1 << 2),
} eSpaceOutliner_Search_Flags;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Graph Editor
 * \{ */

typedef struct SpaceGraph_Runtime
{
  /** #eGraphEdit_Runtime_Flag */
  char flag;
  char _pad[7];
  /** Sampled snapshots of F-Curves used as in-session guides */
  ListBase ghost_curves;
} SpaceGraph_Runtime;

/** 'Graph' Editor (formerly known as the IPO Editor). */
typedef struct SpaceGraph
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /** Settings for filtering animation data
   * \note we use a pointer due to code-linking issues. */
  struct bDopeSheet *ads;

  /** Mode for the Graph editor (eGraphEdit_Mode). */
  short mode;
  /**
   * Time-transform auto-snapping settings for Graph editor
   * (eAnimEdit_AutoSnap in USD_action_types.h).
   */
  short autosnap;
  /** Settings for Graph editor (eGraphEdit_Flag). */
  int flag;

  /** Time value for cursor (when in drivers mode; animation uses current frame). */
  float cursorTime;
  /** Cursor value (y-value, x-value is current frame). */
  float cursorVal;
  /** Pivot point for transforms. */
  int around;
  char _pad[4];

  struct SpaceGraph_Runtime runtime;
} SpaceGraph;

/** #SpaceGraph.flag (Graph Editor Settings) */
typedef enum eGraphEdit_Flag
{
  /* OLD DEPRECATED SETTING */
  /* SIPO_LOCK_VIEW            = (1 << 0), */

  /* don't merge keyframes on the same frame after a transform */
  SIPO_NOTRANSKEYCULL = (1 << 1),
  /* don't show any keyframe handles at all */
  SIPO_NOHANDLES = (1 << 2),
  /* SIPO_NODRAWCFRANUM = (1 << 3), DEPRECATED */
  /* show timing in seconds instead of frames */
  SIPO_DRAWTIME = (1 << 4),
  /* only show keyframes for selected F-Curves */
  SIPO_SELCUVERTSONLY = (1 << 5),
  /* draw names of F-Curves beside the respective curves */
  /* NOTE: currently not used */
  /* SIPO_DRAWNAMES = (1 << 6), */ /* UNUSED */
  /* show sliders in channels list */
  SIPO_SLIDERS = (1 << 7),
  /* don't show the horizontal component of the cursor */
  SIPO_NODRAWCURSOR = (1 << 8),
  /* only show handles of selected keyframes */
  SIPO_SELVHANDLESONLY = (1 << 9),
  /* don't perform realtime updates */
  SIPO_NOREALTIMEUPDATES = (1 << 11),
  /* don't draw curves with AA ("beauty-draw") for performance */
  SIPO_BEAUTYDRAW_OFF = (1 << 12),
  /* draw grouped channels with colors set in group */
  /* SIPO_NODRAWGCOLORS = (1 << 13), DEPRECATED */
  /* normalize curves on display */
  SIPO_NORMALIZE = (1 << 14),
  SIPO_NORMALIZE_FREEZE = (1 << 15),
  /* show markers region */
  SIPO_SHOW_MARKERS = (1 << 16),
  SIPO_NO_DRAW_EXTRAPOLATION = (1 << 17),
} eGraphEdit_Flag;

/** #SpaceGraph.mode (Graph Editor Mode) */
typedef enum eGraphEdit_Mode
{
  /* all animation curves (from all over kraken) */
  SIPO_MODE_ANIMATION = 0,
  /* drivers only */
  SIPO_MODE_DRIVERS = 1,
} eGraphEdit_Mode;

typedef enum eGraphEdit_Runtime_Flag
{
  /** Temporary flag to force channel selections to be synced with main. */
  SIPO_RUNTIME_FLAG_NEED_CHAN_SYNC = (1 << 0),
  /** Temporary flag to force F-Curves to recalculate colors. */
  SIPO_RUNTIME_FLAG_NEED_CHAN_SYNC_COLOR = (1 << 1),

  /**
   * These flags are for the mouse-select code to communicate with the transform code. Click
   * dragging (tweaking) a handle sets the according left/right flag which transform code uses then
   * to limit translation to this side. */
  SIPO_RUNTIME_FLAG_TWEAK_HANDLES_LEFT = (1 << 2),
  SIPO_RUNTIME_FLAG_TWEAK_HANDLES_RIGHT = (1 << 3),
} eGraphEdit_Runtime_Flag;

/** \} */

/* -------------------------------------------------------------------- */
/** \name NLA Editor
 * \{ */

/** NLA Editor */
typedef struct SpaceNla
{
  struct SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /** This uses the same settings as autosnap for Action Editor. */
  short autosnap;
  short flag;
  char _pad[4];

  struct bDopeSheet *ads;
} SpaceNla;

/** #SpaceNla.flag */
typedef enum eSpaceNla_Flag
{
  SNLA_FLAG_UNUSED_0 = (1 << 0),
  SNLA_FLAG_UNUSED_1 = (1 << 1),
  /* draw timing in seconds instead of frames */
  SNLA_DRAWTIME = (1 << 2),
  SNLA_FLAG_UNUSED_3 = (1 << 3),
  /* SNLA_NODRAWCFRANUM = (1 << 4), DEPRECATED */
  /* don't draw influence curves on strips */
  SNLA_NOSTRIPCURVES = (1 << 5),
  /* don't perform realtime updates */
  SNLA_NOREALTIMEUPDATES = (1 << 6),
  /* don't show local strip marker indications */
  SNLA_NOLOCALMARKERS = (1 << 7),
  /* show markers region */
  SNLA_SHOW_MARKERS = (1 << 8),
} eSpaceNla_Flag;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Sequence Editor
 * \{ */

typedef struct SequencerPreviewOverlay
{
  int flag;
  char _pad0[4];
} SequencerPreviewOverlay;

/** #SequencerPreviewOverlay.flag */
typedef enum eSpaceSeq_SequencerPreviewOverlay_Flag
{
  SEQ_PREVIEW_SHOW_2D_CURSOR = (1 << 1),
  SEQ_PREVIEW_SHOW_OUTLINE_SELECTED = (1 << 2),
  SEQ_PREVIEW_SHOW_SAFE_MARGINS = (1 << 3),
  SEQ_PREVIEW_SHOW_GPENCIL = (1 << 4),
  SEQ_PREVIEW_SHOW_SAFE_CENTER = (1 << 9),
  SEQ_PREVIEW_SHOW_METADATA = (1 << 10),
} eSpaceSeq_SequencerPreviewOverlay_Flag;

typedef struct SequencerTimelineOverlay
{
  int flag;
  char _pad0[4];
} SequencerTimelineOverlay;

/** #SequencerTimelineOverlay.flag */
typedef enum eSpaceSeq_SequencerTimelineOverlay_Flag
{
  SEQ_TIMELINE_SHOW_STRIP_OFFSETS = (1 << 1),
  SEQ_TIMELINE_SHOW_THUMBNAILS = (1 << 2),
  SEQ_TIMELINE_SHOW_STRIP_COLOR_TAG = (1 << 3), /* use Sequence->color_tag */
  SEQ_TIMELINE_SHOW_FCURVES = (1 << 5),
  SEQ_TIMELINE_ALL_WAVEFORMS = (1 << 7), /* draw all waveforms */
  SEQ_TIMELINE_NO_WAVEFORMS = (1 << 8),  /* draw no waveforms */
  SEQ_TIMELINE_SHOW_STRIP_NAME = (1 << 14),
  SEQ_TIMELINE_SHOW_STRIP_SOURCE = (1 << 15),
  SEQ_TIMELINE_SHOW_STRIP_DURATION = (1 << 16),
  SEQ_TIMELINE_SHOW_GRID = (1 << 18),
} eSpaceSeq_SequencerTimelineOverlay_Flag;

typedef struct SpaceSeqRuntime
{
  /** Required for Thumbnail job start condition. */
  struct rctf last_thumbnail_area;
  /** Stores lists of most recently displayed thumbnails. */
  struct GHash *last_displayed_thumbnails;
  int rename_channel_index;
  float timeline_clamp_custom_range;
} SpaceSeqRuntime;

/** Sequencer. */
typedef struct SpaceSeq
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /** Weird name for the sequencer subtype (seq, image, luma... etc). */
  short mainb;
  /** ESpaceSeq_Proxy_RenderSize. */
  short render_size;
  short chanshown;
  short zebra;
  int flag;

  /** See SEQ_VIEW_* below. */
  char view;
  char overlay_frame_type;
  /** Overlay an image of the editing on below the strips. */
  char draw_flag;
  char gizmo_flag;
  char _pad[4];

  /** 2D cursor for transform. */
  float cursor[2];

  /** Grease-pencil data. */
  struct bGPdata *gpd;

  /** Different scoped displayed in space. */
  struct SequencerScopes scopes;
  struct SequencerPreviewOverlay preview_overlay;
  struct SequencerTimelineOverlay timeline_overlay;

  /** Multiview current eye - for internal use. */
  char multiview_eye;
  char _pad2[7];

  SpaceSeqRuntime runtime;
} SpaceSeq;

/** #SpaceSeq.mainb */
typedef enum eSpaceSeq_RegionType
{
  SEQ_DRAW_IMG_IMBUF = 1,
  SEQ_DRAW_IMG_WAVEFORM = 2,
  SEQ_DRAW_IMG_VECTORSCOPE = 3,
  SEQ_DRAW_IMG_HISTOGRAM = 4,
} eSpaceSeq_RegionType;

/** #SpaceSeq.draw_flag */
typedef enum eSpaceSeq_DrawFlag
{
  SEQ_DRAW_BACKDROP = (1 << 0),
  SEQ_DRAW_UNUSED_1 = (1 << 1),
  SEQ_DRAW_TRANSFORM_PREVIEW = (1 << 2),
} eSpaceSeq_DrawFlag;

/** #SpaceSeq.flag */
typedef enum eSpaceSeq_Flag
{
  SEQ_DRAWFRAMES = (1 << 0),
  SEQ_MARKER_TRANS = (1 << 1),
  SEQ_DRAW_COLOR_SEPARATED = (1 << 2),
  SEQ_CLAMP_VIEW = (1 << 3),
  SPACE_SEQ_FLAG_UNUSED_4 = (1 << 4),
  SPACE_SEQ_FLAG_UNUSED_5 = (1 << 5),
  SEQ_USE_ALPHA = (1 << 6), /* use RGBA display mode for preview */
  SPACE_SEQ_FLAG_UNUSED_9 = (1 << 9),
  SPACE_SEQ_FLAG_UNUSED_10 = (1 << 10),
  SEQ_SHOW_MARKERS = (1 << 11), /* show markers region */
  SEQ_ZOOM_TO_FIT = (1 << 12),
  SEQ_SHOW_OVERLAY = (1 << 13),
  SPACE_SEQ_FLAG_UNUSED_14 = (1 << 14),
  SPACE_SEQ_FLAG_UNUSED_15 = (1 << 15),
  SPACE_SEQ_FLAG_UNUSED_16 = (1 << 16),
  SEQ_USE_PROXIES = (1 << 17),
  SEQ_SHOW_GRID = (1 << 18),
} eSpaceSeq_Flag;

/** #SpaceSeq.view */
typedef enum eSpaceSeq_Displays
{
  SEQ_VIEW_SEQUENCE = 1,
  SEQ_VIEW_PREVIEW = 2,
  SEQ_VIEW_SEQUENCE_PREVIEW = 3,
} eSpaceSeq_Dispays;

/** #SpaceSeq.render_size */
typedef enum eSpaceSeq_Proxy_RenderSize
{
  SEQ_RENDER_SIZE_NONE = -1,
  SEQ_RENDER_SIZE_SCENE = 0,
  SEQ_RENDER_SIZE_PROXY_25 = 25,
  SEQ_RENDER_SIZE_PROXY_50 = 50,
  SEQ_RENDER_SIZE_PROXY_75 = 75,
  SEQ_RENDER_SIZE_PROXY_100 = 99,
  SEQ_RENDER_SIZE_FULL = 100,
} eSpaceSeq_Proxy_RenderSize;

typedef struct MaskSpaceInfo
{
  /* **** mask editing **** */
  struct Mask *mask;
  /* draw options */
  char draw_flag;
  char draw_type;
  char overlay_mode;
  char _pad3[1];
  float blend_factor;
} MaskSpaceInfo;

/** #SpaceSeq.gizmo_flag */
enum
{
  /** All gizmos. */
  SEQ_GIZMO_HIDE = (1 << 0),
  SEQ_GIZMO_HIDE_NAVIGATE = (1 << 1),
  SEQ_GIZMO_HIDE_CONTEXT = (1 << 2),
  SEQ_GIZMO_HIDE_TOOL = (1 << 3),
};

/** #SpaceSeq.mainb */
typedef enum eSpaceSeq_OverlayFrameType
{
  SEQ_OVERLAY_FRAME_TYPE_RECT = 0,
  SEQ_OVERLAY_FRAME_TYPE_REFERENCE = 1,
  SEQ_OVERLAY_FRAME_TYPE_CURRENT = 2,
} eSpaceSeq_OverlayFrameType;

/** \} */

/* -------------------------------------------------------------------- */
/** \name File Selector
 * \{ */

/** Config and Input for File Selector. */
typedef struct FileSelectParams
{
  /** Title, also used for the text of the execute button. */
  char title[96];
  /**
   * Directory, FILE_MAX_LIBEXTRA, 1024 + 66, this is for extreme case when 1023 length path
   * needs to be linked in, where foo.blend/Armature need adding
   */
  char dir[1090];
  char file[256];

  char renamefile[256];
  short rename_flag;
  char _pad[4];
  /** An ID that was just renamed. Used to identify a renamed asset file over re-reads, similar to
   * `renamefile` but for local IDs (takes precedence). Don't keep this stored across handlers!
   * Would break on undo. */
  const ID *rename_id;
  void *_pad3;

  /** List of file-types to filter (#FILE_MAXFILE). */
  char filter_glob[256];

  /** Text items name must match to be shown. */
  char filter_search[64];
  /** Same as filter, but for ID types (aka library groups). */
  uint64_t filter_id;

  /** Active file used for keyboard navigation. */
  int active_file;
  /** File under cursor. */
  int highlight_file;
  int sel_first;
  int sel_last;
  unsigned short thumbnail_size;
  char _pad1[2];

  /* short */
  /** XXX: for now store type here, should be moved to the operator. */
  short type; /* eFileSelectType */
  /** Settings for filter, hiding dots files. */
  short flag;
  /** Sort order. */
  short sort;
  /** Display mode flag. */
  short display;
  /** Details toggles (file size, creation date, etc.) */
  char details_flags;
  char _pad2[3];

  /** Filter when (flags & FILE_FILTER) is true. */
  int filter;

  /** Max number of levels in directory tree to show at once, 0 to disable recursion. */
  short recursion_level;

  char _pad4[2];
} FileSelectParams;

typedef struct AssetLibraryReference
{
  short type; /* eAssetLibraryType */
  char _pad1[2];
  /**
   * If showing a custom asset library (#ASSET_LIBRARY_CUSTOM), this is the index of the
   * #bUserAssetLibrary within #UserDef.asset_libraries.
   * Should be ignored otherwise (but better set to -1 then, for sanity and debugging).
   */
  int custom_library_index;
} AssetLibraryReference;

typedef struct kUUID
{
  uint32_t time_low;
  uint16_t time_mid;
  uint16_t time_hi_and_version;
  uint8_t clock_seq_hi_and_reserved;
  uint8_t clock_seq_low;
  uint8_t node[6];
} kUUID;

/**
 * File selection parameters for asset browsing mode, with #FileSelectParams as base.
 */
typedef struct FileAssetSelectParams
{
  FileSelectParams base_params;

  AssetLibraryReference asset_library_ref;
  short asset_catalog_visibility; /* eFileSel_Params_AssetCatalogVisibility */
  char _pad[6];
  /** If #asset_catalog_visibility is #FILE_SHOW_ASSETS_FROM_CATALOG, this sets the ID of the
   * catalog to show. */
  kUUID catalog_id;

  short import_type; /* eFileAssetImportType */
  char _pad2[6];
} FileAssetSelectParams;

typedef enum eFileAssetImportType
{
  /** Regular data-block linking. */
  FILE_ASSET_IMPORT_LINK = 0,
  /** Regular data-block appending (basically linking + "Make Local"). */
  FILE_ASSET_IMPORT_APPEND = 1,
  /** Append data-block with the #KLO_LIBLINK_APPEND_LOCAL_ID_REUSE flag enabled. Some typically
   * heavy data dependencies (e.g. the image data-blocks of a material, the mesh of an object) may
   * be reused from an earlier append. */
  FILE_ASSET_IMPORT_APPEND_REUSE = 2,
} eFileAssetImportType;

/**
 * A wrapper to store previous and next folder lists (#FolderList) for a specific browse mode
 * (#eFileBrowse_Mode).
 */
typedef struct FileFolderHistory
{
  struct FileFolderLists *next, *prev;

  /** The browse mode this prev/next folder-lists are created for. */
  char browse_mode; /* eFileBrowse_Mode */
  char _pad[7];

  /** Holds the list of previous directories to show. */
  ListBase folders_prev;
  /** Holds the list of next directories (pushed from previous) to show. */
  ListBase folders_next;
} FileFolderHistory;

/** File Browser. */
typedef struct SpaceFile
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /** Is this a File Browser or an Asset Browser? */
  char browse_mode; /* eFileBrowse_Mode */
  char _pad1[1];

  short tags;

  int scroll_offset;

  /** Config and input for file select. One for each browse-mode, to keep them independent. */
  FileSelectParams *params;
  FileAssetSelectParams *asset_params;

  void *_pad2;

  /**
   * Holds the list of files to show.
   * Currently recreated when browse-mode changes. Could be per browse-mode to avoid refreshes.
   */
  struct FileList *files;

  /**
   * Holds the list of previous directories to show. Owned by `folder_histories` below.
   */
  ListBase *folders_prev;
  /**
   * Holds the list of next directories (pushed from previous) to show. Owned by
   * `folder_histories` below.
   */
  ListBase *folders_next;

  /**
   * This actually owns the prev/next folder-lists above. On browse-mode change, the lists of the
   * new mode get assigned to the above.
   */
  ListBase folder_histories; /* FileFolderHistory */

  /**
   * The operator that is invoking file-select `op->exec()` will be called on the 'Load' button.
   * if operator provides op->cancel(), then this will be invoked on the cancel button.
   */
  struct wmOperator *op;

  struct wmTimer *smoothscroll_timer;
  struct wmTimer *previews_timer;

  struct FileLayout *layout;

  short recentnr, bookmarknr;
  short systemnr, system_bookmarknr;

  SpaceFile_Runtime *runtime;
} SpaceFile;

/** #SpaceFile.browse_mode (File Space Browsing Mode). */
typedef enum eFileBrowse_Mode
{
  /* Regular kraken File Browser */
  FILE_BROWSE_MODE_FILES = 0,
  /* Asset Browser */
  FILE_BROWSE_MODE_ASSETS = 1,
} eFileBrowse_Mode;

/** #FileSelectParams.display */
enum eFileDisplayType
{
  /** Internal (not exposed to users): Keep whatever display type was used during the last File
   * Browser use, or the default if no such record is found. Use this unless there's a good reason
   * to set a specific display type. */
  FILE_DEFAULTDISPLAY = 0,

  /* User selectable choices. */

  FILE_VERTICALDISPLAY = 1,
  FILE_HORIZONTALDISPLAY = 2,
  FILE_IMGDISPLAY = 3,
};

/** #FileSelectParams.sort */
enum eFileSortType
{
  /** Internal (not exposed to users): Sort by whatever was sorted by during the last File Browser
   * use, or the default if no such record is found. Use this unless there's a good reason to set a
   * specific sort order. */
  FILE_SORT_DEFAULT = 0,

  /* User selectable choices. */

  FILE_SORT_ALPHA = 1,
  FILE_SORT_EXTENSION = 2,
  FILE_SORT_TIME = 3,
  FILE_SORT_SIZE = 4,
};

/** #SpaceFile.tags */
enum eFileTags
{
  /** Tag the space as having to update files representing or containing main data. Must be set
   * after file read and undo/redo. */
  FILE_TAG_REBUILD_MAIN_FILES = (1 << 0),
};

/** #FileSelectParams.details_flags */
enum eFileDetails
{
  FILE_DETAILS_SIZE = (1 << 0),
  FILE_DETAILS_DATETIME = (1 << 1),
};

/* these values need to be hardcoded in structs, dna does not recognize defines */
/* also defined in BKE */
#define FILE_MAXDIR 768
#define FILE_MAXFILE 256
#define FILE_MAX 1024

#define FILE_MAX_LIBEXTRA (FILE_MAX + MAX_ID_NAME)

/**
 * Maximum level of recursions accepted for #FileSelectParams.recursion_level. Rather than a
 * completely arbitrary limit or none at all, make it just enough to support the most extreme case
 * where the maximal path length is used with single letter directory/file names only.
 */
#define FILE_SELECT_MAX_RECURSIONS (FILE_MAX_LIBEXTRA / 2)

/** File selector types. */
typedef enum eFileSelectType
{
  FILE_LOADLIB = 1,
  FILE_MAIN = 2,
  /** Load assets from #Main. */
  FILE_MAIN_ASSET = 3,
  /** Load assets of an asset library containing external files. */
  FILE_ASSET_LIBRARY = 4,

  FILE_UNIX = 8,
  FILE_BLENDER = 8, /* don't display relative paths */
  FILE_SPECIAL = 9,
} eFileSelectType;

/**
 * #FileSelectParams.flag / `sfile->params->flag`.
 * \note short flag, also used as 16 lower bits of flags in link/append code
 * (WM and BLO code area, see #eBLOLibLinkFlags in KLO_readfile.h).
 */
typedef enum eFileSelectParamsFlag
{
  FILE_PARAMS_FLAG_UNUSED_1 = (1 << 0),
  FILE_RELPATH = (1 << 1),
  FILE_LINK = (1 << 2),
  FILE_HIDE_DOT = (1 << 3),
  FILE_AUTOSELECT = (1 << 4),
  FILE_ACTIVE_COLLECTION = (1 << 5),
  FILE_PARAMS_FLAG_UNUSED_2 = (1 << 6),
  FILE_DIRSEL_ONLY = (1 << 7),
  FILE_FILTER = (1 << 8),
  FILE_PARAMS_FLAG_UNUSED_3 = (1 << 9),
  FILE_PATH_TOKENS_ALLOW = (1 << 10),
  FILE_SORT_INVERT = (1 << 11),
  FILE_HIDE_TOOL_PROPS = (1 << 12),
  FILE_CHECK_EXISTING = (1 << 13),
  FILE_ASSETS_ONLY = (1 << 14),
  /** Enables filtering by asset catalog. */
  FILE_FILTER_ASSET_CATALOG = (1 << 15),
} eFileSelectParamsFlag;
ENUM_OPERATORS(eFileSelectParamsFlag, FILE_FILTER_ASSET_CATALOG);

typedef enum eFileSel_Params_AssetCatalogVisibility
{
  FILE_SHOW_ASSETS_ALL_CATALOGS,
  FILE_SHOW_ASSETS_FROM_CATALOG,
  FILE_SHOW_ASSETS_WITHOUT_CATALOG,
} eFileSel_Params_AssetCatalogVisibility;

/**
 * #FileSelectParams.rename_flag / `sfile->params->rename_flag`.
 * \note short flag. Defined as bit-flags, but currently only used as exclusive status markers.
 */
typedef enum eFileSel_Params_RenameFlag
{
  /** Used when we only have the name of the entry we want to rename,
   * but not yet access to its matching file entry. */
  FILE_PARAMS_RENAME_PENDING = 1 << 0,
  /** We are actually renaming an entry. */
  FILE_PARAMS_RENAME_ACTIVE = 1 << 1,
  /** Used to scroll to newly renamed entry. */
  FILE_PARAMS_RENAME_POSTSCROLL_PENDING = 1 << 2,
  FILE_PARAMS_RENAME_POSTSCROLL_ACTIVE = 1 << 3,
} eFileSel_Params_RenameFlag;

/**
 * Files in the file selector list: file types
 * Note we could use mere values (instead of bit-flags) for file types themselves,
 * but since we do not lack of bytes currently.
 */
typedef enum eFileSelectFileTypes
{
  FILE_TYPE_PIXAR = (1 << 2),
  FILE_TYPE_PIXAR_BACKUP = (1 << 3),
  FILE_TYPE_IMAGE = (1 << 4),
  FILE_TYPE_MOVIE = (1 << 5),
  FILE_TYPE_PYSCRIPT = (1 << 6),
  FILE_TYPE_FTFONT = (1 << 7),
  FILE_TYPE_SOUND = (1 << 8),
  FILE_TYPE_TEXT = (1 << 9),
  FILE_TYPE_ARCHIVE = (1 << 10),
  /** represents folders for filtering */
  FILE_TYPE_FOLDER = (1 << 11),
  FILE_TYPE_BTX = (1 << 12),
  FILE_TYPE_COLLADA = (1 << 13),
  /** from filter_glob operator property */
  FILE_TYPE_OPERATOR = (1 << 14),
  FILE_TYPE_BUNDLE = (1 << 15),
  FILE_TYPE_ALEMBIC = (1 << 16),
  /** For all kinds of recognized import/export formats. No need for specialized types. */
  FILE_TYPE_OBJECT_IO = (1 << 17),
  FILE_TYPE_BLENDER = (1 << 18),
  FILE_TYPE_VOLUME = (1 << 19),

  FILE_TYPE_ASSET = (1 << 28),
  /** An FS directory (i.e. S_ISDIR on its path is true). */
  FILE_TYPE_DIR = (1 << 30),
  FILE_TYPE_KRAKENLIB = (1u << 31),
} eFileSelectFileTypes;
ENUM_OPERATORS(eFileSelectFileTypes, FILE_TYPE_KRAKENLIB);

/** Selection Flags in filesel: struct direntry, unsigned char selflag. */
typedef enum eDirEntry_SelectFlag
{
  /*  FILE_SEL_ACTIVE         = (1 << 1), */ /* UNUSED */
  FILE_SEL_HIGHLIGHTED = (1 << 2),
  FILE_SEL_SELECTED = (1 << 3),
  FILE_SEL_EDITING = (1 << 4),
} eDirEntry_SelectFlag;

/* ***** Related to file browser, but never saved in USD, only here to help with RNA. ***** */

#
#
typedef struct FileDirEntry
{
  struct FileDirEntry *next, *prev;

  uint32_t uid; /* FileUID */
  /* Name needs freeing if FILE_ENTRY_NAME_FREE is set. Otherwise this is a direct pointer to a
   * name buffer. */
  char *name;

  uint64_t size;
  int64_t time;

  struct
  {
    /* Temp caching of UI-generated strings. */
    char size_str[16];
    char datetime_str[16 + 8];
  } draw_data;

  /** #eFileSel_File_Types. */
  int typeflag;
  /** ID type, in case typeflag has FILE_TYPE_KRAKENLIB set. */
  int usdtype;

  /* Path to item that is relative to current folder root. */
  char *relpath;
  /** Optional argument for shortcuts, aliases etc. */
  char *redirection_path;

  /** When showing local IDs (FILE_MAIN, FILE_MAIN_ASSET), ID this file represents. Note comment
   * for FileListInternEntry.local_data, the same applies here! */
  ID *id;
  /** If this file represents an asset, its asset data is here. Note that we may show assets of
   * external files in which case this is set but not the id above.
   * Note comment for FileListInternEntry.local_data, the same applies here! */
  struct AssetMetaData *asset_data;

  /* The icon_id for the preview image. */
  int preview_icon_id;

  short flags;
  /* eFileAttributes defined in KLI_fileops.h */
  int attributes;
} FileDirEntry;

/**
 * Array of directory entries.
 *
 * Stores the total number of available entries, the number of visible (filtered) entries, and a
 * subset of those in 'entries' ListBase, from idx_start (included) to idx_end (excluded).
 */
#
#
typedef struct FileDirEntryArr
{
  ListBase entries;
  int entries_num;
  int entries_filtered_num;

  /** FILE_MAX. */
  char root[1024];
} FileDirEntryArr;

/** #FileDirEntry.flags */
enum
{
  /* The preview for this entry could not be generated. */
  FILE_ENTRY_INVALID_PREVIEW = 1 << 0,
  /* The entry name needs to be freed when clearing file list. */
  FILE_ENTRY_NAME_FREE = 1 << 1,
  /* The preview for this entry is being loaded on another thread. */
  FILE_ENTRY_PREVIEW_LOADING = 1 << 2,
};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Image/UV Editor
 * \{ */

/* Image/UV Editor */

typedef struct SpaceImageOverlay
{
  int flag;
  char _pad[4];
} SpaceImageOverlay;

typedef struct SpaceImage
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  struct Image *image;
  struct ImageUser iuser;

  /** Histogram waveform and vectorscope. */
  struct Scopes scopes;
  /** Sample line histogram. */
  struct Histogram sample_line_hist;

  /** Grease pencil data. */
  struct bGPdata *gpd;

  /** UV editor 2d cursor. */
  float cursor[2];
  /** User defined offset, image is centered. */
  float xof, yof;
  /** User defined zoom level. */
  float zoom;
  /** Storage for offset while render drawing. */
  float centx, centy;

  /** View/paint/mask. */
  char mode;
  /* Storage for sub-space types. */
  char mode_prev;

  char pin;

  char pixel_round_mode;

  char lock;
  /** UV draw type. */
  char dt_uv;
  /** Sticky selection type. */
  char dt_uvstretch;
  char around;

  char gizmo_flag;
  char _pad1[3];

  int flag;

  float uv_opacity;

  int tile_grid_shape[2];
  /**
   * UV editor custom-grid. Value of `{M,N}` will produce `MxN` grid.
   * Use when #SI_CUSTOM_GRID is set.
   */
  int custom_grid_subdiv[2];

  MaskSpaceInfo mask_info;
  SpaceImageOverlay overlay;
} SpaceImage;

/** #SpaceImage.dt_uv */
typedef enum eSpaceImage_UVDT
{
  SI_UVDT_OUTLINE = 0,
  SI_UVDT_DASH = 1,
  SI_UVDT_BLACK = 2,
  SI_UVDT_WHITE = 3,
} eSpaceImage_UVDT;

/** #SpaceImage.dt_uvstretch */
typedef enum eSpaceImage_UVDT_Stretch
{
  SI_UVDT_STRETCH_ANGLE = 0,
  SI_UVDT_STRETCH_AREA = 1,
} eSpaceImage_UVDT_Stretch;

/** #SpaceImage.pixel_round_mode */
typedef enum eSpaceImage_PixelRoundMode
{
  SI_PIXEL_ROUND_DISABLED = 0,
  SI_PIXEL_ROUND_CENTER = 1,
  SI_PIXEL_ROUND_CORNER = 2,
} eSpaceImage_Round_Mode;

/** #SpaceImage.mode */
typedef enum eSpaceImage_Mode
{
  SI_MODE_VIEW = 0,
  SI_MODE_PAINT = 1,
  SI_MODE_MASK = 2,
  SI_MODE_UV = 3,
} eSpaceImage_Mode;

/** #SpaceImage.flag */
typedef enum eSpaceImage_Flag
{
  SI_FLAG_UNUSED_0 = (1 << 0), /* cleared */
  SI_FLAG_UNUSED_1 = (1 << 1), /* cleared */
  SI_CLIP_UV = (1 << 2),
  SI_FLAG_UNUSED_3 = (1 << 3), /* cleared */
  SI_NO_DRAWFACES = (1 << 4),
  SI_DRAWSHADOW = (1 << 5),
  SI_FLAG_UNUSED_6 = (1 << 6), /* cleared */
  SI_FLAG_UNUSED_7 = (1 << 7), /* cleared */
  SI_FLAG_UNUSED_8 = (1 << 8), /* cleared */
  SI_COORDFLOATS = (1 << 9),

  SI_FLAG_UNUSED_10 = (1 << 10),
  SI_LIVE_UNWRAP = (1 << 11),
  SI_USE_ALPHA = (1 << 12),
  SI_SHOW_ALPHA = (1 << 13),
  SI_SHOW_ZBUF = (1 << 14),

  /* next two for render window display */
  SI_PREVSPACE = (1 << 15),
  SI_FULLWINDOW = (1 << 16),

  SI_FLAG_UNUSED_17 = (1 << 17),
  SI_CUSTOM_GRID = (1 << 18),

  /**
   * This means that the image is drawn until it reaches the view edge,
   * in the image view, it's unrelated to UDIM tiles.
   */
  SI_DRAW_TILE = (1 << 19),
  SI_FLAG_UNUSED_20 = (1 << 20), /* cleared */
  SI_DRAW_STRETCH = (1 << 21),
  SI_SHOW_GPENCIL = (1 << 22),
  SI_FLAG_UNUSED_23 = (1 << 23), /* cleared */

  SI_FLAG_UNUSED_24 = (1 << 24),

  SI_NO_DRAW_TEXPAINT = (1 << 25),
  SI_DRAW_METADATA = (1 << 26),

  SI_SHOW_R = (1 << 27),
  SI_SHOW_G = (1 << 28),
  SI_SHOW_B = (1 << 29),

  SI_GRID_OVER_IMAGE = (1 << 30),
} eSpaceImage_Flag;

typedef enum eSpaceImageOverlay_Flag
{
  SI_OVERLAY_SHOW_OVERLAYS = (1 << 0),
  SI_OVERLAY_SHOW_GRID_BACKGROUND = (1 << 1),
} eSpaceImageOverlay_Flag;

/** #SpaceImage.gizmo_flag */
enum
{
  /** All gizmos. */
  SI_GIZMO_HIDE = (1 << 0),
  SI_GIZMO_HIDE_NAVIGATE = (1 << 1),
};

/** Keep in sync with `STEPS_LEN` in `grid_frag.glsl`. */
#define SI_GRID_STEPS_LEN 8

/** \} */

/* -------------------------------------------------------------------- */
/** \name Text Editor
 * \{ */

typedef struct SpaceText_Runtime
{

  /** Actual line height, scaled by DPI. */
  int lheight_px;

  /** Runtime computed, character width. */
  int cwidth_px;

  /** The handle of the scroll-bar which can be clicked and dragged. */
  struct rcti scroll_region_handle;
  /** The region for selected text to show in the scrolling area. */
  struct rcti scroll_region_select;

  /** Number of digits to show in the line numbers column (when enabled). */
  int line_number_display_digits;

  /** Number of lines this window can display (even when they aren't used). */
  int viewlines;

  /** Use for drawing scroll-bar & calculating scroll operator motion scaling. */
  float scroll_px_per_line;

  /**
   * Run-time for scroll increments smaller than a line (smooth scroll).
   * Values must be between zero and the line, column width: (cwidth, TXT_LINE_HEIGHT(st)).
   */
  int scroll_ofs_px[2];

  char _pad1[4];

  /** Cache for faster drawing. */
  void *drawcache;

} SpaceText_Runtime;

/** Text Editor. */
typedef struct SpaceText
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  struct Text *text;

  /** Determines at what line the top of the text is displayed. */
  int top;

  /** Determines the horizontal scroll (in columns). */
  int left;
  char _pad1[4];

  short flags;

  /** User preference, is font_size! */
  short lheight;

  int tabnumber;

  /* Booleans */
  char wordwrap;
  char doplugins;
  char showlinenrs;
  char showsyntax;
  char line_hlight;
  char overwrite;
  /** Run python while editing, evil. */
  char live_edit;
  char _pad2[1];

  /** ST_MAX_FIND_STR. */
  char findstr[256];
  /** ST_MAX_FIND_STR. */
  char replacestr[256];

  /** Column number to show right margin at. */
  short margin_column;
  char _pad3[2];

  /** Keep last. */
  SpaceText_Runtime runtime;
} SpaceText;

/** SpaceText flags (moved from USD_text_types.h). */
typedef enum eSpaceText_Flags
{
  /* scrollable */
  ST_SCROLL_SELECT = (1 << 0),

  ST_FLAG_UNUSED_4 = (1 << 4), /* Cleared. */

  ST_FIND_WRAP = (1 << 5),
  ST_FIND_ALL = (1 << 6),
  ST_SHOW_MARGIN = (1 << 7),
  ST_MATCH_CASE = (1 << 8),

  ST_FIND_ACTIVATE = (1 << 9),
} eSpaceText_Flags;

/* SpaceText.findstr/replacestr */
#define ST_MAX_FIND_STR 256

/** \} */

/* -------------------------------------------------------------------- */
/** \name Script View (Obsolete)
 * \{ */

/** Script Runtime Data - Obsolete (pre 2.5). */
// typedef struct Script
// {
//   struct ID id;

//   void *py_draw;
//   void *py_event;
//   void *py_button;
//   void *py_browsercallback;
//   void *py_globaldict;

//   int flags, lastspace;
//   /**
//    * Store the script file here so we can re-run it on loading kraken,
//    * if "Enable Scripts" is on
//    */
//   /** 1024 = FILE_MAX. */
//   char scriptname[1024];
//   /** 1024 = FILE_MAX. */
//   char scriptarg[256];
// } Script;
// #define SCRIPT_SET_NULL(_script)                                                            \
//   _script->py_draw = _script->py_event = _script->py_button = _script->py_browsercallback = \
//     _script->py_globaldict = NULL;                                                          \
//   _script->flags = 0

// /** Script View - Obsolete (pre 2.5). */
// typedef struct SpaceScript
// {
//   SpaceLink *next, *prev;
//   /** Storage of regions for inactive spaces. */
//   ListBase regionbase;
//   char spacetype;
//   char link_flag;
//   char _pad0[6];
//   /* End 'SpaceLink' header. */

//   struct Script *script;

//   short flags, menunr;
//   char _pad1[4];

//   void *but_refs;
// } SpaceScript;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Nodes Editor
 * \{ */

typedef struct SpaceNodeOverlay
{
  /* eSpaceNodeOverlay_Flag */
  int flag;
} SpaceNodeOverlay;

typedef enum eSpaceNodeOverlay_Flag
{
  SN_OVERLAY_SHOW_OVERLAYS = (1 << 1),
  SN_OVERLAY_SHOW_WIRE_COLORS = (1 << 2),
  SN_OVERLAY_SHOW_TIMINGS = (1 << 3),
  SN_OVERLAY_SHOW_PATH = (1 << 4),
  SN_OVERLAY_SHOW_NAMED_ATTRIBUTES = (1 << 5),
} eSpaceNodeOverlay_Flag;

typedef struct SpaceNode
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /** Context, no need to save in file? well... pinning... */
  struct ID *id, *from;

  short flag;

  /** Direction for offsetting nodes on insertion. */
  char insert_ofs_dir;
  char _pad1;

  /** Offset for drawing the backdrop. */
  float xof, yof;
  /** Zoom for backdrop. */
  float zoom;

  /**
   * XXX nodetree pointer info is all in the path stack now,
   * remove later on and use kNodeTreePath instead.
   * For now these variables are set when pushing/popping
   * from path stack, to avoid having to update all the functions and operators.
   * Can be done when design is accepted and everything is properly tested.
   */
  ListBase treepath;

  /* The tree farthest down in the group hierarchy. */
  struct kNodeTree *edittree;

  struct kNodeTree *nodetree;

  /* tree type for the current node tree */
  char tree_idname[64];

  /** Texfrom object, world or brush. */
  short texfrom;
  /** Shader from object or world. */
  short shaderfrom;

  /** Grease-pencil data. */
  struct bGPdata *gpd;

  SpaceNodeOverlay overlay;
  char _pad2[4];

  SpaceNode_Runtime *runtime;
} SpaceNode;

/** #SpaceNode.flag */
typedef enum eSpaceNode_Flag
{
  SNODE_BACKDRAW = (1 << 1),
  SNODE_SHOW_GPENCIL = (1 << 2),
  SNODE_USE_ALPHA = (1 << 3),
  SNODE_SHOW_ALPHA = (1 << 4),
  SNODE_SHOW_R = (1 << 7),
  SNODE_SHOW_G = (1 << 8),
  SNODE_SHOW_B = (1 << 9),
  SNODE_AUTO_RENDER = (1 << 5),
  SNODE_FLAG_UNUSED_6 = (1 << 6),   /* cleared */
  SNODE_FLAG_UNUSED_10 = (1 << 10), /* cleared */
  SNODE_FLAG_UNUSED_11 = (1 << 11), /* cleared */
  SNODE_PIN = (1 << 12),
  /** automatically offset following nodes in a chain on insertion */
  SNODE_SKIP_INSOFFSET = (1 << 13),
} eSpaceNode_Flag;

/** #SpaceNode.texfrom */
typedef enum eSpaceNode_TexFrom
{
  /* SNODE_TEX_OBJECT   = 0, */
  SNODE_TEX_WORLD = 1,
  SNODE_TEX_BRUSH = 2,
  SNODE_TEX_LINESTYLE = 3,
} eSpaceNode_TexFrom;

/** #SpaceNode.shaderfrom */
typedef enum eSpaceNode_ShaderFrom
{
  SNODE_SHADER_OBJECT = 0,
  SNODE_SHADER_WORLD = 1,
  SNODE_SHADER_LINESTYLE = 2,
} eSpaceNode_ShaderFrom;

/** #SpaceNode.insert_ofs_dir */
enum
{
  SNODE_INSERTOFS_DIR_RIGHT = 0,
  SNODE_INSERTOFS_DIR_LEFT = 1,
};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Console
 * \{ */

/** Console content. */
typedef struct ConsoleLine
{
  struct ConsoleLine *next, *prev;

  /* Keep these 3 vars so as to share free, realloc functions. */
  /** Allocated length. */
  int len_alloc;
  /** Real len - strlen(). */
  int len;
  char *line;

  int cursor;
  /** Only for use when in the 'scrollback' listbase. */
  int type;
} ConsoleLine;

/** #ConsoleLine.type */
typedef enum eConsoleLine_Type
{
  CONSOLE_LINE_OUTPUT = 0,
  CONSOLE_LINE_INPUT = 1,
  CONSOLE_LINE_INFO = 2, /* autocomp feedback */
  CONSOLE_LINE_ERROR = 3,
} eConsoleLine_Type;

/** Console View. */
typedef struct SpaceConsole
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /* space vars */
  int lheight;
  char _pad[4];

  /** ConsoleLine; output. */
  ListBase scrollback;
  /** ConsoleLine; command history, current edited line is the first. */
  ListBase history;
  char prompt[256];
  /** Multiple consoles are possible, not just python. */
  char language[32];

  /** Selection offset in bytes. */
  int sel_start;
  int sel_end;
} SpaceConsole;

/** \} */

/* -------------------------------------------------------------------- */
/** \name User Preferences
 * \{ */

typedef struct SpaceUserPref
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  char _pad1[7];
  char filter_type;
  /** Search term for filtering in the UI. */
  char filter[64];
} SpaceUserPref;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Motion Tracking
 * \{ */

/** Clip Editor. */
typedef struct SpaceClip
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  char _pad1[4];

  /** User defined offset, image is centered. */
  float xof, yof;
  /** User defined offset from locked position. */
  float xlockof, ylockof;
  /** User defined zoom level. */
  float zoom;

  /** User of clip. */
  // struct MovieClipUser user;
  /** Clip data. */
  // struct MovieClip *clip;
  /** Different scoped displayed in space panels. */
  // struct MovieClipScopes scopes;

  /** Flags. */
  int flag;
  /** Editor mode (editing context being displayed). */
  short mode;
  /** Type of the clip editor view. */
  short view;

  /** Length of displaying path, in frames. */
  int path_length;

  /* current stabilization data */
  /** Pre-composed stabilization data. */
  float loc[2], scale, angle;
  char _pad[4];
  /**
   * Current stabilization matrix and the same matrix in unified space,
   * defined when drawing and used for mouse position calculation.
   */
  float stabmat[4][4], unistabmat[4][4];

  /** Movie postprocessing. */
  int postproc_flag;

  /* grease pencil */
  short gpencil_src;
  char _pad2[2];

  /** Pivot point for transforms. */
  int around;
  char _pad4[4];

  /** Mask editor 2d cursor. */
  float cursor[2];

  MaskSpaceInfo mask_info;
} SpaceClip;

/** #SpaceClip.flag */
typedef enum eSpaceClip_Flag
{
  SC_SHOW_MARKER_PATTERN = (1 << 0),
  SC_SHOW_MARKER_SEARCH = (1 << 1),
  SC_LOCK_SELECTION = (1 << 2),
  SC_SHOW_TINY_MARKER = (1 << 3),
  SC_SHOW_TRACK_PATH = (1 << 4),
  SC_SHOW_BUNDLES = (1 << 5),
  SC_MUTE_FOOTAGE = (1 << 6),
  SC_HIDE_DISABLED = (1 << 7),
  SC_SHOW_NAMES = (1 << 8),
  SC_SHOW_GRID = (1 << 9),
  SC_SHOW_STABLE = (1 << 10),
  SC_MANUAL_CALIBRATION = (1 << 11),
  SC_SHOW_ANNOTATION = (1 << 12),
  SC_SHOW_FILTERS = (1 << 13),
  SC_SHOW_GRAPH_FRAMES = (1 << 14),
  SC_SHOW_GRAPH_TRACKS_MOTION = (1 << 15),
  /*  SC_SHOW_PYRAMID_LEVELS      = (1 << 16), */ /* UNUSED */
  SC_LOCK_TIMECURSOR = (1 << 17),
  SC_SHOW_SECONDS = (1 << 18),
  SC_SHOW_GRAPH_SEL_ONLY = (1 << 19),
  SC_SHOW_GRAPH_HIDDEN = (1 << 20),
  SC_SHOW_GRAPH_TRACKS_ERROR = (1 << 21),
  SC_SHOW_METADATA = (1 << 22),
} eSpaceClip_Flag;

/** #SpaceClip.mode */
typedef enum eSpaceClip_Mode
{
  SC_MODE_TRACKING = 0,
  // SC_MODE_RECONSTRUCTION = 1, /* DEPRECATED */
  // SC_MODE_DISTORTION = 2,     /* DEPRECATED */
  SC_MODE_MASKEDIT = 3,
} eSpaceClip_Mode;

/** #SpaceClip.view */
typedef enum eSpaceClip_View
{
  SC_VIEW_CLIP = 0,
  SC_VIEW_GRAPH = 1,
  SC_VIEW_DOPESHEET = 2,
} eSpaceClip_View;

/** #SpaceClip.gpencil_src */
typedef enum eSpaceClip_GPencil_Source
{
  SC_GPENCIL_SRC_CLIP = 0,
  SC_GPENCIL_SRC_TRACK = 1,
} eSpaceClip_GPencil_Source;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Top Bar
 * \{ */

typedef struct SpaceTopBar
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */
} SpaceTopBar;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Status Bar
 * \{ */

typedef struct SpaceStatusBar
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */
} SpaceStatusBar;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Spreadsheet
 * \{ */

typedef struct SpreadsheetColumnID
{
  char *name;
} SpreadsheetColumnID;

typedef struct SpreadsheetColumn
{
  struct SpreadsheetColumn *next, *prev;
  /**
   * Identifies the data in the column.
   * This is a pointer instead of a struct to make it easier if we want to "subclass"
   * #SpreadsheetColumnID in the future for different kinds of ids.
   */
  SpreadsheetColumnID *id;

  /**
   * An indicator of the type of values in the column, set at runtime.
   * #eSpreadsheetColumnValueType.
   */
  uint8_t data_type;
  char _pad0[7];

  /**
   * The final column name generated by the data source, also just
   * cached at runtime when the data source columns are generated.
   */
  char *display_name;
} SpreadsheetColumn;

typedef struct SpaceSpreadsheet
{
  SpaceLink *next, *prev;
  /** Storage of regions for inactive spaces. */
  ListBase regionbase;
  char spacetype;
  char link_flag;
  char _pad0[6];
  /* End 'SpaceLink' header. */

  /* List of #SpreadsheetColumn. */
  ListBase columns;

  /* SpreadsheetRowFilter. */
  ListBase row_filters;

  /**
   * Context that is currently displayed in the editor. This is usually a either a single object
   * (in original/evaluated mode) or path to a viewer node. This is retrieved from the workspace
   * but can be pinned so that it stays constant even when the active node changes.
   */
  // ViewerPath viewer_path;

  /* eSpaceSpreadsheet_FilterFlag. */
  uint8_t filter_flag;

  /* #GeometryComponentType. */
  uint8_t geometry_component_type;
  /* #eAttrDomain. */
  uint8_t attribute_domain;
  /* eSpaceSpreadsheet_ObjectEvalState. */
  uint8_t object_eval_state;

  /* eSpaceSpreadsheet_Flag. */
  uint32_t flag;

  SpaceSpreadsheet_Runtime *runtime;
} SpaceSpreadsheet;

typedef enum eSpaceSpreadsheet_Flag
{
  SPREADSHEET_FLAG_PINNED = (1 << 0),
  SPREADSHEET_FLAG_CONTEXT_PATH_COLLAPSED = (1 << 1),
} eSpaceSpreadsheet_Flag;

typedef enum eSpaceSpreadsheet_FilterFlag
{
  SPREADSHEET_FILTER_SELECTED_ONLY = (1 << 0),
  SPREADSHEET_FILTER_ENABLE = (1 << 1),
} eSpaceSpreadsheet_FilterFlag;

typedef struct SpreadsheetRowFilter
{
  struct SpreadsheetRowFilter *next, *prev;

  char column_name[64]; /* MAX_NAME. */

  /* eSpreadsheetFilterOperation. */
  uint8_t operation;
  /* eSpaceSpreadsheet_RowFilterFlag. */
  uint8_t flag;

  char _pad0[2];

  int value_int;
  char *value_string;
  float value_float;
  float threshold;
  float value_float2[2];
  float value_float3[3];
  float value_color[4];
  char _pad1[4];
} SpreadsheetRowFilter;

typedef enum eSpaceSpreadsheet_RowFilterFlag
{
  SPREADSHEET_ROW_FILTER_UI_EXPAND = (1 << 0),
  SPREADSHEET_ROW_FILTER_BOOL_VALUE = (1 << 1),
  SPREADSHEET_ROW_FILTER_ENABLED = (1 << 2),
} eSpaceSpreadsheet_RowFilterFlag;

typedef enum eSpreadsheetFilterOperation
{
  SPREADSHEET_ROW_FILTER_EQUAL = 0,
  SPREADSHEET_ROW_FILTER_GREATER = 1,
  SPREADSHEET_ROW_FILTER_LESS = 2,
} eSpreadsheetFilterOperation;

typedef enum eSpaceSpreadsheet_ObjectEvalState
{
  SPREADSHEET_OBJECT_EVAL_STATE_EVALUATED = 0,
  SPREADSHEET_OBJECT_EVAL_STATE_ORIGINAL = 1,
  SPREADSHEET_OBJECT_EVAL_STATE_VIEWER_NODE = 2,
} eSpaceSpreadsheet_Context;

typedef enum eSpaceSpreadsheet_ContextType
{
  SPREADSHEET_CONTEXT_OBJECT = 0,
  SPREADSHEET_CONTEXT_MODIFIER = 1,
  SPREADSHEET_CONTEXT_NODE = 2,
} eSpaceSpreadsheet_ContextType;

typedef enum eSpreadsheetColumnValueType
{
  SPREADSHEET_VALUE_TYPE_UNKNOWN = -1,
  SPREADSHEET_VALUE_TYPE_BOOL = 0,
  SPREADSHEET_VALUE_TYPE_INT32 = 1,
  SPREADSHEET_VALUE_TYPE_FLOAT = 2,
  SPREADSHEET_VALUE_TYPE_FLOAT2 = 3,
  SPREADSHEET_VALUE_TYPE_FLOAT3 = 4,
  SPREADSHEET_VALUE_TYPE_COLOR = 5,
  SPREADSHEET_VALUE_TYPE_INSTANCES = 6,
  SPREADSHEET_VALUE_TYPE_STRING = 7,
  SPREADSHEET_VALUE_TYPE_BYTE_COLOR = 8,
  SPREADSHEET_VALUE_TYPE_INT8 = 9,
} eSpreadsheetColumnValueType;

/**
 * We can't just use UI_UNIT_X, because it does not take `widget.points` into account, which
 * modifies the width of text as well.
 */
#define SPREADSHEET_WIDTH_UNIT \
  (UI_UNIT_X * UI_style_get_dpi()->widget.points / (float)UI_DEFAULT_TEXT_POINTS)

/** \} */

/* -------------------------------------------------------------------- */
/** \name Space Defines (eSpaceType)
 * \{ */

/**
 * Space types: #SpaceLink.spacetype & #ScrArea.spacetype.
 * \note Do NOT change order, append on end. types are hardcoded needed.
 */
typedef enum eSpaceType
{
  SPACE_EMPTY = 0,
  SPACE_VIEW3D = 1,
  SPACE_GRAPH = 2,
  SPACE_OUTLINER = 3,
  SPACE_PROPERTIES = 4,
  SPACE_FILE = 5,
  SPACE_IMAGE = 6,
  SPACE_INFO = 7,
  SPACE_SEQ = 8,
  SPACE_TEXT = 9,
  SPACE_ACTION = 12,
  SPACE_NLA = 13,
  SPACE_SCRIPT = 14,
  SPACE_NODE = 16,
  SPACE_CONSOLE = 18,
  SPACE_USERPREF = 19,
  SPACE_CLIP = 20,
  SPACE_TOPBAR = 21,
  SPACE_STATUSBAR = 22,
  SPACE_SPREADSHEET = 23

#define SPACE_TYPE_NUM (SPACE_SPREADSHEET + 1)
} eSpaceType;

/* use for function args */
#define SPACE_TYPE_ANY -1

#define IMG_SIZE_FALLBACK 256

/** \} */

#ifdef __cplusplus
}
#endif

struct kSpaceLink
{
  /** Storage of regions for inactive spaces. */
  ListBase regions;
  char spacetype;
  char link_flag;
};

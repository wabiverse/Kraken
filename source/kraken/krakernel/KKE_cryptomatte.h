/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2020 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup bke
 */

#pragma once

#include "KLI_sys_types.h"
#include "USD_ID.h"

#include "USD_vec_types.h"
#include "USD_materials.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations. */
struct CryptomatteSession;
struct RenderResult;
struct Scene;

/* move all this where it belongs, not here. */

/* -------------------------------------------------------------------- */
/** @name Read and Write (TODO: Move this somewhere appropriate)
 * @{ */

#define STAMP_NAME_SIZE ((MAX_ID_NAME - 2) + 16)
/* could allow access externally - 512 is for long names,
 * STAMP_NAME_SIZE is for id names, allowing them some room for description */
typedef struct StampDataCustomField
{
  struct StampDataCustomField *next, *prev;
  char key[512];
  char *value;
} StampDataCustomField;

typedef void(StampCallback)(void *data, const char *propname, char *propvalue, int len);

struct StampData
{
  char file[512];
  char note[512];
  char date[512];
  char marker[512];
  char time[512];
  char frame[512];
  char frame_range[512];
  char camera[STAMP_NAME_SIZE];
  char cameralens[STAMP_NAME_SIZE];
  char scene[STAMP_NAME_SIZE];
  char strip[STAMP_NAME_SIZE];
  char rendertime[STAMP_NAME_SIZE];
  char memory[STAMP_NAME_SIZE];
  char hostname[512];

  /* Custom fields are used to put extra meta information header from render
   * engine to the result image.
   *
   * NOTE: This fields are not stamped onto the image. At least for now.
   */
  ListBase custom_fields;
};
#undef STAMP_NAME_SIZE

typedef struct NodeCryptomatte_Runtime
{
  /* Contains `CryptomatteLayer`. */
  ListBase layers;
  /* Temp storage for the cryptomatte picker. */
  float add[3];
  float remove[3];
} NodeCryptomatte_Runtime;

typedef struct NodeCryptomatte
{
  /**
   * `iuser` needs to be first element due to RNA limitations.
   * When we define the #ImageData properties, we can't define them from
   * `storage->iuser`, so storage needs to be cast to #ImageUser directly.
   */
  // ImageUser iuser;

  /* Contains `CryptomatteEntry`. */
  ListBase entries;

  /* MAX_NAME */
  char layer_name[64];
  /* Stores `entries` as a string for opening in 2.80-2.91. */
  char *matte_id;

  /** Legacy attributes */
  /* Number of input sockets. */
  int inputs_num;

  char _pad[4];
  NodeCryptomatte_Runtime runtime;
} NodeCryptomatte;

typedef struct CryptomatteEntry
{
  struct CryptomatteEntry *next, *prev;
  float encoded_hash;
  /** MAX_NAME. */
  char name[64];
  char _pad[4];
} CryptomatteEntry;

typedef struct RenderResult
{
  struct RenderResult *next, *prev;

  /* target image size */
  int rectx, recty;
  short sample_nr;

  /* The following rect32, rectf and rectz buffers are for temporary storage only,
   * for RenderResult structs created in #RE_AcquireResultImage - which do not have RenderView */

  /* optional, 32 bits version of picture, used for ogl render and image curves */
  int *rect32;
  /* if this exists, a copy of one of layers, or result of composited layers */
  float *rectf;
  /* if this exists, a copy of one of layers, or result of composited layers */
  float *rectz;

  /* coordinates within final image (after cropping) */
  rcti tilerect;
  /* offset to apply to get a border render in full image */
  int xof, yof;

  /* the main buffers */
  ListBase layers;

  /* multiView maps to a StringVector in OpenEXR */
  ListBase views; /* RenderView */

  /* allowing live updates: */
  rcti renrect;
  // RenderLayer *renlay;

  /* for render results in Image, verify validity for sequences */
  int framenr;

  /* for acquire image, to indicate if it there is a combined layer */
  int have_combined;

  /* render info text */
  char *text;
  char *error;

  struct StampData *stamp_data;

  bool passes_allocated;
} RenderResult;

/** Light-group Render-pass definition. */
typedef struct ViewLayerLightgroup
{
  struct ViewLayerLightgroup *next, *prev;

  /* Name of the Lightgroup */
  char name[64];
} ViewLayerLightgroup;

/** AOV Render-pass definition. */
typedef struct ViewLayerAOV
{
  struct ViewLayerAOV *next, *prev;

  /* Name of the AOV */
  char name[64];
  int flag;
  /* Type of AOV (color/value)
   * matches `eViewLayerAOVType` */
  int type;
} ViewLayerAOV;

typedef struct ViewLayer
{
  struct ViewLayer *next, *prev;
  /** MAX_NAME. */
  char name[64];
  short flag;
  char _pad[6];
  /** ObjectBase. */
  ListBase object_bases;
  /** Default allocated now. */
  struct SceneStats *stats;
  struct Base *basact;

  /** A view layer has one top level layer collection, because a scene has only one top level
   * collection. The layer_collections list always contains a single element. ListBase is
   * convenient when applying functions to all layer collections recursively. */
  ListBase layer_collections;
  // LayerCollection *active_collection;

  /* Old SceneRenderLayer data. */
  int layflag;
  /** Pass_xor has to be after passflag. */
  int passflag;
  float pass_alpha_threshold;
  short cryptomatte_flag;
  short cryptomatte_levels;
  char _pad1[4];

  int samples;

  struct Material *mat_override;
  /** Equivalent to datablocks ID properties. */
  struct IDProperty *id_properties;

  /* List containing the `ViewLayerAOV`s */
  ListBase aovs;
  ViewLayerAOV *active_aov;

  /* List containing the 'ViewLayerLightgroup`s */
  ListBase lightgroups;
  ViewLayerLightgroup *active_lightgroup;

  /* Runtime data */
  /** ViewLayerEngineData. */
  ListBase drawdata;
  // struct Base **object_bases_array;
  struct RHash *object_bases_hash;
} ViewLayer;

/* #ViewLayer.cryptomatte_flag */
typedef enum eViewLayerCryptomatteFlags
{
  VIEW_LAYER_CRYPTOMATTE_OBJECT = (1 << 0),
  VIEW_LAYER_CRYPTOMATTE_MATERIAL = (1 << 1),
  VIEW_LAYER_CRYPTOMATTE_ASSET = (1 << 2),
  VIEW_LAYER_CRYPTOMATTE_ACCURATE = (1 << 3),
} eViewLayerCryptomatteFlags;
#define VIEW_LAYER_CRYPTOMATTE_ALL \
  (VIEW_LAYER_CRYPTOMATTE_OBJECT | VIEW_LAYER_CRYPTOMATTE_MATERIAL | VIEW_LAYER_CRYPTOMATTE_ASSET)

struct CryptomatteSession *KKE_cryptomatte_init(void);
struct CryptomatteSession *KKE_cryptomatte_init_from_render_result(
  const struct RenderResult *render_result);
struct CryptomatteSession *KKE_cryptomatte_init_from_scene(const struct kScene *scene);
struct CryptomatteSession *KKE_cryptomatte_init_from_view_layer(
  const struct ViewLayer *view_layer);
void KKE_cryptomatte_free(struct CryptomatteSession *session);
void KKE_cryptomatte_add_layer(struct CryptomatteSession *session, const char *layer_name);

uint32_t KKE_cryptomatte_hash(const char *name, int name_len);
uint32_t KKE_cryptomatte_object_hash(struct CryptomatteSession *session,
                                     const char *layer_name,
                                     const struct Object *object);
uint32_t KKE_cryptomatte_material_hash(struct CryptomatteSession *session,
                                       const char *layer_name,
                                       const struct Material *material);
uint32_t KKE_cryptomatte_asset_hash(struct CryptomatteSession *session,
                                    const char *layer_name,
                                    const struct Object *object);
float KKE_cryptomatte_hash_to_float(uint32_t cryptomatte_hash);
/**
 * Find an ID in the given main that matches the given encoded float.
 */
bool KKE_cryptomatte_find_name(const struct CryptomatteSession *session,
                               float encoded_hash,
                               char *r_name,
                               int name_len);

char *KKE_cryptomatte_entries_to_matte_id(struct NodeCryptomatte *node_storage);
void KKE_cryptomatte_matte_id_to_entries(struct NodeCryptomatte *node_storage,
                                         const char *matte_id);

void KKE_cryptomatte_store_metadata(const struct CryptomatteSession *session,
                                    struct RenderResult *render_result,
                                    const ViewLayer *view_layer);

#ifdef __cplusplus
}
#endif

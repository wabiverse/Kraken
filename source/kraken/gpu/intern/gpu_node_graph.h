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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender GPU library. (source/blender/gpu).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * GPU.
 * Pixel Magic.
 *
 * Intermediate node graph for generating GLSL shaders.
 */

#include "USD_customdata_types.h"
#include "USD_listBase.h"

#include "KLI_rhash.h"

#include "GPU_material.h"
#include "GPU_shader.h"

#ifdef __cplusplus
extern "C" {
#endif

struct GPUNode;
struct GPUOutput;
struct ListBase;

typedef enum eGPUDataSource
{
  GPU_SOURCE_OUTPUT,
  GPU_SOURCE_CONSTANT,
  GPU_SOURCE_UNIFORM,
  GPU_SOURCE_ATTR,
  GPU_SOURCE_UNIFORM_ATTR,
  GPU_SOURCE_LAYER_ATTR,
  GPU_SOURCE_STRUCT,
  GPU_SOURCE_TEX,
  GPU_SOURCE_TEX_TILED_MAPPING,
  GPU_SOURCE_FUNCTION_CALL,
  GPU_SOURCE_CRYPTOMATTE,
} eGPUDataSource;

typedef enum
{
  GPU_NODE_LINK_NONE = 0,
  GPU_NODE_LINK_ATTR,
  GPU_NODE_LINK_UNIFORM_ATTR,
  GPU_NODE_LINK_LAYER_ATTR,
  GPU_NODE_LINK_COLORBAND,
  GPU_NODE_LINK_CONSTANT,
  GPU_NODE_LINK_IMAGE,
  GPU_NODE_LINK_IMAGE_TILED,
  GPU_NODE_LINK_IMAGE_TILED_MAPPING,
  GPU_NODE_LINK_IMAGE_SKY,
  GPU_NODE_LINK_OUTPUT,
  GPU_NODE_LINK_UNIFORM,
  GPU_NODE_LINK_DIFFERENTIATE_FLOAT_FN,
} GPUNodeLinkType;

typedef enum
{
  GPU_NODE_TAG_NONE = 0,
  GPU_NODE_TAG_SURFACE = (1 << 0),
  GPU_NODE_TAG_VOLUME = (1 << 1),
  GPU_NODE_TAG_DISPLACEMENT = (1 << 2),
  GPU_NODE_TAG_THICKNESS = (1 << 3),
  GPU_NODE_TAG_AOV = (1 << 4),
  GPU_NODE_TAG_FUNCTION = (1 << 5),
  GPU_NODE_TAG_COMPOSITOR = (1 << 6),
} eGPUNodeTag;

ENUM_OPERATORS(eGPUNodeTag, GPU_NODE_TAG_FUNCTION)

struct GPUNode
{
  struct GPUNode *next, *prev;

  const char *name;

  /* Internal flag to mark nodes during pruning */
  eGPUNodeTag tag;

  ListBase inputs;
  ListBase outputs;
};

struct GPUNodeLink
{
  GPUNodeStack *socket;

  GPUNodeLinkType link_type;
  int users; /* Refcount */

  union
  {
    /* GPU_NODE_LINK_CONSTANT | GPU_NODE_LINK_UNIFORM */
    const float *data;
    /* GPU_NODE_LINK_COLORBAND */
    struct GPUTexture **colorband;
    /* GPU_NODE_LINK_OUTPUT */
    struct GPUOutput *output;
    /* GPU_NODE_LINK_ATTR */
    struct GPUMaterialAttribute *attr;
    /* GPU_NODE_LINK_UNIFORM_ATTR */
    struct GPUUniformAttr *uniform_attr;
    /* GPU_NODE_LINK_LAYER_ATTR */
    struct GPULayerAttr *layer_attr;
    /* GPU_NODE_LINK_IMAGE_BLENDER */
    struct GPUMaterialTexture *texture;
    /* GPU_NODE_LINK_DIFFERENTIATE_FLOAT_FN */
    const char *function_name;
  };
};

typedef struct GPUOutput
{
  struct GPUOutput *next, *prev;

  GPUNode *node;
  eGPUType type;     /* data type = length of vector/matrix */
  GPUNodeLink *link; /* output link */
  int id;            /* unique id as created by code generator */
} GPUOutput;

typedef struct GPUInput
{
  struct GPUInput *next, *prev;

  GPUNode *node;
  eGPUType type; /* data-type. */
  GPUNodeLink *link;
  int id; /* unique id as created by code generator */

  eGPUDataSource source; /* data source */

  /* Content based on eGPUDataSource */
  union
  {
    /* GPU_SOURCE_CONSTANT | GPU_SOURCE_UNIFORM */
    float vec[16]; /* vector data */
    /* GPU_SOURCE_TEX | GPU_SOURCE_TEX_TILED_MAPPING */
    struct GPUMaterialTexture *texture;
    /* GPU_SOURCE_ATTR */
    struct GPUMaterialAttribute *attr;
    /* GPU_SOURCE_UNIFORM_ATTR */
    struct GPUUniformAttr *uniform_attr;
    /* GPU_SOURCE_LAYER_ATTR */
    struct GPULayerAttr *layer_attr;
    /* GPU_SOURCE_FUNCTION_CALL */
    char function_call[64];
  };
} GPUInput;

typedef struct GPUNodeGraphOutputLink
{
  struct GPUNodeGraphOutputLink *next, *prev;
  int hash;
  GPUNodeLink *outlink;
} GPUNodeGraphOutputLink;

typedef struct GPUNodeGraphFunctionLink
{
  struct GPUNodeGraphFunctionLink *next, *prev;
  char name[16];
  GPUNodeLink *outlink;
} GPUNodeGraphFunctionLink;

typedef struct GPUNodeGraph
{
  /* Nodes */
  ListBase nodes;

  /* Main Outputs. */
  GPUNodeLink *outlink_surface;
  GPUNodeLink *outlink_volume;
  GPUNodeLink *outlink_displacement;
  GPUNodeLink *outlink_thickness;
  /* List of GPUNodeGraphOutputLink */
  ListBase outlink_aovs;
  /* List of GPUNodeGraphFunctionLink */
  ListBase material_functions;
  /* List of GPUNodeGraphOutputLink */
  ListBase outlink_compositor;

  /* Requested attributes and textures. */
  ListBase attributes;
  ListBase textures;

  /* The list of uniform attributes. */
  GPUUniformAttrList uniform_attrs;

  /* The list of layer attributes. */
  ListBase layer_attrs;

  /** Set of all the GLSL lib code blocks . */
  RSet *used_libraries;
} GPUNodeGraph;

/* Node Graph */

void gpu_node_graph_prune_unused(GPUNodeGraph *graph);
void gpu_node_graph_finalize_uniform_attrs(GPUNodeGraph *graph);

/**
 * Free intermediate node graph.
 */
void gpu_node_graph_free_nodes(GPUNodeGraph *graph);
/**
 * Free both node graph and requested attributes and textures.
 */
void gpu_node_graph_free(GPUNodeGraph *graph);

/* Material calls */

struct GPUNodeGraph *gpu_material_node_graph(struct GPUMaterial *material);
/**
 * Returns the address of the future pointer to coba_tex.
 */
struct GPUTexture **gpu_material_ramp_texture_row_set(struct GPUMaterial *mat,
                                                      int size,
                                                      float *pixels,
                                                      float *row);
/**
 * Returns the address of the future pointer to sky_tex
 */
struct GPUTexture **gpu_material_sky_texture_layer_set(struct GPUMaterial *mat,
                                                       int width,
                                                       int height,
                                                       const float *pixels,
                                                       float *layer);

#ifdef __cplusplus
}
#endif

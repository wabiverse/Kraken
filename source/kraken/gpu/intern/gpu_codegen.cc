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

/**
 * @file
 * @ingroup GPU.
 * Pixel Magic.
 *
 * Convert material node-trees to GLSL.
 */

#include "MEM_guardedalloc.h"

#include "USD_customdata_types.h"
// #include "USD_image_types.h"
// #include "USD_material_types.h"

#include "KLI_rhash.h"
#include "KLI_hash_mm2a.h"
#include "KLI_link_utils.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"

#include "KLI_time.h"

#include "KKE_cryptomatte.hh"
// #include "KKE_material.h"

#include "GPU_capabilities.h"
#include "GPU_material.h"
#include "GPU_shader.h"
#include "GPU_uniform_buffer.h"
#include "GPU_vertex_format.h"

#include "KLI_sys_types.h" /* for intptr_t support */
#include "KLI_vector.hh"

#include "gpu_codegen.h"
#include "gpu_node_graph.h"
#include "gpu_shader_create_info.hh"
#include "gpu_shader_dependency_private.h"

#include <cstdarg>
#include <cstring>

#include <sstream>
#include <string>

using namespace kraken::gpu::shader;

/**
 * IMPORTANT: Never add external reference. The GPUMaterial used to create the GPUPass (and its
 * GPUCodegenCreateInfo) can be free before actually compiling. This happens if there is an update
 * before deferred compilation happens and the GPUPass gets picked up by another GPUMaterial
 * (because of GPUPass reuse).
 */
struct GPUCodegenCreateInfo : ShaderCreateInfo
{
  struct NameBuffer
  {
    using NameEntry = std::array<char, 32>;

    /** Duplicate attribute names to avoid reference the GPUNodeGraph directly. */
    char attr_names[16][GPU_MAX_SAFE_ATTR_NAME + 1];
    char var_names[16][8];
    kraken::Vector<std::unique_ptr<NameEntry>, 16> sampler_names;

    /* Returns the appended name memory location */
    const char *append_sampler_name(const char name[32])
    {
      auto index = sampler_names.size();
      sampler_names.append(std::make_unique<NameEntry>());
      char *name_buffer = sampler_names[index]->data();
      memcpy(name_buffer, name, 32);
      return name_buffer;
    }
  };

  /** Optional generated interface. */
  StageInterfaceInfo *interface_generated = nullptr;
  /** Optional name buffer containing names referenced by StringRefNull. */
  NameBuffer name_buffer;

  GPUCodegenCreateInfo(const char *name) : ShaderCreateInfo(name){};
  ~GPUCodegenCreateInfo()
  {
    delete interface_generated;
  };
};

struct GPUPass
{
  struct GPUPass *next;

  GPUShader *shader;
  GPUCodegenCreateInfo *create_info = nullptr;
  /** Orphaned GPUPasses gets freed by the garbage collector. */
  uint refcount;
  /** Identity hash generated from all GLSL code. */
  uint32_t hash;
  /** Did we already tried to compile the attached GPUShader. */
  bool compiled;
};

/* -------------------------------------------------------------------- */
/** \name GPUPass Cache
 *
 * Internal shader cache: This prevent the shader recompilation / stall when
 * using undo/redo AND also allows for GPUPass reuse if the Shader code is the
 * same for 2 different Materials. Unused GPUPasses are free by Garbage collection.
 * \{ */

/* Only use one linklist that contains the GPUPasses grouped by hash. */
static GPUPass *pass_cache = nullptr;
static SpinLock pass_cache_spin;

/* Search by hash only. Return first pass with the same hash.
 * There is hash collision if (pass->next && pass->next->hash == hash) */
static GPUPass *gpu_pass_cache_lookup(uint32_t hash)
{
  KLI_spin_lock(&pass_cache_spin);
  /* Could be optimized with a Lookup table. */
  for (GPUPass *pass = pass_cache; pass; pass = pass->next) {
    if (pass->hash == hash) {
      KLI_spin_unlock(&pass_cache_spin);
      return pass;
    }
  }
  KLI_spin_unlock(&pass_cache_spin);
  return nullptr;
}

static void gpu_pass_cache_insert_after(GPUPass *node, GPUPass *pass)
{
  KLI_spin_lock(&pass_cache_spin);
  if (node != nullptr) {
    /* Add after the first pass having the same hash. */
    pass->next = node->next;
    node->next = pass;
  } else {
    /* No other pass have same hash, just prepend to the list. */
    KLI_LINKS_PREPEND(pass_cache, pass);
  }
  KLI_spin_unlock(&pass_cache_spin);
}

/* Check all possible passes with the same hash. */
static GPUPass *gpu_pass_cache_resolve_collision(GPUPass *pass,
                                                 GPUShaderCreateInfo *info,
                                                 uint32_t hash)
{
  KLI_spin_lock(&pass_cache_spin);
  for (; pass && (pass->hash == hash); pass = pass->next) {
    if (*reinterpret_cast<ShaderCreateInfo *>(info) ==
        *reinterpret_cast<ShaderCreateInfo *>(pass->create_info)) {
      KLI_spin_unlock(&pass_cache_spin);
      return pass;
    }
  }
  KLI_spin_unlock(&pass_cache_spin);
  return nullptr;
}

static bool gpu_pass_is_valid(GPUPass *pass)
{
  /* Shader is not null if compilation is successful. */
  return (pass->compiled == false || pass->shader != nullptr);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Type > string conversion
 * \{ */

static std::ostream &operator<<(std::ostream &stream, const GPUInput *input)
{
  switch (input->source) {
    case GPU_SOURCE_FUNCTION_CALL:
    case GPU_SOURCE_OUTPUT:
      return stream << "tmp" << input->id;
    case GPU_SOURCE_CONSTANT:
      return stream << "cons" << input->id;
    case GPU_SOURCE_UNIFORM:
      return stream << "node_tree.u" << input->id;
    case GPU_SOURCE_ATTR:
      return stream << "var_attrs.v" << input->attr->id;
    case GPU_SOURCE_UNIFORM_ATTR:
      return stream << "unf_attrs[resource_id].attr" << input->uniform_attr->id;
    case GPU_SOURCE_STRUCT:
      return stream << "strct" << input->id;
    case GPU_SOURCE_TEX:
      return stream << input->texture->sampler_name;
    case GPU_SOURCE_TEX_TILED_MAPPING:
      return stream << input->texture->tiled_mapping_name;
    default:
      KLI_assert(0);
      return stream;
  }
}

static std::ostream &operator<<(std::ostream &stream, const GPUOutput *output)
{
  return stream << "tmp" << output->id;
}

/* Trick type to change overload and keep a somewhat nice syntax. */
struct GPUConstant : public GPUInput
{};

/* Print data constructor (i.e: vec2(1.0f, 1.0f)). */
static std::ostream &operator<<(std::ostream &stream, const GPUConstant *input)
{
  stream << input->type << "(";
  for (int i = 0; i < input->type; i++) {
    char formated_float[32];
    /* Use uint representation to allow exact same bit pattern even if NaN. This is because we can
     * pass UINTs as floats for constants. */
    const uint32_t *uint_vec = reinterpret_cast<const uint32_t *>(input->vec);
    SNPRINTF(formated_float, "uintBitsToFloat(%uu)", uint_vec[i]);
    stream << formated_float;
    if (i < input->type - 1) {
      stream << ", ";
    }
  }
  stream << ")";
  return stream;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GLSL code generation
 * \{ */

class GPUCodegen
{
 public:

  GPUMaterial &mat;
  GPUNodeGraph &graph;
  GPUCodegenOutput output = {};
  GPUCodegenCreateInfo *create_info = nullptr;

 private:

  uint32_t hash_ = 0;
  KLI_HashMurmur2A hm2a_;
  ListBase ubo_inputs_ = {nullptr, nullptr};
  GPUInput *cryptomatte_input_ = nullptr;

 public:

  GPUCodegen(GPUMaterial *mat_, GPUNodeGraph *graph_) : mat(*mat_), graph(*graph_)
  {
    KLI_hash_mm2a_init(&hm2a_, GPU_material_uuid_get(&mat));
    KLI_hash_mm2a_add_int(&hm2a_, GPU_material_flag(&mat));
    create_info = new GPUCodegenCreateInfo("codegen");
    output.create_info = reinterpret_cast<GPUShaderCreateInfo *>(
      static_cast<ShaderCreateInfo *>(create_info));

    if (GPU_material_flag_get(mat_, GPU_MATFLAG_OBJECT_INFO)) {
      create_info->additional_info("draw_object_infos");
    }
  }

  ~GPUCodegen()
  {
    MEM_SAFE_FREE(output.attr_load);
    MEM_SAFE_FREE(output.surface);
    MEM_SAFE_FREE(output.volume);
    MEM_SAFE_FREE(output.thickness);
    MEM_SAFE_FREE(output.displacement);
    MEM_SAFE_FREE(output.composite);
    MEM_SAFE_FREE(output.material_functions);
    MEM_SAFE_FREE(cryptomatte_input_);
    delete create_info;
    KLI_freelistN(&ubo_inputs_);
  };

  void generate_graphs();
  void generate_cryptomatte();
  void generate_uniform_buffer();
  void generate_attribs();
  void generate_resources();
  void generate_library();

  uint32_t hash_get() const
  {
    return hash_;
  }

 private:

  void set_unique_ids();

  void node_serialize(std::stringstream &eval_ss, const GPUNode *node);
  char *graph_serialize(eGPUNodeTag tree_tag, GPUNodeLink *output_link);
  char *graph_serialize(eGPUNodeTag tree_tag);

  static char *extract_c_str(std::stringstream &stream)
  {
    auto string = stream.str();
    return KLI_strdup(string.c_str());
  }
};

void GPUCodegen::generate_attribs()
{
  if (KLI_listbase_is_empty(&graph.attributes)) {
    output.attr_load = nullptr;
    return;
  }

  GPUCodegenCreateInfo &info = *create_info;

  info.interface_generated = new StageInterfaceInfo("codegen_iface", "var_attrs");
  StageInterfaceInfo &iface = *info.interface_generated;
  info.vertex_out(iface);

  /* Input declaration, loading / assignment to interface and geometry shader passthrough. */
  std::stringstream load_ss;

  int slot = 15;
  LISTBASE_FOREACH(GPUMaterialAttribute *, attr, &graph.attributes)
  {
    if (slot == -1) {
      KLI_assert_msg(0, "Too many attributes");
      break;
    }
    STRNCPY(info.name_buffer.attr_names[slot], attr->input_name);
    SNPRINTF(info.name_buffer.var_names[slot], "v%d", attr->id);

    kraken::StringRefNull attr_name = info.name_buffer.attr_names[slot];
    kraken::StringRefNull var_name = info.name_buffer.var_names[slot];

    eGPUType input_type, iface_type;

    load_ss << "var_attrs." << var_name;
    switch (attr->type) {
      case CD_ORCO:
        /* Need vec4 to detect usage of default attribute. */
        input_type = GPU_VEC4;
        iface_type = GPU_VEC3;
        load_ss << " = attr_load_orco(" << attr_name << ");\n";
        break;
      case CD_HAIRLENGTH:
        iface_type = input_type = GPU_FLOAT;
        load_ss << " = attr_load_" << input_type << "(" << attr_name << ");\n";
        break;
      case CD_TANGENT:
        iface_type = input_type = GPU_VEC4;
        load_ss << " = attr_load_tangent(" << attr_name << ");\n";
        break;
      default:
        iface_type = input_type = GPU_VEC4;
        load_ss << " = attr_load_" << input_type << "(" << attr_name << ");\n";
        break;
    }

    info.vertex_in(slot--, to_type(input_type), attr_name);
    iface.smooth(to_type(iface_type), var_name);
  }

  output.attr_load = extract_c_str(load_ss);
}

void GPUCodegen::generate_resources()
{
  GPUCodegenCreateInfo &info = *create_info;

  /* Ref. T98190: Defines are optimizations for old compilers.
   * Might become unnecessary with EEVEE-Next. */
  if (GPU_material_flag_get(&mat, GPU_MATFLAG_PRINCIPLED_CLEARCOAT)) {
    info.define("PRINCIPLED_CLEARCOAT");
  }
  if (GPU_material_flag_get(&mat, GPU_MATFLAG_PRINCIPLED_METALLIC)) {
    info.define("PRINCIPLED_METALLIC");
  }
  if (GPU_material_flag_get(&mat, GPU_MATFLAG_PRINCIPLED_DIELECTRIC)) {
    info.define("PRINCIPLED_DIELECTRIC");
  }
  if (GPU_material_flag_get(&mat, GPU_MATFLAG_PRINCIPLED_GLASS)) {
    info.define("PRINCIPLED_GLASS");
  }
  if (GPU_material_flag_get(&mat, GPU_MATFLAG_PRINCIPLED_ANY)) {
    info.define("PRINCIPLED_ANY");
  }

  std::stringstream ss;

  /* Textures. */
  int slot = 0;
  LISTBASE_FOREACH(GPUMaterialTexture *, tex, &graph.textures)
  {
    if (tex->colorband) {
      const char *name = info.name_buffer.append_sampler_name(tex->sampler_name);
      info.sampler(slot++, ImageType::FLOAT_1D_ARRAY, name, Frequency::BATCH);
    } else if (tex->sky) {
      const char *name = info.name_buffer.append_sampler_name(tex->sampler_name);
      info.sampler(0, ImageType::FLOAT_2D_ARRAY, name, Frequency::BATCH);
    } else if (tex->tiled_mapping_name[0] != '\0') {
      const char *name = info.name_buffer.append_sampler_name(tex->sampler_name);
      info.sampler(slot++, ImageType::FLOAT_2D_ARRAY, name, Frequency::BATCH);

      const char *name_mapping = info.name_buffer.append_sampler_name(tex->tiled_mapping_name);
      info.sampler(slot++, ImageType::FLOAT_1D_ARRAY, name_mapping, Frequency::BATCH);
    } else {
      const char *name = info.name_buffer.append_sampler_name(tex->sampler_name);
      info.sampler(slot++, ImageType::FLOAT_2D, name, Frequency::BATCH);
    }
  }

  if (!KLI_listbase_is_empty(&ubo_inputs_)) {
    /* NOTE: generate_uniform_buffer() should have sorted the inputs before this. */
    ss << "struct NodeTree {\n";
    LISTBASE_FOREACH(LinkData *, link, &ubo_inputs_)
    {
      GPUInput *input = (GPUInput *)(link->data);
      if (input->source == GPU_SOURCE_CRYPTOMATTE) {
        ss << input->type << " crypto_hash;\n";
      } else {
        ss << input->type << " u" << input->id << ";\n";
      }
    }
    ss << "};\n\n";

    info.uniform_buf(1, "NodeTree", GPU_UBO_BLOCK_NAME, Frequency::BATCH);
  }

  if (!KLI_listbase_is_empty(&graph.uniform_attrs.list)) {
    ss << "struct UniformAttrs {\n";
    LISTBASE_FOREACH(GPUUniformAttr *, attr, &graph.uniform_attrs.list)
    {
      ss << "vec4 attr" << attr->id << ";\n";
    }
    ss << "};\n\n";

    /* TODO(fclem): Use the macro for length. Currently not working for EEVEE. */
    /* DRW_RESOURCE_CHUNK_LEN = 512 */
    info.uniform_buf(2, "UniformAttrs", GPU_ATTRIBUTE_UBO_BLOCK_NAME "[512]", Frequency::BATCH);
  }

  info.typedef_source_generated = ss.str();
}

void GPUCodegen::generate_library()
{
  GPUCodegenCreateInfo &info = *create_info;

  void *value;
  RSetIterState pop_state = {};
  while (KLI_rset_pop(graph.used_libraries, &pop_state, &value)) {
    auto deps = gpu_shader_dependency_get_resolved_source((const char *)value);
    info.dependencies_generated.extend_non_duplicates(deps);
  }
}

void GPUCodegen::node_serialize(std::stringstream &eval_ss, const GPUNode *node)
{
  /* Declare constants. */
  LISTBASE_FOREACH(GPUInput *, input, &node->inputs)
  {
    switch (input->source) {
      case GPU_SOURCE_FUNCTION_CALL:
        eval_ss << input->type << " " << input << "; " << input->function_call << input << ");\n";
        break;
      case GPU_SOURCE_STRUCT:
        eval_ss << input->type << " " << input << " = CLOSURE_DEFAULT;\n";
        break;
      case GPU_SOURCE_CONSTANT:
        eval_ss << input->type << " " << input << " = " << (GPUConstant *)input << ";\n";
        break;
      default:
        break;
    }
  }
  /* Declare temporary variables for node output storage. */
  LISTBASE_FOREACH(GPUOutput *, output, &node->outputs)
  {
    eval_ss << output->type << " " << output << ";\n";
  }

  /* Function call. */
  eval_ss << node->name << "(";
  /* Input arguments. */
  LISTBASE_FOREACH(GPUInput *, input, &node->inputs)
  {
    switch (input->source) {
      case GPU_SOURCE_OUTPUT:
      case GPU_SOURCE_ATTR: {
        /* These inputs can have non matching types. Do conversion. */
        eGPUType to = input->type;
        eGPUType from = (input->source == GPU_SOURCE_ATTR) ? input->attr->gputype :
                                                             input->link->output->type;
        if (from != to) {
          /* Use defines declared inside codegen_lib (i.e: vec4_from_float). */
          eval_ss << to << "_from_" << from << "(";
        }

        if (input->source == GPU_SOURCE_ATTR) {
          eval_ss << input;
        } else {
          eval_ss << input->link->output;
        }

        if (from != to) {
          eval_ss << ")";
        }
        break;
      }
      default:
        eval_ss << input;
        break;
    }
    eval_ss << ", ";
  }
  /* Output arguments. */
  LISTBASE_FOREACH(GPUOutput *, output, &node->outputs)
  {
    eval_ss << output;
    if (output->next) {
      eval_ss << ", ";
    }
  }
  eval_ss << ");\n\n";
}

char *GPUCodegen::graph_serialize(eGPUNodeTag tree_tag, GPUNodeLink *output_link)
{
  if (output_link == nullptr) {
    return nullptr;
  }

  std::stringstream eval_ss;
  /* NOTE: The node order is already top to bottom (or left to right in node editor)
   * because of the evaluation order inside ntreeExecGPUNodes(). */
  LISTBASE_FOREACH(GPUNode *, node, &graph.nodes)
  {
    if ((node->tag & tree_tag) == 0) {
      continue;
    }
    node_serialize(eval_ss, node);
  }
  eval_ss << "return " << output_link->output << ";\n";

  char *eval_c_str = extract_c_str(eval_ss);
  KLI_hash_mm2a_add(&hm2a_, (uchar *)eval_c_str, eval_ss.str().size());
  return eval_c_str;
}

char *GPUCodegen::graph_serialize(eGPUNodeTag tree_tag)
{
  std::stringstream eval_ss;
  LISTBASE_FOREACH(GPUNode *, node, &graph.nodes)
  {
    if (node->tag & tree_tag) {
      node_serialize(eval_ss, node);
    }
  }
  char *eval_c_str = extract_c_str(eval_ss);
  KLI_hash_mm2a_add(&hm2a_, (uchar *)eval_c_str, eval_ss.str().size());
  return eval_c_str;
}

void GPUCodegen::generate_cryptomatte()
{
  cryptomatte_input_ = static_cast<GPUInput *>(MEM_callocN(sizeof(GPUInput), __func__));
  cryptomatte_input_->type = GPU_FLOAT;
  cryptomatte_input_->source = GPU_SOURCE_CRYPTOMATTE;

  float material_hash = 0.0f;
  Material *material = GPU_material_get_material(&mat);
  if (material) {
    kraken::kke::cryptomatte::CryptomatteHash hash(material->id.name,
                                                   KLI_strnlen(material->id.name, MAX_NAME - 2));
    material_hash = hash.float_encoded();
  }
  cryptomatte_input_->vec[0] = material_hash;

  KLI_addtail(&ubo_inputs_, KLI_genericNodeN(cryptomatte_input_));
}

void GPUCodegen::generate_uniform_buffer()
{
  /* Extract uniform inputs. */
  LISTBASE_FOREACH(GPUNode *, node, &graph.nodes)
  {
    LISTBASE_FOREACH(GPUInput *, input, &node->inputs)
    {
      if (input->source == GPU_SOURCE_UNIFORM && !input->link) {
        /* We handle the UBO uniforms separately. */
        KLI_addtail(&ubo_inputs_, KLI_genericNodeN(input));
      }
    }
  }
  if (!KLI_listbase_is_empty(&ubo_inputs_)) {
    /* This sorts the inputs based on size. */
    GPU_material_uniform_buffer_create(&mat, &ubo_inputs_);
  }
}

/* Sets id for unique names for all inputs, resources and temp variables. */
void GPUCodegen::set_unique_ids()
{
  int id = 1;
  LISTBASE_FOREACH(GPUNode *, node, &graph.nodes)
  {
    LISTBASE_FOREACH(GPUInput *, input, &node->inputs)
    {
      input->id = id++;
    }
    LISTBASE_FOREACH(GPUOutput *, output, &node->outputs)
    {
      output->id = id++;
    }
  }
}

void GPUCodegen::generate_graphs()
{
  set_unique_ids();

  output.surface = graph_serialize(GPU_NODE_TAG_SURFACE | GPU_NODE_TAG_AOV, graph.outlink_surface);
  output.volume = graph_serialize(GPU_NODE_TAG_VOLUME, graph.outlink_volume);
  output.displacement = graph_serialize(GPU_NODE_TAG_DISPLACEMENT, graph.outlink_displacement);
  output.thickness = graph_serialize(GPU_NODE_TAG_THICKNESS, graph.outlink_thickness);
  if (!KLI_listbase_is_empty(&graph.outlink_compositor)) {
    output.composite = graph_serialize(GPU_NODE_TAG_COMPOSITOR);
  }

  if (!KLI_listbase_is_empty(&graph.material_functions)) {
    std::stringstream eval_ss;
    eval_ss << "\n/* Generated Functions */\n\n";
    LISTBASE_FOREACH(GPUNodeGraphFunctionLink *, func_link, &graph.material_functions)
    {
      char *fn = graph_serialize(GPU_NODE_TAG_FUNCTION, func_link->outlink);
      eval_ss << "float " << func_link->name << "() {\n" << fn << "}\n\n";
      MEM_SAFE_FREE(fn);
    }
    output.material_functions = extract_c_str(eval_ss);
  }

  LISTBASE_FOREACH(GPUMaterialAttribute *, attr, &graph.attributes)
  {
    KLI_hash_mm2a_add(&hm2a_, (uchar *)attr->name, strlen(attr->name));
  }

  hash_ = KLI_hash_mm2a_end(&hm2a_);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GPUPass
 * \{ */

GPUPass *GPU_generate_pass(GPUMaterial *material,
                           GPUNodeGraph *graph,
                           GPUCodegenCallbackFn finalize_source_cb,
                           void *thunk)
{
  gpu_node_graph_prune_unused(graph);

  /* Extract attributes before compiling so the generated VBOs are ready to accept the future
   * shader. */
  gpu_node_graph_finalize_uniform_attrs(graph);

  GPUCodegen codegen(material, graph);
  codegen.generate_graphs();
  codegen.generate_cryptomatte();
  codegen.generate_uniform_buffer();

  /* Cache lookup: Reuse shaders already compiled. */
  GPUPass *pass_hash = gpu_pass_cache_lookup(codegen.hash_get());

  /* FIXME(fclem): This is broken. Since we only check for the hash and not the full source
   * there is no way to have a collision currently. Some advocated to only use a bigger hash. */
  if (pass_hash && (pass_hash->next == nullptr || pass_hash->next->hash != codegen.hash_get())) {
    if (!gpu_pass_is_valid(pass_hash)) {
      /* Shader has already been created but failed to compile. */
      return nullptr;
    }
    /* No collision, just return the pass. */
    KLI_spin_lock(&pass_cache_spin);
    pass_hash->refcount += 1;
    KLI_spin_unlock(&pass_cache_spin);
    return pass_hash;
  }

  /* Either the shader is not compiled or there is a hash collision...
   * continue generating the shader strings. */
  codegen.generate_attribs();
  codegen.generate_resources();
  codegen.generate_library();

  /* Make engine add its own code and implement the generated functions. */
  finalize_source_cb(thunk, material, &codegen.output);

  GPUPass *pass = nullptr;
  if (pass_hash) {
    /* Cache lookup: Reuse shaders already compiled. */
    pass = gpu_pass_cache_resolve_collision(pass_hash,
                                            codegen.output.create_info,
                                            codegen.hash_get());
  }

  if (pass) {
    /* Cache hit. Reuse the same GPUPass and GPUShader. */
    if (!gpu_pass_is_valid(pass)) {
      /* Shader has already been created but failed to compile. */
      return nullptr;
    }
    KLI_spin_lock(&pass_cache_spin);
    pass->refcount += 1;
    KLI_spin_unlock(&pass_cache_spin);
  } else {
    /* We still create a pass even if shader compilation
     * fails to avoid trying to compile again and again. */
    pass = (GPUPass *)MEM_callocN(sizeof(GPUPass), "GPUPass");
    pass->shader = nullptr;
    pass->refcount = 1;
    pass->create_info = codegen.create_info;
    pass->hash = codegen.hash_get();
    pass->compiled = false;

    codegen.create_info = nullptr;

    gpu_pass_cache_insert_after(pass_hash, pass);
  }
  return pass;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Compilation
 * \{ */

static int count_active_texture_sampler(GPUPass *pass, GPUShader *shader)
{
  int num_samplers = 0;

  for (const ShaderCreateInfo::Resource &res : pass->create_info->pass_resources_) {
    if (res.bind_type == ShaderCreateInfo::Resource::BindType::SAMPLER) {
      if (GPU_shader_get_uniform(shader, res.sampler.name.c_str()) != -1) {
        num_samplers += 1;
      }
    }
  }

  return num_samplers;
}

static bool gpu_pass_shader_validate(GPUPass *pass, GPUShader *shader)
{
  if (shader == nullptr) {
    return false;
  }

  /* NOTE: The only drawback of this method is that it will count a sampler
   * used in the fragment shader and only declared (but not used) in the vertex
   * shader as used by both. But this corner case is not happening for now. */
  int active_samplers_len = count_active_texture_sampler(pass, shader);

  /* Validate against opengl limit. */
  if ((active_samplers_len > GPU_max_textures_frag()) ||
      (active_samplers_len > GPU_max_textures_vert())) {
    return false;
  }

  if (pass->create_info->geometry_source_.is_empty() == false) {
    if (active_samplers_len > GPU_max_textures_geom()) {
      return false;
    }
  }

  return (active_samplers_len * 3 <= GPU_max_textures());
}

bool GPU_pass_compile(GPUPass *pass, const char *shname)
{
  bool success = true;
  if (!pass->compiled) {
    GPUShaderCreateInfo *info = reinterpret_cast<GPUShaderCreateInfo *>(
      static_cast<ShaderCreateInfo *>(pass->create_info));

    pass->create_info->name_ = shname;

    GPUShader *shader = GPU_shader_create_from_info(info);

    /* NOTE: Some drivers / gpu allows more active samplers than the opengl limit.
     * We need to make sure to count active samplers to avoid undefined behavior. */
    if (!gpu_pass_shader_validate(pass, shader)) {
      success = false;
      if (shader != nullptr) {
        fprintf(stderr, "GPUShader: error: too many samplers in shader.\n");
        GPU_shader_free(shader);
        shader = nullptr;
      }
    }
    pass->shader = shader;
    pass->compiled = true;
  }
  return success;
}

GPUShader *GPU_pass_shader_get(GPUPass *pass)
{
  return pass->shader;
}

void GPU_pass_release(GPUPass *pass)
{
  KLI_spin_lock(&pass_cache_spin);
  KLI_assert(pass->refcount > 0);
  pass->refcount--;
  KLI_spin_unlock(&pass_cache_spin);
}

static void gpu_pass_free(GPUPass *pass)
{
  KLI_assert(pass->refcount == 0);
  if (pass->shader) {
    GPU_shader_free(pass->shader);
  }
  delete pass->create_info;
  MEM_freeN(pass);
}

void GPU_pass_cache_garbage_collect(void)
{
  static int lasttime = 0;
  const int shadercollectrate = 60; /* hardcoded for now. */
  int ctime = int(PIL_check_seconds_timer());

  if (ctime < shadercollectrate + lasttime) {
    return;
  }

  lasttime = ctime;

  KLI_spin_lock(&pass_cache_spin);
  GPUPass *next, **prev_pass = &pass_cache;
  for (GPUPass *pass = pass_cache; pass; pass = next) {
    next = pass->next;
    if (pass->refcount == 0) {
      /* Remove from list */
      *prev_pass = next;
      gpu_pass_free(pass);
    } else {
      prev_pass = &pass->next;
    }
  }
  KLI_spin_unlock(&pass_cache_spin);
}

void GPU_pass_cache_init(void)
{
  KLI_spin_init(&pass_cache_spin);
}

void GPU_pass_cache_free(void)
{
  KLI_spin_lock(&pass_cache_spin);
  while (pass_cache) {
    GPUPass *next = pass_cache->next;
    gpu_pass_free(pass_cache);
    pass_cache = next;
  }
  KLI_spin_unlock(&pass_cache_spin);

  KLI_spin_end(&pass_cache_spin);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Module
 * \{ */

/* TODO: Put this in a materials file on it's own. */
static Material default_material_empty;
static Material default_material_holdout;
static Material default_material_surface;
static Material default_material_volume;
static Material default_material_gpencil;

static Material *default_materials[] = {&default_material_empty,
                                        &default_material_holdout,
                                        &default_material_surface,
                                        &default_material_volume,
                                        &default_material_gpencil,
                                        NULL};

static void KKE_material_defaults_free_gpu(void)
{
  for (int i = 0; default_materials[i]; i++) {
    Material *ma = default_materials[i];
    if (ma->gpumaterial.first) {
      GPU_material_free(&ma->gpumaterial);
    }
  }
}


void gpu_codegen_init(void) {}

void gpu_codegen_exit(void)
{
  // KKE_world_defaults_free_gpu();
  KKE_material_defaults_free_gpu();
  GPU_shader_free_builtin_shaders();
}

/** \} */
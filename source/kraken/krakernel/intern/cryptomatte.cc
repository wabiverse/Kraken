/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2020 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup bke
 */

#include "KKE_cryptomatte.h"
#include "KKE_cryptomatte.hh"
// #include "KKE_image.h"
#include "KKE_main.h"

// #include "USD_layer_types.h"
// #include "USD_material_types.h"
// #include "USD_node_types.h"
// #include "USD_object_types.h"
#include "USD_scene_types.h"
#include "USD_scene.h"

#include "KLI_compiler_attrs.h"
#include "KLI_dynstr.h"
#include "KLI_hash_mm3.h"
#include "KLI_listbase.h"
#include "KLI_string.h"

// #include "RE_pipeline.h"

#include "MEM_guardedalloc.h"

#include <cctype>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>


/* stampdata doesn't belong here. Move it where it belongs. */


inline void KKE_stamp_info_callback(void *data,
                                    struct StampData *stamp_data,
                                    StampCallback callback,
                                    bool noskip)
{
  if ((callback == nullptr) || (stamp_data == nullptr)) {
    return;
  }

#define CALL(member, value_str)                                                \
  if (noskip || stamp_data->member[0]) {                                       \
    callback(data, value_str, stamp_data->member, sizeof(stamp_data->member)); \
  }                                                                            \
  ((void)0)

  CALL(file, "File");
  CALL(note, "Note");
  CALL(date, "Date");
  CALL(marker, "Marker");
  CALL(time, "Time");
  CALL(frame, "Frame");
  CALL(frame_range, "FrameRange");
  CALL(camera, "Camera");
  CALL(cameralens, "Lens");
  CALL(scene, "Scene");
  CALL(strip, "Strip");
  CALL(rendertime, "RenderTime");
  CALL(memory, "Memory");
  CALL(hostname, "Hostname");

  LISTBASE_FOREACH(StampDataCustomField *, custom_field, &stamp_data->custom_fields)
  {
    if (noskip || custom_field->value[0]) {
      callback(data, custom_field->key, custom_field->value, strlen(custom_field->value) + 1);
    }
  }

#undef CALL
}

inline void KKE_render_result_stamp_data(RenderResult *rr, const char *key, const char *value)
{
  StampData *stamp_data;
  if (rr->stamp_data == nullptr) {
    rr->stamp_data = MEM_cnew<StampData>("RenderResult.stamp_data");
  }
  stamp_data = rr->stamp_data;
  StampDataCustomField *field = static_cast<StampDataCustomField *>(
    MEM_mallocN(sizeof(StampDataCustomField), "StampData Custom Field"));
  STRNCPY(field->key, key);
  field->value = KLI_strdup(value);
  KLI_addtail(&stamp_data->custom_fields, field);
}


struct CryptomatteSession
{
  kraken::Map<std::string, kraken::kke::cryptomatte::CryptomatteLayer> layers;
  /* Layer names in order of creation. */
  kraken::Vector<std::string> layer_names;

  CryptomatteSession() = default;
  CryptomatteSession(const Main *kmain);
  CryptomatteSession(StampData *stamp_data);
  CryptomatteSession(const ViewLayer *view_layer);
  CryptomatteSession(const kScene *scene);
  void init(const ViewLayer *view_layer);

  kraken::kke::cryptomatte::CryptomatteLayer &add_layer(std::string layer_name);
  std::optional<std::string> operator[](float encoded_hash) const;

#ifdef WITH_CXX_GUARDEDALLOC
  MEM_CXX_CLASS_ALLOC_FUNCS("cryptomatte:CryptomatteSession")
#endif
};

CryptomatteSession::CryptomatteSession(const Main *kmain)
{
  if (!KLI_listbase_is_empty(&kmain->objects)) {
    kraken::kke::cryptomatte::CryptomatteLayer &objects = add_layer(
      RE_PASSNAME_CRYPTOMATTE_OBJECT);
    LISTBASE_FOREACH(ID *, id, &kmain->objects)
    {
      objects.add_ID(*id);
    }
  }
  if (!KLI_listbase_is_empty(&kmain->materials)) {
    kraken::kke::cryptomatte::CryptomatteLayer &materials = add_layer(
      RE_PASSNAME_CRYPTOMATTE_MATERIAL);
    LISTBASE_FOREACH(ID *, id, &kmain->materials)
    {
      materials.add_ID(*id);
    }
  }
}

CryptomatteSession::CryptomatteSession(StampData *stamp_data)
{
  kraken::kke::cryptomatte::CryptomatteStampDataCallbackData callback_data;
  callback_data.session = this;
  KKE_stamp_info_callback(
    &callback_data,
    stamp_data,
    kraken::kke::cryptomatte::CryptomatteStampDataCallbackData::extract_layer_names,
    false);
  KKE_stamp_info_callback(
    &callback_data,
    stamp_data,
    kraken::kke::cryptomatte::CryptomatteStampDataCallbackData::extract_layer_manifest,
    false);
}

CryptomatteSession::CryptomatteSession(const ViewLayer *view_layer)
{
  init(view_layer);
}

CryptomatteSession::CryptomatteSession(const kScene *scene)
{
  LISTBASE_FOREACH(const ViewLayer *, view_layer, &scene->view_layers)
  {
    init(view_layer);
  }
}

void CryptomatteSession::init(const ViewLayer *view_layer)
{
  eViewLayerCryptomatteFlags cryptoflags = static_cast<eViewLayerCryptomatteFlags>(
    view_layer->cryptomatte_flag & VIEW_LAYER_CRYPTOMATTE_ALL);
  if (cryptoflags == 0) {
    cryptoflags = static_cast<eViewLayerCryptomatteFlags>(VIEW_LAYER_CRYPTOMATTE_ALL);
  }

  if (cryptoflags & VIEW_LAYER_CRYPTOMATTE_OBJECT) {
    add_layer(kraken::StringRefNull(view_layer->name) + "." + RE_PASSNAME_CRYPTOMATTE_OBJECT);
  }
  if (cryptoflags & VIEW_LAYER_CRYPTOMATTE_ASSET) {
    add_layer(kraken::StringRefNull(view_layer->name) + "." + RE_PASSNAME_CRYPTOMATTE_ASSET);
  }
  if (cryptoflags & VIEW_LAYER_CRYPTOMATTE_MATERIAL) {
    add_layer(kraken::StringRefNull(view_layer->name) + "." + RE_PASSNAME_CRYPTOMATTE_MATERIAL);
  }
}

kraken::kke::cryptomatte::CryptomatteLayer &CryptomatteSession::add_layer(std::string layer_name)
{
  if (!layer_names.contains(layer_name)) {
    layer_names.append(layer_name);
  }
  return layers.lookup_or_add_default(layer_name);
}

std::optional<std::string> CryptomatteSession::operator[](float encoded_hash) const
{
  for (const kraken::kke::cryptomatte::CryptomatteLayer &layer : layers.values()) {
    std::optional<std::string> result = layer[encoded_hash];
    if (result) {
      return result;
    }
  }
  return std::nullopt;
}

CryptomatteSession *KKE_cryptomatte_init()
{
  CryptomatteSession *session = new CryptomatteSession();
  return session;
}

struct CryptomatteSession *KKE_cryptomatte_init_from_render_result(
  const struct RenderResult *render_result)
{
  CryptomatteSession *session = new CryptomatteSession(render_result->stamp_data);
  return session;
}

struct CryptomatteSession *KKE_cryptomatte_init_from_scene(const struct kScene *scene)
{
  CryptomatteSession *session = new CryptomatteSession(scene);
  return session;
}

struct CryptomatteSession *KKE_cryptomatte_init_from_view_layer(const struct ViewLayer *view_layer)
{
  CryptomatteSession *session = new CryptomatteSession(view_layer);
  return session;
}

void KKE_cryptomatte_add_layer(struct CryptomatteSession *session, const char *layer_name)
{
  session->add_layer(layer_name);
}

void KKE_cryptomatte_free(CryptomatteSession *session)
{
  KLI_assert(session != nullptr);
  delete session;
}

uint32_t KKE_cryptomatte_hash(const char *name, const int name_len)
{
  kraken::kke::cryptomatte::CryptomatteHash hash(name, name_len);
  return hash.hash;
}

uint32_t KKE_cryptomatte_object_hash(CryptomatteSession *session,
                                     const char *layer_name,
                                     const Object *object)
{
  kraken::kke::cryptomatte::CryptomatteLayer *layer = session->layers.lookup_ptr(layer_name);
  KLI_assert(layer);
  return layer->add_ID(object->id);
}

uint32_t KKE_cryptomatte_material_hash(CryptomatteSession *session,
                                       const char *layer_name,
                                       const Material *material)
{
  if (material == nullptr) {
    return 0.0f;
  }
  kraken::kke::cryptomatte::CryptomatteLayer *layer = session->layers.lookup_ptr(layer_name);
  KLI_assert(layer);
  return layer->add_ID(material->id);
}

uint32_t KKE_cryptomatte_asset_hash(CryptomatteSession *session,
                                    const char *layer_name,
                                    const Object *object)
{
  const Object *asset_object = object;
  while (asset_object->parent != nullptr) {
    asset_object = asset_object->parent;
  }
  return KKE_cryptomatte_object_hash(session, layer_name, asset_object);
}

float KKE_cryptomatte_hash_to_float(uint32_t cryptomatte_hash)
{
  return kraken::kke::cryptomatte::CryptomatteHash(cryptomatte_hash).float_encoded();
}

bool KKE_cryptomatte_find_name(const CryptomatteSession *session,
                               const float encoded_hash,
                               char *r_name,
                               int name_len)
{
  std::optional<std::string> name = (*session)[encoded_hash];
  if (!name) {
    return false;
  }

  KLI_strncpy(r_name, name->c_str(), name_len);
  return true;
}

char *KKE_cryptomatte_entries_to_matte_id(NodeCryptomatte *node_storage)
{
  DynStr *matte_id = KLI_dynstr_new();
  bool first = true;
  LISTBASE_FOREACH(CryptomatteEntry *, entry, &node_storage->entries)
  {
    if (!first) {
      KLI_dynstr_append(matte_id, ",");
    }
    if (KLI_strnlen(entry->name, sizeof(entry->name)) != 0) {
      KLI_dynstr_nappend(matte_id, entry->name, sizeof(entry->name));
    } else {
      KLI_dynstr_appendf(matte_id, "<%.9g>", entry->encoded_hash);
    }
    first = false;
  }
  char *result = KLI_dynstr_get_cstring(matte_id);
  KLI_dynstr_free(matte_id);
  return result;
}

void KKE_cryptomatte_matte_id_to_entries(NodeCryptomatte *node_storage, const char *matte_id)
{
  KLI_freelistN(&node_storage->entries);

  if (matte_id == nullptr) {
    MEM_SAFE_FREE(node_storage->matte_id);
    return;
  }
  /* Update the matte_id so the files can be opened in versions that don't
   * use `CryptomatteEntry`. */
  if (matte_id != node_storage->matte_id && node_storage->matte_id &&
      STREQ(node_storage->matte_id, matte_id)) {
    MEM_SAFE_FREE(node_storage->matte_id);
    node_storage->matte_id = static_cast<char *>(MEM_dupallocN(matte_id));
  }

  std::istringstream ss(matte_id);
  while (ss.good()) {
    CryptomatteEntry *entry = nullptr;
    std::string token;
    getline(ss, token, ',');
    /* Ignore empty tokens. */
    if (token.length() > 0) {
      size_t first = token.find_first_not_of(' ');
      size_t last = token.find_last_not_of(' ');
      if (first == std::string::npos || last == std::string::npos) {
        break;
      }
      token = token.substr(first, (last - first + 1));
      if (*token.begin() == '<' && *(--token.end()) == '>') {
        float encoded_hash = atof(token.substr(1, token.length() - 2).c_str());
        entry = MEM_cnew<CryptomatteEntry>(__func__);
        entry->encoded_hash = encoded_hash;
      } else {
        const char *name = token.c_str();
        int name_len = token.length();
        entry = MEM_cnew<CryptomatteEntry>(__func__);
        STRNCPY(entry->name, name);
        uint32_t hash = KKE_cryptomatte_hash(name, name_len);
        entry->encoded_hash = KKE_cryptomatte_hash_to_float(hash);
      }
    }
    if (entry != nullptr) {
      KLI_addtail(&node_storage->entries, entry);
    }
  }
}

static std::string cryptomatte_determine_name(const ViewLayer *view_layer,
                                              const kraken::StringRefNull cryptomatte_layer_name)
{
  std::stringstream stream;
  const size_t view_layer_name_len = KLI_strnlen(view_layer->name, sizeof(view_layer->name));
  stream << std::string(view_layer->name, view_layer_name_len) << "." << cryptomatte_layer_name;
  return stream.str();
}

static uint32_t cryptomatte_determine_identifier(const kraken::StringRef name)
{
  return KLI_hash_mm3(reinterpret_cast<const uchar *>(name.data()), name.size(), 0);
}

static void add_render_result_meta_data(RenderResult *render_result,
                                        const kraken::StringRef layer_name,
                                        const kraken::StringRefNull key_name,
                                        const kraken::StringRefNull value)
{
  KKE_render_result_stamp_data(
    render_result,
    kraken::kke::cryptomatte::KKE_cryptomatte_meta_data_key(layer_name, key_name).c_str(),
    value.data());
}

void KKE_cryptomatte_store_metadata(const struct CryptomatteSession *session,
                                    RenderResult *render_result,
                                    const ViewLayer *view_layer)
{
  for (const kraken::Map<std::string, kraken::kke::cryptomatte::CryptomatteLayer>::Item item :
       session->layers.items()) {
    const kraken::StringRefNull layer_name(item.key);
    const kraken::kke::cryptomatte::CryptomatteLayer &layer = item.value;

    const std::string manifest = layer.manifest();
    const std::string name = cryptomatte_determine_name(view_layer, layer_name);

    add_render_result_meta_data(render_result, name, "name", name);
    add_render_result_meta_data(render_result, name, "hash", "MurmurHash3_32");
    add_render_result_meta_data(render_result, name, "conversion", "uint32_to_float32");
    add_render_result_meta_data(render_result, name, "manifest", manifest);
  }
}

namespace kraken::kke::cryptomatte
{
  namespace manifest
  {
    constexpr StringRef WHITESPACES = " \t\n\v\f\r";

    static constexpr kraken::StringRef skip_whitespaces_(kraken::StringRef ref)
    {
      size_t skip = ref.find_first_not_of(WHITESPACES);
      if (skip == kraken::StringRef::not_found) {
        return ref;
      }
      return ref.drop_prefix(skip);
    }

    static constexpr int quoted_string_len_(kraken::StringRef ref)
    {
      int len = 1;
      bool skip_next = false;
      while (len < ref.size()) {
        char current_char = ref[len];
        if (skip_next) {
          skip_next = false;
        } else {
          if (current_char == '\\') {
            skip_next = true;
          }
          if (current_char == '\"') {
            len += 1;
            break;
          }
        }
        len += 1;
      }
      return len;
    }

    static std::string unquote_(const kraken::StringRef ref)
    {
      std::ostringstream stream;
      for (char c : ref) {
        if (c != '\\') {
          stream << c;
        }
      }
      return stream.str();
    }

    static bool from_manifest(CryptomatteLayer &layer, kraken::StringRefNull manifest)
    {
      StringRef ref = manifest;
      ref = skip_whitespaces_(ref);
      if (ref.is_empty() || ref.front() != '{') {
        return false;
      }
      ref = ref.drop_prefix(1);
      while (!ref.is_empty()) {
        char front = ref.front();

        if (front == '\"') {
          const int quoted_name_len = quoted_string_len_(ref);
          const int name_len = quoted_name_len - 2;
          std::string name = unquote_(ref.substr(1, name_len));
          ref = ref.drop_prefix(quoted_name_len);
          ref = skip_whitespaces_(ref);

          if (ref.is_empty()) {
            return false;
          }
          char colon = ref.front();
          if (colon != ':') {
            return false;
          }
          ref = ref.drop_prefix(1);
          ref = skip_whitespaces_(ref);

          if (ref.is_empty() || ref.front() != '\"') {
            return false;
          }

          const int quoted_hash_len = quoted_string_len_(ref);
          if (quoted_hash_len < 2) {
            return false;
          }
          const int hash_len = quoted_hash_len - 2;
          CryptomatteHash hash = CryptomatteHash::from_hex_encoded(ref.substr(1, hash_len));
          ref = ref.drop_prefix(quoted_hash_len);
          layer.add_hash(name, hash);
        } else if (front == ',') {
          ref = ref.drop_prefix(1);
        } else if (front == '}') {
          ref = ref.drop_prefix(1);
          ref = skip_whitespaces_(ref);
          break;
        }
        ref = skip_whitespaces_(ref);
      }

      if (!ref.is_empty()) {
        return false;
      }

      return true;
    }

    static std::string to_manifest(const CryptomatteLayer *layer)
    {
      std::stringstream manifest;

      bool is_first = true;
      const kraken::Map<std::string, CryptomatteHash> &const_map = layer->hashes;
      manifest << "{";
      for (kraken::Map<std::string, CryptomatteHash>::Item item : const_map.items()) {
        if (is_first) {
          is_first = false;
        } else {
          manifest << ",";
        }
        manifest << quoted(item.key) << ":\"" << (item.value.hex_encoded()) << "\"";
      }
      manifest << "}";
      return manifest.str();
    }

  }  // namespace manifest

  /* Return the hash of the given cryptomatte layer name.
   *
   * The cryptomatte specification limits the hash to 7 characters.
   * The 7 position limitation solves issues when using cryptomatte together with OpenEXR.
   * The specification suggests to use the first 7 chars of the hashed layer_name.
   */
  static std::string cryptomatte_layer_name_hash(const StringRef layer_name)
  {
    std::stringstream stream;
    const uint32_t render_pass_identifier = cryptomatte_determine_identifier(layer_name);
    stream << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex
           << render_pass_identifier;
    return stream.str().substr(0, 7);
  }

  std::string KKE_cryptomatte_meta_data_key(const StringRef layer_name,
                                            const StringRefNull key_name)
  {
    return "cryptomatte/" + cryptomatte_layer_name_hash(layer_name) + "/" + key_name;
  }

  StringRef KKE_cryptomatte_extract_layer_name(const StringRef render_pass_name)
  {
    int64_t last_token = render_pass_name.size();
    while (last_token > 0 && std::isdigit(render_pass_name[last_token - 1])) {
      last_token -= 1;
    }
    return render_pass_name.substr(0, last_token);
  }

  CryptomatteHash::CryptomatteHash(uint32_t hash) : hash(hash) {}

  CryptomatteHash CryptomatteHash::from_hex_encoded(kraken::StringRef hex_encoded)
  {
    CryptomatteHash result(0);
    std::istringstream(hex_encoded) >> std::hex >> result.hash;
    return result;
  }

  std::string CryptomatteHash::hex_encoded() const
  {
    std::stringstream encoded;
    encoded << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << hash;
    return encoded.str();
  }

  std::unique_ptr<CryptomatteLayer> CryptomatteLayer::read_from_manifest(
    kraken::StringRefNull manifest)
  {
    std::unique_ptr<CryptomatteLayer> layer = std::make_unique<CryptomatteLayer>();
    kraken::kke::cryptomatte::manifest::from_manifest(*layer, manifest);
    return layer;
  }

  uint32_t CryptomatteLayer::add_ID(const ID &id)
  {
    const char *name = &id.name[2];
    const int name_len = KLI_strnlen(name, MAX_NAME - 2);
    uint32_t cryptohash_int = KKE_cryptomatte_hash(name, name_len);

    add_hash(kraken::StringRef(name, name_len), cryptohash_int);

    return cryptohash_int;
  }

  void CryptomatteLayer::add_hash(kraken::StringRef name, CryptomatteHash cryptomatte_hash)
  {
    hashes.add_overwrite(name, cryptomatte_hash);
  }

  std::optional<std::string> CryptomatteLayer::operator[](float encoded_hash) const
  {
    const kraken::Map<std::string, CryptomatteHash> &const_map = hashes;
    for (kraken::Map<std::string, CryptomatteHash>::Item item : const_map.items()) {
      if (KKE_cryptomatte_hash_to_float(item.value.hash) == encoded_hash) {
        return std::make_optional(item.key);
      }
    }
    return std::nullopt;
  }

  std::string CryptomatteLayer::manifest() const
  {
    return kraken::kke::cryptomatte::manifest::to_manifest(this);
  }

  kraken::StringRef CryptomatteStampDataCallbackData::extract_layer_hash(kraken::StringRefNull key)
  {
    KLI_assert(key.startswith("cryptomatte/"));

    size_t start_index = key.find_first_of('/');
    size_t end_index = key.find_last_of('/');
    if (start_index == kraken::StringRef::not_found) {
      return "";
    }
    if (end_index == kraken::StringRef::not_found) {
      return "";
    }
    if (end_index <= start_index) {
      return "";
    }
    return key.substr(start_index + 1, end_index - start_index - 1);
  }

  void CryptomatteStampDataCallbackData::extract_layer_names(void *_data,
                                                             const char *propname,
                                                             char *propvalue,
                                                             int UNUSED(len))
  {
    CryptomatteStampDataCallbackData *data = static_cast<CryptomatteStampDataCallbackData *>(
      _data);

    kraken::StringRefNull key(propname);
    if (!key.startswith("cryptomatte/")) {
      return;
    }
    if (!key.endswith("/name")) {
      return;
    }
    kraken::StringRef layer_hash = extract_layer_hash(key);
    data->hash_to_layer_name.add(layer_hash, propvalue);
  }

  void CryptomatteStampDataCallbackData::extract_layer_manifest(void *_data,
                                                                const char *propname,
                                                                char *propvalue,
                                                                int UNUSED(len))
  {
    CryptomatteStampDataCallbackData *data = static_cast<CryptomatteStampDataCallbackData *>(
      _data);

    kraken::StringRefNull key(propname);
    if (!key.startswith("cryptomatte/")) {
      return;
    }
    if (!key.endswith("/manifest")) {
      return;
    }
    kraken::StringRef layer_hash = extract_layer_hash(key);
    if (!data->hash_to_layer_name.contains(layer_hash)) {
      return;
    }

    kraken::StringRef layer_name = data->hash_to_layer_name.lookup(layer_hash);
    kraken::kke::cryptomatte::CryptomatteLayer &layer = data->session->add_layer(layer_name);
    kraken::kke::cryptomatte::manifest::from_manifest(layer, propvalue);
  }

  const kraken::Vector<std::string> &KKE_cryptomatte_layer_names_get(
    const CryptomatteSession &session)
  {
    return session.layer_names;
  }

  CryptomatteLayer *KKE_cryptomatte_layer_get(CryptomatteSession &session, StringRef layer_name)
  {
    return session.layers.lookup_ptr(layer_name);
  }

}  // namespace kraken::kke::cryptomatte
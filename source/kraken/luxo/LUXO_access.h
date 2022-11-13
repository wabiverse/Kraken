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

#ifndef __LUXO_ACCESS_H__
#define __LUXO_ACCESS_H__

/**
 * @file
 * Luxo.
 * The Universe Gets Animated.
 */

#include <stdarg.h>
#include <stdbool.h>

#include "LUXO_runtime.h"
#include "LUXO_types.h"

#include "KLI_compiler_attrs.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_utils.h"

#include "USD_wm_types.h"
#include "USD_api.h"
#include "USD_ID.h"
#include "USD_types.h"
#include "USD_object.h"

struct Scene;

struct Main;
struct ReportList;
struct kContext;
struct KrakenPROP;

extern KrakenSTAGE KRAKEN_STAGE;

extern KrakenPRIM PRIM_Context;

extern KrakenPRIM PRIM_Action;
extern KrakenPRIM PRIM_Area;
extern KrakenPRIM PRIM_Armature;
extern KrakenPRIM PRIM_Brush;
extern KrakenPRIM PRIM_Camera;
extern KrakenPRIM PRIM_CacheFile;
extern KrakenPRIM PRIM_Collection;
extern KrakenPRIM PRIM_Curve;
extern KrakenPRIM PRIM_GreasePencil;
extern KrakenPRIM PRIM_ID;
extern KrakenPRIM PRIM_Image;
extern KrakenPRIM PRIM_Key;
extern KrakenPRIM PRIM_Light;
extern KrakenPRIM PRIM_LightProbe;
extern KrakenPRIM PRIM_Library;
extern KrakenPRIM PRIM_FreestyleLineStyle;
extern KrakenPRIM PRIM_Lattice;
extern KrakenPRIM PRIM_Material;
extern KrakenPRIM PRIM_MetaBall;
extern KrakenPRIM PRIM_MovieClip;
extern KrakenPRIM PRIM_Mesh;
extern KrakenPRIM PRIM_Mask;
extern KrakenPRIM PRIM_NodeTree;
extern KrakenPRIM PRIM_Object;
extern KrakenPRIM PRIM_ParticleSettings;
extern KrakenPRIM PRIM_Palette;
extern KrakenPRIM PRIM_PaintCurve;
extern KrakenPRIM PRIM_PointCloud;
extern KrakenPRIM PRIM_Region;
extern KrakenPRIM PRIM_Scene;
extern KrakenPRIM PRIM_Screen;
extern KrakenPRIM PRIM_Simulation;
extern KrakenPRIM PRIM_Sound;
extern KrakenPRIM PRIM_Speaker;
extern KrakenPRIM PRIM_Texture;
extern KrakenPRIM PRIM_Text;
extern KrakenPRIM PRIM_VectorFont;
extern KrakenPRIM PRIM_Volume;
extern KrakenPRIM PRIM_Window;
extern KrakenPRIM PRIM_WindowManager;
extern KrakenPRIM PRIM_World;
extern KrakenPRIM PRIM_WorkSpace;

extern KrakenPRIM PRIM_Struct;

extern KrakenPRIM PRIM_StageData;
extern KrakenPRIM PRIM_KrakenPRIM;
extern KrakenPRIM PRIM_KrakenPROP;

bool LUXO_prim_undo_check(const KrakenPRIM *type);

#define PRIM_POINTER_INVALIDATE(ptr) \
  {                                  \
    (ptr)->type = NULL;              \
    (ptr)->owner_id = NULL;          \
  }                                  \
  (void)0

void LUXO_init(void);

const wabi::TfToken LUXO_prim_identifier(const KrakenPRIM *type);
KrakenPRIM *LUXO_prim_base(KrakenPRIM *type);
int LUXO_prim_ui_icon(const KrakenPRIM *type);

bool LUXO_prim_is_ID(const KrakenPRIM *type);

bool LUXO_prim_idprops_register_check(const KrakenPRIM *type);
bool LUXO_prim_idprops_datablock_allowed(const KrakenPRIM *type);
bool LUXO_prim_idprops_contains_datablock(const KrakenPRIM *type);

IDProperty **LUXO_prim_idprops_p(KrakenPRIM *ptr);
IDProperty *LUXO_prim_idprops(KrakenPRIM *ptr, bool create);

IDProperty *prim_idproperty_check(KrakenPROP **prop, KrakenPRIM *ptr);

void *LUXO_prim_py_type_get(KrakenPRIM *sprim);
void LUXO_prim_py_type_set(KrakenPRIM *sprim, void *type);
void *LUXO_prop_py_data_get(KrakenPROP *prop);

bool LUXO_prop_array_check(KrakenPROP *prop);
int LUXO_prop_array_dimension(const KrakenPRIM *ptr, KrakenPROP *prop, int length[]);
int LUXO_prop_multi_array_length(KrakenPRIM *ptr, KrakenPROP *prop, int dim);

void LUXO_save_usd(void);

void LUXO_id_pointer_create(ID *id, KrakenPRIM *r_ptr);
void LUXO_kraken_luxo_pointer_create(KrakenPRIM *r_ptr);
void LUXO_main_pointer_create(Main *main, KrakenPRIM *r_ptr);
void LUXO_pointer_create(ID *id, KrakenPRIM *type, void *data, KrakenPRIM *r_ptr);
void LUXO_stage_pointer_ensure(KrakenPRIM *r_ptr);
KrakenPRIM LUXO_prop_pointer_get(KrakenPRIM *ptr, KrakenPROP *prop);

const wabi::TfToken LUXO_prop_identifier(const KrakenPROP *prop);

PropScaleTYPE LUXO_prop_ui_scale(KrakenPROP *prop);
PropertyType LUXO_prop_type_enum(KrakenPROP *prop);
PropertyType LUXO_prop_type(KrakenPROP *prop);
PropertySubType LUXO_prop_subtype(KrakenPROP *prop);
KrakenPROP *LUXO_prim_find_property(KrakenPRIM *ptr, const char *identifier);

PrimRegisterFUNC LUXO_prim_register(const KrakenPRIM *ptr);
PrimUnregisterFUNC LUXO_prim_unregister(KrakenPRIM *ptr);

void LUXO_object_find_property(KrakenPRIM *ptr, const TfToken &name, KrakenPROP *r_ptr);
void **LUXO_prim_instance(KrakenPRIM *ptr);
bool LUXO_prim_is_a(const KrakenPRIM *type, const KrakenPRIM *sprim);

std::vector<KrakenPRIM *> &LUXO_prim_type_functions(KrakenPRIM *sprim);

const char *LUXO_function_identifier(KrakenFUNC *func);
int LUXO_function_flag(KrakenFUNC *func);
int LUXO_function_defined(KrakenFUNC *func);

KrakenPRIM *sprim_from_ptr(KrakenPRIM *ptr);

void LUXO_collection_begin(KrakenPRIM *ptr, const char *name, CollectionPropIT *iter);
bool LUXO_prop_collection_type_get(KrakenPRIM *ptr, KrakenPROP *prop, KrakenPRIM *r_ptr);
void LUXO_prop_collection_next(CollectionPropIT *iter);
void LUXO_prop_collection_begin(KrakenPRIM *ptr, KrakenPROP *prop, CollectionPropIT *iter);
void LUXO_prop_collection_end(CollectionPropIT *iter);

void LUXO_set_stage_ctx(kContext *C);

char *LUXO_pointer_as_string_id(kContext *C, KrakenPRIM *ptr);
char *LUXO_pointer_as_string_keywords(kContext *C,
                                      KrakenPRIM *ptr,
                                      const bool as_function,
                                      const bool all_args,
                                      const bool nested_args,
                                      const int max_prop_length);
char *LUXO_pointer_as_string_keywords_ex(kContext *C,
                                         KrakenPRIM *ptr,
                                         const bool as_function,
                                         const bool all_args,
                                         const bool nested_args,
                                         const int max_prop_length,
                                         KrakenPROP *iterprop);
char *LUXO_pointer_as_string_id(kContext *C, KrakenPRIM *ptr);

bool LUXO_enum_value_from_id(const EnumPROP *item, const char *identifier, int *r_value);
bool LUXO_enum_id_from_value(const EnumPROP *item, int value, const char **r_identifier);
bool LUXO_enum_icon_from_value(const EnumPROP *item, int value, int *r_icon);
bool LUXO_enum_name_from_value(const EnumPROP *item, int value, const char **r_name);

int LUXO_enum_from_value(const EnumPROP *item, const int value);
int LUXO_enum_from_identifier(const EnumPROP *item, const char *identifier);
int LUXO_enum_get(KrakenPRIM *ptr, const char *name);
void LUXO_enum_set(KrakenPRIM *ptr, const char *name, int value);
void LUXO_enum_set_identifier(kContext *C, KrakenPRIM *ptr, const char *name, const char *id);
bool LUXO_enum_is_equal(kContext *C, KrakenPRIM *ptr, const char *name, const char *enumname);

bool LUXO_prop_enum_value(kContext *C,
                          KrakenPRIM *ptr,
                          KrakenPROP *prop,
                          const char *identifier,
                          int *r_value);
int LUXO_prop_enum_get(KrakenPRIM *ptr, KrakenPROP *prop);
void LUXO_prop_enum_set(KrakenPRIM *ptr, KrakenPROP *prop, int value);
// int LUXO_prop_enum_get_default(KrakenPRIM *ptr, KrakenPROP *prop);

void LUXO_prop_enum_items(kContext *C,
                          KrakenPRIM *ptr,
                          KrakenPROP *prop,
                          const EnumPROP **r_item,
                          int *r_totitem,
                          bool *r_free);
void LUXO_prop_enum_items_ex(kContext *C,
                             KrakenPRIM *ptr,
                             KrakenPROP *prop,
                             const bool use_static,
                             const EnumPROP **r_item,
                             int *r_totitem,
                             bool *r_free);

short PRIM_type_to_ID_code(const KrakenPRIM *type);

KrakenPROP *LUXO_prim_iterator_property(KrakenPRIM *type);
KrakenPRIM *LUXO_prop_pointer_type(KrakenPRIM *ptr, KrakenPROP *prop);

void LUXO_prop_float_range(KrakenPRIM *ptr, KrakenPROP *prop, float *hardmin, float *hardmax);
void LUXO_prop_int_range(KrakenPRIM *ptr, KrakenPROP *prop, int *hardmin, int *hardmax);

void LUXO_prop_update(struct kContext *C, KrakenPRIM *ptr, KrakenPROP *prop);
int LUXO_prop_array_length(KrakenPRIM *ptr, KrakenPROP *prop);

KrakenPROP *prim_struct_find_nested(KrakenPRIM *ptr, KrakenPRIM *sprim);
char *LUXO_path_from_ID_to_struct(const KrakenPRIM *ptr);
char *LUXO_path_from_ID_to_property_index(const KrakenPRIM *ptr,
                                          KrakenPROP *prop,
                                          int index_dim,
                                          int index);
char *LUXO_path_from_ID_to_property(const KrakenPRIM *ptr, KrakenPROP *prop);
char *LUXO_path_from_real_ID_to_property_index(Main *bmain,
                                               const KrakenPRIM *ptr,
                                               KrakenPROP *prop,
                                               int index_dim,
                                               int index,
                                               ID **r_real_id);

#define LUXO_BEGIN(sptr, itemptr, propname)                                              \
  {                                                                                      \
    CollectionPropIT prim_macro_iter;                                                    \
    for (LUXO_collection_begin(sptr, propname, &prim_macro_iter); prim_macro_iter.valid; \
         LUXO_prop_collection_next(&prim_macro_iter)) {                                  \
      KrakenPRIM itemptr = prim_macro_iter.ptr;

#define LUXO_END                              \
  }                                           \
  LUXO_prop_collection_end(&prim_macro_iter); \
  }                                           \
  ((void)0)

#define LUXO_PROP_BEGIN(sptr, itemptr, prop)                                              \
  {                                                                                       \
    CollectionPropIT prim_macro_iter;                                                     \
    for (LUXO_prop_collection_begin(sptr, prop, &prim_macro_iter); prim_macro_iter.valid; \
         LUXO_prop_collection_next(&prim_macro_iter)) {                                   \
      KrakenPRIM itemptr = prim_macro_iter.ptr;

#define LUXO_PROP_END                         \
  }                                           \
  LUXO_prop_collection_end(&prim_macro_iter); \
  }                                           \
  ((void)0)

#define LUXO_PRIM_BEGIN(sptr, prop)                                            \
  {                                                                            \
    CollectionPropIT prim_macro_iter;                                          \
    for (LUXO_prop_collection_begin(sptr,                                      \
                                    LUXO_prim_iterator_property((sptr)->type), \
                                    &prim_macro_iter);                         \
         prim_macro_iter.valid;                                                \
         LUXO_prop_collection_next(&prim_macro_iter)) {                        \
      KrakenPROP *prop = (KrakenPROP *)prim_macro_iter.ptr.data;

#define LUXO_PRIM_BEGIN_SKIP_LUXO_TYPE(sptr, prop)                               \
  {                                                                              \
    CollectionPropIT prim_macro_iter;                                            \
    LUXO_prop_collection_begin(sptr,                                             \
                               LUXO_prim_iterator_property((sptr)->type),        \
                               &prim_macro_iter);                                \
    if (prim_macro_iter.valid) {                                                 \
      LUXO_prop_collection_next(&prim_macro_iter);                               \
    }                                                                            \
    for (; prim_macro_iter.valid; LUXO_prop_collection_next(&prim_macro_iter)) { \
      KrakenPROP *prop = (KrakenPROP *)prim_macro_iter.ptr.data;

#define LUXO_PRIM_END                         \
  }                                           \
  LUXO_prop_collection_end(&prim_macro_iter); \
  }                                           \
  ((void)0)

#endif /* __LUXO_ACCESS_H__ */
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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KLI_utildefines.h"
#include "KLI_kraklib.h"

#include "USD_defs.h"
#include "USD_listBase.h"
#include "USD_object.h"

#ifdef __cplusplus
#  include "USD_types.h"
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

struct ARegion;
struct Base;
struct CacheFile;
struct Collection;
struct Hydra;
struct EditBone;
struct ID;
struct Image;
struct KrakenPRIM;
struct KrakenPROP;
struct LayerCollection;
struct ListBase;
struct Main;
struct Object;
struct RegionView3D;
struct RenderEngineType;
struct ReportList;
struct Scene;
struct ScrArea;
struct SpaceClip;
struct SpaceImage;
struct SpaceLink;
struct SpaceText;
struct Text;
struct ToolSettings;
struct uiBlock;
struct View3D;
struct ViewLayer;
struct kGPDframe;
struct kGPDlayer;
struct kGPdata;
struct kPoseChannel;
struct kScene;
struct kScreen;
struct kUserDef;
struct wmWindow;
struct wmWindowManager;
struct WorkSpace;

/* Structs */

struct kContext;
typedef struct kContext kContext;

struct kContextDataResult;
typedef struct kContextDataResult kContextDataResult;

/**
 * @note Result of context lookups.
 * The specific values are important, and used implicitly in ctx_data_get(). Some functions also
 * still accept/return `int` instead, to ensure that the compiler uses the correct storage size
 * when mixing C/C++ code. */
typedef enum eContextResult
{
  /* The context member was found, and its data is available. */
  CTX_RESULT_OK = 1,

  /* The context member was not found. */
  CTX_RESULT_MEMBER_NOT_FOUND = 0,

  /* The context member was found, but its data is not available.
   * For example, "active_bone" is a valid context member, but has not data in Object mode. */
  CTX_RESULT_NO_DATA = -1,
} eContextResult;

#ifdef __cplusplus
typedef struct kContextStoreEntry
{
  /** @note default ctor. */
  kContextStoreEntry() = default;

  /** @note copy ctor. */
  kContextStoreEntry(const kContextStoreEntry &other)
  {
    next = other.next;
    prev = other.prev;

    KLI_strncpy(name, other.name, sizeof(name));
    ptr = other.ptr;
  }

  struct kContextStoreEntry *next, *prev;

  char name[MAX_NAME];
  KrakenPRIM ptr;
} kContextStoreEntry;
#endif /* __cplusplus */

typedef struct kContextStore
{
  struct kContextStore *next, *prev;

  ListBase entries;
  bool used;
} kContextStore;

/**
 * @note for the context's prim mode enum
 * keep aligned with data_mode_strings in KKE_context.cpp */
typedef enum eContextObjectMode
{
  CTX_MODE_EDIT_MESH = 0,
  CTX_MODE_EDIT_CURVE,
  CTX_MODE_EDIT_SURFACE,
  CTX_MODE_EDIT_TEXT,
  CTX_MODE_EDIT_ARMATURE,
  CTX_MODE_EDIT_METABALL,
  CTX_MODE_EDIT_LATTICE,
  CTX_MODE_POSE,
  CTX_MODE_SCULPT,
  CTX_MODE_PAINT_WEIGHT,
  CTX_MODE_PAINT_VERTEX,
  CTX_MODE_PAINT_TEXTURE,
  CTX_MODE_PARTICLE,
  CTX_MODE_OBJECT,
  CTX_MODE_PAINT_GPENCIL,
  CTX_MODE_EDIT_GPENCIL,
  CTX_MODE_SCULPT_GPENCIL,
  CTX_MODE_WEIGHT_GPENCIL,
  CTX_MODE_VERTEX_GPENCIL,
  CTX_MODE_SCULPT_CURVES,
} eContextObjectMode;
#define CTX_MODE_NUM (CTX_MODE_SCULPT_CURVES + 1)

/* Context */

struct kContext *CTX_create(void);
void CTX_free(struct kContext *C);

kContext *CTX_copy(const struct kContext *C);

/* Stored Context */

struct kContextStore *CTX_store_add(ListBase *contexts,
                                    const char *name,
                                    const struct KrakenPRIM *ptr);
struct kContextStore *CTX_store_add_all(ListBase *contexts, struct kContextStore *context);
struct kContextStore *CTX_store_get(struct kContext *C);
void CTX_store_set(struct kContext *C, struct kContextStore *store);
const struct KrakenPRIM *CTX_store_ptr_lookup(const struct kContextStore *store,
                                              const char *name,
                                              const struct KrakenPRIM *type);
struct kContextStore *CTX_store_copy(struct kContextStore *store);
void CTX_store_free(struct kContextStore *store);
void CTX_store_free_list(ListBase *contexts);

/* need to store if python is initialized or not */
bool CTX_py_init_get(struct kContext *C);
void CTX_py_init_set(struct kContext *C, bool value);

void *CTX_py_dict_get(const struct kContext *C);
void *CTX_py_dict_get_orig(const struct kContext *C);

struct kContext_PyState
{
  void *py_context;
  void *py_context_orig;
};
void CTX_py_state_push(kContext *C, struct kContext_PyState *pystate, void *value);
void CTX_py_state_pop(kContext *C, struct kContext_PyState *pystate);

/* Window Manager Context */

struct wmWindowManager *CTX_wm_manager(const struct kContext *C);
struct wmWindow *CTX_wm_window(const struct kContext *C);
struct WorkSpace *CTX_wm_workspace(const struct kContext *C);
struct kScreen *CTX_wm_screen(const struct kContext *C);
struct ScrArea *CTX_wm_area(const struct kContext *C);
struct ARegion *CTX_wm_region(const struct kContext *C);
struct ARegion *CTX_wm_menu(const struct kContext *C);
struct wmMsgBus *CTX_wm_message_bus(const kContext *C);
struct ReportList *CTX_wm_reports(const struct kContext *C);

void CTX_wm_manager_set(struct kContext *C, struct wmWindowManager *wm);
void CTX_wm_window_set(struct kContext *C, struct wmWindow *win);
void CTX_wm_screen_set(struct kContext *C, struct kScreen *screen);
void CTX_wm_area_set(struct kContext *C, struct ScrArea *area);
void CTX_wm_region_set(struct kContext *C, struct ARegion *region);
void CTX_wm_menu_set(struct kContext *C, struct ARegion *region);

/**
 * Values to create the message that describes the reason poll failed.
 *
 * @note This must be called in the same context as the poll function that created it.
 */
struct kContextPollMsgParams
{
  char *(*get_fn)(struct kContext *C, void *user_data);
  void (*free_fn)(struct kContext *C, void *user_data);
  void *user_data;
};

const char *CTX_wm_operator_poll_msg_get(struct kContext *C, bool *r_free);
void CTX_wm_operator_poll_msg_set(struct kContext *C, const char *msg);
void CTX_wm_operator_poll_msg_set_dynamic(struct kContext *C,
                                          const struct kContextPollMsgParams *params);
void CTX_wm_operator_poll_msg_clear(struct kContext *C);

/**
 * Data Context
 *
 * - #ListBase consists of #CollectionPrimLINK items and must be
 *   freed with #MEM_delete followed by #KLI_listbase_clear!
 * - The dir #ListBase consists of #LinkData items.
 */

enum
{
  CTX_DATA_TYPE_POINTER = 0,
  CTX_DATA_TYPE_COLLECTION,
};

void CTX_data_pointer_set_ptr(struct kContextDataResult *result, const struct KrakenPRIM *ptr);
void CTX_data_type_set(struct kContextDataResult *result, short type);
void CTX_data_list_add_ptr(struct kContextDataResult *result, const struct KrakenPRIM *ptr);

#define CTX_DATA_BEGIN(C, Type, instance, member)                           \
  {                                                                         \
    ListBase ctx_data_list;                                                 \
    CollectionPrimLINK *ctx_link;                                        \
    CTX_data_##member(C, &ctx_data_list);                                   \
    for (ctx_link = (CollectionPrimLINK *)ctx_data_list.first; ctx_link; \
         ctx_link = ctx_link->next) {                                       \
      Type instance = (Type)ctx_link->ptr.data;

#define CTX_DATA_END             \
  }                              \
  KLI_freelistN(&ctx_data_list); \
  }                              \
  (void)0

#define CTX_DATA_BEGIN_WITH_ID(C, Type, instance, member, Type_id, instance_id) \
  CTX_DATA_BEGIN(C, Type, instance, member)                                     \
  Type_id instance_id = (Type_id)ctx_link->ptr.owner_id;

int ctx_data_list_count(const kContext *C, int (*func)(const kContext *, ListBase *));

#define CTX_DATA_COUNT(C, member) ctx_data_list_count(C, CTX_data_##member)

/* Data Context Members */

struct Main *CTX_data_main(const struct kContext *C);
struct kScene *CTX_data_scene(const struct kContext *C);
struct kUserDef *CTX_data_prefs(const struct kContext *C);

const char *CTX_data_mode_string(const kContext *C);
enum eContextObjectMode CTX_data_mode_enum_ex(const struct Object *obedit,
                                              const struct Object *ob,
                                              eObjectMode object_mode);
enum eContextObjectMode CTX_data_mode_enum(const struct kContext *C);

void CTX_data_main_set(struct kContext *C, struct Main *kmain);
void CTX_data_scene_set(struct kContext *C, struct kScene *cscene);
void CTX_data_prefs_set(struct kContext *C, struct kUserDef *uprefs);

struct Object *CTX_data_active_object(const kContext *C);
struct Base *CTX_data_active_base(const kContext *C);
struct Object *CTX_data_edit_object(const kContext *C);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* no c linkage, just cxx. */

KrakenSTAGE CTX_data_stage(const struct kContext *C);

#endif /* __cplusplus */

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
 *
 * General operations, lookup, etc. for materials.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct ID;
struct Main;
struct Material;
struct Object;
struct Scene;
struct kNode;

/* -------------------------------------------------------------------- */
/** \name Module
 * \{ */

void KKE_materials_init(void);
void KKE_materials_exit(void);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Materials
 * \{ */

void KKE_object_materials_test(struct Main *bmain, struct Object *ob, struct ID *id);
void KKE_objects_materials_test_all(struct Main *bmain, struct ID *id);
void KKE_object_material_resize(struct Main *bmain,
                                struct Object *ob,
                                short totcol,
                                bool do_id_user);
void KKE_object_material_remap(struct Object *ob, const unsigned int *remap);
/**
 * Calculate a material remapping from \a ob_src to \a ob_dst.
 *
 * \param remap_src_to_dst: An array the size of `ob_src->totcol`
 * where index values are filled in which map to \a ob_dst materials.
 */
void KKE_object_material_remap_calc(struct Object *ob_dst,
                                    struct Object *ob_src,
                                    short *remap_src_to_dst);
/**
 * Copy materials from evaluated geometry to the original geometry of an object.
 */
void KKE_object_material_from_eval_data(struct Main *bmain,
                                        struct Object *ob_orig,
                                        const struct ID *data_eval);
struct Material *KKE_material_add(struct Main *bmain, const char *name);
struct Material *KKE_gpencil_material_add(struct Main *bmain, const char *name);
void KKE_gpencil_material_attr_init(struct Material *ma);

/* UNUSED */
// void automatname(struct Material *);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Material Slots
 * \{ */

struct Material ***KKE_object_material_array_p(struct Object *ob);
short *KKE_object_material_len_p(struct Object *ob);
/**
 * \note Same as #KKE_object_material_len_p but for ID's.
 */
struct Material ***KKE_id_material_array_p(struct ID *id); /* same but for ID's */
short *KKE_id_material_len_p(struct ID *id);

enum
{
  /* use existing link option */
  KKE_MAT_ASSIGN_EXISTING,
  KKE_MAT_ASSIGN_USERPREF,
  KKE_MAT_ASSIGN_OBDATA,
  KKE_MAT_ASSIGN_OBJECT,
};

struct Material **KKE_object_material_get_p(struct Object *ob, short act);
struct Material *KKE_object_material_get(struct Object *ob, short act);
void KKE_id_material_assign(struct Main *bmain, struct ID *id, struct Material *ma, short act);
void KKE_object_material_assign(struct Main *bmain,
                                struct Object *ob,
                                struct Material *ma,
                                short act,
                                int assign_type);

/**
 * Similar to #KKE_object_material_assign with #KKE_MAT_ASSIGN_OBDATA type,
 * but does not scan whole Main for other usages of the same obdata. Only
 * use in cases where you know that the object's obdata is only used by this one
 * object.
 */
void KKE_object_material_assign_single_obdata(struct Main *bmain,
                                              struct Object *ob,
                                              struct Material *ma,
                                              short act);
/**
 * \warning this calls many more update calls per object then are needed, could be optimized.
 */
void KKE_object_material_array_assign(struct Main *bmain,
                                      struct Object *ob,
                                      struct Material ***matar,
                                      int totcol,
                                      bool to_object_only);

short KKE_object_material_slot_find_index(struct Object *ob, struct Material *ma);
bool KKE_object_material_slot_add(struct Main *bmain, struct Object *ob);
bool KKE_object_material_slot_remove(struct Main *bmain, struct Object *ob);
bool KKE_object_material_slot_used(struct Object *object, short actcol);

struct Material *KKE_gpencil_material(struct Object *ob, short act);
struct MaterialGPencilStyle *KKE_gpencil_material_settings(struct Object *ob, short act);

void KKE_texpaint_slot_refresh_cache(struct Scene *scene,
                                     struct Material *ma,
                                     const struct Object *ob);
void KKE_texpaint_slots_refresh_object(struct Scene *scene, struct Object *ob);
struct kNode *KKE_texpaint_slot_material_find_node(struct Material *ma, short texpaint_slot);

/** \} */

/* -------------------------------------------------------------------- */
/** \name RNA API
 * \{ */

void KKE_id_materials_copy(struct Main *bmain, struct ID *id_src, struct ID *id_dst);
void KKE_id_material_resize(struct Main *bmain, struct ID *id, short totcol, bool do_id_user);
void KKE_id_material_append(struct Main *bmain, struct ID *id, struct Material *ma);
struct Material *KKE_id_material_pop(struct Main *bmain,
                                     struct ID *id,
                                     /* index is an int because of RNA. */
                                     int index);
void KKE_id_material_clear(struct Main *bmain, struct ID *id);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Evaluation API
 * \{ */

/**
 * On evaluated objects the number of materials on an object and its data might go out of sync.
 * This is because during evaluation materials can be added/removed on the object data.
 *
 * For rendering or exporting we generally use the materials on the object data. However, some
 * material indices might be overwritten by the object.
 */
struct Material *KKE_object_material_get_eval(struct Object *ob, short act);
int KKE_object_material_count_eval(struct Object *ob);
void KKE_id_material_eval_assign(struct ID *id, int slot, struct Material *material);
/**
 * Add an empty material slot if the id has no material slots. This material slot allows the
 * material to be overwritten by object-linked materials.
 */
void KKE_id_material_eval_ensure_default_slot(struct ID *id);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Rendering
 * \{ */

/**
 * \param r_col: current value.
 * \param col: new value.
 * \param fac: Zero for is no change.
 */
void ramp_blend(int type, float r_col[3], float fac, const float col[3]);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Copy/Paste
 * \{ */

void KKE_material_copybuf_clear(void);
void KKE_material_copybuf_free(void);
void KKE_material_copybuf_copy(struct Main *bmain, struct Material *ma);
void KKE_material_copybuf_paste(struct Main *bmain, struct Material *ma);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Default Materials
 * \{ */

struct Material *KKE_material_default_empty(void);
struct Material *KKE_material_default_holdout(void);
struct Material *KKE_material_default_surface(void);
struct Material *KKE_material_default_volume(void);
struct Material *KKE_material_default_gpencil(void);

void KKE_material_defaults_free_gpu(void);

/** \} */

/* -------------------------------------------------------------------- */
/** \name Dependency graph evaluation
 * \{ */

struct Depsgraph;

void KKE_material_eval(struct Depsgraph *depsgraph, struct Material *material);

/** \} */

#ifdef __cplusplus
}
#endif

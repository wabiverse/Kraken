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

#ifndef __KKE_IDPROP_H__
#define __KKE_IDPROP_H__

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KLI_compiler_attrs.h"
#include "KLI_sys_types.h"

#include <wabi/base/tf/token.h>

struct UsdDataReader;
struct UsdExpander;
struct UsdLibReader;
struct UsdWriter;
struct ID;
struct IDProperty;
struct IDPropertyUIData;
struct Library;

typedef union IDPropertyTemplate
{
  int i;
  float f;
  double d;
  struct
  {
    const char *str;
    int len;
    char subtype;
  } string;
  struct ID *id;
  struct
  {
    int len;
    char type;
  } array;
  struct
  {
    int matvec_size;
    const float *example;
  } matrix_or_vector;
} IDPropertyTemplate;

/* ----------- Property Array Type ---------- */

/**
 * @note as a start to move away from the stupid #IDP_New function,
 * this type has its own allocation function.
 */
struct IDProperty *IDP_NewIDPArray(const char *name) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
struct IDProperty *IDP_CopyIDPArray(const struct IDProperty *array,
                                    int flag) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

/* ----------- Numeric Array Type ----------- */

/**
 * This function works for strings too!
 */
void IDP_ResizeArray(struct IDProperty *prop, int newlen);
void IDP_FreeArray(struct IDProperty *prop);

/* ---------- String Type ------------ */
/**
 * @param st: The string to assign.
 * @param name: The property name.
 * @param maxlen: The size of the new string (including the \0 terminator).
 * @return The new string property.
 */
void IDP_FreeString(struct IDProperty *prop);

IDProperty *IDP_NewString(const wabi::TfToken &st, const wabi::TfToken &name, int maxlen);

/*-------- ID Type -------*/

typedef void (*IDPWalkFunc)(void *userData, struct IDProperty *idp);

/*-------- Group Functions -------*/

/**
 * Replaces all properties with the same name in a destination group from a source group.
 */
void IDP_ReplaceGroupInGroup(struct IDProperty *dest, const struct IDProperty *src) ATTR_NONNULL();
void IDP_ReplaceInGroup(struct IDProperty *group, struct IDProperty *prop) ATTR_NONNULL();
/**
 * Checks if a property with the same name as prop exists, and if so replaces it.
 * Use this to preserve order!
 */
void IDP_ReplaceInGroup_ex(struct IDProperty *group,
                           struct IDProperty *prop,
                           struct IDProperty *prop_exist);

/**
 * This function has a sanity check to make sure ID properties with the same name don't
 * get added to the group.
 *
 * The sanity check just means the property is not added to the group if another property
 * exists with the same name; the client code using ID properties then needs to detect this
 * (the function that adds new properties to groups, #IDP_AddToGroup,
 * returns false if a property can't be added to the group, and true if it can)
 * and free the property.
 */
bool IDP_AddToGroup(struct IDProperty *group, struct IDProperty *prop);

struct IDProperty *IDP_GetPropertyFromGroup(const struct IDProperty *prop,
                                            const wabi::TfToken &name) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();

/*-------- Main Functions --------*/
/**
 * Get the Group property that contains the id properties for ID `id`.
 *
 * @param create_if_needed: Set to create the group property and attach it to id if it doesn't
 * exist; otherwise the function will return NULL if there's no Group property attached to the ID.
 */
struct IDProperty *IDP_GetProperties(struct ID *id, bool create_if_needed) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();
struct IDProperty *IDP_CopyProperty(const struct IDProperty *prop) ATTR_WARN_UNUSED_RESULT
  ATTR_NONNULL();
struct IDProperty *IDP_CopyProperty_ex(const struct IDProperty *prop,
                                       int flag) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

/**
 * @param is_strict: When false treat missing items as a match.
 */
bool IDP_EqualsProperties_ex(struct IDProperty *prop1,
                             struct IDProperty *prop2,
                             const bool is_strict);
bool IDP_EqualsProperties(struct IDProperty *prop1, struct IDProperty *prop2);

/**
 * Allocate a new ID.
 *
 * This function takes three arguments: the ID property type, a union which defines
 * its initial value, and a name.
 *
 * The union is simple to use; see the top of KKE_idprop.h for its definition.
 * An example of using this function:
 *
 * @code{.c}
 * IDPropertyTemplate val;
 * IDProperty *group, *idgroup, *color;
 * group = IDP_New(IDP_GROUP, val, "group1"); // groups don't need a template.
 *
 * val.array.len = 4
 * val.array.type = IDP_FLOAT;
 * color = IDP_New(IDP_ARRAY, val, "color1");
 *
 * idgroup = IDP_GetProperties(some_id, 1);
 * IDP_AddToGroup(idgroup, color);
 * IDP_AddToGroup(idgroup, group);
 * @endcode
 *
 * Note that you MUST either attach the id property to an id property group with
 * IDP_AddToGroup or MEM_freeN the property, doing anything else might result in
 * a memory leak.
 */
IDProperty *IDP_New(const char type, const IDPropertyTemplate *val, const wabi::TfToken &name);

/**
 * Call a callback for each #IDproperty in the hierarchy under given root one (included).
 */
typedef void (*IDPForeachPropertyCallback)(struct IDProperty *id_property, void *user_data);

void IDP_foreach_property(struct IDProperty *id_property_root,
                          const int type_filter,
                          IDPForeachPropertyCallback callback,
                          void *user_data);

/**
 * @note This will free allocated data, all child properties of arrays and groups, and unlink IDs!
 * But it does not free the actual #IDProperty struct itself.
 */
void IDP_FreePropertyContent_ex(struct IDProperty *prop, bool do_id_user);
void IDP_FreePropertyContent(struct IDProperty *prop);
void IDP_FreeProperty_ex(struct IDProperty *prop, bool do_id_user);
void IDP_FreeProperty(struct IDProperty *prop);

#define IDP_Int(prop) ((prop)->data.val)
#define IDP_Array(prop) ((prop)->data.pointer)
/* C11 const correctness for casts */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define IDP_Float(prop) \
    _Generic((prop), \
  struct IDProperty *:             (*(float *)&(prop)->data.val), \
  const struct IDProperty *: (*(const float *)&(prop)->data.val))
#  define IDP_Double(prop) \
    _Generic((prop), \
  struct IDProperty *:             (*(double *)&(prop)->data.val), \
  const struct IDProperty *: (*(const double *)&(prop)->data.val))
#  define IDP_String(prop) \
    _Generic((prop), \
  struct IDProperty *:             ((char *) (prop)->data.pointer), \
  const struct IDProperty *: ((const char *) (prop)->data.pointer))
#  define IDP_IDPArray(prop) \
    _Generic((prop), \
  struct IDProperty *:             ((struct IDProperty *) (prop)->data.pointer), \
  const struct IDProperty *: ((const struct IDProperty *) (prop)->data.pointer))
#  define IDP_Id(prop) \
    _Generic((prop), \
  struct IDProperty *:             ((ID *) (prop)->data.pointer), \
  const struct IDProperty *: ((const ID *) (prop)->data.pointer))
#else
#  define IDP_Float(prop) (*(float *)&(prop)->data.val)
#  define IDP_Double(prop) (*(double *)&(prop)->data.val)
#  define IDP_String(prop) ((char *)(prop)->data.pointer)
#  define IDP_IDPArray(prop) ((struct IDProperty *)(prop)->data.pointer)
#  define IDP_Id(prop) ((ID *)(prop)->data.pointer)
#endif

void IDP_print(const struct IDProperty *prop);

typedef enum eIDPropertyUIDataType
{
  /** Other properties types that don't support RNA UI data. */
  IDP_UI_DATA_TYPE_UNSUPPORTED = -1,
  /** IDP_INT or IDP_ARRAY with subtype IDP_INT. */
  IDP_UI_DATA_TYPE_INT = 0,
  /** IDP_FLOAT and IDP_DOUBLE or IDP_ARRAY properties with a float or double sub-types. */
  IDP_UI_DATA_TYPE_FLOAT = 1,
  /** IDP_STRING properties. */
  IDP_UI_DATA_TYPE_STRING = 2,
  /** IDP_ID. */
  IDP_UI_DATA_TYPE_ID = 3,
} eIDPropertyUIDataType;

bool IDP_ui_data_supported(const struct IDProperty *prop);
eIDPropertyUIDataType IDP_ui_data_type(const struct IDProperty *prop);
void IDP_ui_data_free(struct IDProperty *prop);
/**
 * Free allocated pointers in the UI data that isn't shared with the UI data in the `other`
 * argument. Useful for returning early on failure when updating UI data in place, or when
 * replacing a subset of the UI data's allocated pointers.
 */
struct IDPropertyUIData *IDP_ui_data_copy(const struct IDProperty *prop);

#endif /* __KKE_IDPROP_H__ */
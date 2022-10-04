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

typedef union IDPropertyTemplate {
  int i;
  float f;
  double d;
  struct {
    const char *str;
    int len;
    char subtype;
  } string;
  struct ID *id;
  struct {
    int len;
    char type;
  } array;
  struct {
    int matvec_size;
    const float *example;
  } matrix_or_vector;
} IDPropertyTemplate;

typedef enum eIDPropertyUIDataType {
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

IDProperty *IDP_CopyProperty(const IDProperty *prop);
IDProperty *IDP_CopyProperty_ex(const IDProperty *prop, const int flag);

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

IDProperty *IDP_CopyIDPArray(const IDProperty *array, const int flag);
IDPropertyUIData *IDP_ui_data_copy(const IDProperty *prop);
eIDPropertyUIDataType IDP_ui_data_type(const IDProperty *prop);

IDProperty *IDP_New(const char type, const IDPropertyTemplate *val, const wabi::TfToken &name);
IDProperty *IDP_GetPropertyFromGroup(const IDProperty *prop, const wabi::TfToken &name);
bool IDP_AddToGroup(IDProperty *group, IDProperty *prop);
IDProperty *IDP_NewString(const wabi::TfToken &st, const wabi::TfToken &name, int maxlen);
bool IDP_EqualsProperties(IDProperty *prop1, IDProperty *prop2);
bool IDP_EqualsProperties_ex(IDProperty *prop1, IDProperty *prop2, const bool is_strict);
void IDP_ReplaceInGroup(IDProperty *group, IDProperty *prop);
void IDP_ReplaceInGroup_ex(IDProperty *group, IDProperty *prop, IDProperty *prop_exist);
void IDP_ReplaceGroupInGroup(IDProperty *dest, const IDProperty *src);

#endif /* __KKE_IDPROP_H__ */
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

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "KLI_endian_switch.h"
#include "KLI_listbase.h"
#include "KLI_math.h"
#include "KLI_string.h"
#include "KLI_utildefines.h"

#include "KKE_main.h"
#include "KKE_idprop.h"
#include "KKE_lib_id.h"

#include "MEM_guardedalloc.h"

#include "KLI_strict_flags.h"

/* IDPropertyTemplate is a union in USD_ID.h */

/**
 * if the new is 'IDP_ARRAY_REALLOC_LIMIT' items less,
 * than #IDProperty.totallen, reallocate anyway.
 */
#define IDP_ARRAY_REALLOC_LIMIT 200

/* Local size table. */
static size_t idp_size_table[] = {
  1, /*strings*/
  sizeof(int),
  sizeof(float),
  sizeof(float[3]),  /* Vector type, deprecated. */
  sizeof(float[16]), /* Matrix type, deprecated. */
  0,                 /* Arrays don't have a fixed size. */
  sizeof(ListBase),  /* Group type. */
  sizeof(void *),
  sizeof(double),
};

/* -------------------------------------------------------------------- */
/** \name Group Functions (IDProperty Group API)
 * \{ */

static IDProperty *idp_generic_copy(const IDProperty *prop, const int UNUSED(flag))
{
  IDProperty *newp = (IDProperty *)MEM_callocN(sizeof(IDProperty), __func__);

  int stlen = (int)strlen(prop->name) + 1;
  KLI_strncpy(newp->name, prop->name, (size_t)stlen);

  newp->type = prop->type;
  newp->flag = prop->flag;
  newp->data.val = prop->data.val;
  newp->data.val2 = prop->data.val2;

  if (prop->ui_data != NULL) {
    newp->ui_data = IDP_ui_data_copy(prop);
  }

  return newp;
}

/**
 * Checks if a property with the same name as prop exists, and if so replaces it.
 */
static IDProperty *IDP_CopyGroup(const IDProperty *prop, const int flag)
{
  KLI_assert(prop->type == IDP_GROUP);
  IDProperty *newp = idp_generic_copy(prop, flag);
  newp->len = prop->len;
  newp->subtype = prop->subtype;

  LISTBASE_FOREACH(IDProperty *, link, &newp->data.group)
  {
    KLI_addtail(&newp->data.group, IDP_CopyProperty_ex(link, flag));
  }

  return newp;
}

static IDProperty *IDP_CopyString(const IDProperty *prop, const int flag)
{
  KLI_assert(prop->type == IDP_STRING);
  IDProperty *newp = idp_generic_copy(prop, flag);

  if (prop->data.pointer) {
    newp->data.pointer = MEM_dupallocN(prop->data.pointer);
  }
  newp->len = prop->len;
  newp->subtype = prop->subtype;
  newp->totallen = prop->totallen;

  return newp;
}

static IDProperty *IDP_CopyID(const IDProperty *prop, const int flag)
{
  KLI_assert(prop->type == IDP_ID);
  IDProperty *newp = idp_generic_copy(prop, flag);

  newp->data.pointer = prop->data.pointer;
  if ((flag & LIB_ID_CREATE_NO_USER_REFCOUNT) == 0) {
    id_us_plus(IDP_Id(newp));
  }

  return newp;
}

static IDProperty *IDP_CopyArray(const IDProperty *prop, const int flag)
{
  IDProperty *newp = idp_generic_copy(prop, flag);

  if (prop->data.pointer) {
    newp->data.pointer = MEM_dupallocN(prop->data.pointer);

    if (prop->type == IDP_GROUP) {
      IDProperty **array = (IDProperty **)newp->data.pointer;
      int a;

      for (a = 0; a < prop->len; a++) {
        array[a] = IDP_CopyProperty_ex(array[a], flag);
      }
    }
  }
  newp->len = prop->len;
  newp->subtype = prop->subtype;
  newp->totallen = prop->totallen;

  return newp;
}

/** \} */

eIDPropertyUIDataType IDP_ui_data_type(const IDProperty *prop)
{
  if (prop->type == IDP_STRING) {
    return IDP_UI_DATA_TYPE_STRING;
  }
  if (prop->type == IDP_ID) {
    return IDP_UI_DATA_TYPE_ID;
  }
  if (prop->type == IDP_INT || (prop->type == IDP_ARRAY && prop->subtype == IDP_INT)) {
    return IDP_UI_DATA_TYPE_INT;
  }
  if (ELEM(prop->type, IDP_FLOAT, IDP_DOUBLE) ||
      (prop->type == IDP_ARRAY && ELEM(prop->subtype, IDP_FLOAT, IDP_DOUBLE))) {
    return IDP_UI_DATA_TYPE_FLOAT;
  }
  return IDP_UI_DATA_TYPE_UNSUPPORTED;
}

IDPropertyUIData *IDP_ui_data_copy(const IDProperty *prop)
{
  IDPropertyUIData *dst_ui_data = (IDPropertyUIData *)MEM_dupallocN(prop->ui_data);

  /* Copy extra type specific data. */
  switch (IDP_ui_data_type(prop)) {
    case IDP_UI_DATA_TYPE_STRING: {
      const IDPropertyUIDataString *src = (const IDPropertyUIDataString *)prop->ui_data;
      IDPropertyUIDataString *dst = (IDPropertyUIDataString *)dst_ui_data;
      dst->default_value = (char *)MEM_dupallocN(src->default_value);
      break;
    }
    case IDP_UI_DATA_TYPE_ID: {
      break;
    }
    case IDP_UI_DATA_TYPE_INT: {
      const IDPropertyUIDataInt *src = (const IDPropertyUIDataInt *)prop->ui_data;
      IDPropertyUIDataInt *dst = (IDPropertyUIDataInt *)dst_ui_data;
      dst->default_array = (int *)MEM_dupallocN(src->default_array);
      break;
    }
    case IDP_UI_DATA_TYPE_FLOAT: {
      const IDPropertyUIDataFloat *src = (const IDPropertyUIDataFloat *)prop->ui_data;
      IDPropertyUIDataFloat *dst = (IDPropertyUIDataFloat *)dst_ui_data;
      dst->default_array = (double *)MEM_dupallocN(src->default_array);
      break;
    }
    case IDP_UI_DATA_TYPE_UNSUPPORTED: {
      break;
    }
  }

  dst_ui_data->description = (char *)MEM_dupallocN(prop->ui_data->description);

  return dst_ui_data;
}

/* -------------------------------------------------------------------- */
/** \name Array Functions (IDP Array API)
 * \{ */

#define GETPROP(prop, i) &(IDP_IDPArray(prop)[i])

/** \} */

IDProperty *IDP_CopyIDPArray(const IDProperty *array, const int flag)
{
  /* don't use MEM_dupallocN because this may be part of an array */
  KLI_assert(array->type == IDP_IDPARRAY);

  IDProperty *narray = (IDProperty *)MEM_mallocN(sizeof(IDProperty), __func__);
  *narray = *array;

  narray->data.pointer = MEM_dupallocN(array->data.pointer);
  for (int i = 0; i < narray->len; i++) {
    /* OK, the copy functions always allocate a new structure,
     * which doesn't work here.  instead, simply copy the
     * contents of the new structure into the array cell,
     * then free it.  this makes for more maintainable
     * code than simply re-implementing the copy functions
     * in this loop. */
    IDProperty *tmp = IDP_CopyProperty_ex(GETPROP(narray, i), flag);
    memcpy(GETPROP(narray, i), tmp, sizeof(IDProperty));
    MEM_freeN(tmp);
  }

  return narray;
}

IDProperty *IDP_CopyProperty_ex(const IDProperty *prop, const int flag)
{
  switch (prop->type) {
    case IDP_GROUP:
      return IDP_CopyGroup(prop, flag);
    case IDP_STRING:
      return IDP_CopyString(prop, flag);
    case IDP_ID:
      return IDP_CopyID(prop, flag);
    case IDP_ARRAY:
      return IDP_CopyArray(prop, flag);
    case IDP_IDPARRAY:
      return IDP_CopyIDPArray(prop, flag);
    default:
      return idp_generic_copy(prop, flag);
  }
}

IDProperty *IDP_CopyProperty(const IDProperty *prop)
{
  return IDP_CopyProperty_ex(prop, 0);
}

IDProperty *IDP_New(const char type, const IDPropertyTemplate *val, const char *name)
{
  IDProperty *prop = NULL;

  switch (type) {
    case IDP_INT:
      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty int");
      prop->data.val = val->i;
      break;
    case IDP_FLOAT:
      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty float");
      *(float *)&prop->data.val = val->f;
      break;
    case IDP_DOUBLE:
      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty double");
      *(double *)&prop->data.val = val->d;
      break;
    case IDP_ARRAY: {
      /* for now, we only support float and int and double arrays */
      if (ELEM(val->array.type, IDP_FLOAT, IDP_INT, IDP_DOUBLE, IDP_GROUP)) {
        prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty array");
        prop->subtype = val->array.type;
        if (val->array.len) {
          prop->data.pointer = MEM_callocN(idp_size_table[val->array.type] *
                                             (size_t)val->array.len,
                                           "id property array");
        }
        prop->len = prop->totallen = val->array.len;
        break;
      }
      return NULL;
    }
    case IDP_STRING: {
      const char *st = val->string.str;

      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty string");
      if (val->string.subtype == IDP_STRING_SUB_BYTE) {
        /* NOTE: Intentionally not null terminated. */
        if (st == NULL) {
          prop->data.pointer = MEM_mallocN(DEFAULT_ALLOC_FOR_NULL_STRINGS, "id property string 1");
          *IDP_String(prop) = '\0';
          prop->totallen = DEFAULT_ALLOC_FOR_NULL_STRINGS;
          prop->len = 0;
        } else {
          prop->data.pointer = MEM_mallocN((size_t)val->string.len, "id property string 2");
          prop->len = prop->totallen = val->string.len;
          memcpy(prop->data.pointer, st, (size_t)val->string.len);
        }
        prop->subtype = IDP_STRING_SUB_BYTE;
      } else {
        if (st == NULL || val->string.len <= 1) {
          prop->data.pointer = MEM_mallocN(DEFAULT_ALLOC_FOR_NULL_STRINGS, "id property string 1");
          *IDP_String(prop) = '\0';
          prop->totallen = DEFAULT_ALLOC_FOR_NULL_STRINGS;
          /* NULL string, has len of 1 to account for null byte. */
          prop->len = 1;
        } else {
          KLI_assert((int)val->string.len <= (int)strlen(st) + 1);
          prop->data.pointer = MEM_mallocN((size_t)val->string.len, "id property string 3");
          memcpy(prop->data.pointer, st, (size_t)val->string.len - 1);
          IDP_String(prop)[val->string.len - 1] = '\0';
          prop->len = prop->totallen = val->string.len;
        }
        prop->subtype = IDP_STRING_SUB_UTF8;
      }
      break;
    }
    case IDP_GROUP: {
      /* Values are set properly by calloc. */
      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty group");
      break;
    }
    case IDP_ID: {
      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty datablock");
      prop->data.pointer = (void *)val->id;
      prop->type = IDP_ID;
      id_us_plus(IDP_Id(prop));
      break;
    }
    default: {
      prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty array");
      break;
    }
  }

  prop->type = type;

  int stlen = (int)strlen(name) + 1;
  KLI_strncpy(prop->name, name, (size_t)stlen);

  return prop;
}

IDProperty *IDP_GetPropertyFromGroup(const IDProperty *prop, const wabi::TfToken &name)
{
  KLI_assert(prop->type == IDP_GROUP);

  LISTBASE_FOREACH(IDProperty *, gprop, &prop->data.group)
  {
    if (gprop->name == name) {
      return gprop;
    }
  }

  return nullptr;
}

bool IDP_AddToGroup(IDProperty *group, IDProperty *prop)
{
  KLI_assert(group->type == IDP_GROUP);

  if (IDP_GetPropertyFromGroup(group, TfToken(prop->name)) == NULL) {
    group->len++;
    // group->data.group.push_back(prop);
    KLI_addtail(&group->data.group, prop);
    return true;
  }

  return false;
}

IDProperty *IDP_NewString(const wabi::TfToken &st, const wabi::TfToken &name, int maxlen)
{
  IDProperty *prop = (IDProperty *)MEM_callocN(sizeof(IDProperty), "IDProperty string");

  if (st.IsEmpty()) {
    prop->data.pointer = MEM_mallocN(DEFAULT_ALLOC_FOR_NULL_STRINGS, "id property string 1");
    *IDP_String(prop) = '\0';
    prop->totallen = DEFAULT_ALLOC_FOR_NULL_STRINGS;
    prop->len = 1; /* NULL string, has len of 1 to account for null byte. */
  } else {
    /* include null terminator '\0' */
    int stlen = (int)strlen(st.GetText()) + 1;

    if (maxlen > 0 && maxlen < stlen) {
      stlen = maxlen;
    }

    prop->data.pointer = MEM_mallocN((size_t)stlen, "id property string 2");
    prop->len = prop->totallen = stlen;
    KLI_strncpy((char *)prop->data.pointer, st.GetText(), (size_t)stlen);
  }

  prop->type = IDP_STRING;
  // prop->name = name;

  int stlen = (int)strlen(name.GetText()) + 1;
  KLI_strncpy(prop->name, name.data(), (size_t)stlen);

  return prop;
}

bool IDP_EqualsProperties_ex(IDProperty *prop1, IDProperty *prop2, const bool is_strict)
{
  if (prop1 == NULL && prop2 == NULL) {
    return true;
  }
  if (prop1 == NULL || prop2 == NULL) {
    return is_strict ? false : true;
  }
  if (prop1->type != prop2->type) {
    return false;
  }

  switch (prop1->type) {
    case IDP_INT:
      return (IDP_Int(prop1) == IDP_Int(prop2));
    case IDP_FLOAT:
#if !defined(NDEBUG) && defined(WITH_PYTHON)
    {
      float p1 = IDP_Float(prop1);
      float p2 = IDP_Float(prop2);
      if ((p1 != p2) && ((fabsf(p1 - p2) / max_ff(p1, p2)) < 0.001f)) {
        printf(
          "WARNING: Comparing two float properties that have nearly the same value (%f vs. "
          "%f)\n",
          p1,
          p2);
        printf("    p1: ");
        //        IDP_print(prop1);
        printf("    p2: ");
        //        IDP_print(prop2);
      }
    }
#endif
      return (IDP_Float(prop1) == IDP_Float(prop2));
    case IDP_DOUBLE:
      return (IDP_Double(prop1) == IDP_Double(prop2));
    case IDP_STRING: {
      return (((prop1->len == prop2->len) &&
               STREQLEN(IDP_String(prop1), IDP_String(prop2), (size_t)prop1->len)));
    }
    case IDP_ARRAY:
      if (prop1->len == prop2->len && prop1->subtype == prop2->subtype) {
        return (memcmp(IDP_Array(prop1),
                       IDP_Array(prop2),
                       idp_size_table[(int)prop1->subtype] * (size_t)prop1->len) == 0);
      }
      return false;
    case IDP_GROUP: {
      if (is_strict && prop1->len != prop2->len) {
        return false;
      }

      LISTBASE_FOREACH(IDProperty *, link1, &prop1->data.group)
      {
        IDProperty *link2 = IDP_GetPropertyFromGroup(prop2, TfToken(link1->name));

        if (!IDP_EqualsProperties_ex(link1, link2, is_strict)) {
          return false;
        }
      }

      return true;
    }
    case IDP_IDPARRAY: {
      IDProperty *array1 = IDP_IDPArray(prop1);
      IDProperty *array2 = IDP_IDPArray(prop2);

      if (prop1->len != prop2->len) {
        return false;
      }

      for (int i = 0; i < prop1->len; i++) {
        if (!IDP_EqualsProperties_ex(&array1[i], &array2[i], is_strict)) {
          return false;
        }
      }
      return true;
    }
    case IDP_ID:
      return (IDP_Id(prop1) == IDP_Id(prop2));
    default:
      KLI_assert_unreachable();
      break;
  }

  return true;
}

bool IDP_EqualsProperties(IDProperty *prop1, IDProperty *prop2)
{
  return IDP_EqualsProperties_ex(prop1, prop2, true);
}

void IDP_ReplaceGroupInGroup(IDProperty *dest, const IDProperty *src)
{
  KLI_assert(dest->type == IDP_GROUP);
  KLI_assert(src->type == IDP_GROUP);


  LISTBASE_FOREACH(IDProperty *, prop, &src->data.group)
  {
    size_t index = 0;
    LISTBASE_FOREACH(IDProperty *, loop, &dest->data.group)
    {
      if (STREQ(loop->name, prop->name)) {
        KLI_insertlinkafter(&dest->data.group, loop, IDP_CopyProperty(prop));
        // dest->data.group.insert(dest->data.group.begin() + index, IDP_CopyProperty(prop));
        // IDP_FreeProperty(loop);
        break;
      }

      index++;
    }

    /* only add at end if not added yet */
    if (index == KLI_listbase_count(&dest->data.group)) {
      IDProperty *copy = IDP_CopyProperty(prop);
      dest->len++;
      KLI_addtail(&dest->data.group, copy);
    }
  }
}

void IDP_ReplaceInGroup_ex(IDProperty *group, IDProperty *prop, IDProperty *prop_exist)
{
  KLI_assert(group->type == IDP_GROUP);
  KLI_assert(prop_exist == IDP_GetPropertyFromGroup(group, TfToken(prop->name)));

  if (prop_exist != NULL) {
    int idx = KLI_findindex(&group->data.group, prop_exist);
    // const auto &idx = std::find(group->data.group.begin(), group->data.group.end(), prop_exist);
    if (idx != KLI_listbase_count(&group->data.group)) {
      KLI_insertlinkreplace(&group->data.group, prop_exist, prop);
      // group->data.group.insert(idx, prop);
    }
    // IDP_FreeProperty(prop_exist);
  } else {
    group->len++;
    KLI_addtail(&group->data.group, prop);
    // group->data.group.push_back(prop);
  }
}

void IDP_ReplaceInGroup(IDProperty *group, IDProperty *prop)
{
  IDProperty *prop_exist = IDP_GetPropertyFromGroup(group, TfToken(prop->name));

  IDP_ReplaceInGroup_ex(group, prop, prop_exist);
}

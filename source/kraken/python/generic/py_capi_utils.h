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

#ifndef __PY_CAPI_UTILS_H__
#define __PY_CAPI_UTILS_H__

#include "KLI_sys_types.h"
#include "KLI_utildefines_variadic.h"

#ifdef __cplusplus
extern "C" {
#endif

void PyC_ObSpit(const char *name, PyObject *var);
void PyC_ObSpitStr(char *result, size_t result_len, PyObject *var);

PyObject *PyC_ExceptionBuffer(void);
PyObject *PyC_ExceptionBuffer_Simple(void);
PyObject *PyC_FrozenSetFromStrings(const char **strings);

PyObject *PyC_Err_Format_Prefix(PyObject *exception_type_prefix, const char *format, ...);
PyObject *PyC_Err_SetString_Prefix(PyObject *exception_type_prefix, const char *str);

void PyC_FileAndNum(const char **r_filename, int *r_lineno);
void PyC_FileAndNum_Safe(const char **r_filename, int *r_lineno);

PyObject *PyC_UnicodeFromByte(const char *str);
PyObject *PyC_UnicodeFromByteAndSize(const char *str, Py_ssize_t size);

PyObject *PyC_DefaultNameSpace(const char *filename);

bool PyC_NameSpace_ImportArray(PyObject *py_dict, const char *imports[]);
void PyC_MainModule_Backup(PyObject **r_main_mod);
void PyC_MainModule_Restore(PyObject *main_mod);
bool PyC_IsInterpreterActive(void);

/**
 * @return success
 *
 * @note it is caller's responsibility to acquire & release GIL!
 */
bool PyC_RunString_AsNumber(const char **imports,
                            const char *expr,
                            const char *filename,
                            double *r_value);

int PyC_ParseBool(PyObject *o, void *p);

struct PyC_StringEnumItems
{
  int value;
  const char *id;
};
struct PyC_StringEnum
{
  const struct PyC_StringEnumItems *items;
  int value_found;
};

int PyC_ParseStringEnum(PyObject *o, void *p);

/* Integer parsing (with overflow checks), -1 on error. */
int PyC_Long_AsBool(PyObject *value);
int8_t PyC_Long_AsI8(PyObject *value);
int16_t PyC_Long_AsI16(PyObject *value);
#if 0 /* inline */
int32_t PyC_Long_AsI32(PyObject *value);
int64_t PyC_Long_AsI64(PyObject *value);
#endif

uint8_t PyC_Long_AsU8(PyObject *value);
uint16_t PyC_Long_AsU16(PyObject *value);
uint32_t PyC_Long_AsU32(PyObject *value);
#if 0 /* inline */
uint64_t PyC_Long_AsU64(PyObject *value);
#endif

/* inline so type signatures match as expected */
Py_LOCAL_INLINE(int32_t) PyC_Long_AsI32(PyObject *value)
{
  return (int32_t)_PyLong_AsInt(value);
}
Py_LOCAL_INLINE(int64_t) PyC_Long_AsI64(PyObject *value)
{
  return (int64_t)PyLong_AsLongLong(value);
}
Py_LOCAL_INLINE(uint64_t) PyC_Long_AsU64(PyObject *value)
{
  return (uint64_t)PyLong_AsUnsignedLongLong(value);
}

#ifdef __cplusplus
}
#endif

#endif /* __PY_CAPI_UTILS_H__ */

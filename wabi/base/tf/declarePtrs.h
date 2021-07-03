//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef WABI_BASE_TF_DECLARE_PTRS_H
#define WABI_BASE_TF_DECLARE_PTRS_H

/// \file tf/declarePtrs.h
/// Standard pointer typedefs.
///
/// This file provides typedefs for standard pointer types.

#include "wabi/base/tf/refPtr.h"
#include "wabi/base/tf/weakPtr.h"
#include "wabi/wabi.h"

#include <vector>

WABI_NAMESPACE_BEGIN

/// \struct TfDeclarePtrs
/// Templated struct used for type definition macros.
template<typename T>
struct TfDeclarePtrs
{
  typedef TfWeakPtr<T> Ptr;
  typedef TfWeakPtr<const T> ConstPtr;
  typedef std::vector<Ptr> PtrVector;
  typedef std::vector<ConstPtr> ConstPtrVector;

  typedef TfRefPtr<T> RefPtr;
  typedef TfRefPtr<const T> ConstRefPtr;
  typedef std::vector<RefPtr> RefPtrVector;
  typedef std::vector<ConstRefPtr> ConstRefPtrVector;
};

/// Define standard weak pointer types.
///
/// \param type is a class name.
///
/// \c TF_DECLARE_WEAK_PTRS(Class) declares ClassPtr, ClassConstPtr,
/// ClassPtrVector and ClassConstPtrVector.
///
/// \hideinitializer
#define TF_DECLARE_WEAK_PTRS(type) \
  typedef TfDeclarePtrs<class type>::Ptr type##Ptr; \
  typedef TfDeclarePtrs<class type>::ConstPtr type##ConstPtr; \
  typedef TfDeclarePtrs<class type>::PtrVector type##PtrVector; \
  typedef TfDeclarePtrs<class type>::ConstPtrVector type##ConstPtrVector

/// Define standard ref pointer types.
///
/// \param type is a class name.
///
/// \c TF_DECLARE_REF_PTRS(Class) declares ClassRefPtr and ClassConstRefPtr.
///
/// \hideinitializer
#define TF_DECLARE_REF_PTRS(type) \
  typedef TfDeclarePtrs<class type>::RefPtr type##RefPtr; \
  typedef TfDeclarePtrs<class type>::ConstRefPtr type##ConstRefPtr; \
  typedef TfDeclarePtrs<class type>::RefPtrVector type##RefPtrVector; \
  typedef TfDeclarePtrs<class type>::ConstRefPtrVector type##ConstRefPtrVector

/// Define standard weak pointer types. (struct variant)
///
/// \param type is a struct name.
///
/// \c TF_DECLARE_WEAK_PTRS(Struct) declares StructPtr, StructConstPtr,
/// StructPtrVector and StructConstPtrVector.
///
/// \hideinitializer
#define TF_DECLARE_WEAK_STRUCT_PTRS(type) \
  typedef TfDeclarePtrs<struct type>::Ptr type##Ptr; \
  typedef TfDeclarePtrs<struct type>::ConstPtr type##ConstPtr; \
  typedef TfDeclarePtrs<struct type>::PtrVector type##PtrVector; \
  typedef TfDeclarePtrs<struct type>::ConstPtrVector type##ConstPtrVector

/// Define standard ref pointer types. (struct variant)
///
/// \param type is a struct name.
///
/// \c TF_DECLARE_REF_PTRS(Class) declares StructRefPtr and StructConstRefPtr.
///
/// \hideinitializer
#define TF_DECLARE_REF_STRUCT_PTRS(type) \
  typedef TfDeclarePtrs<struct type>::RefPtr type##RefPtr; \
  typedef TfDeclarePtrs<struct type>::ConstRefPtr type##ConstRefPtr; \
  typedef TfDeclarePtrs<struct type>::RefPtrVector type##RefPtrVector; \
  typedef TfDeclarePtrs<struct type>::ConstRefPtrVector type##ConstRefPtrVector

/// Define standard weak, ref, and vector pointer types.
///
/// \param type is a class name.
///
/// \c TF_DECLARE_WEAK_AND_REF_PTRS(Class) declares ClassPtr, ClassConstPtr,
/// ClassPtrVector, ClassConstPtrVector, ClassRefPtr and ClassConstRefPtr.
///
/// \hideinitializer
#define TF_DECLARE_WEAK_AND_REF_PTRS(type) \
  TF_DECLARE_WEAK_PTRS(type); \
  TF_DECLARE_REF_PTRS(type)

#define TF_DECLARE_WEAK_AND_REF_STRUCT_PTRS(type) \
  TF_DECLARE_WEAK_STRUCT_PTRS(type); \
  TF_DECLARE_REF_STRUCT_PTRS(type)

WABI_NAMESPACE_END

#endif

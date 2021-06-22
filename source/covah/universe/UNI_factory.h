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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include "UNI_api.h"

#include <wabi/usd/usd/attribute.h>

WABI_NAMESPACE_BEGIN

/**
 * The form factory. */

#define PROP_IFACE(x) x
#define TIMECODE_IFACE(x) x
#define VALUE_IFACE(x) x

class FormFactory
{
 public:
  FormFactory(UsdAttribute formfactor, UsdTimeCode time = UsdTimeCode::Default())
    : m_describe(formfactor),
      m_time(time)
  {}

  template<typename T>
  FormFactory(UsdAttribute formfactor, T val, UsdTimeCode time = UsdTimeCode::Default())
    : m_describe(formfactor),
      m_time(time)
  {
    m_describe.Set(val, time);
  }

  template<typename T>
  FormFactory(UsdRelationship formfactor, T val)
    : m_relate(formfactor)
  {
    m_relate.AddTarget(val);
  }

  FormFactory(UsdRelationship formfactor)
    : m_relate(formfactor)
  {}

  template<typename T>
  operator T()
  {
    T factor;
    m_describe.Get(&factor, m_time);
    return factor;
  }

 private:
  UsdAttribute m_describe;
  UsdRelationship m_relate;
  UsdTimeCode m_time;
};

template<>
inline FormFactory::operator bool()
{
  bool factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator int()
{
  int factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator TfToken()
{
  TfToken factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator SdfPathVector()
{
  SdfPathVector factor;
  m_relate.GetTargets(&factor);
  return factor;
}

template<>
inline FormFactory::operator SdfAssetPath()
{
  SdfAssetPath factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator GfVec2i()
{
  GfVec2i factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator GfVec2f()
{
  GfVec2f factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator GfVec4i()
{
  GfVec4i factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

template<>
inline FormFactory::operator GfVec4f()
{
  GfVec4f factor;
  m_describe.Get(&factor, m_time);
  return factor;
}

// template<typename T>
// UsdAttributeVector UniStageGetAll(UsdTyped *typed, UsdTimeCode time = UsdTimeCode::Default())
// {
//   UsdAttributeVector attrs;
//   UsdPrimDefinition *def = typed->GetSchemaClassPrimDefinition();
//   TfTokenVector propnames = def->GetPropertyNames();

//   TF_FOR_ALL (name, propnames)
//   {
//     SdfAttributeSpecHandle attrspec = def->GetSchemaAttributeSpec(name);
//     auto valuetype = attrspec->GetTypeName();
//     if (valuetype.GetType().IsA(T))
//     {
//       UsdAttribute attr = attrspec->GetFieldAs(name);
//       attrs.push_back(attr);
//     }
//   }
//   return attrs;
// }

WABI_NAMESPACE_END
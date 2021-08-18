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
 * Creator.
 * Creating Chaos.
 */

#include "pch.h"

#include <memory>
#include <string>
#include <regex>
#include <unknwn.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Interop.h>

#include "creator_xaml_typeinfo.h"
#include "creator_xaml_metadata.h"

#include "main.h"

namespace winrt::Kraken::implementation
{
  using IXamlMember = ::winrt::Windows::UI::Xaml::Markup::IXamlMember;
  using IXamlType = ::winrt::Windows::UI::Xaml::Markup::IXamlType;
  using TypeKind = ::winrt::Windows::UI::Xaml::Interop::TypeKind;

  template<typename T>
  ::winrt::Windows::Foundation::IInspectable ActivateType()
  {
    return T();
  }

  template<typename T>
  ::winrt::Windows::Foundation::IInspectable ActivateLocalType()
  {
    return ::winrt::make<T>();
  }

  template<typename TInstance, typename TItem>
  void CollectionAdd(::winrt::Windows::Foundation::IInspectable const &instance,
                     ::winrt::Windows::Foundation::IInspectable const &item)
  {
    instance.as<TInstance>().Append(::winrt::unbox_value<TItem>(item));
  }

  template<typename TInstance, typename TKey, typename TItem>
  void DictionaryAdd(::winrt::Windows::Foundation::IInspectable const &instance,
                     ::winrt::Windows::Foundation::IInspectable const &key,
                     ::winrt::Windows::Foundation::IInspectable const &item)
  {
    instance.as<TInstance>().Insert(::winrt::unbox_value<TKey>(key), ::winrt::unbox_value<TItem>(item));
  }

  template<typename T>
  ::winrt::Windows::Foundation::IInspectable FromStringConverter(XamlUserType const &userType,
                                                                 ::winrt::hstring const &input)
  {
    return ::winrt::box_value(static_cast<T>(userType.CreateEnumUIntFromString(input)));
  }

  template<typename TDeclaringType, typename TValue>
  ::winrt::Windows::Foundation::IInspectable GetValueTypeMember_MyProperty(::winrt::Windows::Foundation::IInspectable const &instance)
  {
    return ::winrt::box_value<TValue>(instance.as<TDeclaringType>().MyProperty());
  }

  template<typename TDeclaringType, typename TValue>
  void SetValueTypeMember_MyProperty(::winrt::Windows::Foundation::IInspectable const &instance,
                                     ::winrt::Windows::Foundation::IInspectable const &value)
  {
    instance.as<TDeclaringType>().MyProperty(::winrt::unbox_value<TValue>(value));
  }

  enum TypeInfo_Flags
  {
    TypeInfo_Flags_None = 0x00,
    TypeInfo_Flags_IsLocalType = 0x01,
    TypeInfo_Flags_IsSystemType = 0x02,
    TypeInfo_Flags_IsReturnTypeStub = 0x04,
    TypeInfo_Flags_IsBindable = 0x08,
    TypeInfo_Flags_IsMarkupExtension = 0x10,
  };

  struct TypeInfo
  {
    const wchar_t *typeName{nullptr};
    const wchar_t *contentPropertyName{nullptr};
    ::winrt::Windows::Foundation::IInspectable (*activator)();
    void (*collectionAdd)(::winrt::Windows::Foundation::IInspectable const &,
                          ::winrt::Windows::Foundation::IInspectable const &);
    void (*dictionaryAdd)(::winrt::Windows::Foundation::IInspectable const &,
                          ::winrt::Windows::Foundation::IInspectable const &,
                          ::winrt::Windows::Foundation::IInspectable const &);
    ::winrt::Windows::Foundation::IInspectable (*fromStringConverter)(XamlUserType const &, ::winrt::hstring const &);
    int baseTypeIndex;
    int firstMemberIndex;
    int firstEnumValueIndex;
    int createFromStringIndex;
    TypeKind kindOfType;
    unsigned int flags;
    int boxedTypeIndex;
  };


  const TypeInfo TypeInfos[] =
    {
      //   0
      L"Int32",
      L"",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      -1,
      0,
      0,
      -1,
      TypeKind::Metadata,
      TypeInfo_Flags_IsSystemType | TypeInfo_Flags_None,
      -1,
      //   1
      L"Kraken.Main",
      L"",
      &ActivateLocalType<::winrt::Kraken::implementation::Main>,
      nullptr,
      nullptr,
      nullptr,
      2,  // Windows.UI.Xaml.Controls.Page
      0,
      0,
      -1,
      TypeKind::Custom,
      TypeInfo_Flags_IsLocalType | TypeInfo_Flags_None,
      -1,
      //   2
      L"Windows.UI.Xaml.Controls.Page",
      L"",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      -1,
      1,
      0,
      -1,
      TypeKind::Metadata,
      TypeInfo_Flags_IsSystemType | TypeInfo_Flags_None,
      -1,
      //   3
      L"Windows.UI.Xaml.Controls.UserControl",
      L"",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      -1,
      1,
      0,
      -1,
      TypeKind::Metadata,
      TypeInfo_Flags_IsSystemType | TypeInfo_Flags_None,
      -1,
      //  Last type here is for padding
      L"",
      L"",
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      -1,
      1,
      0,
      -1,
      TypeKind::Custom,
      TypeInfo_Flags_None,
  };

  constexpr uint32_t TypeInfoLookup[] = {
    0,  //   0
    0,  //   1
    0,  //   2
    0,  //   3
    0,  //   4
    0,  //   5
    1,  //   6
    1,  //   7
    1,  //   8
    1,  //   9
    1,  //  10
    1,  //  11
    1,  //  12
    1,  //  13
    1,  //  14
    1,  //  15
    1,  //  16
    1,  //  17
    1,  //  18
    2,  //  19
    2,  //  20
    2,  //  21
    2,  //  22
    2,  //  23
    2,  //  24
    2,  //  25
    2,  //  26
    2,  //  27
    2,  //  28
    2,  //  29
    3,  //  30
    3,  //  31
    3,  //  32
    3,  //  33
    3,  //  34
    3,  //  35
    3,  //  36
    4,  //  37
  };

  struct MemberInfo
  {
    const wchar_t *shortName{nullptr};
    ::winrt::Windows::Foundation::IInspectable (*getter)(::winrt::Windows::Foundation::IInspectable const &);
    void (*setter)(::winrt::Windows::Foundation::IInspectable const &, ::winrt::Windows::Foundation::IInspectable const &);
    int typeIndex;
    int targetTypeIndex;
    bool isReadOnly;
    bool isDependencyProperty;
    bool isAttachable;
  };

  const MemberInfo MemberInfos[] =
    {
      //   0 - Kraken.Main.MyProperty
      L"",
      // &GetValueTypeMember_MyProperty<::winrt::Kraken::Main, int32_t>,
      // &SetValueTypeMember_MyProperty<::winrt::Kraken::Main, int32_t>,
      // 0, // Int32
      // -1,
      // false, false, false,
  };

  const wchar_t *GetShortName(const wchar_t *longName)
  {
    const auto separator = wcsrchr(longName, '.');
    return separator ? separator + 1 : longName;
  }

  const TypeInfo *GetTypeInfo(::winrt::hstring const &typeName)
  {
    size_t typeNameLength = typeName.size();
    if (typeNameLength < _countof(TypeInfoLookup) - 1)
    {
      const auto begin = TypeInfos + TypeInfoLookup[typeNameLength];
      const auto end = TypeInfos + TypeInfoLookup[typeNameLength + 1];
      auto pos = std::find_if(begin, end, [&typeName](TypeInfo const &elem) {
        return wcscmp(typeName.data(), elem.typeName) == 0;
      });
      if (pos != end)
      {
        return pos;
      }
    }
    return nullptr;
  }

  const MemberInfo *GetMemberInfo(::winrt::hstring const &longMemberName)
  {
    const auto dotPosition = std::find(longMemberName.crbegin(), longMemberName.crend(), L'.').base();
    if (dotPosition != longMemberName.end())
    {
      const auto sizeBeforeDot = static_cast<::winrt::hstring::size_type>(dotPosition - longMemberName.begin()) - 1;
      const TypeInfo *pTypeInfo = GetTypeInfo(::winrt::hstring{longMemberName.data(), sizeBeforeDot});
      if (pTypeInfo)
      {
        const TypeInfo *pNextTypeInfo = pTypeInfo + 1;
        const auto shortMemberName = GetShortName(longMemberName.data());
        const auto begin = MemberInfos + pTypeInfo->firstMemberIndex;
        const auto end = MemberInfos + pNextTypeInfo->firstMemberIndex;
        auto info = std::find_if(begin, end, [shortMemberName](const MemberInfo &elem) {
          return wcscmp(shortMemberName, elem.shortName) == 0;
        });
        if (info != end)
        {
          return info;
        }
      }
    }
    return nullptr;
  }

  std::vector<::winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider> const &XamlTypeInfoProvider::OtherProviders()
  {
    return _otherProviders;
  }

  IXamlType XamlTypeInfoProvider::CreateXamlType(::winrt::hstring const &typeName)
  {
    const TypeInfo *pTypeInfo = GetTypeInfo(typeName);
    const TypeInfo *pNextTypeInfo = pTypeInfo + 1;
    if (!pTypeInfo || !pNextTypeInfo)
    {
      return nullptr;
    } else if (pTypeInfo->flags & TypeInfo_Flags_IsSystemType)
    {
      return ::winrt::make<XamlSystemBaseType>(typeName);
    } else
    {
      ::winrt::hstring baseName{pTypeInfo->baseTypeIndex >= 0 ? TypeInfos[pTypeInfo->baseTypeIndex].typeName : L""};
      ::winrt::hstring boxedName{pTypeInfo->boxedTypeIndex >= 0 ? TypeInfos[pTypeInfo->boxedTypeIndex].typeName : L""};
      auto userType = ::winrt::make_self<XamlUserType>(shared_from_this(), pTypeInfo->typeName, GetXamlTypeByName(baseName));
      userType->_kindOfType = pTypeInfo->kindOfType;
      userType->_activator = pTypeInfo->activator;
      userType->_collectionAdd = pTypeInfo->collectionAdd;
      userType->_dictionaryAdd = pTypeInfo->dictionaryAdd;
      userType->_fromStringConverter = pTypeInfo->fromStringConverter;
      userType->ContentPropertyName(pTypeInfo->contentPropertyName);
      userType->IsLocalType(pTypeInfo->flags & TypeInfo_Flags_IsLocalType);
      userType->IsReturnTypeStub(pTypeInfo->flags & TypeInfo_Flags_IsReturnTypeStub);
      userType->IsBindable(pTypeInfo->flags & TypeInfo_Flags_IsBindable);
      userType->IsMarkupExtension(pTypeInfo->flags & TypeInfo_Flags_IsMarkupExtension);
      userType->_createFromStringMethod = nullptr;
      userType->SetBoxedType(GetXamlTypeByName(boxedName));
      for (int i = pTypeInfo->firstMemberIndex; i < pNextTypeInfo->firstMemberIndex; ++i)
      {
        userType->AddMemberName(MemberInfos[i].shortName);
      }
      return userType.as<IXamlType>();
    }
  }

  IXamlMember XamlTypeInfoProvider::CreateXamlMember(::winrt::hstring const &longMemberName)
  {
    const MemberInfo *pMemberInfo = GetMemberInfo(longMemberName);
    if (!pMemberInfo)
    {
      return nullptr;
    }
    auto xamlMember = ::winrt::make_self<XamlMember>(shared_from_this(),
                                                     pMemberInfo->shortName,
                                                     TypeInfos[pMemberInfo->typeIndex].typeName);
    xamlMember->_getter = pMemberInfo->getter;
    xamlMember->_setter = pMemberInfo->setter;
    xamlMember->TargetTypeName(pMemberInfo->targetTypeIndex >= 0 ? TypeInfos[pMemberInfo->targetTypeIndex].typeName : L"");
    xamlMember->IsReadOnly(pMemberInfo->isReadOnly);
    xamlMember->IsDependencyProperty(pMemberInfo->isDependencyProperty);
    xamlMember->IsAttachable(pMemberInfo->isAttachable);

    return xamlMember.as<IXamlMember>();
  }
}  // namespace winrt::Kraken::implementation

namespace winrt::Kraken::implementation
{
  using namespace ::winrt::Windows::UI::Xaml::Markup;
  using namespace ::winrt::Windows::UI::Xaml::Interop;

  /**
   * XamlMetaDataProvider */


  IXamlType XamlMetaDataProvider::GetXamlType(TypeName const &type)
  {
    return Provider()->GetXamlTypeByType(type);
  }

  IXamlType XamlMetaDataProvider::GetXamlType(::winrt::hstring const &fullName)
  {
    return Provider()->GetXamlTypeByName(fullName);
  }

  ::winrt::com_array<XmlnsDefinition> XamlMetaDataProvider::GetXmlnsDefinitions()
  {
    return ::winrt::com_array<XmlnsDefinition>(0);
  }

  std::shared_ptr<XamlTypeInfoProvider> XamlMetaDataProvider::Provider()
  {
    if (!_provider)
    {
      _provider = std::make_shared<XamlTypeInfoProvider>();
    }
    return _provider;
  }

  /**
   * XamlTypeInfoProvider */

  IXamlType XamlTypeInfoProvider::GetXamlTypeByType(TypeName const &type)
  {
    auto xamlType = GetXamlTypeByName(type.Name);
    auto userXamlType = xamlType ? xamlType.try_as<IXamlUserType>() : nullptr;
    if (!xamlType || (userXamlType && userXamlType->IsReturnTypeStub() && !userXamlType->IsLocalType()))
    {
      auto libXamlType = CheckOtherMetadataProvidersForType(type);
      if (libXamlType)
      {
        if (libXamlType.IsConstructible() || !xamlType)
        {
          xamlType = libXamlType;
        }
      }
    }
    return xamlType;
  }

  IXamlType XamlTypeInfoProvider::GetXamlTypeByName(::winrt::hstring const &typeName)
  {
    if (typeName.empty())
    {
      return nullptr;
    }

    auto val = _xamlTypes.find(typeName.data());
    if (val != _xamlTypes.end())
    {
      auto xamlType = (val->second).get();
      if (xamlType)
      {
        return xamlType;
      }
    }

    auto xamlType = CreateXamlType(typeName);
    auto userXamlType = xamlType ? xamlType.try_as<IXamlUserType>() : nullptr;
    if (!xamlType || (userXamlType && userXamlType->IsReturnTypeStub() && !userXamlType->IsLocalType()))
    {
      IXamlType libXamlType = CheckOtherMetadataProvidersForName(typeName);
      if (libXamlType)
      {
        if (libXamlType.IsConstructible() || !xamlType)
        {
          xamlType = libXamlType;
        }
      }
    }

    if (xamlType)
    {
      _xamlTypes.insert_or_assign(xamlType.FullName().data(), xamlType);
    }
    return xamlType;
  }

  IXamlMember XamlTypeInfoProvider::GetMemberByLongName(::winrt::hstring const &longMemberName)
  {
    if (longMemberName.empty())
    {
      return nullptr;
    }

    auto val = _xamlMembers.find(longMemberName.data());
    if (val != _xamlMembers.end())
    {
      return val->second;
    }

    auto xamlMember = CreateXamlMember(longMemberName);

    if (xamlMember)
    {
      _xamlMembers.insert_or_assign(longMemberName.data(), xamlMember);
    }
    return xamlMember;
  }

  IXamlType XamlTypeInfoProvider::CheckOtherMetadataProvidersForName(::winrt::hstring const &typeName)
  {
    IXamlType foundXamlType;
    for (auto const &provider : OtherProviders())
    {
      auto xamlType = provider.GetXamlType(typeName);
      if (xamlType)
      {
        if (xamlType.IsConstructible())
        {
          return xamlType;
        }
        foundXamlType = xamlType;
      }
    }
    return foundXamlType;
  }

  IXamlType XamlTypeInfoProvider::CheckOtherMetadataProvidersForType(TypeName const &t)
  {
    IXamlType foundXamlType;
    for (auto const &provider : OtherProviders())
    {
      auto xamlType = provider.GetXamlType(t);
      if (xamlType)
      {
        if (xamlType.IsConstructible())
        {
          return xamlType;
        }
        foundXamlType = xamlType;
      }
    }
    return foundXamlType;
  }

  /**
   * XamlSystemBaseType */

  XamlSystemBaseType::XamlSystemBaseType(::winrt::hstring const &name)
    : _fullName(name)
  {
  }

  IXamlType XamlSystemBaseType::BaseType() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  IXamlMember XamlSystemBaseType::ContentProperty() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  ::winrt::hstring XamlSystemBaseType::FullName() const
  {
    return _fullName;
  }

  ::winrt::hstring XamlSystemBaseType::Name() const
  {
    const wchar_t *separator = wcsrchr(_fullName.c_str(), '.');
    if (!separator)
    {
      return _fullName;
    }
    return ::winrt::hstring{separator};
  }

  bool XamlSystemBaseType::IsArray() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsCollection() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsConstructible() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsDictionary() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsMarkupExtension() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsEnum() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsSystemType() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  bool XamlSystemBaseType::IsBindable() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  IXamlType XamlSystemBaseType::ItemType() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  IXamlType XamlSystemBaseType::KeyType() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  TypeName XamlSystemBaseType::UnderlyingType() const
  {
    return {_fullName, TypeKind::Primitive};
  }

  ::winrt::Windows::Foundation::IInspectable XamlSystemBaseType::ActivateInstance() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  IXamlMember XamlSystemBaseType::GetMember(::winrt::hstring const &) const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  void XamlSystemBaseType::AddToVector(::winrt::Windows::Foundation::IInspectable const &, ::winrt::Windows::Foundation::IInspectable const &) const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  void XamlSystemBaseType::AddToMap(::winrt::Windows::Foundation::IInspectable const &, ::winrt::Windows::Foundation::IInspectable const &, ::winrt::Windows::Foundation::IInspectable const &) const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  void XamlSystemBaseType::RunInitializer() const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  ::winrt::Windows::Foundation::IInspectable XamlSystemBaseType::CreateFromString(::winrt::hstring const &) const
  {
    throw ::winrt::hresult_not_implemented{};
  }

  /**
   * XamlUserType */

  XamlUserType::XamlUserType(std::shared_ptr<XamlTypeInfoProvider> const &provider,
                             ::winrt::hstring const &fullName,
                             IXamlType baseType)
    : _provider(provider),
      _fullName(fullName),
      _baseType(baseType)
  {}

  ::winrt::hstring XamlUserType::GetRuntimeClassName() const
  {
    static ::winrt::hstring name{::winrt::name_of<::winrt::Windows::UI::Xaml::Markup::IXamlType>()};
    return name;
  }

  ::winrt::hstring XamlUserType::FullName() const
  {
    return _fullName;
  }

  ::winrt::hstring XamlUserType::Name() const
  {
    const wchar_t *separator = wcsrchr(_fullName.c_str(), '.');
    if (!separator)
    {
      return _fullName;
    }
    return separator;
  }

  TypeName XamlUserType::UnderlyingType() const
  {
    return {_fullName, _kindOfType};
  }

  bool XamlUserType::IsSystemType() const
  {
    return true;
  }

  IXamlType XamlUserType::BaseType() const
  {
    return _baseType;
  }

  bool XamlUserType::IsArray() const
  {
    return _isArray;
  }

  void XamlUserType::IsArray(bool value)
  {
    _isArray = value;
  }

  bool XamlUserType::IsCollection() const
  {
    return _collectionAdd;
  }

  bool XamlUserType::IsConstructible() const
  {
    return _activator;
  }

  bool XamlUserType::IsDictionary() const
  {
    return _dictionaryAdd;
  }

  bool XamlUserType::IsMarkupExtension() const
  {
    return _isMarkupExtension;
  }

  void XamlUserType::IsMarkupExtension(bool value)
  {
    _isMarkupExtension = value;
  }

  bool XamlUserType::IsEnum() const
  {
    return _isEnum;
  }

  void XamlUserType::IsEnum(bool value)
  {
    _isEnum = value;
  }

  bool XamlUserType::IsBindable() const
  {
    return _isBindable;
  }

  void XamlUserType::IsBindable(bool value)
  {
    _isBindable = value;
  }

  bool XamlUserType::IsReturnTypeStub() const
  {
    return _isReturnTypeStub;
  }

  void XamlUserType::IsReturnTypeStub(bool value)
  {
    _isReturnTypeStub = value;
  }

  bool XamlUserType::IsLocalType() const
  {
    return _isLocalType;
  }

  void XamlUserType::IsLocalType(bool value)
  {
    _isLocalType = value;
  }

  IXamlMember XamlUserType::ContentProperty() const
  {
    return _provider->GetMemberByLongName(_contentPropertyName);
  }

  IXamlType XamlUserType::ItemType() const
  {
    return _provider->GetXamlTypeByName(_itemTypeName);
  }

  IXamlType XamlUserType::KeyType() const
  {
    return _provider->GetXamlTypeByName(_keyTypeName);
  }

  IXamlType XamlUserType::BoxedType() const
  {
    return _boxedType;
  }


  IXamlMember XamlUserType::GetMember(::winrt::hstring const &name) const
  {
    auto val = _memberNames.find(name.data());
    if (val != _memberNames.end())
    {
      return _provider->GetMemberByLongName(val->second);
    }
    return nullptr;
  }

  ::winrt::Windows::Foundation::IInspectable XamlUserType::ActivateInstance() const
  {
    return _activator();
  }

  void XamlUserType::AddToMap(::winrt::Windows::Foundation::IInspectable const &instance, ::winrt::Windows::Foundation::IInspectable const &key, ::winrt::Windows::Foundation::IInspectable const &item) const
  {
    _dictionaryAdd(instance, key, item);
  }

  void XamlUserType::AddToVector(::winrt::Windows::Foundation::IInspectable const &instance, ::winrt::Windows::Foundation::IInspectable const &item) const
  {
    _collectionAdd(instance, item);
  }

  void XamlUserType::RunInitializer() const
  {
    /**
     * The C++ runtime will have already run all the Static Initializers at start up. */
  }

  ::winrt::Windows::Foundation::IInspectable XamlUserType::CreateFromString(::winrt::hstring const &input) const
  {
    /**
     * For boxed types, run the boxed type's CreateFromString method and boxing. */
    if (BoxedType() != nullptr)
    {
      return BoxedType().CreateFromString(input);
    }

    if (_createFromStringMethod)
    {
      return (*_createFromStringMethod)(input);
    } else
    {
      return _fromStringConverter(*this, input);
    }
  }

  void XamlUserType::ContentPropertyName(::winrt::hstring const &value)
  {
    _contentPropertyName = value;
  }

  void XamlUserType::ItemTypeName(::winrt::hstring const &value)
  {
    _itemTypeName = value;
  }

  void XamlUserType::KeyTypeName(::winrt::hstring const &value)
  {
    _keyTypeName = value;
  }
  void XamlUserType::SetBoxedType(IXamlType boxedType)
  {
    _boxedType = boxedType;
  }


  void XamlUserType::AddMemberName(::winrt::hstring const &shortName)
  {
    std::wstring longName = FullName().data();
    longName += L".";
    longName += shortName;
    _memberNames.insert_or_assign(shortName.data(), longName);
  }

  void XamlUserType::AddEnumValue(::winrt::hstring const &name, ::winrt::Windows::Foundation::IInspectable value)
  {
    _enumValues.insert_or_assign(name.data(), value);
  }

  uint32_t XamlUserType::CreateEnumUIntFromString(::winrt::hstring const &input) const
  {
    bool found = false;

    const std::wregex regularExpression(L"^\\s+|\\s*,\\s*|\\s+$");
    uint32_t val = 0;

    for (std::wcregex_token_iterator it{input.begin(), input.end(), regularExpression, -1}, end; it != end; ++it)
    {
      std::wcsub_match const &subMatch = *it;
      if (subMatch.length() == 0)
      {
        continue;
      }

      auto lookup{subMatch.str()};

      try
      {
        auto entry = _enumValues.find(lookup);
        if (entry != _enumValues.end())
        {
          val = winrt::unbox_value<int>(entry->second);
        } else
        {
          val |= std::stoi(subMatch);
        }
        found = true;
      }
      catch (std::invalid_argument const &)
      {
        found = false;
        break;
      }
    }

    if (found)
    {
      return val;
    }
    throw ::winrt::hresult_invalid_argument{};
  }


  XamlMember::XamlMember(std::shared_ptr<XamlTypeInfoProvider> const &provider,
                         ::winrt::hstring const &name,
                         ::winrt::hstring const &typeName)
    : _provider(provider),
      _name(name),
      _typeName(typeName)
  {}

  void XamlMember::TargetTypeName(::winrt::hstring const &value)
  {
    _targetTypeName = value;
  }

  bool XamlMember::IsAttachable() const
  {
    return _isAttachable;
  }

  void XamlMember::IsAttachable(bool value)
  {
    _isAttachable = value;
  }

  bool XamlMember::IsDependencyProperty() const
  {
    return _isDependencyProperty;
  }

  void XamlMember::IsDependencyProperty(bool value)
  {
    _isDependencyProperty = value;
  }

  bool XamlMember::IsReadOnly() const
  {
    return _isReadOnly;
  }

  void XamlMember::IsReadOnly(bool value)
  {
    _isReadOnly = value;
  }

  ::winrt::hstring XamlMember::Name() const
  {
    return _name;
  }

  IXamlType XamlMember::Type() const
  {
    return _provider->GetXamlTypeByName(_typeName);
  }

  IXamlType XamlMember::TargetType() const
  {
    return _provider->GetXamlTypeByName(_targetTypeName);
  }

  ::winrt::Windows::Foundation::IInspectable XamlMember::GetValue(::winrt::Windows::Foundation::IInspectable const &instance) const
  {
    return _getter(instance);
  }

  void XamlMember::SetValue(::winrt::Windows::Foundation::IInspectable const &instance, ::winrt::Windows::Foundation::IInspectable const &value)
  {
    _setter(instance, value);
  }
}  // namespace winrt::Kraken::implementation

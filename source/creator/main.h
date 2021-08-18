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

#pragma once

#include "pch.h"

#include "winrt/Windows.UI.Composition.h"
#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Controls.h"

#include "creator_xaml_typeinfo.h"
#include "creator_xaml_metadata.h"

/**
 * Microsoft's xaml implementation
 * reduced to convenience macros to
 * reduce incredibly long and complex
 * template deductions since cmake is
 * not able to properly generate the
 * necessary xaml data types */

#define KRAKEN_GUI_IMPL(namespaced_name, base_name)                                                               \
  template<typename D, typename... I>                                                                             \
  struct __declspec(empty_bases) base_name : implements<D,                                                        \
                                                        namespaced_name,                                          \
                                                        composing,                                                \
                                                        winrt::Windows::UI::Xaml::Controls::IPageOverrides,       \
                                                        winrt::Windows::UI::Xaml::Controls::IControlOverrides,    \
                                                        winrt::Windows::UI::Xaml::Controls::IControlOverrides6,   \
                                                        winrt::Windows::UI::Xaml::IFrameworkElementOverrides,     \
                                                        winrt::Windows::UI::Xaml::IFrameworkElementOverrides2,    \
                                                        winrt::Windows::UI::Xaml::IUIElementOverrides,            \
                                                        winrt::Windows::UI::Xaml::IUIElementOverrides7,           \
                                                        winrt::Windows::UI::Xaml::IUIElementOverrides8,           \
                                                        winrt::Windows::UI::Xaml::IUIElementOverrides9,           \
                                                        I...>,                                                    \
                                             impl::require<D,                                                     \
                                                           winrt::Windows::UI::Xaml::Controls::IPage,             \
                                                           winrt::Windows::UI::Xaml::Controls::IUserControl,      \
                                                           winrt::Windows::UI::Xaml::Controls::IControl,          \
                                                           winrt::Windows::UI::Xaml::Controls::IControl2,         \
                                                           winrt::Windows::UI::Xaml::Controls::IControl3,         \
                                                           winrt::Windows::UI::Xaml::Controls::IControl4,         \
                                                           winrt::Windows::UI::Xaml::Controls::IControl5,         \
                                                           winrt::Windows::UI::Xaml::Controls::IControl7,         \
                                                           winrt::Windows::UI::Xaml::Controls::IControlProtected, \
                                                           winrt::Windows::UI::Xaml::IFrameworkElement,           \
                                                           winrt::Windows::UI::Xaml::IFrameworkElement2,          \
                                                           winrt::Windows::UI::Xaml::IFrameworkElement3,          \
                                                           winrt::Windows::UI::Xaml::IFrameworkElement4,          \
                                                           winrt::Windows::UI::Xaml::IFrameworkElement6,          \
                                                           winrt::Windows::UI::Xaml::IFrameworkElement7,          \
                                                           winrt::Windows::UI::Xaml::IFrameworkElementProtected7, \
                                                           winrt::Windows::UI::Xaml::IUIElement,                  \
                                                           winrt::Windows::UI::Xaml::IUIElement2,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement3,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement4,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement5,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement7,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement8,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement9,                 \
                                                           winrt::Windows::UI::Xaml::IUIElement10,                \
                                                           winrt::Windows::UI::Composition::IAnimationObject,     \
                                                           winrt::Windows::UI::Composition::IVisualElement,       \
                                                           winrt::Windows::UI::Xaml::IDependencyObject,           \
                                                           winrt::Windows::UI::Xaml::IDependencyObject2>,         \
                                             impl::base<D,                                                        \
                                                        winrt::Windows::UI::Xaml::Controls::Page,                 \
                                                        winrt::Windows::UI::Xaml::Controls::UserControl,          \
                                                        winrt::Windows::UI::Xaml::Controls::Control,              \
                                                        winrt::Windows::UI::Xaml::FrameworkElement,               \
                                                        winrt::Windows::UI::Xaml::UIElement,                      \
                                                        winrt::Windows::UI::Xaml::DependencyObject>,              \
                                             winrt::Windows::UI::Xaml::Controls::IPageOverridesT<D>,              \
                                             winrt::Windows::UI::Xaml::Controls::IControlOverridesT<D>,           \
                                             winrt::Windows::UI::Xaml::Controls::IControlOverrides6T<D>,          \
                                             winrt::Windows::UI::Xaml::IFrameworkElementOverridesT<D>,            \
                                             winrt::Windows::UI::Xaml::IFrameworkElementOverrides2T<D>,           \
                                             winrt::Windows::UI::Xaml::IUIElementOverridesT<D>,                   \
                                             winrt::Windows::UI::Xaml::IUIElementOverrides7T<D>,                  \
                                             winrt::Windows::UI::Xaml::IUIElementOverrides8T<D>,                  \
                                             winrt::Windows::UI::Xaml::IUIElementOverrides9T<D>

namespace winrt::Kraken::implementation
{
  KRAKEN_GUI_IMPL(Kraken::Main, Main_base)
  {
    using base_type = Main_base;
    using class_type = Kraken::Main;
    using implements_type = typename Main_base::implements_type;
    using implements_type::implements_type;
    using composable_base = winrt::Windows::UI::Xaml::Controls::Page;

    hstring GetRuntimeClassName() const
    {
      return L"Kraken.Main";
    }

    Main_base()
    {
      impl::call_factory<winrt::Windows::UI::Xaml::Controls::Page,
                         winrt::Windows::UI::Xaml::Controls::IPageFactory>(
        [&](winrt::Windows::UI::Xaml::Controls::IPageFactory const &f) {
          [[maybe_unused]] auto winrt_impl_discarded = f.CreateInstance(*this, this->m_inner);
        });
    }
  };
}  // namespace winrt::Kraken::implementation

namespace winrt::Kraken::factory_implementation
{
  template<typename D, typename T, typename... I>
  struct __declspec(empty_bases) MainT : implements<D, winrt::Windows::Foundation::IActivationFactory, I...>
  {
    using instance_type = Kraken::Main;

    hstring GetRuntimeClassName() const
    {
      return L"Kraken.Main";
    }

    auto ActivateInstance() const
    {
      return make<T>();
    }
  };
}  // namespace winrt::Kraken::factory_implementation

namespace winrt::Kraken::implementation
{
  using IInspectable = ::winrt::Windows::Foundation::IInspectable;

  template<typename D, typename... I>
  struct MainT : public ::winrt::Kraken::implementation::Main_base<D,
                                                                   ::winrt::Windows::UI::Xaml::Markup::IComponentConnector,
                                                                   ::winrt::Windows::UI::Xaml::Markup::IComponentConnector2,
                                                                   I...>
  {
    using base_type = typename MainT::base_type;
    using base_type::base_type;
    using class_type = typename MainT::class_type;

    void InitializeComponent();
    void Connect(int32_t connectionId, IInspectable const &target);
    ::winrt::Windows::UI::Xaml::Markup::IComponentConnector GetBindingConnector(int32_t connectionId, IInspectable const &target);
    void UnloadObject(::winrt::Windows::UI::Xaml::DependencyObject const &dependencyObject);
    void DisconnectUnloadedObject(int32_t connectionId);

    ::winrt::Windows::UI::Xaml::Controls::Button myButton()
    {
      return _myButton;
    }
    void myButton(::winrt::Windows::UI::Xaml::Controls::Button value)
    {
      _myButton = value;
    }

   protected:
    bool _contentLoaded{false};

   private:
    struct Main_obj1_Bindings;

    ::winrt::Windows::UI::Xaml::Controls::Button _myButton{nullptr};
  };
}  // namespace winrt::Kraken::implementation

namespace winrt::Kraken::implementation
{
  struct Main : MainT<Main>
  {
    Main();

    int32_t MyProperty();
    void MyProperty(int32_t value);

    void ClickHandler(winrt::Windows::Foundation::IInspectable const &sender,
                      winrt::Windows::UI::Xaml::RoutedEventArgs const &args);
  };
}  // namespace winrt::Kraken::implementation

namespace winrt::Kraken::factory_implementation
{
  struct Main : MainT<Main, implementation::Main>
  {};
}  // namespace winrt::Kraken::factory_implementation

#pragma warning(push)
/* unreferenced formal parameter */
#pragma warning(disable : 4100)

namespace winrt::Kraken::implementation
{
  using Application = ::winrt::Windows::UI::Xaml::Application;
  using ComponentResourceLocation = ::winrt::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation;
  using DataTemplate = ::winrt::Windows::UI::Xaml::DataTemplate;
  using DependencyObject = ::winrt::Windows::UI::Xaml::DependencyObject;
  using DependencyProperty = ::winrt::Windows::UI::Xaml::DependencyProperty;
  using IComponentConnector = ::winrt::Windows::UI::Xaml::Markup::IComponentConnector;
  using Uri = ::winrt::Windows::Foundation::Uri;
  using XamlBindingHelper = ::winrt::Windows::UI::Xaml::Markup::XamlBindingHelper;

  template<typename D, typename... I>
  void MainT<D, I...>::InitializeComponent()
  {
    if (!_contentLoaded)
    {
      _contentLoaded = true;
      Uri resourceLocator{L"ms-appx:///Main.xaml"};
      Application::LoadComponent(*this, resourceLocator, ComponentResourceLocation::Application);
    }
  }

  template<typename D, typename... I>
  void MainT<D, I...>::Connect(int32_t connectionId, IInspectable const &target)
  {
    switch (connectionId)
    {
      case 2: {
        auto targetElement = target.as<::winrt::Windows::UI::Xaml::Controls::Button>();
        this->myButton(targetElement);
        auto weakThis = ::winrt::make_weak<class_type>(*this);
        targetElement.Click([weakThis](::winrt::Windows::Foundation::IInspectable const &p0,
                                       ::winrt::Windows::UI::Xaml::RoutedEventArgs const &p1) {
          if (auto t = weakThis.get())
          {
            ::winrt::get_self<D>(t)->ClickHandler(p0, p1);
          }
        });
      }
      break;
    }
    _contentLoaded = true;
  }

  template<typename D, typename... I>
  void MainT<D, I...>::DisconnectUnloadedObject(int32_t)
  {
    throw ::winrt::hresult_invalid_argument{L"No unloadable objects to disconnect."};
  }

  template<typename D, typename... I>
  void MainT<D, I...>::UnloadObject(DependencyObject const &)
  {
    throw ::winrt::hresult_invalid_argument{L"No unloadable objects."};
  }


  template<typename D, typename... I>
  IComponentConnector MainT<D, I...>::GetBindingConnector(int32_t, IInspectable const &)
  {
    return nullptr;
  }

  template struct MainT<struct Main>;
}  // namespace winrt::Kraken::implementation


#pragma warning(pop)
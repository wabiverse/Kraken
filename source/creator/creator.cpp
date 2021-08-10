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

#if defined(ARCH_OS_WINDOWS)

#  include <Windows.h>

/**
 * The official Windows Runtime. Kraken
 * supports the Windows platform as a
 * first class citizen.
 *
 * Copyright (C) Microsoft Corporation. */

/*  Windows :: Base */
#  include <winrt/base.h>

/*  Windows :: Foundation */
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Foundation.Collections.h>

/*  Windows :: ApplicationModel */
#  include <winrt/Windows.ApplicationModel.h>
#  include <winrt/Windows.ApplicationModel.Activation.h>

/*  Windows :: Storage */
#  include <winrt/Windows.Storage.h>

/*  Windows :: System */
#  include <winrt/Windows.System.h>

/*  Windows :: UI */
#  include <winrt/Windows.UI.h>

/*  Windows :: XAML */
#  include <winrt/Windows.UI.Xaml.h>
#  include <winrt/Windows.UI.Xaml.Input.h>
#  include <winrt/Windows.UI.Xaml.Controls.h>
#  include <winrt/Windows.UI.Xaml.Shapes.h>
#  include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#  include <winrt/Windows.UI.Xaml.Data.h>
#  include <winrt/Windows.UI.Xaml.Interop.h>
#  include <winrt/Windows.UI.Xaml.Markup.h>
#  include <winrt/Windows.UI.Xaml.Navigation.h>

#endif /* ARCH_OS_WINDOWS */

#include "KLI_threads.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"

#include "UNI_pixar_utils.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "creator.h"

WABI_NAMESPACE_USING

#if defined(ARCH_OS_WINDOWS)

using namespace MICROSOFT;
using namespace MICROSOFT::Windows;
using namespace MICROSOFT::Windows::ApplicationModel;
using namespace MICROSOFT::Windows::ApplicationModel::Activation;
using namespace MICROSOFT::Windows::Foundation;
using namespace MICROSOFT::Windows::UI;
using namespace MICROSOFT::Windows::UI::Xaml;
using namespace MICROSOFT::Windows::UI::Xaml::Controls;
using namespace MICROSOFT::Windows::UI::Xaml::Navigation;

using namespace winrt::Kraken;
using namespace winrt::Kraken::implementation;

Creator::Creator()
{
  InitializeComponent();
  Suspending({this, &Creator::OnSuspending});

#  if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
  UnhandledException([this](IInspectable const &, UnhandledExceptionEventArgs const &e) {
    if (IsDebuggerPresent())
    {
      auto errorMessage = e.Message();
      __debugbreak();
    }
  });
#  endif
}

void Creator::OnLaunched(LaunchActivatedEventArgs const &e)
{
  Frame rootFrame{nullptr};
  auto content = Window::Current().Content();
  if (content)
  {
    rootFrame = content.try_as<Frame>();
  }

  /**
   * Do not repeat app initialization when
   * the Window already has content, just
   * ensure that the window is active. */
  if (rootFrame == nullptr)
  {
    /**
     * Create a Frame to act as the navigation
     * context and associate it with a key from
     * SuspensionManager */
    rootFrame = Frame();

    rootFrame.NavigationFailed({this, &Creator::OnNavigationFailed});

    if (e.PreviousExecutionState() == ApplicationExecutionState::Terminated)
    {
      /**
       * Restore the saved session state only when
       * appropriate, scheduling the final launch
       * steps after the restore is complete. */
    }

    if (e.PrelaunchActivated() == false)
    {
      if (rootFrame.Content() == nullptr)
      {
        /**
         * When the navigation stack isn't restored navigate
         * to the first page, configuring the new page  by
         * passing required information as a navigation
         * parameter */
        // rootFrame.Navigate(xaml_typename<Kraken::MainPage>(), box_value(e.Arguments()));
      }
      /**
       * Place the frame in the current Window */
      Window::Current().Content(rootFrame);
      /**
       * Ensure the current window is active */
      Window::Current().Activate();
    }
  } else
  {
    if (e.PrelaunchActivated() == false)
    {
      if (rootFrame.Content() == nullptr)
      {
        /**
         * When the navigation stack isn't restored navigate
         * to the first page, configuring the new page  by
         * passing required information as a navigation
         * parameter. */
        // rootFrame.Navigate(xaml_typename<Kraken::MainPage>(), box_value(e.Arguments()));
      }
      /**
       * Ensure the current window is active. */
      Window::Current().Activate();
    }
  }

  kContext *C;

  /* Environment variables. */
  CREATOR_kraken_env_init();

  /* Create Context C. */
  C = CTX_create();

  /* Initialize path to executable. */
  KKE_appdir_program_path_init();

  /* Initialize Threads. */
  // KLI_threadapi_init();

  /* Initialize Globals (paths, sys). */
  KKE_kraken_globals_init();

  /* Init plugins. */
  KKE_kraken_plugins_init();

  /* Init & parse args.
   * CREATOR_setup_args(argc, (const char **)argv);
   * if (CREATOR_parse_args(argc, (const char **)argv) != 0)
   * {
   *   return 0;
   * } */

  KKE_appdir_init();

  /* Determining Stage Configuration and Loadup. */
  KKE_kraken_main_init(C);

#ifdef WITH_MAIN_INIT
 /**
   * The great refactor for WinRT. */

  /* Initialize main Runtime. */
  WM_init(C);

  /* Initialize kraken python module. */
  CTX_py_init_set(C, true);

  /* Run the main event loop. */
  WM_main(C);
#endif /* WITH_MAIN_INIT */
}

void Creator::OnSuspending([[maybe_unused]] IInspectable const &sender, [[maybe_unused]] SuspendingEventArgs const &e)
{
  // Save application state and stop any background activity
}

void Creator::OnNavigationFailed(IInspectable const &, NavigationFailedEventArgs const &e)
{
  throw hresult_error(E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

// void* winrt_make_Kraken_MainPage();
void *winrt_make_Kraken_XamlMetaDataProvider();

bool __stdcall winrt_can_unload_now() noexcept
{
  if (MICROSOFT::get_module_lock())
  {
    return false;
  }

  MICROSOFT::clear_factory_cache();
  return true;
}

void *__stdcall winrt_get_activation_factory([[maybe_unused]] std::wstring_view const &name)
{
  auto requal = [](std::wstring_view const &left, std::wstring_view const &right) noexcept {
    return std::equal(left.rbegin(), left.rend(), right.rbegin(), right.rend());
  };

  // if (requal(name, L"Kraken.MainPage"))
  // {
  //     return winrt_make_Kraken_MainPage();
  // }

  if (requal(name, L"Kraken.XamlMetaDataProvider"))
  {
    return winrt_make_Kraken_XamlMetaDataProvider();
  }

  return nullptr;
}

int32_t __stdcall WINRT_CanUnloadNow() noexcept
{
#  ifdef _WRL_MODULE_H_
  if (!::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().Terminate())
  {
    return 1;
  }
#  endif

  return winrt_can_unload_now() ? 0 : 1;
}

int32_t __stdcall WINRT_GetActivationFactory(void *classId, void **factory) noexcept
try
{
  std::wstring_view const name{*reinterpret_cast<MICROSOFT::hstring *>(&classId)};
  *factory = winrt_get_activation_factory(name);

  if (*factory)
  {
    return 0;
  }

#  ifdef _WRL_MODULE_H_
  return ::Microsoft::WRL::Module<::Microsoft::WRL::InProc>::GetModule().GetActivationFactory(static_cast<HSTRING>(classId), reinterpret_cast<::IActivationFactory **>(factory));
#  else
  return MICROSOFT::hresult_class_not_available(name).to_abi();
#  endif
}
catch (...)
{
  return MICROSOFT::to_hresult();
}

#  if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#  endif


#  if (defined(_M_IX86) || defined(_M_AMD64)) && !defined(_VSDESIGNER_DONT_LOAD_AS_DLL)
#    if defined(_M_IX86)
#      pragma comment(linker, "/EXPORT:DllGetActivationFactory=_VSDesignerDllGetActivationFactory@8,PRIVATE")
#      pragma comment(linker, "/EXPORT:DllCanUnloadNow=_VSDesignerCanUnloadNow@0,PRIVATE")
#      pragma comment(linker, "/EXPORT:VSDesignerDllMain=_VSDesignerDllMain@12,PRIVATE")
#    elif defined(_M_AMD64)
#      pragma comment(linker, "/EXPORT:DllGetActivationFactory=VSDesignerDllGetActivationFactory,PRIVATE")
#      pragma comment(linker, "/EXPORT:VSDesignerCanUnloadNow,PRIVATE")
#      pragma comment(linker, "/EXPORT:VSDesignerDllMain,PRIVATE")
#    endif

extern "C" {
int __stdcall _DllMainCRTStartup(void *hinstDLL, unsigned long fdwReason, void **lpvReserved);

int __stdcall VSDesignerDllGetActivationFactory(void *classId, void **factory)
{
  return WINRT_GetActivationFactory(classId, factory);
}

int __stdcall VSDesignerCanUnloadNow()
{
  return WINRT_CanUnloadNow();
}

int __stdcall VSDesignerDllMain(void *hinstDLL, unsigned long fdwReason, void **lpvReserved)
{
  return _DllMainCRTStartup(hinstDLL, fdwReason, lpvReserved);
}
}

#  endif  // (defined(_M_IX86) || defined(_M_AMD64)) && !defined(_VSDESIGNER_DONT_LOAD_AS_DLL)

#  ifndef DISABLE_XAML_GENERATED_MAIN
int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
  MICROSOFT::init_apartment();
  ::MICROSOFT::Windows::UI::Xaml::Application::Start(
    [](auto &&) {
      ::MICROSOFT::make<::MICROSOFT::Kraken::implementation::Creator>();
    });
  return 0;
}
#  endif

#else  /* ARCH_OS_WINDOWS */

int main(int argc, const char **argv)
{
  kContext *C;

  /* Environment variables. */
  CREATOR_kraken_env_init();

  /* Create Context C. */
  C = CTX_create();

  /* Initialize path to executable. */
  KKE_appdir_program_path_init();

  /* Initialize Threads. */
  KLI_threadapi_init();

  /* Initialize Globals (paths, sys). */
  KKE_kraken_globals_init();

  /* Init plugins. */
  KKE_kraken_plugins_init();

  /* Init & parse args. */
  CREATOR_setup_args(argc, (const char **)argv);
  if (CREATOR_parse_args(argc, (const char **)argv) != 0)
  {
    return 0;
  }

  KKE_appdir_init();

  /* Determining Stage Configuration and Loadup. */
  KKE_kraken_main_init(C, argc, (const char **)argv);

  /* Runtime. */
  WM_init(C, argc, (const char **)argv);

  CTX_py_init_set(C, true);

  WM_main(C);

  return KRAKEN_SUCCESS;
}
#endif /* defined(ARCH_OS_LINUX) || defined(ARCH_OS_DARWIN) */

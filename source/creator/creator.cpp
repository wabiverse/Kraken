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

#ifdef _WIN32
#  include "pch.h"
#  include "winrt/Kraken.h"

#  include "ChaosEngine/Kraken.Foundation.App.h"
#  ifdef WITH_WINUI3
#    include "ChaosEngine/Kraken.UIKit.UIScreen.h"
#  endif /* WITH_WINUI3 */
#endif   /* _WIN32 */

#include "creator.h"

#include "KLI_threads.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"

#include "USD_pixar_utils.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "LUXO_access.h"

#if defined(ARCH_OS_WINDOWS)
using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

#  ifdef WITH_WINUI3
using namespace winrt::Microsoft::UI::Xaml::Navigation;
#  endif /* WITH_WINUI3 */

using namespace Kraken;
using namespace Kraken::implementation;
#endif /* defined(ARCH_OS_WINDOWS) */

WABI_NAMESPACE_USING

void CREATOR_kraken_main(int argc, const char **argv)
{
  kContext *C;

  /* Create Context C. */
  C = CTX_create();

  /* Initialize path to executable. */
  KKE_appdir_program_path_init();

#if !defined(ARCH_OS_WINDOWS)
  /* Initialize Threads. */
  KLI_threadapi_init();
#endif /* !defined(ARCH_OS_WINDOWS) */

  /* Initialize Globals (paths, sys). */
  /* Already called in KrakenSTAGE ctor. */
  // KKE_kraken_globals_init();

  /* Init plugins. */
  KKE_kraken_plugins_init();

#if !defined(ARCH_OS_WINDOWS)
  /* Init & parse args. */
  CREATOR_setup_args(argc, (const char **)argv);
  if (CREATOR_parse_args(argc, (const char **)argv) != 0) {
    return;
  }
#endif /* !defined(ARCH_OS_WINDOWS) */

  KKE_appdir_init();

  /* Determining Stage Configuration and Loadup. */
  KKE_kraken_main_init(C);

  /* Setup and create all root level prims. */
  LUXO_init();

#if !defined(ARCH_OS_WINDOWS)
  /* Initialize main Runtime. */
  WM_init(C);

  /* Initialize kraken python module. */
  CTX_py_init_set(C, true);

  /* Run the main event loop. */
  WM_main(C);
#endif /* !defined(ARCH_OS_WINDOWS) */
}

#if defined(ARCH_OS_WINDOWS)

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
  /**
   * 🚀 Kraken Launch with Microsoft WinRT Superpowers. */

  winrt::init_apartment();
  Windows::UI::Xaml::Application::Start([](auto &&) {
    ::winrt::make<::winrt::Kraken::implementation::App>();
  });

  CREATOR_kraken_main();

  return KRAKEN_SUCCESS;
}

#else /* ARCH_OS_WINDOWS */

int main(int argc, const char **argv)
{
  CREATOR_kraken_main(argc, argv);

  return KRAKEN_SUCCESS;
}

#endif /* ARCH_OS_LINUX || ARCH_OS_DARWIN */

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

#include "App.h"
#include "MainWindow.h"

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;

using namespace Kraken;
using namespace Kraken::implementation;

/**
 * Initializes the singleton application object.
 * This is the first line of authored code executed,
 * and as such is the logical equivalent of main()
 * or WinMain(). */
App::App()
{
  InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
  UnhandledException([this](IInspectable const &, UnhandledExceptionEventArgs const &e) {
    if (IsDebuggerPresent())
    {
      auto errorMessage = e.Message();
      __debugbreak();
    }
  });
#endif
}

/**
 * Invoked when the application is launched normally
 * by the end user. Other entry points will be used
 * such as when the application is launched to open
 * a specific file. */
void App::OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const &e)
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
     * context and associate it with a given
     * SuspensionManager key. */
    rootFrame = Frame();

    rootFrame.NavigationFailed({this, &App::OnNavigationFailed});

    if (e.UWPLaunchActivatedEventArgs().PreviousExecutionState() == ApplicationExecutionState::Terminated)
    {
      /**
       * Restore the saved session state only when appropriate,
       * scheduling the final launch steps after the restore is
       * complete. */
    }

    if (e.UWPLaunchActivatedEventArgs().PrelaunchActivated() == false)
    {
      if (rootFrame.Content() == nullptr)
      {
        /**
         * When the navigation stack isn't restored navigate
         * to the first page, configuring the new page by
         * passing the required information as a navigation
         * parameter. */
        rootFrame.Navigate(xaml_typename<Kraken::MainWindow>(), box_value(e.Arguments()));
      }

      /**
       * Place the frame in the current Window. */
      Window::Current().Content(rootFrame);

      /**
       * Ensure the current window is active. */
      Window::Current().Activate();
    }
  } else
  {
    if (e.UWPLaunchActivatedEventArgs().PrelaunchActivated() == false)
    {
      if (rootFrame.Content() == nullptr)
      {
        /**
         * When the navigation stack isn't restored
         * navigate to the first page, configuring
         * the new page by passing the required
         * information as a navigation parameter. */
        rootFrame.Navigate(xaml_typename<Kraken::MainWindow>(), box_value(e.Arguments()));
      }

      /**
       * Ensure the current window is active. */
      Window::Current().Activate();
    }
  }
}

/**
 * Invoked when application execution is being suspended.
 * Application state is saved without knowing whether the
 * application will be terminated or resumed with the
 * contents of memory still intact. */
void App::OnSuspending([[maybe_unused]] IInspectable const &sender, [[maybe_unused]] SuspendingEventArgs const &e)
{
  /**
   * Save application state and stop any background activity. */
}

/**
 * Invoked when Navigation to a certain page fails. */
void App::OnNavigationFailed(IInspectable const &, NavigationFailedEventArgs const &e)
{
  throw hresult_error(E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}
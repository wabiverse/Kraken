#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.h"
#include "winrt/Microsoft.UI.Xaml.h"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Navigation;

using namespace Kraken;
using namespace Kraken::implementation;

App::App()
{
  InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
  UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
  {
    if (IsDebuggerPresent()) {
      auto errorMessage = e.Message();
      __debugbreak();
    }
  });
#endif
}

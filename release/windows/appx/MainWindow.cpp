
#include "pch.h"
#include "MainWindow.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

using namespace Kraken;
using namespace Kraken::implementation;

namespace winrt::Kraken::implementation
{
  MainWindow::MainWindow()
  {
    winrt_make_Kraken_MainWindow();
  }

  MainWindow::~MainWindow()
  {}
}
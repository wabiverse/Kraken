#pragma once
#include "MainWindow.g.h"

namespace winrt::Kraken::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow();
    };
}
namespace winrt::Kraken::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {};
}

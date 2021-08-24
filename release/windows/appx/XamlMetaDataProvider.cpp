#include "pch.h"
#include "XamlMetaDataProvider.h"
#include "XamlMetaDataProvider.g.cpp"

namespace winrt::Kraken::implementation
{
    winrt::Microsoft::UI::Xaml::Markup::IXamlType XamlMetaDataProvider::GetXamlType(winrt::Windows::UI::Xaml::Interop::TypeName const& type)
    {
        throw hresult_not_implemented();
    }

    winrt::Microsoft::UI::Xaml::Markup::IXamlType XamlMetaDataProvider::GetXamlType(hstring const& fullName)
    {
        throw hresult_not_implemented();
    }
    
    com_array<winrt::Microsoft::UI::Xaml::Markup::XmlnsDefinition> XamlMetaDataProvider::GetXmlnsDefinitions()
    {
        throw hresult_not_implemented();
    }
}

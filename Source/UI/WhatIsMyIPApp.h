#pragma once

#include "XamlApplication.h"

namespace UI
{

class WhatIsMyIPApp :
	public XamlApplication
{
private:
	WRL::ComPtr<ABI::Windows::UI::Xaml::IUIElement> m_RootElement;
	WRL::ComPtr<ABI::Windows::UI::Xaml::Controls::ITextBlock> m_TextBlock;

	HRESULT CreateXamlLayout();
	HRESULT RefreshIPInformationText();

public:
	virtual HRESULT STDMETHODCALLTYPE OnLaunched(ABI::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs* args) override;
};

}
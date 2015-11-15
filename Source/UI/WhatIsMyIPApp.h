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
	WRL::ComPtr<ABI::Windows::UI::Xaml::IUIElement> m_ProgressBar;
	uint32_t m_ActiveRefreshTaskCount;
	EventRegistrationToken m_OnNetworkStatusChangedToken;

	HRESULT CreateXamlLayout();
	HRESULT RefreshIPInformationText();

	HRESULT CreatePage(ABI::Windows::UI::Xaml::IUIElement** page);
	HRESULT CreateRootGrid(ABI::Windows::UI::Xaml::IUIElement** outGrid);
	HRESULT CreateScrollViewer(ABI::Windows::UI::Xaml::IUIElement** outScrollViewer);
	HRESULT CreateProgressBar(ABI::Windows::UI::Xaml::IUIElement** outProgressBar);
	HRESULT CreateIPInformationTextBlock(ABI::Windows::UI::Xaml::IUIElement** outTextBlock);

protected:
	virtual void Cleanup() override;

public:
	WhatIsMyIPApp();

	virtual HRESULT STDMETHODCALLTYPE OnLaunched(ABI::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnWindowActivated(ABI::Windows::UI::Core::IWindowActivatedEventArgs* e) override;
};

}
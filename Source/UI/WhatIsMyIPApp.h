#pragma once

#include "Networking\ConnectionProperties.h"
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
	WRL::ComPtr<ABI::Windows::UI::Xaml::Controls::Primitives::IButtonBase> m_RefreshButton;
	uint32_t m_ActiveRefreshTaskCount;
	bool m_HadFirstWindowActivation;

	uint32_t m_WatchIPInformationChangesToken;
	EventRegistrationToken m_RefreshButtonClickedToken;

	HRESULT CreateXamlLayout();
	void RequestIPInformationUpdate();
	void OnIPInformationRefreshed(const std::vector<Networking::ConnectionProperties>& connectionProperties);

	HRESULT CreatePage(ABI::Windows::UI::Xaml::IUIElement** page);
	HRESULT CreateRootGrid(ABI::Windows::UI::Xaml::IUIElement** outGrid);
	HRESULT CreateScrollViewerForTextBlock(ABI::Windows::UI::Xaml::IUIElement** outScrollViewer);
	HRESULT CreateIPInformationTextBlock(ABI::Windows::UI::Xaml::Controls::ITextBlock** outTextBlock);

	HRESULT CreateProgressBar(ABI::Windows::UI::Xaml::IUIElement** outProgressBar);

	HRESULT CreateBottomAppBar(ABI::Windows::UI::Xaml::Controls::IAppBar** outAppBar);
	HRESULT CreateStackPanelForAppBar(ABI::Windows::UI::Xaml::Controls::IStackPanel** outStackPanel);
	HRESULT CreateRefreshButtomForAppBar(ABI::Windows::UI::Xaml::IUIElement** outButton);

protected:
	virtual void Cleanup() override;

public:
	WhatIsMyIPApp();

	virtual HRESULT STDMETHODCALLTYPE OnLaunched(ABI::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnWindowActivated(ABI::Windows::UI::Core::IWindowActivatedEventArgs* e) override;
};

}
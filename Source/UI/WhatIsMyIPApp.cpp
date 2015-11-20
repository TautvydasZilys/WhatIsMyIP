#include "PrecompiledHeader.h"
#include "Networking\ConnectionProperties.h"
#include "Networking\IPInformationGenerator.h"
#include "PlugNPlay\PlugNPlayObjectRegistry.h"
#include "Utilities\EventHandler.h"
#include "VisualObjects.h"
#include "WhatIsMyIPApp.h"

using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking::Connectivity;
using namespace ABI::Windows::UI;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::ViewManagement;
using namespace ABI::Windows::UI::Xaml;
using namespace ABI::Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Xaml::Controls::Primitives;
using namespace ABI::Windows::UI::Xaml::Media;
using namespace UI;

WhatIsMyIPApp::WhatIsMyIPApp() :
	m_ActiveRefreshTaskCount(0)
{
	m_OnNetworkStatusChangedToken.value = 0;
	m_RefreshButtonClickedToken.value = 0;
}

void WhatIsMyIPApp::Cleanup()
{
	HRESULT hr;

	if (m_OnNetworkStatusChangedToken.value != 0)
	{
		hr = Networking::IPInformation::UnsubscribeFromOnNetworkStatusChanged(m_OnNetworkStatusChangedToken);
		Assert(SUCCEEDED(hr));
	}
	
	if (m_RefreshButton != nullptr && m_RefreshButtonClickedToken.value != 0)
	{
		hr = m_RefreshButton->remove_Click(m_RefreshButtonClickedToken);
		Assert(SUCCEEDED(hr));
	}

	PlugNPlay::PlugNPlayObjectRegistry::Destroy();
	XamlApplication::Cleanup();
}

HRESULT WhatIsMyIPApp::CreatePage(IUIElement** outPage)
{
	WRL::ComPtr<IPage> page;
	HRESULT hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.Page"), &page);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> grid;
	hr = CreateRootGrid(&grid);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUserControl> pageControl;
	hr = page.As(&pageControl);
	ReturnIfFailed(hr);

	hr = pageControl->put_Content(grid.Get());
	ReturnIfFailed(hr);

	WRL::ComPtr<IAppBar> bottomAppBar;
	hr = CreateBottomAppBar(&bottomAppBar);
	ReturnIfFailed(hr);

	page->put_BottomAppBar(bottomAppBar.Get());
	ReturnIfFailed(hr);

	return page.Get()->QueryInterface(outPage);
}

static inline HRESULT AddGridRow(IVector<RowDefinition*>* rows, double height, GridUnitType unitType)
{
	WRL::ComPtr<IRowDefinition> row;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.RowDefinition"), &row);
	ReturnIfFailed(hr);

	GridLength rowHeight = { height, unitType };
	hr = row->put_Height(rowHeight);
	ReturnIfFailed(hr);

	return rows->Append(row.Get());
}

static inline HRESULT SetGridRow(IGridStatics* gridStatics, IUIElement* child, int row)
{
	WRL::ComPtr<IFrameworkElement> frameworkElement;
	auto hr = child->QueryInterface(__uuidof(IFrameworkElement), &frameworkElement);
	ReturnIfFailed(hr);

	return gridStatics->SetRow(frameworkElement.Get(), row);
}

HRESULT WhatIsMyIPApp::CreateRootGrid(IUIElement** outGrid)
{
	WRL::ComPtr<IGridStatics> gridStatics;
	auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.Grid"), &gridStatics);
	ReturnIfFailed(hr);

	WRL::ComPtr<IGrid> grid;
	hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.Grid"), &grid);
	ReturnIfFailed(hr);

	WRL::ComPtr<IPanel> gridPanel;
	hr = grid.As(&gridPanel);
	ReturnIfFailed(hr);

	// Row layout
	{
		WRL::ComPtr<IVector<RowDefinition*>> rowDefinitions;
		hr = grid->get_RowDefinitions(&rowDefinitions);
		ReturnIfFailed(hr);

		hr = AddGridRow(rowDefinitions.Get(), 15.0, GridUnitType_Pixel);
		ReturnIfFailed(hr);

		hr = AddGridRow(rowDefinitions.Get(), 0.0, GridUnitType_Auto);
		ReturnIfFailed(hr);
	}

	// Background
	{
		WRL::ComPtr<IBrush> gridBackground;
		hr = UI::VisualObjects::GetBrushFromColor(0, 0, 0, 255, &gridBackground);
		ReturnIfFailed(hr);

		hr = gridPanel->put_Background(gridBackground.Get());
		ReturnIfFailed(hr);
	}

	// Children
	{
		WRL::ComPtr<IVector<UIElement*>> gridChildren;
		hr = gridPanel->get_Children(&gridChildren);
		ReturnIfFailed(hr);

		WRL::ComPtr<IUIElement> uiElement;

		// Progress bar
		{
			hr = CreateProgressBar(&uiElement);
			ReturnIfFailed(hr);

			hr = gridChildren->Append(uiElement.Get());
			ReturnIfFailed(hr);

			hr = SetGridRow(gridStatics.Get(), uiElement.Get(), 0);
			ReturnIfFailed(hr);
		}

		// Scrollviewer
		{
			hr = CreateScrollViewerForTextBlock(&uiElement);
			ReturnIfFailed(hr);

			hr = gridChildren->Append(uiElement.Get());
			ReturnIfFailed(hr);

			hr = SetGridRow(gridStatics.Get(), uiElement.Get(), 1);
			ReturnIfFailed(hr);
		}
	}

	return grid.Get()->QueryInterface(outGrid);
}

static inline HRESULT CreateScrollViewerWithContent(IInspectable* content, IScrollViewer** outScrollViewer)
{
	WRL::ComPtr<IScrollViewer> scrollViewer;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.ScrollViewer"), &scrollViewer);
	ReturnIfFailed(hr);

	WRL::ComPtr<IContentControl> scrollViewerContentControl;
	hr = scrollViewer.As(&scrollViewerContentControl);
	ReturnIfFailed(hr);

	hr = scrollViewerContentControl->put_Content(content);
	ReturnIfFailed(hr);

	*outScrollViewer = scrollViewer.Detach();
	return S_OK;
}

HRESULT WhatIsMyIPApp::CreateScrollViewerForTextBlock(IUIElement** outScrollViewer)
{
	WRL::ComPtr<ITextBlock> textBlock;
	auto hr = CreateIPInformationTextBlock(&textBlock);
	ReturnIfFailed(hr);

	WRL::ComPtr<IScrollViewer> scrollViewer;
	hr = CreateScrollViewerWithContent(textBlock.Get(), &scrollViewer);
	ReturnIfFailed(hr);

	hr = scrollViewer->put_HorizontalScrollBarVisibility(ScrollBarVisibility_Auto);
	ReturnIfFailed(hr);

	return scrollViewer.Get()->QueryInterface(outScrollViewer);
}

HRESULT WhatIsMyIPApp::CreateProgressBar(IUIElement** outProgressBar)
{
	WRL::ComPtr<IProgressBar> progressBar;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.ProgressBar"), &progressBar);
	ReturnIfFailed(hr);

	hr = progressBar->put_IsIndeterminate(true);
	ReturnIfFailed(hr);

	WRL::ComPtr<IFrameworkElement> progressBarFrameworkElement;
	hr = progressBar.As(&progressBarFrameworkElement);
	ReturnIfFailed(hr);

	hr = progressBarFrameworkElement->put_VerticalAlignment(VerticalAlignment_Top);
	ReturnIfFailed(hr);

	hr = progressBarFrameworkElement->put_HorizontalAlignment(HorizontalAlignment_Stretch);
	ReturnIfFailed(hr);

	progressBar.As(&m_ProgressBar);
	ReturnIfFailed(hr);

	hr = m_ProgressBar->put_Visibility(Visibility_Collapsed);
	ReturnIfFailed(hr);

	*outProgressBar = m_ProgressBar.Get();
	(*outProgressBar)->AddRef();
	return S_OK;
}

HRESULT WhatIsMyIPApp::CreateIPInformationTextBlock(ITextBlock** outTextBlock)
{
	WRL::ComPtr<ITextBlock> textBlock;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.TextBlock"), &textBlock);
	ReturnIfFailed(hr);

	WRL::ComPtr<IBrush> textBrush;
	hr = VisualObjects::GetBrushFromColor(255, 255, 255, 255, &textBrush);
	ReturnIfFailed(hr);

	hr = textBlock->put_Foreground(textBrush.Get());
	ReturnIfFailed(hr);

	WRL::ComPtr<IFontFamily> font;
	hr = VisualObjects::GetMonospaceFont(&font);
	ReturnIfFailed(hr);

	if (hr != S_FALSE)
	{
		hr = textBlock->put_FontFamily(font.Get());
		ReturnIfFailed(hr);
	}

	m_TextBlock = textBlock.Detach();
	*outTextBlock = m_TextBlock.Get();
	(*outTextBlock)->AddRef();
	return S_OK;
}

HRESULT WhatIsMyIPApp::CreateBottomAppBar(IAppBar** outAppBar)
{
	WRL::ComPtr<IAppBar> appBar;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.AppBar"), &appBar);
	if (SUCCEEDED(hr))
	{
		// This path fails on WP8.1... AppBar creation fails with ERROR_NOT_SUPPORTED
		WRL::ComPtr<IStackPanel> stackPanel;
		hr = CreateStackPanelForAppBar(&stackPanel);
		ReturnIfFailed(hr);

		WRL::ComPtr<IContentControl> appBarContentControl;
		hr = appBar.As(&appBarContentControl);
		ReturnIfFailed(hr);

		hr = appBarContentControl->put_Content(stackPanel.Get());
		ReturnIfFailed(hr);
	}
	else
	{
		hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.CommandBar"), &appBar);
		ReturnIfFailed(hr);

		WRL::ComPtr<ICommandBar> commandBar;
		hr = appBar.As(&commandBar);
		ReturnIfFailed(hr);

		WRL::ComPtr<IUIElement> refreshButton;
		hr = CreateRefreshButtomForAppBar(&refreshButton);
		ReturnIfFailed(hr);

		WRL::ComPtr<ICommandBarElement> refreshCommand;
		hr = refreshButton.As(&refreshCommand);
		ReturnIfFailed(hr);

		WRL::ComPtr<IObservableVector<ICommandBarElement*>> primaryCommands;
		hr = commandBar->get_PrimaryCommands(&primaryCommands);
		ReturnIfFailed(hr);

		WRL::ComPtr<IVector<ICommandBarElement*>> primaryCommandsVector;
		hr = primaryCommands.As(&primaryCommandsVector);
		ReturnIfFailed(hr);

		hr = primaryCommandsVector->Append(refreshCommand.Get());
		ReturnIfFailed(hr);
	}

	*outAppBar = appBar.Detach();
	return S_OK;
}

HRESULT WhatIsMyIPApp::CreateStackPanelForAppBar(IStackPanel** outStackPanel)
{
	WRL::ComPtr<IStackPanel> stackPanel;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.StackPanel"), &stackPanel);
	ReturnIfFailed(hr);

	hr = stackPanel->put_Orientation(Orientation_Horizontal);
	ReturnIfFailed(hr);

	WRL::ComPtr<IPanel> panel;
	hr = stackPanel.As(&panel);
	ReturnIfFailed(hr);

	WRL::ComPtr<IVector<UIElement*>> children;
	hr = panel->get_Children(&children);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> refreshButton;
	hr = CreateRefreshButtomForAppBar(&refreshButton);
	ReturnIfFailed(hr);

	hr = children->Append(refreshButton.Get());
	ReturnIfFailed(hr);

	*outStackPanel = stackPanel.Detach();
	return S_OK;
}

HRESULT WhatIsMyIPApp::CreateRefreshButtomForAppBar(IUIElement** outButton)
{
	WRL::ComPtr<IAppBarButton> button;
	auto hr = Windows::Foundation::ActivateInstance(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.AppBarButton"), &button);
	ReturnIfFailed(hr);

	hr = button->put_Label(Utilities::HStringReference(L"Refresh"));
	ReturnIfFailed(hr);

	WRL::ComPtr<ISymbolIconFactory> symbolIconFactory;
	hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.UI.Xaml.Controls.SymbolIcon"), &symbolIconFactory);
	ReturnIfFailed(hr);

	WRL::ComPtr<ISymbolIcon> symbolIcon;
	hr = symbolIconFactory->CreateInstanceWithSymbol(Symbol_Refresh, &symbolIcon);
	ReturnIfFailed(hr);

	WRL::ComPtr<IIconElement> iconElement;
	hr = symbolIcon.As(&iconElement);
	ReturnIfFailed(hr);

	hr = button->put_Icon(iconElement.Get());
	ReturnIfFailed(hr);

	hr = button.As(&m_RefreshButton);
	ReturnIfFailed(hr);

	WRL::ComPtr<WhatIsMyIPApp> _this = this;
	m_RefreshButton->add_Click(Utilities::EventHandlerFactory<IRoutedEventHandler>::Make([_this](IInspectable* sender, IRoutedEventArgs* args)
	{
		return _this->RefreshIPInformationText();
	}).Get(), &m_RefreshButtonClickedToken);

	return button.Get()->QueryInterface(outButton);
}

#if !WINDOWS_8_1

static HRESULT SetViewBoundsMode(ApplicationViewBoundsMode boundsMode)
{
	WRL::ComPtr<IApplicationViewStatics2> applicationViewStatics;
	auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.UI.ViewManagement.ApplicationView"), &applicationViewStatics);
	ReturnIfFailed(hr);

	WRL::ComPtr<IApplicationView> applicationView;
	hr = applicationViewStatics->GetForCurrentView(&applicationView);
	ReturnIfFailed(hr);

	WRL::ComPtr<IApplicationView2> applicationView2;
	hr = applicationView.As(&applicationView2);
	ReturnIfFailed(hr);

	boolean success;
	hr = applicationView2->SetDesiredBoundsMode(boundsMode, &success);
	return hr;
}

#endif

HRESULT WhatIsMyIPApp::CreateXamlLayout()
{
	HRESULT hr;

#if !WINDOWS_8_1
	hr = SetViewBoundsMode(ApplicationViewBoundsMode_UseVisible);
	ReturnIfFailed(hr);
#endif

	hr = CreatePage(&m_RootElement);
	ReturnIfFailed(hr);

	return S_OK;
}

HRESULT WhatIsMyIPApp::RefreshIPInformationText()
{
	if (m_ActiveRefreshTaskCount == 0)
	{
		auto hr = m_ProgressBar->put_Visibility(Visibility_Visible);
		ReturnIfFailed(hr);
	}

	m_ActiveRefreshTaskCount++;

	WRL::ComPtr<WhatIsMyIPApp> _this = this;
	return Networking::GenerateIPInformationAsync([_this](const std::vector<Networking::ConnectionProperties>& connectionProperties)
	{
		std::wstringstream textStream;

		if (connectionProperties.size() > 0)
		{
			for (auto& properties : connectionProperties)
			{
				textStream << properties.name << std::endl;

				for (auto& property : properties.properties)
				{
					textStream << L"    ";
					textStream << std::setw(28) << std::left << property.first;
					textStream << property.second << std::endl;
				}

				textStream << std::endl;
			}
		}
		else
		{
			textStream << L"There are no connections available." << std::endl;
		}

		auto str = textStream.str();
		Utilities::HString text(str.c_str(), str.length());

		auto hr = _this->ExecuteOnUIThread([_this, text]() -> HRESULT
		{
			auto hr = _this->m_TextBlock->put_Text(text);
			ReturnIfFailed(hr);

			_this->m_ActiveRefreshTaskCount--;
			if (_this->m_ActiveRefreshTaskCount == 0)
				return _this->m_ProgressBar->put_Visibility(Visibility_Collapsed);

			return S_OK;
		});

		return hr;
	});
}

HRESULT STDMETHODCALLTYPE WhatIsMyIPApp::OnLaunched(ILaunchActivatedEventArgs* args)
{
	auto hr = CreateXamlLayout();
	ReturnIfFailed(hr);

	hr = GetWindow()->put_Content(m_RootElement.Get());
	ReturnIfFailed(hr);

	WRL::ComPtr<WhatIsMyIPApp> _this = this;
	hr = Networking::IPInformation::SubscribeToOnNetworkStatusChanged(Utilities::EventHandlerFactory<INetworkStatusChangedEventHandler>::Make([_this](IInspectable* sender) -> HRESULT
	{
		return _this->ExecuteOnUIThread([_this]()
		{
			return _this->RefreshIPInformationText();
		});
	}).Get(), &m_OnNetworkStatusChangedToken);

	hr = PlugNPlay::PlugNPlayObjectRegistry::Create();
	ReturnIfFailed(hr);

	return XamlApplication::OnLaunched(args);
}

HRESULT STDMETHODCALLTYPE WhatIsMyIPApp::OnWindowActivated(IWindowActivatedEventArgs* e)
{
	CoreWindowActivationState windowActivationState;
	HRESULT hr = e->get_WindowActivationState(&windowActivationState);
	ReturnIfFailed(hr);

	if (windowActivationState != CoreWindowActivationState_Deactivated)
	{
		hr = RefreshIPInformationText();
		ReturnIfFailed(hr);
	}

	return S_OK;
}
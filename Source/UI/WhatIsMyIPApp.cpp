#include "PrecompiledHeader.h"
#include "WhatIsMyIPApp.h"
#include "Networking\IPInformationGenerator.h"
#include "Utilities\EventHandler.h"

using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::ViewManagement;
using namespace ABI::Windows::UI::Xaml;
using namespace ABI::Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Xaml::Media;
using namespace UI;

static HRESULT ApplyTextColor(ITextBlock* textBlock)
{
	WRL::ComPtr<ISolidColorBrushFactory> brushFactory;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.UI.Xaml.Media.SolidColorBrush").Get(), &brushFactory);
	ReturnIfFailed(hr);

	Color white = { 255, 255, 255, 255 };
	WRL::ComPtr<ISolidColorBrush> solidColorBrush;
	hr = brushFactory->CreateInstanceWithColor(white, &solidColorBrush);
	ReturnIfFailed(hr);

	WRL::ComPtr<IBrush> brush;
	hr = solidColorBrush.As(&brush);
	ReturnIfFailed(hr);

	return S_OK;
}

HRESULT WhatIsMyIPApp::CreatePage(IUIElement** outPage)
{
	WRL::ComPtr<IPage> page;
	HRESULT hr = Windows::Foundation::ActivateInstance(WRL::HStringReference(L"Windows.UI.Xaml.Controls.Page").Get(), &page);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> grid;
	hr = CreateRootGrid(&grid);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUserControl> pageControl;
	hr = page.As(&pageControl);
	ReturnIfFailed(hr);

	hr = pageControl->put_Content(grid.Get());
	ReturnIfFailed(hr);

	hr = page.As(&m_RootElement);
	ReturnIfFailed(hr);
	
	return page.Get()->QueryInterface(outPage);
}

HRESULT WhatIsMyIPApp::CreateRootGrid(IUIElement** outGrid)
{
	WRL::ComPtr<IGrid> grid;
	HRESULT hr = Windows::Foundation::ActivateInstance(WRL::HStringReference(L"Windows.UI.Xaml.Controls.Grid").Get(), &grid);
	ReturnIfFailed(hr);

	WRL::ComPtr<IPanel> gridPanel;
	hr = grid.As(&gridPanel);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> scrollViewer;
	hr = CreateScrollViewer(&scrollViewer);
	ReturnIfFailed(hr);

	WRL::ComPtr<IVector<UIElement*>> gridChildren;
	hr = gridPanel->get_Children(&gridChildren);
	ReturnIfFailed(hr);

	hr = gridChildren->Append(scrollViewer.Get());
	ReturnIfFailed(hr);

	return grid.Get()->QueryInterface(outGrid);
}

HRESULT WhatIsMyIPApp::CreateScrollViewer(IUIElement** outScrollViewer)
{
	WRL::ComPtr<IScrollViewer> scrollViewer;
	auto hr = Windows::Foundation::ActivateInstance(WRL::HStringReference(L"Windows.UI.Xaml.Controls.ScrollViewer").Get(), &scrollViewer);
	ReturnIfFailed(hr);

	hr = scrollViewer->put_HorizontalScrollBarVisibility(ScrollBarVisibility_Auto);
	ReturnIfFailed(hr);

	WRL::ComPtr<IContentControl> scrollViewerContentControl;
	hr = scrollViewer.As(&scrollViewerContentControl);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> textBlockElement;
	hr = CreateIPInformationTextBlock(&textBlockElement);
	ReturnIfFailed(hr);

	hr = scrollViewerContentControl->put_Content(textBlockElement.Get());
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> scrollViewerElement;
	hr = scrollViewer.As(&scrollViewerElement);
	ReturnIfFailed(hr);

	return scrollViewer.Get()->QueryInterface(outScrollViewer);
}

HRESULT WhatIsMyIPApp::CreateIPInformationTextBlock(IUIElement** outTextBlock)
{
	WRL::ComPtr<ITextBlock> textBlock;
	auto hr = Windows::Foundation::ActivateInstance(WRL::HStringReference(L"Windows.UI.Xaml.Controls.TextBlock").Get(), &textBlock);
	ReturnIfFailed(hr);

	hr = ApplyTextColor(textBlock.Get());
	ReturnIfFailed(hr);

	m_TextBlock = textBlock.Detach();
	return m_TextBlock.Get()->QueryInterface(outTextBlock);
}

#if !WINDOWS_8_1

static HRESULT SetViewBoundsMode(ApplicationViewBoundsMode boundsMode)
{
	WRL::ComPtr<IApplicationViewStatics2> applicationViewStatics;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.UI.ViewManagement.ApplicationView").Get(), &applicationViewStatics);
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
	return Networking::GenerateIPInformationAsync([this](HSTRING contents)
	{
		WRL::ComPtr<IAsyncAction> asyncAction;
		HSTRING text;

		auto hr = WindowsDuplicateString(contents, &text);
		ReturnIfFailed(hr);

		hr = GetDispatcher()->RunAsync(CoreDispatcherPriority_Normal, Utilities::EventHandlerFactory<IDispatchedHandler>::Make([this, text]() -> HRESULT
		{
			WRL::HString str;
			str.Attach(text);

			auto hr = m_TextBlock->put_Text(text);
			ReturnIfFailed(hr);

			return S_OK;
		}).Get(), &asyncAction);

		if (FAILED(hr))
			WindowsDeleteString(text);

		return hr;
	});
}

HRESULT STDMETHODCALLTYPE WhatIsMyIPApp::OnLaunched(ILaunchActivatedEventArgs* args)
{
	auto hr = CreateXamlLayout();
	ReturnIfFailed(hr);

	hr = GetWindow()->put_Content(m_RootElement.Get());
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
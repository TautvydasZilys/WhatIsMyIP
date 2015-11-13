#include "PrecompiledHeader.h"
#include "WhatIsMyIPApp.h"
#include "Networking\IPInformationGenerator.h"
#include "Utilities\EventHandler.h"

using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Xaml;
using namespace ABI::Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Xaml::Media;
using namespace UI;

#define MultiplyString1(x) x x x x x x x x x x
#define MultiplyString2(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x) MultiplyString1(x)

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
	//return textBlock->put_Foreground(brush.Get());
}

static HRESULT CreateScrollViewer(IScrollViewer** outScrollViewer)
{
	WRL::ComPtr<IScrollViewer> scrollViewer;
	auto hr = Windows::Foundation::ActivateInstance(WRL::HStringReference(L"Windows.UI.Xaml.Controls.ScrollViewer").Get(), &scrollViewer);
	ReturnIfFailed(hr);

	hr = scrollViewer->put_HorizontalScrollBarVisibility(ScrollBarVisibility_Auto);
	ReturnIfFailed(hr);

	*outScrollViewer = scrollViewer.Detach();
	return S_OK;
}

static HRESULT CreateTextBlock(ITextBlock** outTextBlock)
{
	WRL::ComPtr<ITextBlock> textBlock;
	auto hr = Windows::Foundation::ActivateInstance(WRL::HStringReference(L"Windows.UI.Xaml.Controls.TextBlock").Get(), &textBlock);
	ReturnIfFailed(hr);

	hr = ApplyTextColor(textBlock.Get());
	ReturnIfFailed(hr);

	*outTextBlock = textBlock.Detach();
	return S_OK;
}

HRESULT WhatIsMyIPApp::CreateXamlLayout()
{
	WRL::ComPtr<IScrollViewer> scrollViewer;
	auto hr = CreateScrollViewer(&scrollViewer);
	ReturnIfFailed(hr);

	WRL::ComPtr<IContentControl> scrollViewerContentControl;
	hr = scrollViewer.As(&scrollViewerContentControl);
	ReturnIfFailed(hr);

	hr = CreateTextBlock(&m_TextBlock);
	ReturnIfFailed(hr);

	WRL::ComPtr<IUIElement> textBlockElement;
	hr = m_TextBlock.As(&textBlockElement);
	ReturnIfFailed(hr);

	hr = scrollViewerContentControl->put_Content(textBlockElement.Get());
	ReturnIfFailed(hr);

	hr = scrollViewer.As(&m_RootElement);
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

HRESULT WhatIsMyIPApp::OnLaunched(ILaunchActivatedEventArgs* args)
{
	auto hr = CreateXamlLayout();
	ReturnIfFailed(hr);

	hr = GetWindow()->put_Content(m_RootElement.Get());
	ReturnIfFailed(hr);

	hr = RefreshIPInformationText();
	ReturnIfFailed(hr);

	return XamlApplication::OnLaunched(args);
}

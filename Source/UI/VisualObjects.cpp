#include "PrecompiledHeader.h"
#include "VisualObjects.h"

using namespace ABI::Windows::UI;
using namespace ABI::Windows::UI::Xaml::Media;

HRESULT UI::VisualObjects::GetBrushFromColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, IBrush** outBrush)
{
	WRL::ComPtr<ISolidColorBrushFactory> brushFactory;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.UI.Xaml.Media.SolidColorBrush").Get(), &brushFactory);
	ReturnIfFailed(hr);

	Color color = { r, g, b, a };
	WRL::ComPtr<ISolidColorBrush> solidColorBrush;
	hr = brushFactory->CreateInstanceWithColor(color, &solidColorBrush);
	ReturnIfFailed(hr);

	WRL::ComPtr<IBrush> brush;
	hr = solidColorBrush.As(&brush);
	ReturnIfFailed(hr);

	*outBrush = brush.Detach();
	return S_OK;
}

static HRESULT IsFontFamilyMonospace(IDWriteFontFamily* fontFamily, bool* isMonospace)
{
	WRL::ComPtr<IDWriteFontList> fonts;
	auto hr = fontFamily->GetMatchingFonts(DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, &fonts);
	ReturnIfFailed(hr);

	auto fontCount = fonts->GetFontCount();
	if (fontCount == 0)
	{
		*isMonospace = false;
		return S_OK;
	}

	WRL::ComPtr<IDWriteFont> font;
	hr = fonts->GetFont(0, &font);
	ReturnIfFailed(hr);

	WRL::ComPtr<IDWriteFontFace> fontFace;
	hr = font->CreateFontFace(&fontFace);
	ReturnIfFailed(hr);

	WRL::ComPtr<IDWriteFontFace1> fontFace1;
	hr = fontFace.As(&fontFace1);
	ReturnIfFailed(hr);

	*isMonospace = fontFace1->IsMonospacedFont() != FALSE;
	return S_OK;
}

static HRESULT GetFontFamilyName(IDWriteFontFamily* fontFamily, std::wstring* fontName)
{
	WRL::ComPtr<IDWriteLocalizedStrings> familyNames;
	auto hr = fontFamily->GetFamilyNames(&familyNames);
	ReturnIfFailed(hr);

	uint32_t fontNameLength;
	hr = familyNames->GetStringLength(0, &fontNameLength);
	ReturnIfFailed(hr);

	fontName->resize(fontNameLength);
	return familyNames->GetString(0, &(*fontName)[0], fontNameLength + 1);
}

template <typename CallbackType>
HRESULT EnumerateSystemFonts(CallbackType callback)
{
	WRL::ComPtr<IDWriteFactory> directWriteFactory;

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &directWriteFactory);
	ReturnIfFailed(hr);

	WRL::ComPtr<IDWriteFontCollection> systemFonts;
	hr = directWriteFactory->GetSystemFontCollection(&systemFonts);
	ReturnIfFailed(hr);

	uint32_t fontCount = systemFonts->GetFontFamilyCount();

	for (uint32_t i = 0; i < fontCount; ++i)
	{
		WRL::ComPtr<IDWriteFontFamily> fontFamily;
		hr = systemFonts->GetFontFamily(i, &fontFamily);
		ContinueIfFailed(hr);

		hr = callback(fontFamily.Get());
		ContinueIfFailed(hr);
	}

	return S_OK;
}

const std::initializer_list<wchar_t*> kFontPreferences = 
{
	L"Consolas",
	L"Lucida Console",
	L"Segoe UI Mono",
	L"SimHei",
	L"Prestige Elite Std",
	L"Courier New"
};

static HRESULT GetBestMonospaceFontName(std::wstring* chosenFontName)
{
	std::vector<std::wstring> monospaceFonts;

	auto hr = EnumerateSystemFonts([&monospaceFonts](IDWriteFontFamily* fontFamily)
	{
		bool isMonospace;
		auto hr = IsFontFamilyMonospace(fontFamily, &isMonospace);
		ReturnIfFailed(hr);

		if (isMonospace)
		{
			std::wstring fontName;
			hr = GetFontFamilyName(fontFamily, &fontName);
			ReturnIfFailed(hr);

			monospaceFonts.push_back(std::move(fontName));
		}

		return S_OK;
	});
	ReturnIfFailed(hr);

	if (monospaceFonts.size() == 0)
		return S_FALSE;

	for (const auto& fontPreference : kFontPreferences)
	{
		for (auto& fontName : monospaceFonts)
		{
			if (fontPreference == fontName)
			{
				*chosenFontName = std::move(fontName);
				return S_OK;
			}
		}
	}

	// Fallback to first monospace font on the system
	*chosenFontName = std::move(monospaceFonts[0]);
	return S_OK;
}

HRESULT UI::VisualObjects::GetMonospaceFont(ABI::Windows::UI::Xaml::Media::IFontFamily** outFont)
{
	std::wstring chosenFontName;
	auto hr = GetBestMonospaceFontName(&chosenFontName);
	ReturnIfFailed(hr);

	if (hr == S_FALSE)
		return hr;

	WRL::ComPtr<IFontFamilyFactory> fontFactory;
	hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.UI.Xaml.Media.FontFamily").Get(), &fontFactory);
	ReturnIfFailed(hr);

	WRL::ComPtr<IInspectable> outer;
	WRL::ComPtr<IInspectable> inner;
	return fontFactory->CreateInstanceWithName(WRL::HStringReference(chosenFontName.c_str()).Get(), nullptr, &inner, outFont);
}
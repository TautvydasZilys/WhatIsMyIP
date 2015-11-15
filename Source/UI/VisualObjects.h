#pragma once

namespace UI
{

namespace VisualObjects
{

HRESULT GetBrushFromColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, ABI::Windows::UI::Xaml::Media::IBrush** outBrush);
HRESULT GetMonospaceFont(ABI::Windows::UI::Xaml::Media::IFontFamily** outFont);

}

}
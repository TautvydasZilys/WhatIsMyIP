#pragma once

#include <roapi.h>
#include <windows.h>
#include <windows.networking.connectivity.h>
#include <windows.ui.h>
#include <windows.ui.xaml.h>
#include <windows.ui.xaml.controls.h>
#include <windows.ui.xaml.media.h>
#include <wrl.h>

#undef min
#undef max

#include <cstdint>
#include <utility>
#include <set>
#include <vector>

namespace WRL
{
	using namespace Microsoft::WRL;
	using namespace Microsoft::WRL::Wrappers;
}

#if _DEBUG
#define Assert(x) do { if (!(x)) { OutputDebugStringA("Assertion failed: " #x "\r\n"); __debugbreak(); } } while (false)
#else
#define Assert(x) do { } while (false)
#endif

#define DoIfFailed(hr, action) do { Assert(SUCCEEDED(hr)); if (FAILED(hr)) { action; } } while (false)
#define ContinueIfFailed(hr) DoIfFailed(hr, continue)
#define ReturnIfFailed(hr) DoIfFailed(hr, return hr)
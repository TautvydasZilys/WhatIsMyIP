#pragma once

#include <dwrite_1.h>
#include <roapi.h>
#include <windows.h>
#include <windows.devices.enumeration.pnp.h>
#include <windows.networking.connectivity.h>
#include <windows.ui.h>
#include <windows.ui.viewmanagement.h>
#include <windows.ui.xaml.h>
#include <windows.ui.xaml.controls.h>
#include <windows.ui.xaml.media.h>
#include <wrl.h>

#undef min
#undef max

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace WRL
{
	using namespace Microsoft::WRL;
}

#if _DEBUG
#define Assert(x) do { if (!(x)) { OutputDebugStringA("Assertion failed: " #x "\r\n"); __debugbreak(); } } while (false)
#else
#define Assert(x) do { } while (false)
#endif

#define DoIfFailed(hr, action) do { Assert(SUCCEEDED(hr)); if (FAILED(hr)) { action; } } while (false)
#define ContinueIfFailed(hr) DoIfFailed(hr, continue)
#define ReturnIfFailed(hr) DoIfFailed(hr, return hr)
#define ReturnSuccessIfFailed(hr) DoIfFailed(hr, return S_OK)
#define AdvanceFenceAndContinueIfFailed(hr, fence) DoIfFailed(hr, fence.Advance(); continue)
#define AdvanceFenceAndReturnIfFailed(hr, fence) DoIfFailed(hr, fence.Advance(); return hr)
#define AdvanceFenceAndReturnSuccessIfFailed(hr, fence) DoIfFailed(hr, fence.Advance(); return S_OK)

#ifndef WINDOWS_8_1
#define WINDOWS_8_1 0
#endif

#ifndef WINDOWS_PHONE_8_1
#define WINDOWS_PHONE_8_1 0
#endif
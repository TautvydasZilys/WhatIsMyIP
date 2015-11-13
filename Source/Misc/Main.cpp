#include "PrecompiledHeader.h"

#include "UI\WhatIsMyIPApp.h"
#include "Utilities\EventHandler.h"
#include "Utilities\RoInitializer.h"

using namespace	ABI::Windows::UI::Xaml;
using namespace UI;
using namespace Utilities;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	RoInitializer roInit;

	WRL::ComPtr<IApplicationStatics> applicationStatics;
	auto hr = Windows::Foundation::GetActivationFactory(WRL::HStringReference(L"Windows.UI.Xaml.Application").Get(), &applicationStatics);
	Assert(SUCCEEDED(hr));
	
	return applicationStatics->Start(
		EventHandlerFactory<IApplicationInitializationCallback>::Make(
			[applicationStatics](IApplicationInitializationCallbackParams* initializationCallbackParams) -> HRESULT
	{
		return XamlApplication::Run<WhatIsMyIPApp>(applicationStatics.Get());
	}).Get());
}
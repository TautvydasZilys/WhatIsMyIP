#include "PrecompiledHeader.h"

#include "Etw\EtwInitializer.h"
#include "UI\WhatIsMyIPApp.h"
#include "Utilities\EventHandler.h"
#include "Utilities\HString.h"
#include "Utilities\RoInitializer.h"

using namespace	ABI::Windows::UI::Xaml;
using namespace UI;
using namespace Utilities;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Etw::EtwInitializer etwInit;
	Etw::EtwSingleEvent("Lifetime", "wWinMain startup");

	RoInitializer roInit;

	WRL::ComPtr<IApplicationStatics> applicationStatics;
	auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.UI.Xaml.Application"), &applicationStatics);
	Assert(SUCCEEDED(hr));
	
	return applicationStatics->Start(
		EventHandlerFactory<IApplicationInitializationCallback>::Make(
			[applicationStatics](IApplicationInitializationCallbackParams* initializationCallbackParams) -> HRESULT
	{
		return XamlApplication::Run<WhatIsMyIPApp>(applicationStatics.Get());
	}).Get());
}
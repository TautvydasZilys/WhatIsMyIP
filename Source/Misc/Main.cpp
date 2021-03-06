#include "PrecompiledHeader.h"

#include "Etw\EtwInitializer.h"
#include "Networking\IPInformationWatcher.h"
#include "PlugNPlay\PlugNPlayObjectRegistry.h"
#include "UI\WhatIsMyIPApp.h"
#include "Utilities\EventHandler.h"
#include "Utilities\HString.h"
#include "Utilities\RoInitializer.h"
#include "Utilities\ThreadPoolRunner.h"

using namespace	ABI::Windows::UI::Xaml;
using namespace UI;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Etw::EtwInitializer etwInit;
	Etw::EtwSingleEvent("Lifetime", "wWinMain startup");

	Utilities::RoInitializer roInit;
	Utilities::ThreadPoolRunner::ScopedSingleton threadPoolRunner;
	PlugNPlay::PlugNPlayObjectRegistry::ScopedSingleton pnpRegistry;
	Networking::IPInformationWatcher::ScopedSingleton ipInfoWatcher;

	Etw::EtwSingleEvent("Lifetime", "Calling Application.Run");

	WRL::ComPtr<IApplicationStatics> applicationStatics;
	auto hr = Windows::Foundation::GetActivationFactory(Utilities::HStringReference(L"Windows.UI.Xaml.Application"), &applicationStatics);
	FastFailIfFailed(hr);

	hr = applicationStatics->Start(
		Utilities::EventHandlerFactory<IApplicationInitializationCallback>::Make(
			[applicationStatics](IApplicationInitializationCallbackParams* initializationCallbackParams) -> HRESULT
	{
		return XamlApplication::Run<WhatIsMyIPApp>(applicationStatics.Get());
	}).Get());
	FastFailIfFailed(hr);

	return 0;
}
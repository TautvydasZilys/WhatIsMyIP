#include "PrecompiledHeader.h"
#include "XamlApplication.h"

using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Xaml;
using namespace UI;

HRESULT XamlApplication::SetBaseInstance(IInspectable* nonDelegatingBase)
{
	m_NonDelegatingBase = nonDelegatingBase;

	auto hr = m_NonDelegatingBase.As(&m_BaseApplication);
	ReturnIfFailed(hr);

	hr = m_NonDelegatingBase.As(&m_BaseApplicationOverrides);
	ReturnIfFailed(hr);

	return hr;
}

HRESULT XamlApplication::get_Resources(IResourceDictionary** value)
{
	return m_BaseApplication->get_Resources(value);
}

HRESULT XamlApplication::put_Resources(IResourceDictionary* value)
{
	return m_BaseApplication->put_Resources(value);
}

HRESULT XamlApplication::get_DebugSettings(IDebugSettings** value)
{
	return m_BaseApplication->get_DebugSettings(value);
}

HRESULT XamlApplication::get_RequestedTheme(ApplicationTheme* value)
{
	return m_BaseApplication->get_RequestedTheme(value);
}

HRESULT XamlApplication::put_RequestedTheme(ApplicationTheme value)
{
	return m_BaseApplication->put_RequestedTheme(value);
}

HRESULT XamlApplication::add_UnhandledException(IUnhandledExceptionEventHandler* value, EventRegistrationToken* token)
{
	return m_BaseApplication->add_UnhandledException(value, token);
}

HRESULT XamlApplication::remove_UnhandledException(EventRegistrationToken token)
{
	return m_BaseApplication->remove_UnhandledException(token);
}

HRESULT XamlApplication::add_Suspending(ISuspendingEventHandler* value, EventRegistrationToken* token)
{
	return m_BaseApplication->add_Suspending(value, token);
}

HRESULT XamlApplication::remove_Suspending(EventRegistrationToken token)
{
	return m_BaseApplication->remove_Suspending(token);
}

HRESULT XamlApplication::add_Resuming(::IEventHandler<IInspectable*>* value, EventRegistrationToken* token)
{
	return m_BaseApplication->add_Resuming(value, token);
}

HRESULT XamlApplication::remove_Resuming(EventRegistrationToken token)
{
	return m_BaseApplication->remove_Resuming(token);
}

HRESULT XamlApplication::Exit()
{
	return m_BaseApplication->Exit();
}

HRESULT XamlApplication::OnActivated(IActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnActivated(args);
}

HRESULT XamlApplication::OnLaunched(ILaunchActivatedEventArgs* args)
{
	auto hr = m_Window->Activate();
	ReturnIfFailed(hr);

	return m_BaseApplicationOverrides->OnLaunched(args);
}

HRESULT XamlApplication::OnFileActivated(IFileActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnFileActivated(args);
}

HRESULT XamlApplication::OnSearchActivated(ISearchActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnSearchActivated(args);
}

HRESULT XamlApplication::OnShareTargetActivated(IShareTargetActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnShareTargetActivated(args);
}

HRESULT XamlApplication::OnFileOpenPickerActivated(IFileOpenPickerActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnFileOpenPickerActivated(args);
}

HRESULT XamlApplication::OnFileSavePickerActivated(IFileSavePickerActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnFileSavePickerActivated(args);
}

HRESULT XamlApplication::OnCachedFileUpdaterActivated(ICachedFileUpdaterActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnCachedFileUpdaterActivated(args);
}

HRESULT XamlApplication::OnWindowCreated(IWindowCreatedEventArgs* args)
{
	auto hr = args->get_Window(&m_Window);
	ReturnIfFailed(hr);

	return m_BaseApplicationOverrides->OnWindowCreated(args);
}

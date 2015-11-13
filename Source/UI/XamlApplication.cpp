#include "PrecompiledHeader.h"
#include "XamlApplication.h"

using namespace ABI::Windows::ApplicationModel::Activation;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Xaml;
using namespace UI;

XamlApplication::XamlApplication()
{
	m_WindowActivatedToken.value = 0;
}

HRESULT XamlApplication::SetBaseInstance(IInspectable* nonDelegatingBase)
{
	m_NonDelegatingBase = nonDelegatingBase;

	auto hr = m_NonDelegatingBase.As(&m_BaseApplication);
	ReturnIfFailed(hr);

	hr = m_NonDelegatingBase.As(&m_BaseApplicationOverrides);
	ReturnIfFailed(hr);

	return hr;
}

void XamlApplication::Cleanup()
{
	HRESULT hr;

	if (m_WindowActivatedToken.value != 0)
	{
		hr = m_Window->remove_Activated(m_WindowActivatedToken);
		Assert(SUCCEEDED(hr));
	}
}

HRESULT STDMETHODCALLTYPE XamlApplication::get_Resources(IResourceDictionary** value)
{
	return m_BaseApplication->get_Resources(value);
}

HRESULT STDMETHODCALLTYPE XamlApplication::put_Resources(IResourceDictionary* value)
{
	return m_BaseApplication->put_Resources(value);
}

HRESULT STDMETHODCALLTYPE XamlApplication::get_DebugSettings(IDebugSettings** value)
{
	return m_BaseApplication->get_DebugSettings(value);
}

HRESULT STDMETHODCALLTYPE XamlApplication::get_RequestedTheme(ApplicationTheme* value)
{
	return m_BaseApplication->get_RequestedTheme(value);
}

HRESULT STDMETHODCALLTYPE XamlApplication::put_RequestedTheme(ApplicationTheme value)
{
	return m_BaseApplication->put_RequestedTheme(value);
}

HRESULT STDMETHODCALLTYPE XamlApplication::add_UnhandledException(IUnhandledExceptionEventHandler* value, EventRegistrationToken* token)
{
	return m_BaseApplication->add_UnhandledException(value, token);
}

HRESULT STDMETHODCALLTYPE XamlApplication::remove_UnhandledException(EventRegistrationToken token)
{
	return m_BaseApplication->remove_UnhandledException(token);
}

HRESULT STDMETHODCALLTYPE XamlApplication::add_Suspending(ISuspendingEventHandler* value, EventRegistrationToken* token)
{
	return m_BaseApplication->add_Suspending(value, token);
}

HRESULT STDMETHODCALLTYPE XamlApplication::remove_Suspending(EventRegistrationToken token)
{
	return m_BaseApplication->remove_Suspending(token);
}

HRESULT STDMETHODCALLTYPE XamlApplication::add_Resuming(::IEventHandler<IInspectable*>* value, EventRegistrationToken* token)
{
	return m_BaseApplication->add_Resuming(value, token);
}

HRESULT STDMETHODCALLTYPE XamlApplication::remove_Resuming(EventRegistrationToken token)
{
	return m_BaseApplication->remove_Resuming(token);
}

HRESULT STDMETHODCALLTYPE XamlApplication::Exit()
{
	Cleanup();
	return m_BaseApplication->Exit();
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnActivated(IActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnLaunched(ILaunchActivatedEventArgs* args)
{
	auto hr = m_Window->Activate();
	ReturnIfFailed(hr);

	return m_BaseApplicationOverrides->OnLaunched(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnFileActivated(IFileActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnFileActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnSearchActivated(ISearchActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnSearchActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnShareTargetActivated(IShareTargetActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnShareTargetActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnFileOpenPickerActivated(IFileOpenPickerActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnFileOpenPickerActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnFileSavePickerActivated(IFileSavePickerActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnFileSavePickerActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnCachedFileUpdaterActivated(ICachedFileUpdaterActivatedEventArgs* args)
{
	return m_BaseApplicationOverrides->OnCachedFileUpdaterActivated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::OnWindowCreated(IWindowCreatedEventArgs* args)
{
	auto hr = args->get_Window(&m_Window);
	ReturnIfFailed(hr);

	hr = m_Window->get_Dispatcher(&m_Dispatcher);
	ReturnIfFailed(hr);

	hr = m_Window->add_Activated(this, &m_WindowActivatedToken);
	ReturnIfFailed(hr);

	return m_BaseApplicationOverrides->OnWindowCreated(args);
}

HRESULT STDMETHODCALLTYPE XamlApplication::Invoke(IInspectable* sender, IWindowActivatedEventArgs* e)
{
	return OnWindowActivated(e);
}
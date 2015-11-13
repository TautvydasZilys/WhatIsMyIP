#pragma once

namespace UI
{

class XamlApplication :
	public WRL::RuntimeClass<
	WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>,
	ABI::Windows::UI::Xaml::IApplication,
	ABI::Windows::UI::Xaml::IApplicationOverrides,
	ABI::Windows::UI::Xaml::IWindowActivatedEventHandler,
	WRL::FtmBase>
{
private:
	WRL::ComPtr<IInspectable> m_NonDelegatingBase;
	WRL::ComPtr<ABI::Windows::UI::Xaml::IApplication> m_BaseApplication;
	WRL::ComPtr<ABI::Windows::UI::Xaml::IApplicationOverrides> m_BaseApplicationOverrides;
	WRL::ComPtr<ABI::Windows::UI::Xaml::IWindow> m_Window;
	WRL::ComPtr<ABI::Windows::UI::Core::ICoreDispatcher> m_Dispatcher;

	EventRegistrationToken m_WindowActivatedToken;

	void Cleanup();

protected:
	HRESULT SetBaseInstance(IInspectable* nonDelegatingBase);
	inline WRL::ComPtr<ABI::Windows::UI::Xaml::IWindow> GetWindow() const { return m_Window; }
	inline WRL::ComPtr<ABI::Windows::UI::Core::ICoreDispatcher> GetDispatcher() const { return m_Dispatcher; }

public:
	XamlApplication();

	template <typename App = XamlApplication>
	static HRESULT Run(ABI::Windows::UI::Xaml::IApplicationStatics* applicationStatics)
	{
		static_assert(std::is_base_of<XamlApplication, App>::value, "App must be derived from XamlApplication");

		WRL::ComPtr<IApplicationFactory> applicationFactory;
		auto hr = applicationStatics->QueryInterface(__uuidof(IApplicationFactory), &applicationFactory);
		ReturnIfFailed(hr);

		auto xamlApplication = WRL::Make<App>();

		WRL::ComPtr<IInspectable> innerApplication;
		WRL::ComPtr<IApplication> application;
		hr = applicationFactory->CreateInstance(static_cast<IApplication*>(xamlApplication.Get()), &innerApplication, &application);
		ReturnIfFailed(hr);

		return xamlApplication->SetBaseInstance(innerApplication.Get());
	}

	// IApplication
	virtual HRESULT STDMETHODCALLTYPE get_Resources(ABI::Windows::UI::Xaml::IResourceDictionary** value) override;
	virtual HRESULT STDMETHODCALLTYPE put_Resources(ABI::Windows::UI::Xaml::IResourceDictionary* value) override;
	virtual HRESULT STDMETHODCALLTYPE get_DebugSettings(ABI::Windows::UI::Xaml::IDebugSettings** value) override;
	virtual HRESULT STDMETHODCALLTYPE get_RequestedTheme(ABI::Windows::UI::Xaml::ApplicationTheme* value) override;
	virtual HRESULT STDMETHODCALLTYPE put_RequestedTheme(ABI::Windows::UI::Xaml::ApplicationTheme value) override;
	virtual HRESULT STDMETHODCALLTYPE add_UnhandledException(ABI::Windows::UI::Xaml::IUnhandledExceptionEventHandler* value, EventRegistrationToken* token) override;
	virtual HRESULT STDMETHODCALLTYPE remove_UnhandledException(EventRegistrationToken token) override;
	virtual HRESULT STDMETHODCALLTYPE add_Suspending(ABI::Windows::UI::Xaml::ISuspendingEventHandler* value, EventRegistrationToken* token) override;
	virtual HRESULT STDMETHODCALLTYPE remove_Suspending(EventRegistrationToken token) override;
	virtual HRESULT STDMETHODCALLTYPE add_Resuming(ABI::Windows::Foundation::IEventHandler<IInspectable*>* value, EventRegistrationToken* token) override;
	virtual HRESULT STDMETHODCALLTYPE remove_Resuming(EventRegistrationToken token) override;
	virtual HRESULT STDMETHODCALLTYPE Exit() override;

	// IApplicationOverrides
	virtual HRESULT STDMETHODCALLTYPE OnActivated(ABI::Windows::ApplicationModel::Activation::IActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnLaunched(ABI::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnFileActivated(ABI::Windows::ApplicationModel::Activation::IFileActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnSearchActivated(ABI::Windows::ApplicationModel::Activation::ISearchActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnShareTargetActivated(ABI::Windows::ApplicationModel::Activation::IShareTargetActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnFileOpenPickerActivated(ABI::Windows::ApplicationModel::Activation::IFileOpenPickerActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnFileSavePickerActivated(ABI::Windows::ApplicationModel::Activation::IFileSavePickerActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnCachedFileUpdaterActivated(ABI::Windows::ApplicationModel::Activation::ICachedFileUpdaterActivatedEventArgs* args) override;
	virtual HRESULT STDMETHODCALLTYPE OnWindowCreated(ABI::Windows::UI::Xaml::IWindowCreatedEventArgs* args) override;

	// IWindowActivatedEventHandler
	virtual HRESULT STDMETHODCALLTYPE Invoke(IInspectable* sender, ABI::Windows::UI::Core::IWindowActivatedEventArgs* e) override;

	virtual HRESULT STDMETHODCALLTYPE OnWindowActivated(ABI::Windows::UI::Core::IWindowActivatedEventArgs* e) = 0;
};

}
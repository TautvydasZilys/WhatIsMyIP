#pragma once

#include "TemplateHelpers\ArgumentTraits.h"

namespace Utilities
{

template <typename DelegateInterface, typename CallbackType>
class EventHandlerBase :
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::ClassicCom>,
		DelegateInterface>
{
protected:
	CallbackType m_Callback;

public:
	EventHandlerBase(CallbackType&& callback) :
		m_Callback(std::forward<CallbackType>(callback))
	{
	}
};

template <typename DelegateInterface, typename CallbackType, size_t EventArgumentCount = TemplateHelpers::ArgumentTraits<decltype(&DelegateInterface::Invoke)>::ArgumentCount>
class EventHandler;

template <typename DelegateInterface, typename CallbackType>
class EventHandler<DelegateInterface, CallbackType, 0> :
	public EventHandlerBase<DelegateInterface, CallbackType>
{
public:
	EventHandler(CallbackType&& callback) :
		EventHandlerBase(std::forward<CallbackType>(callback))
	{
	}

	HRESULT STDMETHODCALLTYPE Invoke() override
	{
		return m_Callback();
	}
};

template <typename DelegateInterface, typename CallbackType>
class EventHandler<DelegateInterface, CallbackType, 1> :
	public EventHandlerBase<DelegateInterface, CallbackType>
{
	typedef typename TemplateHelpers::ArgumentTraits<decltype(&DelegateInterface::Invoke)>::Arg1 Arg1Type;
public:
	EventHandler(CallbackType&& callback) :
		EventHandlerBase(std::forward<CallbackType>(callback))
	{
	}

	HRESULT STDMETHODCALLTYPE Invoke(Arg1Type arg1) override
	{
		return m_Callback(arg1);
	}
};

template <typename DelegateInterface, typename CallbackType>
class EventHandler<DelegateInterface, CallbackType, 2> :
	public EventHandlerBase<DelegateInterface, CallbackType>
{
	typedef typename TemplateHelpers::ArgumentTraits<decltype(&DelegateInterface::Invoke)>::Arg1 Arg1Type;
	typedef typename TemplateHelpers::ArgumentTraits<decltype(&DelegateInterface::Invoke)>::Arg2 Arg2Type;
public:
	EventHandler(CallbackType&& callback) :
		EventHandlerBase(std::forward<CallbackType>(callback))
	{
	}

	HRESULT STDMETHODCALLTYPE Invoke(Arg1Type arg1, Arg2Type arg2) override
	{
		return m_Callback(arg1, arg2);
	}
};

template <typename DelegateInterface>
class EventHandlerFactory
{
public:
	template <typename CallbackType>
	static inline WRL::ComPtr<EventHandler<DelegateInterface, CallbackType>> Make(CallbackType&& callback)
	{
		return WRL::Make<EventHandler<DelegateInterface, CallbackType>>(std::forward<CallbackType>(callback));
	}
};

}

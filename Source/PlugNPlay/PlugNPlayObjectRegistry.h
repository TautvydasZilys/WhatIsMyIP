#pragma once

#include "Utilities\Comparers.h"
#include "Utilities\CriticalSection.h"
#include "Utilities\HandleHolder.h"
#include "Utilities\HString.h"
#include "Utilities\ReaderWriterLock.h"

namespace PlugNPlay
{

// PlugNPlayObjectRegistry provides an extremely fast way to query 
// a particular PnP device name from a substring of its interface device instance id
// It assumes that enumerating all PnP devices on the system is expensive, therefore
// it uses PnpObjectWatcher to get notified about device updates. It also assumes
// that these updates will not be frequent, as PnP devices will not be added and removed
// from a system very often. It optimizes towards fast querying, and in order to
// support it, it builds up the following data structure:
// 
// m_InterfaceInstanceIds - contiguous memory consisting of all PnP interface device instance ID strings
// m_InstanceIdIndices - sorted array containing indices of each ID in m_InterfaceInstanceIds
// m_DeviceNames - device names array matching m_InstanceIdIndices
//
// Conversion procedure for converting instance id substring into a device name:
//
// 1. Find the substring start index in m_InterfaceInstanceIds
// 2. Look for first not bigger index value in m_InstanceIdIndices
// 3. The index of first non-bigger index value is the index of device name in m_DeviceNames
//
class PlugNPlayObjectRegistry :
	public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IUnknown>
{
private:
	Utilities::HandleHolder m_InitialEnumerationCompletedEvent;
	WRL::ComPtr<ABI::Windows::Devices::Enumeration::Pnp::IPnpObjectStatics> m_PnpObjectStatics;
	WRL::ComPtr<ABI::Windows::Devices::Enumeration::Pnp::IPnpObjectWatcher> m_PnpObjectWatcher;

	Utilities::CriticalSection m_ObjectCriticalSection;
	std::map<Utilities::HString, WRL::ComPtr<ABI::Windows::Devices::Enumeration::Pnp::IPnpObject>, Utilities::HStringLess> m_PnpObjects;

	Utilities::ReaderWriterLock m_RegistryLock;
	std::vector<wchar_t> m_InterfaceInstanceIds;
	std::vector<size_t> m_InstanceIdIndices;
	std::vector<Utilities::HString> m_DeviceNames;

	EventRegistrationToken m_AddedEventToken;
	EventRegistrationToken m_UpdatedEventToken;
	EventRegistrationToken m_RemovedEventToken;
	EventRegistrationToken m_InitialEnumerationCompletedToken;

	enum class DirtyStatus : LONG
	{
		kNotDirty,
		kDirty,
		kRebuilding
	};

	std::atomic<DirtyStatus> m_DirtyStatus;
	bool m_IsPnpObjectWatcherRunning;
	volatile bool m_IsInitialEnumerationCompleted;

	static PlugNPlayObjectRegistry* s_Instance;

	PlugNPlayObjectRegistry();
	HRESULT Initialize();
	void Cleanup();

	void RebuildRegistry();
	void RebuildRegistryIfNeeded();

	void OnObjectAdded(ABI::Windows::Devices::Enumeration::Pnp::IPnpObject* pnpObject);
	void OnObjectUpdated(ABI::Windows::Devices::Enumeration::Pnp::IPnpObjectUpdate* pnpObjectUpdate);
	void OnObjectRemoved(ABI::Windows::Devices::Enumeration::Pnp::IPnpObjectUpdate* pnpObjectUpdate);
	void OnInitialEnumerationCompleted();
	HRESULT LookupImpl(const wchar_t* interfaceInstanceIdSubstring, HSTRING* outName);

	friend WRL::ComPtr<PlugNPlayObjectRegistry> WRL::Make<PlugNPlayObjectRegistry>();

public:
	static HRESULT Create();
	static void Destroy();

	static HRESULT Lookup(const wchar_t* interfaceInstanceIdSubstring, HSTRING* outName);
};

}
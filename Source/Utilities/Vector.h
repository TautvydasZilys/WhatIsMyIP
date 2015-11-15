#pragma once

#include "ReferenceCounter.h"
#include "TemplateHelpers\ArgumentTraits.h"

namespace Utilities
{

template <typename T>
class Vector :
	public WRL::RuntimeClass<
		WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>,
		ABI::Windows::Foundation::Collections::IVector<T>,
		ABI::Windows::Foundation::Collections::IVectorView<T>,
		ABI::Windows::Foundation::Collections::IIterable<T>>
{
private:
	typedef typename TemplateHelpers::ArgumentTraits<decltype(&ABI::Windows::Foundation::Collections::IVector<T>::SetAt)>::Arg2 ItemType;
	std::vector<ItemType> m_Items;

	static void AddItemRef(T& item)
	{
		ReferenceCounter::AddRef<T>(item);
	}

	static void ReleaseItem(T& item)
	{
		ReferenceCounter::Release<T>(item);
	}

	template <typename T>
	static inline bool Equals(const T& left, const T& right)
	{
		return left == right;
	}

	template <>
	static inline bool Equals(const HSTRING& left, const HSTRING& right)
	{
		int32_t comparison;
		auto hr = WindowsCompareStringOrdinal(left, right, &comparison);
		Assert(SUCCEEDED(hr));
		return comparison == 0;
	}

	inline void ClearInternal()
	{
		for (auto& item : m_Items)
			ReleaseItem(item);

		m_Items.clear();
	}

	class Iterator :
		public WRL::RuntimeClass<
			WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>,
			ABI::Windows::Foundation::Collections::IIterator<T>>
	{
	private:
		WRL::ComPtr<Vector<T>> m_Vector;
		uint32_t m_Index;

	public:
		Iterator(Vector<T>* vector) :
			m_Index(0),
			m_Vector(vector)
		{
		}

		virtual HRESULT STDMETHODCALLTYPE get_Current(ItemType* current) override
		{
			if (m_Index < m_Vector->m_Items.size())
			{
				*current = m_Vector->m_Items[m_Index];
				AddItemRef(*current);
				return S_OK;
			}

			return E_BOUNDS;
		}

		virtual HRESULT STDMETHODCALLTYPE get_HasCurrent(boolean* hasCurrent) override
		{
			*hasCurrent = m_Index < m_Vector->m_Items.size();
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE MoveNext(boolean* hasCurrent)
		{
			if (m_Index < m_Vector->m_Items.size())
				m_Index++;

			return get_HasCurrent(hasCurrent);
		}
	};

	friend class Iterator;

public:
	virtual ~Vector()
	{
		ClearInternal();
	}

	virtual HRESULT STDMETHODCALLTYPE GetAt(unsigned index, ItemType* item) override
	{
		if (index >= m_Items.size())
			return E_BOUNDS;

		*item = m_Items[index];
		AddItemRef(*item);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE get_Size(unsigned* size) override
	{
		*size = static_cast<uint32_t>(m_Items.size());
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetView(ABI::Windows::Foundation::Collections::IVectorView<T>** view) override
	{
		*view = this;
		AddRef();
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE IndexOf(ItemType value, unsigned* index, boolean* found) override
	{
		*found = false;

		for (uint32_t i = 0; i < m_Items.size(); i++)
		{
			if (Equals(m_Items[i], value))
			{
				*index = i;
				*found = true;
			}
		}

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetAt(unsigned index, ItemType item) override
	{
		if (index >= m_Items.size())
			return E_BOUNDS;

		ReleaseItem(m_Items[index]);
		AddItemRef(item);
		m_Items[index] = item;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE InsertAt(unsigned index, ItemType item) override
	{
		if (index > m_Items.size())
			return E_BOUNDS;

		AddItemRef(item);
		m_Items.insert(m_Items.begin() + index, item);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE RemoveAt(unsigned index) override
	{
		if (index >= m_Items.size())
			return E_BOUNDS;

		ReleaseItem(m_Items[index]);
		m_Items.erase(m_Items.begin() + index);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Append(ItemType item) override
	{
		AddItemRef(item);
		m_Items.push_back(item);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE RemoveAtEnd() override
	{
		ReleaseItem(m_Items.back());
		m_Items.pop_back();
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Clear() override
	{
		ClearInternal();
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE First(ABI::Windows::Foundation::Collections::IIterator<T>** first) override
	{
		*first = WRL::Make<Iterator>(this).Detach();
		return S_OK;
	}
};

}
#pragma once

namespace Utilities
{

class ReaderWriterLock
{
private:
	SRWLOCK m_Lock;

	inline void AcquireExclusive()
	{
		AcquireSRWLockExclusive(&m_Lock);
	}

	inline void ReleaseExclusive()
	{
		ReleaseSRWLockExclusive(&m_Lock);
	}

	inline void AcquireShared()
	{
		AcquireSRWLockShared(&m_Lock);
	}

	inline void ReleaseShared()
	{
		ReleaseSRWLockShared(&m_Lock);
	}

	friend class WriterLock;
	friend class ReaderLock;

public:
	inline ReaderWriterLock()
	{
		InitializeSRWLock(&m_Lock);
	}

	ReaderWriterLock(const ReaderWriterLock&) = delete;
	ReaderWriterLock& operator=(const ReaderWriterLock&) = delete;

	class WriterLock
	{
	private:
		ReaderWriterLock& m_Lock;

	public:
		inline WriterLock(ReaderWriterLock& lock) :
			m_Lock(lock)
		{
			m_Lock.AcquireExclusive();
		}

		inline ~WriterLock()
		{
			m_Lock.ReleaseExclusive();
		}
	};

	class ReaderLock
	{
	private:
		ReaderWriterLock& m_Lock;

	public:
		inline ReaderLock(ReaderWriterLock& lock) :
			m_Lock(lock)
		{
			m_Lock.AcquireShared();
		}

		inline ~ReaderLock()
		{
			m_Lock.ReleaseShared();
		}
	};
};

}
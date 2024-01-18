#pragma once

class CHandle
{
public:
	CHandle() : m_handle(INVALID_HANDLE_VALUE), m_owner(false) {};
	CHandle(const HANDLE handle, bool owner = true) : m_handle(handle), m_owner(owner) {};
	HANDLE GetHandle() const { return m_handle; };

	void operator=(const HANDLE handle)
	{
		m_handle = handle;
		m_owner = true;
	}

	void CopyFrom(const CHandle& source)
	{
		m_handle = source.m_handle;
		m_owner = false;
	}

	void Take(CHandle& source)
	{
		m_handle = source.m_handle;
		m_owner = true;
		source.Invalidate();
	}

	bool IsValid() const
	{
		return m_handle && m_handle != INVALID_HANDLE_VALUE;
	}

	~CHandle()
	{
		if (m_owner && m_handle != INVALID_HANDLE_VALUE && m_handle != NULL)
		{
			CloseHandle(m_handle);
			Invalidate();
		}
	}

private:
	void Invalidate()
	{
		m_handle = INVALID_HANDLE_VALUE;
		m_owner = false;
	}

	bool m_owner = false;
	HANDLE m_handle = INVALID_HANDLE_VALUE;
};
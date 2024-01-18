#pragma once
#include "handle.h"

class CShMemView;
class CShMem
{
public:
	CShMem(const CHandle& handle, size_t nSize) : m_handle(handle), m_size(nSize) {}
	CShMem(size_t nSize, DWORD flProtect)
		: m_size(nSize)
	{
		SECURITY_ATTRIBUTES saAttr = { 0 };
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		LARGE_INTEGER size = { 0 };
		size.QuadPart = nSize;

		m_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, &saAttr, flProtect, size.HighPart, size.LowPart, NULL);

		if (!m_handle.IsValid())
			exit(0);
	}

	CHandle& GetHandle() { return m_handle; };

private:
	friend CShMemView;
	CHandle m_handle = INVALID_HANDLE_VALUE;
	size_t m_size = 0;
};

class CShMemView
{
public:
	CShMemView(std::shared_ptr<CShMem> memory, DWORD allowedAccess) :
		m_memory(memory),
		m_size(memory->m_size),
		m_access(allowedAccess)
	{
		assert(memory->m_handle.GetHandle() != INVALID_HANDLE_VALUE && memory->m_handle.GetHandle() != NULL);
		m_pData = MapViewOfFile(memory->m_handle.GetHandle(), allowedAccess, 0, 0, memory->m_size);
		assert(m_pData != nullptr);
	}

	void* GetPtr() const { return m_pData; };
	size_t GetSize() const { return m_size; };

	void Write(const void* pBytes, size_t nSize) const
	{
		assert(m_access & FILE_MAP_WRITE);
		assert(m_pData != nullptr);
		assert(pBytes != nullptr);
		assert(nSize > 0);
		assert(nSize <= m_size);

		if (nSize > m_size)
			return;

		memcpy(m_pData, pBytes, nSize);
	}

	~CShMemView()
	{
		if (m_pData)
			UnmapViewOfFile(m_pData);
	}

private:
	std::shared_ptr<CShMem> m_memory;
	DWORD m_access = 0;
	void* m_pData = nullptr;
	size_t m_size = 0;
};

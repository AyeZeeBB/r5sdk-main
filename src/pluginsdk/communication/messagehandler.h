#pragma once
#include "sharedqueue.h"

class CMessage
{
	struct Meta
	{
		HANDLE txHandle = INVALID_HANDLE_VALUE;
		HANDLE rxHandle = INVALID_HANDLE_VALUE;
		HANDLE readReady = INVALID_HANDLE_VALUE;
		LARGE_INTEGER queueSize = { 0 };
	};

public:

	int CreateServer();
	int ConnectClient(HANDLE hMetaData);
	size_t Tx(const void* pBuff, size_t nSize) const;
	static void ReadThread(CMessage* thisp);
	
	void SetReadCallback(void (*Callback)(void* pData, size_t nBytes, void* pUsr), void* pUsr)
	{
		m_usrPtr = pUsr;
		m_messageReadyCallback = Callback;
	}

	~CMessage()
	{
		m_killReadThread.store(true, std::memory_order_release);

		//Wait for the read thread to die
		while (m_readThreadActive.load(std::memory_order_relaxed)) _mm_pause();

		if(m_pTxQueue->GetQueue())
			UnmapViewOfFile(m_pTxQueue->GetQueue());

		if (m_pTxQueue->GetQueue())
			UnmapViewOfFile(m_pRxQueue->GetQueue());

		CloseHandle(m_linkMetaDataHandle);
		CloseHandle(m_linkMetaData.txHandle);
		CloseHandle(m_linkMetaData.rxHandle);
		CloseHandle(m_linkMetaData.readReady);
	}

	HANDLE GetMetaDataHandle() const { return m_linkMetaDataHandle; };
private:

	std::atomic_bool m_killReadThread = false;
	std::atomic_bool m_readThreadActive = true;

	HANDLE m_linkMetaDataHandle = INVALID_HANDLE_VALUE;
	Meta m_linkMetaData;

	CSharedQueue* m_pTxQueue = nullptr;
	CSharedQueue* m_pRxQueue = nullptr;

	void (*m_messageReadyCallback)(void* pData, size_t nBytes, void* pUsr) = nullptr;
	void* m_usrPtr = nullptr;
};
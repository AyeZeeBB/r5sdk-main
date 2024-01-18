#pragma once
#include "sharedqueue.h"
#include "handle.h"

class CMessage
{
	struct Meta
	{
		CHandle m_txHandle = INVALID_HANDLE_VALUE;
		CHandle m_rxHandle = INVALID_HANDLE_VALUE;
		CHandle m_readReadyEventHandle = INVALID_HANDLE_VALUE;
		size_t m_queueBufferSize = 0;
	};

public:
	int CreateServer();
	int ConnectClient(const CHandle& hMetaData);
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

		if (m_pTxQueue)
			delete m_pTxQueue;

		if (m_pRxQueue)
			delete m_pRxQueue;
	}

	HANDLE GetMetaDataHandle() const { return m_linkMetaDataHandle.GetHandle(); };
private:

	std::atomic_bool m_killReadThread = false;
	std::atomic_bool m_readThreadActive = true;

	CHandle m_linkMetaDataHandle = INVALID_HANDLE_VALUE;
	Meta m_linkMetaData;

	CSharedQueue* m_pTxQueue = nullptr;
	CSharedQueue* m_pRxQueue = nullptr;

	void (*m_messageReadyCallback)(void* pData, size_t nBytes, void* pUsr) = nullptr;
	void* m_usrPtr = nullptr;
};
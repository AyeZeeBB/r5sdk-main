#include "messagehandler.h"
#include <thread>

int CMessage::CreateServer()
{
	SECURITY_ATTRIBUTES saAttr = { 0 };
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	m_linkMetaData.queueSize.QuadPart = CSharedQueue::NewQueueSize(16);

	//Create the tx buffer
	m_pTxQueue = new CSharedQueue();

	const HANDLE txFileMap = CreateFileMappingA(INVALID_HANDLE_VALUE, &saAttr, PAGE_READWRITE, m_linkMetaData.queueSize.HighPart, m_linkMetaData.queueSize.LowPart, NULL);

	if (!txFileMap)
		return 1;

	Queue* const txBuffView = static_cast<Queue* const>(MapViewOfFile(txFileMap, FILE_MAP_ALL_ACCESS, 0, 0, m_linkMetaData.queueSize.QuadPart));

	if (!txBuffView)
	{
		CloseHandle(txFileMap);
		return 1;
	}

	m_pTxQueue->NewQueue(txBuffView, 16);
	m_linkMetaData.txHandle = txFileMap;

	//Create the rx buffer
	m_pRxQueue = new CSharedQueue();

	const HANDLE rxFileMap = CreateFileMappingA(INVALID_HANDLE_VALUE, &saAttr, PAGE_READWRITE, m_linkMetaData.queueSize.HighPart, m_linkMetaData.queueSize.LowPart, NULL);

	if (!rxFileMap)
		return 1;

	Queue* const rxBuffView = static_cast<Queue* const>(MapViewOfFile(rxFileMap, FILE_MAP_ALL_ACCESS, 0, 0, m_linkMetaData.queueSize.QuadPart));

	if (!rxBuffView)
	{
		CloseHandle(rxFileMap);
		return 1;
	}

	m_pRxQueue->NewQueue(rxBuffView, 16);
	m_linkMetaData.rxHandle = rxFileMap;
	m_linkMetaData.readReady = CreateEventA(&saAttr, FALSE, FALSE, NULL);

	//Create the misc data section
	Meta metaData = m_linkMetaData;

	//Swap rx and tx around for client
	metaData.rxHandle = m_linkMetaData.txHandle;
	metaData.txHandle = m_linkMetaData.rxHandle;

	LARGE_INTEGER metaBuffSize;
	metaBuffSize.QuadPart = sizeof(Meta);

	const HANDLE hMetaDataMap = CreateFileMappingA(INVALID_HANDLE_VALUE, &saAttr, PAGE_READWRITE, metaBuffSize.HighPart, metaBuffSize.LowPart, NULL);

	if (!hMetaDataMap)
	{
		CloseHandle(m_linkMetaData.rxHandle);
		CloseHandle(m_linkMetaData.txHandle);
		return 1;
	}

	Meta* const pBuff = static_cast<Meta* const>(MapViewOfFile(hMetaDataMap, FILE_MAP_ALL_ACCESS, 0, 0, metaBuffSize.QuadPart));

	if (!pBuff)
	{
		CloseHandle(hMetaDataMap);
		CloseHandle(m_linkMetaData.rxHandle);
		CloseHandle(m_linkMetaData.txHandle);
		return 1;
	}

	m_linkMetaDataHandle = hMetaDataMap;
	memcpy(pBuff, &metaData, sizeof(Meta));

	UnmapViewOfFile(pBuff);

	std::thread(ReadThread, this).detach();

	return 0;
}

int CMessage::ConnectClient(HANDLE hMetaData)
{
	const Meta* const pMetaData = static_cast<const Meta* const>(MapViewOfFile(hMetaData, FILE_MAP_ALL_ACCESS, 0, 0, 0));

	if (!pMetaData)
		return 1;

	memcpy(&m_linkMetaData, pMetaData, sizeof(Meta));

	UnmapViewOfFile(pMetaData);

	Queue* const pTxQueue = static_cast<Queue* const>(MapViewOfFile(m_linkMetaData.txHandle, FILE_MAP_ALL_ACCESS, 0, 0, m_linkMetaData.queueSize.QuadPart));
	m_pTxQueue = new CSharedQueue(pTxQueue);

	if (!m_pTxQueue)
	{
		return 1;
	}

	Queue* const pRxQueue = static_cast<Queue* const>(MapViewOfFile(m_linkMetaData.rxHandle, FILE_MAP_ALL_ACCESS, 0, 0, m_linkMetaData.queueSize.QuadPart));
	m_pRxQueue = new CSharedQueue(pRxQueue);

	if (!m_pRxQueue)
	{
		UnmapViewOfFile(m_pRxQueue);
		return 1;
	}

	std::thread(ReadThread, this).detach();
	return 0;
}

size_t CMessage::Tx(const void* pBuff, size_t nSize) const
{
	if (nSize > m_pTxQueue->GetNodeBuffSize())
		return 0;

	m_pTxQueue->PushBack(pBuff, nSize);
	SetEvent(m_linkMetaData.readReady);

	return nSize;
}

void CMessage::ReadThread(CMessage* thisp)
{
	assert(thisp != nullptr);

	for (;;)
	{
		WaitForSingleObject(thisp->m_linkMetaData.readReady, 1 * 1000);

		assert(thisp != nullptr);

		if (thisp->m_killReadThread.load(std::memory_order_relaxed))
			break;

		while (!thisp->m_pRxQueue->IsEmpty())
		{
			void* pData;
			size_t nSize;

			if (thisp->m_pRxQueue->PopFront(pData, nSize))
				break;

			thisp->m_messageReadyCallback(pData, nSize, thisp->m_usrPtr);
		}
	}

	thisp->m_readThreadActive.store(false, std::memory_order_relaxed);
}
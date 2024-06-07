#include "messagehandler.h"
#include "shmem.h"
#include <thread>

#define QUEUE_SIZE 32
#define QUEUE_NODE_CAPACITY 4096

//-----------------------------------------------------------------------------
// Purpose: Creates and inits the message server.
//-----------------------------------------------------------------------------
int CMessage::CreateServer()
{
	SECURITY_ATTRIBUTES saAttr = { 0 };
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	m_linkMetaData.m_queueBufferSize = CSharedQueue::NewQueueSize(QUEUE_SIZE, QUEUE_NODE_CAPACITY);

	//Create the tx buffer
	m_pTxQueue = new CSharedQueue();
	std::shared_ptr<CShMem> txShMem = std::make_shared<CShMem>(m_linkMetaData.m_queueBufferSize, PAGE_READWRITE);
	std::unique_ptr<CShMemView> txShMemView = std::make_unique<CShMemView>(txShMem, FILE_MAP_ALL_ACCESS);
	m_pTxQueue->NewQueue(std::move(txShMemView), QUEUE_SIZE, QUEUE_NODE_CAPACITY);
	m_linkMetaData.m_txHandle.CopyFrom(txShMem->GetHandle());

	//Create the rx buffer
	m_pRxQueue = new CSharedQueue();
	std::shared_ptr<CShMem> rxShMem = std::make_shared<CShMem>(m_linkMetaData.m_queueBufferSize, PAGE_READWRITE);
	std::unique_ptr<CShMemView> rxShMemView = std::make_unique<CShMemView>(rxShMem, FILE_MAP_ALL_ACCESS);
	m_pRxQueue->NewQueue(std::move(rxShMemView), QUEUE_SIZE, QUEUE_NODE_CAPACITY);
	m_linkMetaData.m_rxHandle.CopyFrom(rxShMem->GetHandle());

	m_linkMetaData.m_readReadyEventHandle = CreateEventA(&saAttr, FALSE, FALSE, NULL);

	//Create the misc data section
	Meta metaData = m_linkMetaData;

	//Swap rx and tx around for client
	metaData.m_rxHandle.CopyFrom(m_linkMetaData.m_txHandle);
	metaData.m_txHandle.CopyFrom(m_linkMetaData.m_rxHandle);
	metaData.m_readReadyEventHandle.CopyFrom(m_linkMetaData.m_readReadyEventHandle);

	std::shared_ptr<CShMem> metaDataSharedMem = std::make_shared<CShMem>(sizeof(Meta), PAGE_READWRITE);
	std::unique_ptr<CShMemView> metaDataSharedMemView = std::make_unique<CShMemView>(metaDataSharedMem, FILE_MAP_WRITE);

	m_linkMetaDataHandle.Take(metaDataSharedMem->GetHandle());
	metaDataSharedMemView->Write(&metaData, sizeof(Meta));

	std::thread(ReadThread, this).detach();

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Pushes data into the send queue and triggers the read ready event.
// Input  : *pData - A pointer to the data to be sent
//			nSize - The amount of data to be sent
//-----------------------------------------------------------------------------
int CMessage::ConnectClient(const CHandle& hMetaData)
{	
	std::shared_ptr<CShMem> metaDataFile = std::make_shared<CShMem>(hMetaData, sizeof(Meta));
	std::unique_ptr<CShMemView> metaDataFileView = std::make_unique<CShMemView>(metaDataFile, FILE_MAP_READ);
	memcpy(&m_linkMetaData, metaDataFileView->GetPtr(), sizeof(Meta));

	std::shared_ptr<CShMem> txDataFile = std::make_shared<CShMem>(m_linkMetaData.m_txHandle, m_linkMetaData.m_queueBufferSize);
	m_pTxQueue = new CSharedQueue(std::make_unique<CShMemView>(txDataFile, FILE_MAP_ALL_ACCESS));

	std::shared_ptr<CShMem> rxDataFile = std::make_shared<CShMem>(m_linkMetaData.m_rxHandle, m_linkMetaData.m_queueBufferSize);
	m_pRxQueue = new CSharedQueue(std::make_unique<CShMemView>(rxDataFile, FILE_MAP_ALL_ACCESS));

	std::thread(ReadThread, this).detach();
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Pushes data into the send queue and triggers the read ready event.
// Input  : *pData - A pointer to the data to be sent
//			nSize - The amount of data to be sent
//-----------------------------------------------------------------------------
size_t CMessage::Tx(const void* pBuff, size_t nSize) const
{
	if (nSize > m_pTxQueue->GetNodeBuffSize())
		return 0;

	m_pTxQueue->PushBack(pBuff, nSize);
	SetEvent(m_linkMetaData.m_readReadyEventHandle.GetHandle());

	return nSize;
}

//---------------------------------------------------------------------------------
// Purpose: Checks the message queue for data and calls the callback with the data.
// Input  : *thisp - A pointer to the CMessage instance to read from
//---------------------------------------------------------------------------------
void CMessage::ReadThread(CMessage* thisp)
{
	assert(thisp != nullptr);

	for (;;)
	{
		WaitForSingleObject(thisp->m_linkMetaData.m_readReadyEventHandle.GetHandle(), 1 * 1000);

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
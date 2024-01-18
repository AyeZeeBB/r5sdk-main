#include "sharedqueue.h"

size_t CSharedQueue::NewQueueSize(size_t nSize, size_t nElementCapacity)
{
	assert(nSize > 0);
	size_t nodeArraySize = NodeArray::NeededSize(nSize, nElementCapacity);
	size_t queueSize = sizeof(Queue) - sizeof(NodeArray);
	return nodeArraySize + queueSize;
}

void CSharedQueue::NewQueue(std::unique_ptr<CShMemView> shmemview, size_t nSize, size_t nNodeCapacity)
{
	assert(nSize > 0);
	assert(nNodeCapacity > 0);

	assert(shmemview->GetPtr() != nullptr);
	assert(shmemview->GetSize() >= NewQueueSize(nSize, nNodeCapacity));

	Queue* const pQueue = static_cast<Queue* const>(shmemview->GetPtr());
	
	pQueue->m_nodes.m_nodeCapacity = nNodeCapacity;
	pQueue->m_nodes.m_nNodes = nSize;
	pQueue->m_alive = true;
	pQueue->m_head = 0;
	pQueue->m_tail = 0;

	for (int i = 0; i < pQueue->m_nodes.m_nNodes; i++)
	{
		pQueue->m_nodes[i]->m_dataSize = 0;
	}

	m_shMemView = std::move(shmemview);
	m_pQueue = pQueue;
}

void CSharedQueue::NewQueue(Queue* pQueue, size_t nSize, size_t nNodeCapacity)
{
	assert(pQueue != nullptr);
	assert(nSize > 0);
	assert(nNodeCapacity > 0);

	pQueue->m_nodes.m_nNodes = nSize;
	pQueue->m_nodes.m_nodeCapacity = nNodeCapacity;
	pQueue->m_alive = true;
	pQueue->m_head = 0;
	pQueue->m_tail = 0;

	for (int i = 0; i < pQueue->m_nodes.m_nNodes; i++)
	{
		pQueue->m_nodes[i]->m_dataSize = 0;
	}

	m_pQueue = pQueue;
}

void CSharedQueue::PushBack(const void* pData, size_t nSize)
{
	assert(nSize < GetNodeBuffSize());
	assert(pData != nullptr);

	if (!nSize)
		return;

	if (!m_pQueue->m_alive.load(std::memory_order_acquire))
		return;

	const uint16_t head = m_pQueue->m_head.load(std::memory_order_acquire);
	const uint16_t nextHead = head < m_pQueue->m_nodes.m_nNodes ? head + 1 : 0;

	m_pQueue->m_nodes[head]->m_dataSize = static_cast<uint16_t>(nSize);
	memcpy(m_pQueue->m_nodes[head]->buff, pData, nSize);

	m_pQueue->m_head.store(nextHead, std::memory_order_release);
}

int CSharedQueue::PopFront(void*& pData, size_t& size)
{
	if (!m_pQueue->m_alive.load(std::memory_order_acquire))
		return 1;

	const uint16_t tail = m_pQueue->m_tail.load(std::memory_order_relaxed);

	if (m_pQueue->m_head.load(std::memory_order_acquire) == tail)
		return 1;

	pData = malloc(m_pQueue->m_nodes[tail]->m_dataSize);

	if (!pData)
		return 1;

	memcpy(pData, m_pQueue->m_nodes[tail]->buff, m_pQueue->m_nodes[tail]->m_dataSize);
	size = m_pQueue->m_nodes[tail]->m_dataSize;

	const uint16_t nextTail = tail > m_pQueue->m_nodes.m_nNodes ? 0 : tail + 1;

	m_pQueue->m_tail.store(nextTail, std::memory_order_release);

	return 0;
}
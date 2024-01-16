#include "sharedqueue.h"

size_t CSharedQueue::NewQueueSize(size_t nSize)
{
	assert(nSize > 0);
	assert(nSize < UINT16_MAX);
	return sizeof(Queue) + (nSize * sizeof(Node));
}

void CSharedQueue::NewQueue(Queue* pQueue, size_t nSize)
{
	assert(pQueue != nullptr);
	assert(nSize > 0);
	assert(nSize < UINT16_MAX);

	pQueue->m_capacity = static_cast<uint16_t>(nSize);
	pQueue->m_size = sizeof(Queue) + (nSize * sizeof(Node));
	pQueue->m_alive = true;
	pQueue->m_head = 0;
	pQueue->m_tail = 0;

	for (int i = 0; i < pQueue->m_capacity; i++)
	{
		pQueue->m_nodes[i].m_dataSize = 0;
		pQueue->m_nodes[i].m_capacity = 256;
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
	const uint16_t nextHead = head < m_pQueue->m_capacity ? head + 1 : 0;

	m_pQueue->m_nodes[head].m_dataSize = static_cast<uint16_t>(nSize);
	memcpy(m_pQueue->m_nodes[head].buff, pData, nSize);

	m_pQueue->m_head.store(nextHead, std::memory_order_release);
}

int CSharedQueue::PopFront(void*& pData, size_t& size)
{
	if (!m_pQueue->m_alive.load(std::memory_order_acquire))
		return 1;

	const uint16_t tail = m_pQueue->m_tail.load(std::memory_order_relaxed);

	if (m_pQueue->m_head.load(std::memory_order_acquire) == tail)
		return 1;

	pData = malloc(m_pQueue->m_nodes[tail].m_dataSize);

	if (!pData)
		return 1;

	memcpy(pData, m_pQueue->m_nodes[tail].buff, m_pQueue->m_nodes[tail].m_dataSize);
	size = m_pQueue->m_nodes[tail].m_dataSize;

	const uint16_t nextTail = tail > m_pQueue->m_capacity ? 0 : tail + 1;

	m_pQueue->m_tail.store(nextTail, std::memory_order_release);

	return 0;
}
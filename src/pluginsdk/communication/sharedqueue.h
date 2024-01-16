#pragma once

struct Node
{
	uint16_t m_dataSize;
	char buff[256];
	uint16_t m_capacity;
};

#pragma warning(push)
#pragma warning(disable:4324)
struct Queue
{
	std::atomic<bool> m_alive = { true };
	uint16_t m_capacity = 0;
	size_t m_size = 0;

	alignas(std::hardware_constructive_interference_size) std::atomic<uint16_t> m_head = { 0 };
	alignas(std::hardware_constructive_interference_size) std::atomic<uint16_t> m_tail = { 0 };

	Node m_nodes[1];
};
#pragma warning(pop)

class CSharedQueue
{
public:

	CSharedQueue() = default;

	CSharedQueue(Queue* const pQueue)
		: m_pQueue(pQueue)
	{
		assert(pQueue != nullptr);
	}

	void NewQueue(Queue* pQueue, size_t nSize);
	static size_t NewQueueSize(size_t nSize);

	void PushBack(const void* pData, size_t nSize);
	int  PopFront(void*& pData, size_t& size);

	bool IsEmpty() const { return m_pQueue->m_head.load(std::memory_order_acquire) == m_pQueue->m_tail.load(std::memory_order_acquire); };
	const Queue* GetQueue() const { return m_pQueue; };
	size_t GetQueueSize() const { return m_pQueue->m_size; };
	uint16_t GetNodeBuffSize() const { return  m_pQueue->m_nodes->m_capacity; };

	~CSharedQueue()
	{
		m_pQueue->m_alive.store(false, std::memory_order_relaxed);
	}
private:
	Queue* m_pQueue;
};
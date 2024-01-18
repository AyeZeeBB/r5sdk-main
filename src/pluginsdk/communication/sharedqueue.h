#pragma once
#include "shmem.h"

#pragma pack(push, 1)
struct Node
{
	uint16_t m_dataSize;
	char buff[1];
};
#pragma pack(pop)

struct NodeArray
{
	Node* operator[](size_t nIndex)
	{
		assert(nIndex <= m_nNodes);
		return reinterpret_cast<Node*>((char*)m_nodes + ((sizeof(Node) - sizeof(Node::buff) + m_nodeCapacity) * nIndex));
	}

	static size_t NeededSize(size_t nNodes, size_t nNodeCapacity)
	{
		size_t arrayMetaDataSize = sizeof(NodeArray) - sizeof(Node);
		size_t nodesSize = ((sizeof(Node) - sizeof(Node::buff)) + nNodeCapacity) * nNodes;
		return arrayMetaDataSize + nodesSize;
	}

	size_t m_nodeCapacity = 0;
	size_t m_nNodes = 0;
private:
	Node* m_nodes[1];
};

#pragma warning(push)
#pragma warning(disable:4324)
struct Queue
{
	std::atomic<bool> m_alive = { true };

	alignas(std::hardware_constructive_interference_size) std::atomic<uint16_t> m_head = { 0 };
	alignas(std::hardware_constructive_interference_size) std::atomic<uint16_t> m_tail = { 0 };

	NodeArray m_nodes;
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

	CSharedQueue(std::unique_ptr<CShMemView> shmemview) : m_shMemView(std::move(shmemview))
	{
		assert(m_shMemView->GetPtr() != nullptr);
		m_pQueue = static_cast<Queue* const>(m_shMemView->GetPtr());
	}

	void NewQueue(Queue* pQueue, size_t nSize, size_t nNodeCapacity);
	void NewQueue(std::unique_ptr<CShMemView> shmemview, size_t nSize, size_t nNodeCapacity);

	static size_t NewQueueSize(size_t nSize, size_t nElementCapacity);

	void PushBack(const void* pData, size_t nSize);
	int  PopFront(void*& pData, size_t& size);

	bool IsEmpty() const { return m_pQueue->m_head.load(std::memory_order_acquire) == m_pQueue->m_tail.load(std::memory_order_acquire); };
	const Queue* GetQueue() const { return m_pQueue; };
	size_t GetQueueSize() const { return m_pQueue->m_nodes.m_nNodes; };
	size_t GetNodeBuffSize() const { return  m_pQueue->m_nodes.m_nodeCapacity; };

	~CSharedQueue()
	{
		m_pQueue->m_alive.store(false, std::memory_order_relaxed);
	}
private:
	std::unique_ptr<CShMemView> m_shMemView = nullptr;
	Queue* m_pQueue = nullptr;
};
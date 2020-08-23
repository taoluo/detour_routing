#include "robin-index.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "ns3/packet.h"

namespace ns3
{
	Index::Index(uint32_t n)
	{
		m_n = n;
	}

	Index::~Index()
	{}

	// ----------------------------------------------------------------------------------------------------------------------------------------

	RandomIndex::RandomIndex(uint32_t base, uint32_t n) : Index(n)
	{
		m_base = base;
	}

	RandomIndex::~RandomIndex()
	{}

	uint32_t RandomIndex::GetNext(Ptr<Packet>)
	{
		return rand() % m_n + m_base;
	}

	// ----------------------------------------------------------------------------------------------------------------------------------------

	InitRandomIndex::InitRandomIndex(uint32_t base, uint32_t n) : Index(n)
	{
		m_value = base + rand() % n;
	}

	InitRandomIndex::~InitRandomIndex()
	{}

	uint32_t InitRandomIndex::GetNext(Ptr<Packet>)
	{
		return m_value;
	}
	
	// ----------------------------------------------------------------------------------------------------------------------------------------

	RoundRobinIndex::RoundRobinIndex(uint32_t n) : Index(n)
	{
		m_index = rand() % n;
	}

	RoundRobinIndex::~RoundRobinIndex()
	{}

	uint32_t RoundRobinIndex::GetNext(Ptr<Packet>)
	{
		uint32_t index = m_index;
		m_index = (m_index + 1) % m_n;
		return index;
	}

	// ---------------------------------------------------------------------------------------------------

	RandomRobinIndex::RandomRobinIndex(uint32_t base, uint32_t n) : Index(n)
	{
		m_index = 0;
		m_indexes = new uint32_t[n];

		for (uint32_t i = 0; i < n; i++)
			m_indexes[i] = base + i;

		for (uint32_t i = 0; i < m_n - 1; i++)
			std::swap(m_indexes[i], m_indexes[i + rand() % (m_n - i)]);
	}

	RandomRobinIndex::~RandomRobinIndex()
	{
		delete m_indexes;
	}

	uint32_t RandomRobinIndex::GetNext(Ptr<Packet>)
	{
		m_index = (m_index + 1) % m_n;
		return m_indexes[m_index];
	}

	// ---------------------------------------------------------------------------------------------------
	BinaryReversalRobinIndex::BinaryReversalRobinIndex(uint32_t n) : Index(n)
	{
		m_index = 0;

		for (uint32_t i = 0; i < sizeof(uint32_t) * 8; i++)
			if ((n - 1) & (1 << i))
				m_bitCount = i + 1;
	}

	BinaryReversalRobinIndex::~BinaryReversalRobinIndex()
	{}

	uint32_t BinaryReversalRobinIndex::getReversed(uint32_t value) const
	{
		uint32_t rvalue = 0;
		for (int i = 0; i < (int)m_bitCount; i++)
		{
			int bitToMove = m_bitCount - 1 - 2 * i;

			if (bitToMove > 0)
				rvalue |= (value & (1 << i)) << bitToMove;
			else if (bitToMove == 0)
				rvalue |= (value & (1 << i));
			else
				rvalue |= (value & (1 << i)) >> (-bitToMove);
		}
		return rvalue;
	}

	uint32_t BinaryReversalRobinIndex::GetNext(Ptr<Packet>)
	{
		uint32_t res = m_index;
		uint32_t rindex = getReversed(m_index);
		rindex = (rindex + 1) % m_n;
		m_index = getReversed(rindex);
		return res;
	}

	// ---------------------------------------------------------------------------------------------------
	DigitalReversalRobinIndex::DigitalReversalRobinIndex(uint32_t n, uint32_t base, bool randomStart) : Index(n)
	{
		if (randomStart == false)
			m_index = 0;
		else
			m_index = random() % n;

		m_base = base;

		m_digitCount = 2;
	}

    DigitalReversalRobinIndex::~DigitalReversalRobinIndex()
    {}

	uint32_t DigitalReversalRobinIndex::GetNext(Ptr<Packet>)
	{
		unsigned char digits[64];

		for (uint32_t i = 0; i < m_digitCount; i++)
		{
			digits[m_digitCount - 1 - i] = m_index % m_base;
			m_index /= m_base;
		}

		digits[0]++;

		for (uint32_t i = 0; i < m_digitCount; i++)
		{
			if (digits[i] >= m_base)
			{
				digits[i] = 0;
				digits[i + 1]++;
			}
			else
				break;
		}

		m_index = 0;

		for (uint32_t i = 0; i < m_digitCount; i++)
			m_index = m_index * m_base + digits[i];

		return m_index;
	}

	// ---------------------------------------------------------------------------------------------------
	TypeId PathTag::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::PathTag").SetParent<Tag>().AddConstructor<PathTag>();
		return tid;
	}

	TypeId PathTag::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	PathTagIndex::PathTagIndex(uint32_t n) : Index(n)
	{}

	PathTagIndex::~PathTagIndex()
	{}

	uint32_t PathTagIndex::GetNext(Ptr<Packet> packet)
	{
		PathTag pathTag;        
        
		if (packet->PeekPacketTag(pathTag))
			return pathTag.GetPath();
		else
			return m_index_set[rand() % m_n];
	}
}

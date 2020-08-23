#include "ns3/vl2-routing-engine.h"
#include "ns3/robin-index.h"
#include "ns3/packet.h"
#include <stdio.h>

namespace ns3
{
	VL2RoutingEngine::VL2RoutingEngine(uint32_t ni, uint32_t na)
	{
		m_ni = ni;
		m_na = na;
	}

	VL2RoutingEngine::~VL2RoutingEngine()
	{}

	// ===================================================================================================================================

	VL2RandomRoutingEngine::VL2RandomRoutingEngine(uint32_t ni, uint32_t na) : VL2RoutingEngine(ni, na)
	{}

	VL2RandomRoutingEngine::~VL2RandomRoutingEngine()
	{}

	void VL2RandomRoutingEngine::FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header)
	{
		header.ports[0] = 0;
		header.ports[1] = 20 + rand() % 2;
		header.ports[2] = m_na / 2 + rand() % (m_na / 2);
		header.ports[3] = (dest / (10 * m_na)) * 2 + rand() % 2;
		header.ports[4] = (dest % (10 * m_na)) / 20;
		header.ports[5] = dest % 20;
	}

	// ===================================================================================================================================
	VL2RoundRobinRoutingEngine::VL2RoundRobinRoutingEngine(uint32_t ni, uint32_t na) : VL2RoutingEngine(ni, na)
	{
		index1 = rand() % 2;
		index2 = rand() % (na / 2);
		index3 = rand() % 2;
	}

	VL2RoundRobinRoutingEngine::~VL2RoundRobinRoutingEngine()
	{}

	void VL2RoundRobinRoutingEngine::FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header)
	{
		header.ports[0] = 0;
		header.ports[1] = 20 + index1;
		header.ports[2] = m_na / 2 + index2;
		header.ports[3] = (dest / (10 * m_na)) * 2 + index3;
		header.ports[4] = (dest % (10 * m_na)) / 20;
		header.ports[5] = dest % 20;

		index1 = (index1 + 1) % 2;
		index2 = (index2 + 1) % (m_na / 2);
		index3 = (index3 + 1) % 2;
	}

	// ===================================================================================================================================
	VL2SequentialRoundRobinRoutingEngine::VL2SequentialRoundRobinRoutingEngine(uint32_t ni, uint32_t na) : VL2RoutingEngine(ni, na)
	{
		index1 = rand() % 2;
		index2 = rand() % (na / 2);
		index3 = rand() % 2;
	}

	VL2SequentialRoundRobinRoutingEngine::~VL2SequentialRoundRobinRoutingEngine()
	{}

	void VL2SequentialRoundRobinRoutingEngine::FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header)
	{
		header.ports[0] = 0;
		header.ports[1] = 20 + index1;
		header.ports[2] = m_na / 2 + index2;
		header.ports[3] = (dest / (10 * m_na)) * 2 + index3;
		header.ports[4] = (dest % (10 * m_na)) / 20;
		header.ports[5] = dest % 20;

		uint32_t carry = (index1 + 1) / 2;
		index1 = (index1 + 1) % 2;
		index3 = (index3 + 1) % 2;

		if (carry == 0)
			return;

		index2 = (index2 + 1) % (m_na / 2);
	}

    // ===================================================================================================================================

    VL2VirtualRandomRoutingEngine::VL2VirtualRandomRoutingEngine(uint32_t ni, uint32_t na) : VL2RoutingEngine(ni, na)
    {}

    VL2VirtualRandomRoutingEngine::~VL2VirtualRandomRoutingEngine()
    {}

    void VL2VirtualRandomRoutingEngine::FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header)
    {
        uint32_t r = rand();

        header.ports[0] = 0;
		header.ports[1] = 20 + r % 2;
		header.ports[2] = m_na / 2 + rand() % (m_na / 2);
        header.ports[3] = (dest / (10 * m_na)) * 2 + r % 2;
		header.ports[4] = (dest % (10 * m_na)) / 20;
		header.ports[5] = dest % 20;
    }
    
	// ===================================================================================================================================

	VL2PathTagRoutingEngine::VL2PathTagRoutingEngine(uint32_t ni, uint32_t na) : VL2RoutingEngine(ni, na)
	{}

	VL2PathTagRoutingEngine::~VL2PathTagRoutingEngine()
	{}

	void VL2PathTagRoutingEngine::FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header)
	{
		uint32_t index1 = rand() % 2;
		uint32_t index2 = rand() % (m_na / 2);
		uint32_t index3 = rand() % 2;

		PathTag pathTag;
		if (packet->PeekPacketTag(pathTag))
		{
			index1 = pathTag.GetPath() % 2;
			index2 = (pathTag.GetPath() % m_na) / 2;
			index3 = index1;
		}

		header.ports[0] = 0;
		header.ports[1] = 20 + index1;
		header.ports[2] = m_na / 2 + index2;
		header.ports[3] = (dest / (10 * m_na)) * 2 + index3;
		header.ports[4] = (dest % (10 * m_na)) / 20;
		header.ports[5] = dest % 20;
	}
}

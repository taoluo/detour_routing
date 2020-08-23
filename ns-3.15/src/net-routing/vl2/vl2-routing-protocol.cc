#include "ns3/vl2-routing-protocol.h"
#include "ns3/vl2-routing-engine.h"
#include "ns3/vl2-header.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/ipv4-interface.h"
#include <stdio.h>

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (VL2RoutingProtocol);

	TypeId VL2RoutingProtocol::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::VL2RoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<VL2RoutingProtocol> ()
			.AddAttribute("NI", "VL2 NI.",
						  UintegerValue(4), MakeUintegerAccessor(&VL2RoutingProtocol::m_ni), MakeUintegerChecker<uint32_t>())
			.AddAttribute("NA", "VL2 NA.",
						  UintegerValue(6), MakeUintegerAccessor(&VL2RoutingProtocol::m_na), MakeUintegerChecker<uint32_t>())
			.AddAttribute("Alg", "Routing Algorithm.",
						  UintegerValue(VL2_ROUTING_ALG_ROUND_ROBIN), MakeUintegerAccessor(&VL2RoutingProtocol::m_alg), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	VL2RoutingProtocol::VL2RoutingProtocol()
	{}

	VL2RoutingProtocol::~VL2RoutingProtocol()
	{}

	Ipv4InterfaceAddress VL2RoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void VL2RoutingProtocol::Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		uint32_t srcId = source.Get();
		uint32_t destId = destination.Get();

		if (srcId != GetNode()->GetId())
			printf("%d != %d\n", srcId, GetNode()->GetId());
		
		NS_ASSERT(srcId == GetNode()->GetId());

		VL2Header header;
		header.srcId = srcId;
		header.destId = destId;
		header.protocol = protocol;
		header.ltime = 0;
		FindAndCreateEngine(destId)->FillRoutingPath(packet, srcId, destId, header);

		packet->AddHeader(header);

		SendPacket(GetNode()->GetDevice(header.ports[header.ltime]), packet);
	}

	void VL2RoutingProtocol::Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();
		
		VL2Header header;
		packet->RemoveHeader(header);

		if (header.destId != GetNode()->GetId())
		{
			header.ltime++;
			
			packet->AddHeader(header);

			SendPacket(GetNode()->GetDevice(header.ports[header.ltime]), packet);
		}
		else
		{
			Ptr<IpL4Protocol> protocol = GetProtocol (header.protocol);

			Ipv4Header ipv4Header;
			ipv4Header.SetSource(Ipv4Address(header.srcId));
			ipv4Header.SetDestination(Ipv4Address(header.destId));

			Ptr<Ipv4Interface> ipv4Interface = Create<Ipv4Interface>();
			protocol->Receive(packet, ipv4Header, ipv4Interface);
		}
	}

	VL2RoutingEngine* VL2RoutingProtocol::CreateEngine()
	{
		if (m_alg == VL2_ROUTING_ALG_RANDOM)
			return new VL2RandomRoutingEngine(m_ni, m_na);
		else if (m_alg == VL2_ROUTING_ALG_ROUND_ROBIN)
			return new VL2RoundRobinRoutingEngine(m_ni, m_na);
		else if (m_alg == VL2_ROUTING_ALG_SEQUENTIAL_ROUND_ROBIN)
			return new VL2SequentialRoundRobinRoutingEngine(m_ni, m_na);
		else if (m_alg == VL2_ROUTING_ALG_PATH_TAG)
			return new VL2PathTagRoutingEngine(m_ni, m_na);
        else if (m_alg == VL2_ROUTING_ALG_VIRTUAL_RANDOM)
            return new VL2VirtualRandomRoutingEngine(m_ni, m_na);

		NS_ASSERT(false);
		return NULL;			
	}

	Ptr<VL2RoutingEngine> VL2RoutingProtocol::FindAndCreateEngine(uint32_t dest)
	{
		RoutingEngines::const_iterator it = m_routingEngines.find(dest);
		if (it != m_routingEngines.end())
			return it->second;

		Ptr<VL2RoutingEngine> engine = CreateEngine();
		m_routingEngines.insert(std::make_pair(dest, engine));
		return engine;
	}
}


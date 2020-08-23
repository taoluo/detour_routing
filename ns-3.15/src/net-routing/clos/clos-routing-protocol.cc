#include "clos-routing-protocol.h"
#include "ns3/node.h"
#include "ns3/clos-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ClosRoutingProtocol);

	TypeId ClosRoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::ClosRoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<ClosRoutingProtocol> ()
			.AddAttribute("NI", "Clos parameter ni.",
						  UintegerValue(4), MakeUintegerAccessor(&ClosRoutingProtocol::m_ni), MakeUintegerChecker<uint32_t>())
			.AddAttribute("NA", "Clos parameter na.",
						  UintegerValue(6), MakeUintegerAccessor(&ClosRoutingProtocol::m_na), MakeUintegerChecker<uint32_t>())
			.AddAttribute("Alg", "Routing algorithm.",
						  UintegerValue(ROBIN_INDEX_DIGITAL_REVERSAL_ROUND_ROBIN),
						  MakeUintegerAccessor(&ClosRoutingProtocol::m_routingAlg), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	ClosRoutingProtocol::ClosRoutingProtocol()
	{
	}

	ClosRoutingProtocol::~ClosRoutingProtocol()
	{
	}

	uint32_t ClosRoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress ClosRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void ClosRoutingProtocol::Send (Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		ClosHeader header;
		header.src = source.Get();
		header.dest = destination.Get();
		header.protocol = protocol;

		header.core = GetNextCoreSwitch(packet, destination.Get(), packet->GetSize());
		header.ltime = 0;

		packet->AddHeader(header);

		Ptr<NetDevice> device = GetForwardingDevice(0, header.core, header.dest);
		SendPacket(device, packet);
	}

	void ClosRoutingProtocol::Receive(Ptr<NetDevice> indev, Ptr<const Packet> p, uint16_t protocol,
											const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();

		ClosHeader header;
		packet->RemoveHeader(header);

		if (header.dest != GetNode()->GetId())
		{
			header.ltime = header.ltime + 1;

			packet->AddHeader(header);

			Ptr<NetDevice> outdev = GetForwardingDevice(header.ltime, header.core, header.dest);
			SendPacket(outdev, packet);
		}
		else
		{
			Ptr<IpL4Protocol> protocol = GetProtocol (header.protocol);

			Ipv4Header ipv4Header;
			ipv4Header.SetSource(Ipv4Address(header.src));
			ipv4Header.SetDestination(Ipv4Address(header.dest));

			Ptr<Ipv4Interface> ipv4Interface = Create<Ipv4Interface>();
			protocol->Receive(packet, ipv4Header, ipv4Interface);
		}
	}

	// ===================================================================================================

	Ptr<Index> ClosRoutingProtocol::CreateIndex()
	{
		switch(m_routingAlg)
		{
		case ROBIN_INDEX_ROUND_ROBIN:
			return Create<RoundRobinIndex>(m_na / 2);

		case ROBIN_INDEX_RANDOM:
			return Create<RandomIndex>(0, m_na / 2);

		case ROBIN_INDEX_DIGITAL_REVERSAL_ROUND_ROBIN: 
			return Create<DigitalReversalRobinIndex>(m_na / 2, m_na / 2, true);

		case ROBIN_INDEX_PATH_TAG:
			return Create<PathTagIndex>(m_na / 2);

		case ROBIN_INDEX_INIT_RANDOM:
			return Create<InitRandomIndex>(0, m_na / 2);

		default:
			NS_ASSERT(false);
		}
		return NULL;
	}

	uint32_t ClosRoutingProtocol::GetNextCoreSwitch(Ptr<Packet> packet, uint32_t dest, uint32_t size)
	{
		uint32_t type = (size < 512 ? PACKET_SIZE_TYPE_1 : PACKET_SIZE_TYPE_2);
		uint32_t key = dest + (type << 16);

		RoutingIndexes::iterator it = m_indexes.find(key);
		if (it != m_indexes.end())
			return (*(it->second)).GetNext(packet);

		Ptr<Index> index = CreateIndex();
		m_indexes.insert(std::make_pair(key, index));
		return (*index).GetNext(packet);
	}

	Ptr<NetDevice> ClosRoutingProtocol::GetForwardingDevice(uint8_t livingTime, uint32_t coreSwitchId, uint32_t targetNodeId)
	{
		int32_t devid = -1;

		if (livingTime == 0)
			devid = 0;
		else if (livingTime == 1)
			devid = coreSwitchId + m_na / 2;
		else if (livingTime == 2)
			devid = targetNodeId / (m_na / 2);
		else if (livingTime == 3)
			devid = targetNodeId % (m_na / 2);

		NS_ASSERT(devid != -1);

		return GetNode()->GetDevice(devid);
	}
}

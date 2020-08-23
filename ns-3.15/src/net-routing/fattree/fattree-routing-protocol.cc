#include "fattree-routing-protocol.h"
#include "ns3/node.h"
#include "ns3/fattree-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (FattreeRoutingProtocol);

	TypeId FattreeRoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::FattreeRoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<FattreeRoutingProtocol> ()
			.AddAttribute("N", "Fattree parameter N.",
						  UintegerValue(4), MakeUintegerAccessor(&FattreeRoutingProtocol::m_n), MakeUintegerChecker<uint32_t>())
			.AddAttribute("RoutingAlgorithm", "Routing algorithm.",
						  UintegerValue(ROBIN_INDEX_DIGITAL_REVERSAL_ROUND_ROBIN),
						  MakeUintegerAccessor(&FattreeRoutingProtocol::m_routingAlg), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	FattreeRoutingProtocol::FattreeRoutingProtocol()
	{
	}

	FattreeRoutingProtocol::~FattreeRoutingProtocol()
	{
	}

	uint32_t FattreeRoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress FattreeRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void FattreeRoutingProtocol::Send (Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		FattreeHeader header;
		header.SetSrc(source.Get());
		header.SetDest(destination.Get());
		header.SetProtocol(protocol);

		uint32_t nextCoreSwitchId = GetNextCoreSwitch(packet, destination.Get(), packet->GetSize());
		header.SetTopSwitchId(nextCoreSwitchId);
		header.SetLivingTime(0);

		packet->AddHeader(header);

		Ptr<NetDevice> device = GetForwardingDevice(0, header.GetTopSwitchId(), header.GetDest());
		SendPacket(device, packet);
	}

	void FattreeRoutingProtocol::Receive(Ptr<NetDevice> indev, Ptr<const Packet> p, uint16_t protocol,
											const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();

		FattreeHeader header;
		packet->RemoveHeader(header);

		if (header.GetDest() != GetNode()->GetId())
		{
			uint8_t livingTime = header.GetLivingTime() + 1;
			header.SetLivingTime(livingTime);

			packet->AddHeader(header);

			Ptr<NetDevice> outdev = GetForwardingDevice(livingTime, header.GetTopSwitchId(), header.GetDest());
			SendPacket(outdev, packet);
		}
		else
		{
			Ptr<IpL4Protocol> protocol = GetProtocol (header.GetProtocol());

			Ipv4Header ipv4Header;
			ipv4Header.SetSource(Ipv4Address(header.GetSrc()));
			ipv4Header.SetDestination(Ipv4Address(header.GetDest()));

			Ptr<Ipv4Interface> ipv4Interface = Create<Ipv4Interface>();
			protocol->Receive(packet, ipv4Header, ipv4Interface);
		}
	}

	// ===================================================================================================

	Ptr<Index> FattreeRoutingProtocol::CreateIndex()
	{
		switch(m_routingAlg)
		{
		case ROBIN_INDEX_ROUND_ROBIN:
			return Create<RoundRobinIndex>(m_n * m_n / 4);

		case ROBIN_INDEX_RANDOM:
			return Create<RandomIndex>(0, m_n * m_n / 4);

		case ROBIN_INDEX_DIGITAL_REVERSAL_ROUND_ROBIN:
			return Create<DigitalReversalRobinIndex>(m_n * m_n / 4, m_n / 2, true);

		case ROBIN_INDEX_PATH_TAG:
			return Create<PathTagIndex>(m_n * m_n / 4);

		case ROBIN_INDEX_INIT_RANDOM:
			return Create<InitRandomIndex>(0, m_n * m_n / 4);

		default:
			NS_ASSERT(false);
		}
		return NULL;
	}

	uint32_t FattreeRoutingProtocol::GetNextCoreSwitch(Ptr<Packet> packet, uint32_t dest, uint32_t size)
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

	Ptr<NetDevice> FattreeRoutingProtocol::GetForwardingDevice(uint8_t livingTime, uint32_t coreSwitchId, uint32_t targetNodeId)
	{
		int32_t devid = -1;

		if (livingTime == 0)
			devid = 0;
		else if (livingTime == 1)
			devid = coreSwitchId / (m_n / 2) + m_n / 2;
		else if (livingTime == 2)
			devid = coreSwitchId % (m_n / 2) + m_n / 2;
		else if (livingTime == 3)
			devid = targetNodeId / (m_n * m_n / 4);
		else if (livingTime == 4)
			devid = (targetNodeId % (m_n * m_n / 4)) / (m_n / 2);
		else if (livingTime == 5)
			devid = targetNodeId % (m_n / 2);

		NS_ASSERT(devid != -1);

		return GetNode()->GetDevice(devid);
	}

	uint32_t FattreeRoutingProtocol::GetPathNumber() const
	{
		return m_n * m_n / 4;
	}
}

#include "ns3/ecmp-vl2-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/ecmp-vl2-header.h"

#include <stdio.h>

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ECMPVL2RoutingProtocol);

	TypeId ECMPVL2RoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::ECMPVL2RoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<ECMPVL2RoutingProtocol> ()
			.AddAttribute("NI", "VL2 parameter ni.",
						  UintegerValue(4), MakeUintegerAccessor(&ECMPVL2RoutingProtocol::m_ni), MakeUintegerChecker<uint32_t>())
			.AddAttribute("NA", "VL2 parameter na.",
						  UintegerValue(6), MakeUintegerAccessor(&ECMPVL2RoutingProtocol::m_na), MakeUintegerChecker<uint32_t>())
			.AddAttribute("type", "Type.",
						  UintegerValue(ECMP_VL2_PACKET_LEVEL), MakeUintegerAccessor(&ECMPVL2RoutingProtocol::m_type), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	uint32_t ECMPVL2RoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress ECMPVL2RoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void ECMPVL2RoutingProtocol::Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		ECMPVL2Header header;
		header.src = source.Get();
		header.dest = destination.Get();
		header.protocol = protocol;
		header.ltime = 0;
		// header.timestamp = (uint32_t)Simulator::Now().GetMicroSeconds();

		packet->AddHeader(header);

		SendPacket(GetNode()->GetDevice(0), packet);
	}

	void ECMPVL2RoutingProtocol::Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();
		ECMPVL2Header header;
		packet->RemoveHeader(header);

		if (header.dest != GetNode()->GetId())
		{
			uint32_t devid = 0;
			// if (IsChild(header.dest) == true)
            if (header.ltime >= 2)
			{
				uint32_t srv_num = 5 * m_na * m_ni;
				uint32_t tor_num = m_na * m_ni / 4;
				uint32_t agg_num = m_ni;
				uint32_t id = GetNode()->GetId();
				
				if (id < srv_num + tor_num)
					devid = header.dest % 20;
				else if (id < srv_num + tor_num + agg_num)
					devid = header.dest % (10 * m_na) / 20;
				else
					devid = header.dest / (10 * m_na) * 2 + hash(header, device->GetIfIndex()) % 2;
			}
			else
			{
				uint32_t srv_num = 5 * m_na * m_ni;
				uint32_t tor_num = m_na * m_ni / 4;
				uint32_t agg_num = m_ni;
				uint32_t id = GetNode()->GetId();

				if (id < srv_num + tor_num)
					devid = 20 + hash(header, device->GetIfIndex()) % 2;
				else if (id < srv_num + tor_num + agg_num)
					devid = hash(header, device->GetIfIndex()) % (m_na / 2) + m_na / 2;
			}

			header.ltime = header.ltime + 1;
			packet->AddHeader(header);
			SendPacket(GetNode()->GetDevice(devid), packet);
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

	// bool ECMPVL2RoutingProtocol::IsChild(uint32_t dest)
	// {
	// 	uint32_t srv_num = 5 * m_na * m_ni;
	// 	uint32_t tor_num = m_na * m_ni / 4;
	// 	uint32_t agg_num = m_ni;

	// 	uint32_t id = GetNode()->GetId();
	// 	if (id < srv_num)
	// 		return false;
	// 	else if (id < srv_num + tor_num)
	// 		return 20 * (id - srv_num) <= dest && dest < 20 * (id - srv_num + 1);
	// 	else if (id < srv_num + tor_num + agg_num)
	// 		return (id - srv_num - tor_num) / 2 * 10 * m_na <= dest && dest < ((id - srv_num - tor_num) / 2 + 1) * 10 * m_na;
	// 	else
	// 		return true;
	// }

	uint16_t ECMPVL2RoutingProtocol::hash(ECMPVL2Header& header, uint32_t inport)
	{
		uint16_t value = 0;
        // uint64_t now = Simulator::Now().GetNanoSeconds();

		value ^= (header.src & 0x0000FFFF);
		value ^= (header.src & 0xFFFF0000) >> 16;

		value ^= (header.dest & 0x0000FFFF);
		value ^= (header.dest & 0xFFFF0000) >> 16;

		if (m_type == ECMP_VL2_PACKET_LEVEL)
		{
            return rand();
            // value ^= now & 0x000000000000FFFF;
            // value ^= (now & 0x00000000FFFF0000) >> 16;
            // value ^= (now & 0x0000FFFF00000000) >> 32;
            // value ^= (now & 0xFFFF000000000000) >> 48;
			// value ^= (header.timestamp & 0x0000FFFF);
			// value ^= (header.timestamp & 0xFFFF0000) >> 16;
		}
		
		return value ^ inport;
	}
}

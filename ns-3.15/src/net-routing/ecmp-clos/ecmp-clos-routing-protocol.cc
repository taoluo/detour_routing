#include "ns3/ecmp-clos-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/ecmp-clos-header.h"

#include <stdio.h>

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ECMPClosRoutingProtocol);

	TypeId ECMPClosRoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::ECMPClosRoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<ECMPClosRoutingProtocol> ()
			.AddAttribute("NI", "Clos parameter ni.",
						  UintegerValue(4), MakeUintegerAccessor(&ECMPClosRoutingProtocol::m_ni), MakeUintegerChecker<uint32_t>())
			.AddAttribute("NA", "Clos parameter na.",
						  UintegerValue(6), MakeUintegerAccessor(&ECMPClosRoutingProtocol::m_na), MakeUintegerChecker<uint32_t>())
			.AddAttribute("type", "Type.",
						  UintegerValue(ECMP_CLOS_PACKET_LEVEL), MakeUintegerAccessor(&ECMPClosRoutingProtocol::m_type), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	uint32_t ECMPClosRoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

  	Ipv4InterfaceAddress ECMPClosRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

 	void ECMPClosRoutingProtocol::Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		ECMPClosHeader header;
		header.src = source.Get();
		header.dest = destination.Get();
		header.protocol = protocol;
		header.ltime = 0;
		// header.timestamp = (uint32_t)Simulator::Now().GetMicroSeconds();

		packet->AddHeader(header);

		SendPacket(GetNode()->GetDevice(0), packet);
	}

	void ECMPClosRoutingProtocol::Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();
		ECMPClosHeader header;
		packet->RemoveHeader(header);

		if (header.dest != GetNode()->GetId())
		{
			uint32_t devid = 0;
			
			// if (IsChild(header.dest) == true)
            if (header.ltime >= 1)
			{
				uint32_t srv_num = m_ni * m_na / 2;
				uint32_t tor_num = m_ni;
				uint32_t id = GetNode()->GetId();
			
				if (id < srv_num + tor_num)
					devid = header.dest % (m_na / 2);
				else
					devid = header.dest / (m_na / 2);
			}
			else
				devid = hash(header, device->GetIfIndex()) % (m_na / 2) + m_na / 2;

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

	// bool ECMPClosRoutingProtocol::IsChild(uint32_t dest)
	// {
	// 	uint32_t srv_num = m_ni * m_na / 2;
	// 	uint32_t tor_num = m_ni;
	// 	uint32_t id = GetNode()->GetId();
		
	// 	if (id < srv_num)
	// 		return false;
	// 	else if (id < srv_num + tor_num)
	// 		return (id - srv_num) * m_na / 2 <= dest && dest < (id - srv_num + 1) * m_na / 2;
	// 	else
	// 		return true;
	// }

	uint16_t ECMPClosRoutingProtocol::hash(ECMPClosHeader& header, uint32_t inport)
	{
		uint16_t value = 0;
        // uint64_t now = Simulator::Now().GetNanoSeconds();

		value ^= (header.src & 0x0000FFFF);
		value ^= (header.src & 0xFFFF0000) >> 16;

		value ^= (header.dest & 0x0000FFFF);
		value ^= (header.dest & 0xFFFF0000) >> 16;

		if (m_type == ECMP_CLOS_PACKET_LEVEL)
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

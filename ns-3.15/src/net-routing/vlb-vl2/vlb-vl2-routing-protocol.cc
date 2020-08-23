#include "ns3/vlb-vl2-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/vlb-vl2-header.h"

#include <stdio.h>

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (VLBVL2RoutingProtocol);

	TypeId VLBVL2RoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::VLBVL2RoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<VLBVL2RoutingProtocol> ()
			.AddAttribute("NI", "VL2 parameter ni.",
						  UintegerValue(4), MakeUintegerAccessor(&VLBVL2RoutingProtocol::m_ni), MakeUintegerChecker<uint32_t>())
			.AddAttribute("NA", "VL2 parameter na.",
						  UintegerValue(6), MakeUintegerAccessor(&VLBVL2RoutingProtocol::m_na), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	uint32_t VLBVL2RoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress VLBVL2RoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void VLBVL2RoutingProtocol::Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		VLBVL2Header header;
		header.src = source.Get();
		header.dest = destination.Get();
		header.protocol = protocol;
		header.ltime = 0;

        VlbIndexMap::iterator it = vlb_indexes.find(header.dest);
        if(it == vlb_indexes.end())
        {
            header.core_index = rand() % (m_na / 2);
            header.topdown_index = rand() % 2;
            vlb_indexes[header.dest] = std::pair< uint32_t, uint32_t >(header.core_index, header.topdown_index);
        }
        else
        {
            header.core_index = (*it).second.first;
            header.topdown_index = (*it).second.second;
        }

		packet->AddHeader(header);

		SendPacket(GetNode()->GetDevice(0), packet);
	}

	void VLBVL2RoutingProtocol::Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();
		VLBVL2Header header;
		packet->RemoveHeader(header);

		if (header.dest != GetNode()->GetId())
		{
			uint32_t devid = 0;
			if (IsChild(header.dest) == true)
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
					devid = header.dest / (10 * m_na) * 2 + header.topdown_index;
			}
			else
			{
				uint32_t srv_num = 5 * m_na * m_ni;
				uint32_t tor_num = m_na * m_ni / 4;
				uint32_t agg_num = m_ni;
				uint32_t id = GetNode()->GetId();

				if (id < srv_num + tor_num)
					devid = 20 + hash(header) % 2;
				else if (id < srv_num + tor_num + agg_num)
					devid = m_na / 2 + header.core_index;
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

	bool VLBVL2RoutingProtocol::IsChild(uint32_t dest)
	{
		uint32_t srv_num = 5 * m_na * m_ni;
		uint32_t tor_num = m_na * m_ni / 4;
		uint32_t agg_num = m_ni;

		uint32_t id = GetNode()->GetId();
		if (id < srv_num)
			return false;
		else if (id < srv_num + tor_num)
			return 20 * (id - srv_num) <= dest && dest < 20 * (id - srv_num + 1);
		else if (id < srv_num + tor_num + agg_num)
			return (id - srv_num - tor_num) / 2 * 10 * m_na <= dest && dest < ((id - srv_num - tor_num) / 2 + 1) * 10 * m_na;
		else
			return true;
	}

	uint8_t VLBVL2RoutingProtocol::hash(VLBVL2Header& header)
	{
		uint8_t value = 0;

		value ^= (header.src & 0x000000FF);
		value ^= (header.src & 0x0000FF00) >> 8;
		value ^= (header.src & 0x00FF0000) >> 16;
		value ^= (header.src & 0xFF000000) >> 24;

		value ^= (header.dest & 0x000000FF);
		value ^= (header.dest & 0x0000FF00) >> 8;
		value ^= (header.dest & 0x00FF0000) >> 16;
		value ^= (header.dest & 0xFF000000) >> 24;
        
		return value;
	}
}

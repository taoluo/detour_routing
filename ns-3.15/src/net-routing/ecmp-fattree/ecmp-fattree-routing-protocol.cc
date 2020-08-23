#include "ns3/ecmp-fattree-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/ecmp-fattree-header.h"
#include "ns3/tcp-header.h"
#include <stdio.h>

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ECMPFattreeRoutingProtocol);

	TypeId ECMPFattreeRoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::ECMPFattreeRoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<ECMPFattreeRoutingProtocol> ()
			.AddAttribute("N", "Fattree parameter N.",
						  UintegerValue(4), MakeUintegerAccessor(&ECMPFattreeRoutingProtocol::m_n), MakeUintegerChecker<uint32_t>())
			.AddAttribute("type", "Type.",
						  UintegerValue(ECMP_FATTREE_FLOW_LEVEL), MakeUintegerAccessor(&ECMPFattreeRoutingProtocol::m_type), MakeUintegerChecker<uint32_t>());

		return tid;
	}

	uint32_t ECMPFattreeRoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress ECMPFattreeRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void ECMPFattreeRoutingProtocol::Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		ECMPFattreeHeader header;
		header.src = source.Get();
		header.dest = destination.Get();
		//header.sport = 
		header.protocol = protocol;
		header.ltime = 0;
		// header.timestamp = (uint32_t)Simulator::Now().GetMicroSeconds();

		packet->AddHeader(header);

		SendPacket(GetNode()->GetDevice(0), packet);
	}

	void ECMPFattreeRoutingProtocol::Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
		Ptr<Packet> packet = p->Copy();
		ECMPFattreeHeader header;
		packet->RemoveHeader(header);
		//std::cout<<"test"<<std::endl;
		//if (GetNode()->GetId()==16){
		//std::cout<<Simulator::Now().GetSeconds()<<" ";
		//std::cout<<GetNode()->GetId()<<":"<<header.src<<"->"<<header.dest<<":"<<std::endl;}
		TcpHeader tcpHeader;
		packet->RemoveHeader(tcpHeader);
		header.sport = tcpHeader.GetSourcePort();
		header.dport = tcpHeader.GetDestinationPort();
		
		//std::cout<<header.sport<<" "<<tcpHeader.GetDestinationPort()<<std::endl;
		packet->AddHeader(tcpHeader);
		if (header.dest != GetNode()->GetId())
		{
			uint32_t devid = 0;
			// if (IsChild(header.dest) == true)
            if (header.ltime >= 2)
			{
				uint32_t srv_num = m_n * m_n * m_n / 4;
				uint32_t pot_switch_num = m_n * m_n;
				uint32_t id = GetNode()->GetId();

				if (id < srv_num + pot_switch_num / 2)
					devid = header.dest % (m_n / 2);
				else if (id < srv_num + pot_switch_num)
					devid = (header.dest % (m_n * m_n / 4)) / (m_n / 2);
				else
					devid = header.dest / (m_n * m_n / 4);
			}
			else
				devid = hash(header, device->GetIfIndex()) % (m_n / 2) + m_n / 2;

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

	// bool ECMPFattreeRoutingProtocol::IsChild(uint32_t dest)
	// {
	// 	uint32_t srv_num = m_n * m_n * m_n / 4;
	// 	uint32_t pot_switch_num = m_n * m_n;

	// 	uint32_t id = GetNode()->GetId();
	// 	if (id < srv_num)
	// 		return false;
	// 	else if (id < srv_num + pot_switch_num / 2)
	// 		return (id - srv_num) * m_n / 2 <= dest && dest < (id - srv_num + 1) * m_n / 2;
	// 	else if (id < srv_num + pot_switch_num)
	// 		return (id - srv_num - pot_switch_num / 2) / (m_n / 2) * m_n * m_n / 4 <= dest
	// 			&& dest < ((id - srv_num - pot_switch_num / 2) / (m_n / 2) + 1) * m_n * m_n / 4;
	// 	else
	// 		return true;
	// }

	uint16_t ECMPFattreeRoutingProtocol::hash(ECMPFattreeHeader& header, uint32_t inport)
	{
		uint16_t value = 0;
        // uint64_t now = Simulator::Now().GetNanoSeconds();

		value ^= (header.src & 0x0000FFFF);
		value ^= (header.src & 0xFFFF0000) >> 16;
		value ^= header.sport;
		value ^= header.dport;
		value ^= (header.dest & 0x0000FFFF);
		value ^= (header.dest & 0xFFFF0000) >> 16;
		//value += 1;
		//std::cout<<header.src<<" "<<header.dest<<" "<<header.sport<<" "<<header.dport<<std::endl;
		if (m_type == ECMP_FATTREE_PACKET_LEVEL)
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
		//return 0;
	}
}

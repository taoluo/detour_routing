#ifndef __ECMP_VL2_ROUTING_PROTOCOL__
#define __ECMP_VL2_ROUTING_PROTOCOL__

#include "ns3/net-routing-protocol.h"
#include "ns3/ecmp-vl2-header.h"

#define ECMP_VL2_PACKET_LEVEL	1
#define ECMP_VL2_FLOW_LEVEL		2

namespace ns3
{
	class Packet;
	
	class ECMPVL2RoutingProtocol : public NetRoutingProtocol
	{
		uint32_t m_na;
		uint32_t m_ni;
		uint32_t m_type;

		uint16_t hash(ECMPVL2Header& header, uint32_t inport);
		
		/* bool IsChild(uint32_t id); */

	public:
		static TypeId GetTypeId (void);

		uint32_t GetNAddresses(uint32_t interface) const;
		
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		virtual void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		virtual void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

		virtual uint32_t GetPathNumber() const { return m_na; }
	};
}

#endif 

#ifndef __ECMP_FATTREE_ROUTING_PROTOCOL__
#define __ECMP_FATTREE_ROUTING_PROTOCOL__

#include "ns3/net-routing-protocol.h"
#include "ns3/ecmp-fattree-header.h"

#define ECMP_FATTREE_PACKET_LEVEL		1
#define ECMP_FATTREE_FLOW_LEVEL			2

namespace ns3
{
	class Packet;
	
	class ECMPFattreeRoutingProtocol : public NetRoutingProtocol
	{
		uint32_t m_n;

		uint16_t hash(ECMPFattreeHeader& header, uint32_t inport);
		
		/* bool IsChild(uint32_t id); */

		uint32_t m_type;
	public:
		static TypeId GetTypeId (void);

		uint32_t GetNAddresses(uint32_t interface) const;
		
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		virtual void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		virtual void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

		virtual uint32_t GetPathNumber() const { return m_n * m_n / 4; }
	};
}

#endif 

#ifndef __VLB_VL2_ROUTING_PROTOCOL__
#define __VLB_VL2_ROUTING_PROTOCOL__

#include <map>
#include "ns3/net-routing-protocol.h"
#include "ns3/vlb-vl2-header.h"

namespace ns3
{
	class Packet;
	
	class VLBVL2RoutingProtocol : public NetRoutingProtocol
	{
		uint32_t m_na;
		uint32_t m_ni;

        typedef std::map< uint32_t, std::pair< uint32_t, uint32_t > > VlbIndexMap;
        VlbIndexMap vlb_indexes;

		uint8_t hash(VLBVL2Header& header);

		bool IsChild(uint32_t id);

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

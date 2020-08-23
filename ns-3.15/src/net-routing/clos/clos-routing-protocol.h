#ifndef __CLOS_ROUTING_PROTOCOL__
#define __CLOS_ROUTING_PROTOCOL__

#include "ns3/net-routing-protocol.h"
#include "ns3/robin-index.h"
#include "ns3/ptr.h"

#define 	PACKET_SIZE_TYPE_1			0
#define 	PACKET_SIZE_TYPE_2 			1

namespace ns3
{
	class Packet;
	
	class ClosRoutingProtocol : public NetRoutingProtocol
	{
		uint32_t m_na;
		uint32_t m_ni;
		uint32_t m_routingAlg;

		typedef std::map<uint32_t, Ptr<Index> > RoutingIndexes;
		RoutingIndexes m_indexes;

		Ptr<Index> CreateIndex();		
		uint32_t GetNextCoreSwitch(Ptr<Packet> packet, uint32_t targetNodeId, uint32_t size);
		Ptr<NetDevice> GetForwardingDevice(uint8_t livingTime, uint32_t coreSwitchId, uint32_t targetNodeId);
	public:
		static TypeId GetTypeId (void);

		ClosRoutingProtocol();
		~ClosRoutingProtocol();

		uint32_t GetNAddresses(uint32_t interface) const;
		
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		virtual void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		virtual void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

		virtual uint32_t GetPathNumber() const { return m_na / 2; }
	};
}

#endif

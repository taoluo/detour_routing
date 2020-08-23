#ifndef __VL2_ROUTING_PROTOCOL__
#define __VL2_ROUTING_PROTOCOL__

#include "ns3/net-routing-protocol.h"
#include "ns3/ptr.h"

#include <map>

#define VL2_ROUTING_ALG_RANDOM					0
#define VL2_ROUTING_ALG_ROUND_ROBIN				1
#define VL2_ROUTING_ALG_SEQUENTIAL_ROUND_ROBIN	2
#define VL2_ROUTING_ALG_PATH_TAG				3
#define VL2_ROUTING_ALG_VIRTUAL_RANDOM          4

namespace ns3
{
	class VL2RoutingEngine;
	class VL2RoutingProtocol : public NetRoutingProtocol
	{
		uint32_t m_ni;
		uint32_t m_na;
		uint32_t m_alg;

		typedef std::map<uint32_t, Ptr<VL2RoutingEngine> > RoutingEngines;
		RoutingEngines m_routingEngines;

		VL2RoutingEngine* CreateEngine();
		Ptr<VL2RoutingEngine> FindAndCreateEngine(uint32_t dest);
		
	public:
		static TypeId GetTypeId (void);

		VL2RoutingProtocol();
		~VL2RoutingProtocol();

		virtual uint32_t GetPathNumber() const { return m_na; }

		virtual uint32_t GetNAddresses (uint32_t interface) const { return 1; }
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		virtual void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		virtual void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);
	};
}

#endif

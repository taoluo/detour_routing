#ifndef __NET_ROUTING_H__
#define __NET_ROUTING_H__

#include <map>

#include "ns3/channel.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/nix-vector.h"
#include "ns3/bridge-net-device.h"

namespace ns3
{
	class NetHeader;
	class NetRouting : public Ipv4RoutingProtocol
	{
		friend class NetRoutingHelper;
		
	public:
		NetRouting ();
		~NetRouting ();

		static TypeId GetTypeId (void);
		
		void SetNode (Ptr<Node> node);
		virtual void SetIpv4 (Ptr<Ipv4> ipv4);

	private:
		// flushes the cache which stores nix-vector based on  destination IP
		void FlushNixCache (void);

		void DoDispose (void);

		// From Ipv4RoutingProtocol
		virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
		virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
								 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
								 LocalDeliverCallback lcb, ErrorCallback ecb);
		
		virtual void NotifyInterfaceUp (uint32_t interface);
		virtual void NotifyInterfaceDown (uint32_t interface);
		virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
		virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
		virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

		Ptr<Ipv4> m_ipv4;
		Ptr<Node> m_node;

		// total Nodes for nix-vector to determine number of bits
		uint32_t m_totalNodes;

		// iterates through the node list and finds the one corresponding to the given Ipv4Address
		Ptr<Node> GetNodeByIp (Ipv4Address);		
	};
} // namespace ns3
#endif

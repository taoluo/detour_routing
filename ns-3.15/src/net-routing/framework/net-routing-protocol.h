#ifndef __NET_ROUTING_PROTOCOL_H__
#define __NET_ROUTING_PROTOCOL_H__

#include <list>
#include <map>
#include <vector>
#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/ipv4.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
namespace ns3
{
	class Packet;
	class NetDevice;
	class Ipv4Interface;
	class Ipv4Address;
	class Ipv4Header;
	class Ipv4RoutingTableEntry;
	class Ipv4Route;
	class Node;
	class Socket;
	class IpL4Protocol;
	class NetHeader;

	class NetRoutingProtocol : public Ipv4
	{
	public:
		static TypeId GetTypeId (void);
		static const uint16_t PROT_NUMBER;

		NetRoutingProtocol();
		virtual ~NetRoutingProtocol ();

		/**
		 * \enum DropReason
		 * \brief Reason why a packet has been dropped.
		 */
		enum DropReason 
		{
			DROP_TTL_EXPIRED = 1,   /**< Packet TTL has expired */
			DROP_NO_ROUTE,   /**< No route to host */
			DROP_BAD_CHECKSUM,   /**< Bad checksum */
			DROP_INTERFACE_DOWN,   /**< Interface is down so can not send packet */
			DROP_ROUTE_ERROR,   /**< Route error */
			DROP_FRAGMENT_TIMEOUT /**< Fragment timeout exceeded */
		};

		void SetNode (Ptr<Node> node);

		// functions defined in base class Ipv4

		void SetRoutingProtocol (Ptr<Ipv4RoutingProtocol> routingProtocol);
		Ptr<Ipv4RoutingProtocol> GetRoutingProtocol (void) const;

		void Insert (Ptr<IpL4Protocol> protocol);

		Ptr<IpL4Protocol> GetProtocol (int protocolNumber) const;

		void Remove (Ptr<IpL4Protocol> protocol);

		void SetDefaultTtl (uint8_t ttl);

		virtual void Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from,
					   const Address &to, NetDevice::PacketType packetType);

		virtual void Send (Ptr<Packet> packet, Ipv4Address source, 
				   Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		void SendWithHeader (Ptr<Packet> packet, Ipv4Header ipHeader, Ptr<Ipv4Route> route);
		
		uint32_t AddInterface (Ptr<NetDevice> device);
		Ptr<Ipv4Interface> GetInterface (uint32_t i) const;
		uint32_t GetNInterfaces (void) const;

		int32_t GetInterfaceForAddress (Ipv4Address addr) const;
		int32_t GetInterfaceForPrefix (Ipv4Address addr, Ipv4Mask mask) const;
		int32_t GetInterfaceForDevice (Ptr<const NetDevice> device) const;
		bool IsDestinationAddress (Ipv4Address address, uint32_t iif) const;

		bool AddAddress (uint32_t i, Ipv4InterfaceAddress address);
		Ipv4InterfaceAddress GetAddress (uint32_t interfaceIndex, uint32_t addressIndex) const;
		uint32_t GetNAddresses (uint32_t interface) const;
		bool RemoveAddress (uint32_t interfaceIndex, uint32_t addressIndex);
		Ipv4Address SelectSourceAddress (Ptr<const NetDevice> device,
										 Ipv4Address dst, Ipv4InterfaceAddress::InterfaceAddressScope_e scope);


		void SetMetric (uint32_t i, uint16_t metric);
		uint16_t GetMetric (uint32_t i) const;
		uint16_t GetMtu (uint32_t i) const;
		bool IsUp (uint32_t i) const;
		void SetUp (uint32_t i);
		void SetDown (uint32_t i);
		bool IsForwarding (uint32_t i) const;
		void SetForwarding (uint32_t i, bool val);

		Ptr<NetDevice> GetNetDevice (uint32_t i);

		void IpForward (Ptr<Ipv4Route> rtentry, Ptr<Packet> p);
		void IpMulticastForward (Ptr<Ipv4MulticastRoute> mrtentry, Ptr<const Packet> p, const Ipv4Header &header);
		void LocalDeliver (Ptr<const Packet> p, uint32_t iif);
		void RouteInputError (Ptr<const Packet> p, const Ipv4Header & ipHeader, Socket::SocketErrno sockErrno);

		virtual uint32_t GetPathNumber() const = 0;

		Ptr<Socket> CreateRawSocket(void);
		void DeleteRawSocket(Ptr<Socket> socket); 
		
	protected:
		Ptr<Node> GetNode() const { return m_node; }

		virtual void DoDispose (void);
		/**
		 * This function will notify other components connected to the node that a new stack member is now connected
		 * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
		 */
		virtual void NotifyNewAggregate ();

		bool SendPacket(Ptr<NetDevice> device, Ptr<Packet> packet);
	private:

		// class Ipv4 attributes
		virtual void SetIpForward (bool forward);
		virtual bool GetIpForward (void) const;
		virtual void SetWeakEsModel (bool model);
		virtual bool GetWeakEsModel (void) const;

		Ptr<Node> m_node;
		typedef std::list<Ptr<IpL4Protocol> > L4List_t;
		L4List_t m_protocols;
		uint8_t m_defaultTtl;
		uint16_t m_identification;

		TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t> m_sendOutgoingTrace;
		TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t> m_unicastForwardTrace;
		TracedCallback<const Ipv4Header &, Ptr<const Packet>, uint32_t> m_localDeliverTrace;

		// The following two traces pass a packet with an IP header
		TracedCallback<Ptr<const Packet>, Ptr<Ipv4>,  uint32_t> m_txTrace;
		TracedCallback<Ptr<const Packet>, Ptr<Ipv4>, uint32_t> m_rxTrace;
		// <ip-header, payload, reason, ifindex> (ifindex not valid if reason is DROP_NO_ROUTE)
		TracedCallback<const Ipv4Header &, Ptr<const Packet>, DropReason, Ptr<Ipv4>, uint32_t> m_dropTrace;

		Ptr<Ipv4RoutingProtocol> m_routingProtocol;
	};


} // Namespace ns3

#endif /* IPV4_L3_PROTOCOL_H */

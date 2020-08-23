#include "ns3/abort.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/object-vector.h"
#include "ns3/ipv4-header.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/net-routing-protocol.h"
#include "ns3/net-routing.h"
#include "ns3/net-header.h"
#include "ns3/ip-l4-protocol.h"


NS_LOG_COMPONENT_DEFINE ("NetRoutingProtocol");

namespace ns3 {

	const uint16_t NetRoutingProtocol::PROT_NUMBER = 0x0800;

	NS_OBJECT_ENSURE_REGISTERED (NetRoutingProtocol);

	TypeId
	NetRoutingProtocol::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::NetRoutingProtocol")
			.SetParent<Ipv4> ()
			.AddAttribute ("DefaultTtl", "The TTL value set by default on all outgoing packets generated on this node.",
						   UintegerValue (64),
						   MakeUintegerAccessor (&NetRoutingProtocol::m_defaultTtl),
						   MakeUintegerChecker<uint8_t> ())
			.AddTraceSource ("Tx", "Send ipv4 packet to outgoing interface.",
							 MakeTraceSourceAccessor (&NetRoutingProtocol::m_txTrace))
			.AddTraceSource ("Rx", "Receive ipv4 packet from incoming interface.",
							 MakeTraceSourceAccessor (&NetRoutingProtocol::m_rxTrace))
			.AddTraceSource ("Drop", "Drop ipv4 packet",
							 MakeTraceSourceAccessor (&NetRoutingProtocol::m_dropTrace))
			.AddTraceSource ("SendOutgoing", "A newly-generated packet by this node is about to be queued for transmission",
							 MakeTraceSourceAccessor (&NetRoutingProtocol::m_sendOutgoingTrace))
			.AddTraceSource ("UnicastForward", "A unicast IPv4 packet was received by this node and is being forwarded to another node",
							 MakeTraceSourceAccessor (&NetRoutingProtocol::m_unicastForwardTrace))
			.AddTraceSource ("LocalDeliver", "An IPv4 packet was received by/for this node, and it is being forward up the stack",
							 MakeTraceSourceAccessor (&NetRoutingProtocol::m_localDeliverTrace))
			;
		return tid;
	}


	NetRoutingProtocol::NetRoutingProtocol()
		: m_identification (0)
	{
		NS_LOG_FUNCTION (this);
	}

	NetRoutingProtocol::~NetRoutingProtocol ()
	{
		NS_LOG_FUNCTION (this);
	}

	void NetRoutingProtocol::Insert (Ptr<IpL4Protocol> protocol)
	{
		m_protocols.push_back (protocol);
	}

	Ptr<IpL4Protocol> NetRoutingProtocol::GetProtocol (int protocolNumber) const
	{
		for (L4List_t::const_iterator i = m_protocols.begin (); i != m_protocols.end (); ++i)
			if ((*i)->GetProtocolNumber () == protocolNumber)
				return *i;
		return 0;
	}

	void
	NetRoutingProtocol::Remove (Ptr<IpL4Protocol> protocol)
	{
		m_protocols.remove (protocol);
	}

	void
	NetRoutingProtocol::SetNode (Ptr<Node> node)
	{
		m_node = node;
	}

	/*
	 * This method is not implemented by AddAgregate and completes the aggregation
	 * by setting the node in the ipv4 stack
	 */
	void
	NetRoutingProtocol::NotifyNewAggregate ()
	{
		if (m_node == 0)
		{
			Ptr<Node>node = this->GetObject<Node>();
			// verify that it's a valid node and that
			// the node has not been set before
			if (node != 0)
			{
				this->SetNode (node);
			}
		}
		Object::NotifyNewAggregate ();
	}

	void
	NetRoutingProtocol::SetRoutingProtocol (Ptr<Ipv4RoutingProtocol> routingProtocol)
	{
		NS_LOG_FUNCTION (this);
		m_routingProtocol = DynamicCast<NetRouting>(routingProtocol);
		m_routingProtocol->SetIpv4 (this);
	}

	Ptr<Ipv4RoutingProtocol>
	NetRoutingProtocol::GetRoutingProtocol (void) const
	{
		return m_routingProtocol;
	}

	void
	NetRoutingProtocol::DoDispose (void)
	{
		NS_LOG_FUNCTION (this);
		for (L4List_t::iterator i = m_protocols.begin (); i != m_protocols.end (); ++i)
		{
			*i = 0;
		}
		m_protocols.clear ();

		m_node = 0;
		m_routingProtocol = 0;
		Object::DoDispose ();
	}

	void
	NetRoutingProtocol::SetDefaultTtl (uint8_t ttl)
	{
		m_defaultTtl = ttl;
	}

	void
	NetRoutingProtocol::Receive ( Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from,
									const Address &to, NetDevice::PacketType packetType)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::Receive() not implemented.");

		/*
		  Ptr<Packet> packet = p->Copy ();

		  Ipv4Header ipHeader;
		  if (Node::ChecksumEnabled ())
		  {
		  ipHeader.EnableChecksum ();
		  }
		  packet->RemoveHeader (ipHeader);

		  // Trim any residual frame padding from underlying devices
		  if (ipHeader.GetPayloadSize () < packet->GetSize ())
		  {
		  packet->RemoveAtEnd (packet->GetSize () - ipHeader.GetPayloadSize ());
		  }

		  if (!ipHeader.IsChecksumOk ())
		  {
		  NS_LOG_LOGIC ("Dropping received packet -- checksum not ok");
		  m_dropTrace (ipHeader, packet, DROP_BAD_CHECKSUM, m_node->GetObject<Ipv4> (), interface);
		  return;
		  }

		  NS_ASSERT_MSG (m_routingProtocol != 0, "Need a routing protocol object to process packets");
		  if (!m_routingProtocol->RouteInput (packet, ipHeader, device,
		  MakeCallback (&NetDeviceL3Protocol::IpForward, this),
		  MakeCallback (&NetDeviceL3Protocol::IpMulticastForward, this),
		  MakeCallback (&NetDeviceL3Protocol::LocalDeliver, this),
		  MakeCallback (&NetDeviceL3Protocol::RouteInputError, this)
		  ))
		  {
		  NS_LOG_WARN ("No route found for forwarding packet.  Drop.");
		  m_dropTrace (ipHeader, packet, DROP_NO_ROUTE, m_node->GetObject<Ipv4> (), interface);
		  }
		*/
	}

	void
	NetRoutingProtocol::Send (Ptr<Packet> packet,
								Ipv4Address source,
								Ipv4Address destination,
								uint8_t protocol,
								Ptr<Ipv4Route> route)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::Send() not implemented.");
		
		// Handle a few cases:
		// 1) packet is destined to limited broadcast address
		// 2) packet is destined to a subnet-directed broadcast address
		// 3) packet is not broadcast, and is passed in with a route entry
		// 4) packet is not broadcast, and is passed in with a route entry but route->GetGateway is not set (e.g., on-demand)
		// 5) packet is not broadcast, and route is NULL (e.g., a raw socket call, or ICMP)

		// To be done.
	}


	void
	NetRoutingProtocol::IpMulticastForward (Ptr<Ipv4MulticastRoute> mrtentry, Ptr<const Packet> p, const Ipv4Header &header)
	{
		NS_LOG_FUNCTION (this << mrtentry << p << header);
		NS_LOG_LOGIC ("Multicast forwarding logic for node: " << m_node->GetId ());

		std::cout << "NetRoutingProtocol::IpMulticastForward()" << std::endl;
	}

	// This function analogous to Linux ip_forward()
	void
	NetRoutingProtocol::IpForward (Ptr<Ipv4Route> rtentry, Ptr<Packet> p)
	{
		rtentry->GetOutputDevice()->Send(p, Address(), 0x0800);
	}

	void
	NetRoutingProtocol::LocalDeliver (Ptr<const Packet> packet, uint32_t iif)
	{
		std::cout << "NetRoutingProtocol::LocalDeliver()" << std::endl;
	}

	void
	NetRoutingProtocol::RouteInputError (Ptr<const Packet> p, const Ipv4Header & ipHeader, Socket::SocketErrno sockErrno)
	{
		std::cout << "NetRoutingProtocol::IpForward()" << std::endl;
	}

	uint32_t
	NetRoutingProtocol::AddInterface (Ptr<NetDevice> device)
	{
		m_node->RegisterProtocolHandler (MakeCallback (&NetRoutingProtocol::Receive, this), NetRoutingProtocol::PROT_NUMBER, device);
		return 0;
	}

	uint32_t
	NetRoutingProtocol::GetNInterfaces (void) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetNInterfaces() not implemented.");
		return 0;
	}

	int32_t
	NetRoutingProtocol::GetInterfaceForAddress (Ipv4Address address) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetInterfaceForAddress() not implemented.");
		return 0;
	}

	bool
	NetRoutingProtocol::IsDestinationAddress (Ipv4Address address, uint32_t iif) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::IsDestinationAddress() not implemented.");
		return false;
	}

	int32_t
	NetRoutingProtocol::GetInterfaceForPrefix (Ipv4Address address, Ipv4Mask mask) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetInterfaceForPrefix() not implemented.");
		return 0;
	}

	int32_t
	NetRoutingProtocol::GetInterfaceForDevice (Ptr<const NetDevice> device) const
	{
		return 0; 
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetInterfaceForDevice() not implemented.");
		return 0;
	}

	bool
	NetRoutingProtocol::AddAddress (uint32_t interface, Ipv4InterfaceAddress address)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::AddAddress() not implemented.");
		return false;
	}

	uint32_t
	NetRoutingProtocol::GetNAddresses (uint32_t interface) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetNAddresses() not implemented.");
		return 0;
	}

	Ipv4InterfaceAddress
	NetRoutingProtocol::GetAddress (uint32_t interface, uint32_t addressIndex) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetAddress() not implemented.");
		return Ipv4InterfaceAddress(Ipv4Address::GetAny(), Ipv4Mask::GetZero());
	}

	bool
	NetRoutingProtocol::RemoveAddress (uint32_t interface, uint32_t addressIndex)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::RemoveAddress() not implemented.");
		return false;
	}

	Ipv4Address
	NetRoutingProtocol::SelectSourceAddress (Ptr<const NetDevice> device,
											   Ipv4Address dst, Ipv4InterfaceAddress::InterfaceAddressScope_e scope)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::SelectSourceAddress() not implemented.");
		return Ipv4Address::GetAny();
	}

	void
	NetRoutingProtocol::SetMetric (uint32_t i, uint16_t metric)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::SetMetric() not implemented.");
	}

	uint16_t
	NetRoutingProtocol::GetMetric (uint32_t i) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetMetric() not implemented.");
		return 0;
	}

	uint16_t
	NetRoutingProtocol::GetMtu (uint32_t i) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetMtu() not implemented.");
		return 0;

	}

	bool
	NetRoutingProtocol::IsUp (uint32_t i) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::IsUp() not implemented.");
		return false;
	}

	void
	NetRoutingProtocol::SetUp (uint32_t i)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::SetUp() not implemented.");
	}

	void
	NetRoutingProtocol::SetDown (uint32_t ifaceIndex)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::SetDown() not implemented.");
	}

	bool
	NetRoutingProtocol::IsForwarding (uint32_t i) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::IsForwarding() not implemented.");
		return false;
	}

	void
	NetRoutingProtocol::SetForwarding (uint32_t i, bool val)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::SetForwarding() not implemented.");
	}

	Ptr<NetDevice>
	NetRoutingProtocol::GetNetDevice (uint32_t i)
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetNetDevice() not implemented.");
		return NULL;
	}

	void
	NetRoutingProtocol::SetIpForward (bool forward)
	{
		// NS_ASSERT_MSG (0, "NetRoutingProtocol::SetIpForward() not implemented.");
	}

	bool
	NetRoutingProtocol::GetIpForward (void) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetIpForward() not implemented.");
		return false;
	}

	void
	NetRoutingProtocol::SetWeakEsModel (bool model)
	{
		// NS_ASSERT_MSG (0, "NetRoutingProtocol::SetWeakEsModel() not implemented.");
	}

	bool
	NetRoutingProtocol::GetWeakEsModel (void) const
	{
		NS_ASSERT_MSG (0, "NetRoutingProtocol::GetWeakEsModel() not implemented.");
		return false;
	}

	bool NetRoutingProtocol::SendPacket(Ptr<NetDevice> device, Ptr<Packet> packet)
	{
	  //std::cout<<packet->GetSize()<<std::endl;
	  return device->Send(packet, Address(), 0x0800);
	}

  

        void
	NetRoutingProtocol::SendWithHeader (Ptr<Packet> packet, Ipv4Header ipHeader, Ptr<Ipv4Route> route)
	{
	  return;
	}
  
  
  
        Ptr<Socket>
	NetRoutingProtocol::CreateRawSocket(void)
	{
	  return NULL;
	}

        void
	NetRoutingProtocol::DeleteRawSocket(Ptr<Socket> socket)
	{
	  return;
	}
  
  
  

  
}


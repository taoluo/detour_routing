#include "net-routing-stack-helper.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/ipv4.h"
#include "ns3/net-routing-protocol.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/net-routing-helper.h"
#include "ns3/net-routing.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/log.h"

namespace ns3
{
    NetRoutingStackHelper::NetRoutingStackHelper()
	{
        m_l4Id = TcpL4Protocol::GetTypeId();
		Initialize();
	}

	NetRoutingStackHelper::~NetRoutingStackHelper()
	{
	}

    void
    NetRoutingStackHelper::SetL4Protocol( TypeId tid )
    {
        m_l4Id = tid;
        Initialize();
    }

    void NetRoutingStackHelper::Initialize()
	{
		m_tcpFactory.SetTypeId(m_l4Id);
	}

	void NetRoutingStackHelper::Install(const NodeContainer& c) const
	{
		for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
			Install (*i);
	}

	void NetRoutingStackHelper::Install(Ptr<Node> node) const
	{
		CreateAndAggregateObjectFromTypeId(node, "ns3::UdpL4Protocol");
		node->AggregateObject (m_tcpFactory.Create<Object>());

		node->AggregateObject (m_protocolFactory.Create <Object> ());

		NetRoutingHelper routing;
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
		Ptr<NetRouting> netRouting = DynamicCast<NetRouting> (routing.Create (node));
		// Ptr<NetRouting> netRouting = (NetRouting*) GetPointer(routing.Create (node));
		ipv4->SetRoutingProtocol (netRouting);

		Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory> ();
		node->AggregateObject (factory);

		for (uint32_t i = 0; i < node->GetNDevices(); i++)
			ipv4->AddInterface(node->GetDevice(i));
	}

	void NetRoutingStackHelper::CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string& typeId)
	{
		ObjectFactory factory;
		factory.SetTypeId (typeId);
		Ptr<Object> protocol = factory.Create <Object> ();
		node->AggregateObject (protocol);
	}

	void NetRoutingStackHelper::SetRoutingAttribute(const std::string& name, const AttributeValue &value)
	{
		m_protocolFactory.Set(name, value);
	}
}

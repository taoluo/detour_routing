#ifndef __NET_ROUTING_STACK_HELPER__
#define __NET_ROUTING_STACK_HELPER__

#include "ns3/ptr.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3
{
	class Node;
	class NetRoutingHelper;
	class Ipv4RoutingHelper;

	class NetRoutingStackHelper
	{
	private:
		ObjectFactory m_protocolFactory;
        TypeId        m_l4Id;
		ObjectFactory m_tcpFactory;
		void Initialize();

		static void CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string& typeId);

	protected:
		ObjectFactory& GetRoutingFactory() { return m_protocolFactory; }

	public:
        NetRoutingStackHelper();
		~NetRoutingStackHelper();

        void SetL4Protocol( TypeId tid );

		void Install (Ptr<Node> node) const;
		void Install (const NodeContainer& c) const;

		void SetRoutingAttribute(const std::string& name, const AttributeValue &value);
	};
}
#endif

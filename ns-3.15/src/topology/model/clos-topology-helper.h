#ifndef __CLOS_TOPOLOGY_HELPER__
#define __CLOS_TOPOLOGY_HELPER__

#include "ns3/topology-helper.h"
#include "ns3/node-container.h"

#include "stdint.h"

namespace ns3
{
	class ClosTopologyHelper : public TopologyHelper
	{
		NodeContainer coreswitchNodes;
		NodeContainer torswitchNodes;
		NodeContainer terminalNodes;

		uint32_t m_ni;
		uint32_t m_na;

	public:
		ClosTopologyHelper();
		~ClosTopologyHelper();

		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		virtual void CreateTopology();
		virtual Ptr<Node> GetNode(uint32_t i) const;
		virtual NodeContainer GetNodes() const;

		virtual uint32_t GetNTerminalNodes() const;
		virtual Ipv4Address GetTerminalInterface(int i) const;

		NodeContainer GetTorSwitchNodes() const { return torswitchNodes; }
		NodeContainer GetCoreSwitchNodes() const { return coreswitchNodes; }
		
		virtual NodeContainer GetTerminalNodes() const { return terminalNodes; }
		virtual Ptr<Node> GetTerminalNode(uint32_t i) const { return terminalNodes.Get(i); }
		virtual NodeContainer GetNonTerminalNodes() const { return NodeContainer(torswitchNodes, coreswitchNodes); }
	};
}

#endif

#ifndef __FATTREE_TOPOLOGY_HELPER__
#define __FATTREE_TOPOLOGY_HELPER__

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/topology-helper.h"
#include <string.h>
namespace ns3
{
	class FattreeTopologyHelper : public TopologyHelper
	{
		NodeContainer terminalNodes;
		NodeContainer potswitchNodes;
		NodeContainer topswitchNodes;

		NetDeviceContainer terminalDevices;
		NetDeviceContainer switchDevices;

		uint32_t m_n;
		uint32_t m_queue_size;
		std::string m_linkspeed;

	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		
		FattreeTopologyHelper ();
		~FattreeTopologyHelper ();

		virtual void CreateTopology();
        void CreateTopologyOversubscription(int oversub, int torbuffer, int corebuffer);
		uint32_t GetNTerminalNodes() const;
		Ptr<Node> GetTerminalNode(int i) const;
		Ptr<Node> GetNode(uint32_t i) const;
		Ipv4Address GetTerminalInterface(int i) const;		

		virtual NodeContainer GetNodes() const;
        virtual NetDeviceContainer GetNetDevices() const;

		virtual NodeContainer GetTerminalNodes() const { return terminalNodes; }
		virtual NodeContainer GetNonTerminalNodes() const { return NodeContainer(potswitchNodes, topswitchNodes); }

		NodeContainer GetPotSwitchNodes() const { return potswitchNodes; }
		NodeContainer GetTopSwitchNodes() const { return topswitchNodes; }
	};
}

#endif

#ifndef __VL2_TOPOLOGY_HELPER__
#define __VL2_TOPOLOGY_HELPER__

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/topology-helper.h"

namespace ns3
{
	class VL2TopologyHelper : public TopologyHelper
	{
        NodeContainer intermediateswitchNodes;
        NodeContainer aggregateswitchNodes;
        NodeContainer torswitchNodes;
        NodeContainer terminalNodes;
        
		NetDeviceContainer terminalDevices;
		NetDeviceContainer switchDevices;

		uint32_t m_ni; // Port number of intermediate switch
        uint32_t m_na; // Port number of aggregate switch
		uint32_t m_queue_size; 
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		
		VL2TopologyHelper ();
		~VL2TopologyHelper ();

		void SetNI(uint32_t ni) { m_ni = ni; }
		void SetNA(uint32_t na) { m_na = na; }

		virtual void CreateTopology();

		uint32_t GetNTerminalNodes() const;
		Ptr<Node> GetTerminalNode(int i) const;
		Ptr<Node> GetNode(uint32_t i) const;
		Ipv4Address GetTerminalInterface(int i) const;

		virtual NodeContainer GetNodes() const;
        virtual NetDeviceContainer GetNetDevices() const;

		virtual NodeContainer GetTerminalNodes() const { return terminalNodes; }
		virtual NodeContainer GetNonTerminalNodes() const { return NodeContainer(aggregateswitchNodes, torswitchNodes); }

        NodeContainer GetIntermediateSwitchNodes() const { return intermediateswitchNodes; }
        NodeContainer GetAggregateSwitchNodes() const { return aggregateswitchNodes; }
        NodeContainer GetTorSwitchNodes() const { return torswitchNodes; }
	};
}

#endif

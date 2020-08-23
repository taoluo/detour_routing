#ifndef __TREE_TOPOLOGY_HELPER__
#define __TREE_TOPOLOGY_HELPER__

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/topology-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/data-rate.h"

/*
 *                O
 *  level2       / \
 *              /   \  ...    
 *             O
 *  level1    /|\      ...
 *           / | \
 *          O  O  O    ...
 */ 

namespace ns3
{
	class TreeTopologyHelper : public TopologyHelper
	{
		NodeContainer m_terminals;
		NodeContainer m_potswitches;
		NodeContainer m_topswitches;

		NetDeviceContainer m_terminalDevices;
		NetDeviceContainer m_switchDevices;

        PointToPointHelper m_p2p1;
        PointToPointHelper m_p2p2;
		uint32_t m_n1;
        uint32_t m_n2;

      public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		
		TreeTopologyHelper ();
		~TreeTopologyHelper ();
        
        void SetLevel1DataRate(const DataRateValue &dataRate);
        void SetLevel1Delay(const TimeValue &delay);
        void SetLevel2DataRate(const DataRateValue &dataRate);
        void SetLevel2Delay(const TimeValue &delay);

		virtual void CreateTopology();

        virtual Ptr<Node> GetNode(uint32_t i) const;

		const NodeContainer& GetTerminals() const { return m_terminals; }
		const NodeContainer& GetPotSwitches() const { return m_potswitches; }
		const NodeContainer& GetTopSwitches() const { return m_topswitches; }

		virtual NodeContainer GetNodes() const;

		virtual NodeContainer GetTerminalNodes() const { return m_terminals; }
		virtual NodeContainer GetNonTerminalNodes() const { return NodeContainer(m_potswitches, m_topswitches); }

        virtual uint32_t GetNTerminalNodes() const;
		virtual Ipv4Address GetTerminalInterface(int i) const;
	};
}

#endif

#ifndef BCUBE_TOPOLOGY_HELPER_H
#define BCUBE_TOPOLOGY_HELPER_H

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/topology-helper.h"
#include "ns3/data-rate.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/topology-helper.h"
#include <vector>

namespace ns3
{
	class BCubeTopologyHelper : public TopologyHelper
	{
	public:
		BCubeTopologyHelper();
		~BCubeTopologyHelper();
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;		

		// Create the topology, including Nodes, Links, Devices, Ip & Stack
		void CreateTopology();

		// Get Terminal Nodes (Servers)
		NodeContainer GetTerminals();

		// Get Switching Nodes
		NodeContainer GetSwitches();

		// Get All Nodes
		NodeContainer GetNodes() const;

		virtual Ptr<Node> GetNode(uint32_t i) const;

		// Set parameters
		void SetDataRate(const DataRateValue &dataRate);
		void SetDelay(const TimeValue &delay);
		void SetBCubeSize(const uint32_t &n, const uint32_t &k);

		//private:
		std::vector <uint32_t> NumToBit(const uint32_t &num);
		uint32_t BitToNum(const std::vector <uint32_t> &bit);

		PointToPointHelper m_p2p;
		uint32_t m_n;			   // BCube(n,k)
		uint32_t m_k;
	       

		NodeContainer m_terminals;
		NodeContainer m_switches;
		NetDeviceContainer m_terminalDevices;
		NetDeviceContainer m_switchDevices;

		virtual uint32_t GetNTerminalNodes() const { return m_terminals.GetN(); }
		virtual Ipv4Address GetTerminalInterface(int i) const { return Ipv4Address(i); }

		virtual NodeContainer GetTerminalNodes() const { return m_terminals; }
		virtual NodeContainer GetNonTerminalNodes() const { return m_switches; }
	};

}// namespace ns3

#endif

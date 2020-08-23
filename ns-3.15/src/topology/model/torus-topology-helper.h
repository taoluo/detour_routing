#ifndef __TORUS_TOPOLOGY_HELPER__
#define __TORUS_TOPOLOGY_HELPER__

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/topology-helper.h"
#include "ns3/object-factory.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"

namespace ns3
{
	class TorusTopologyHelper : public TopologyHelper
	{
		NodeContainer terminalNodes;

		NetDeviceContainer terminalDevices;

		uint32_t m_n;
		uint32_t m_k;
		uint32_t m_queue_size;

		double m_rate;
		double m_delay;

		ObjectFactory queueFactory;
		ObjectFactory channelFactory;
		ObjectFactory deviceFactory;		
		
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		
		TorusTopologyHelper ();
		~TorusTopologyHelper ();

		void SetLinkRate(const DataRateValue& rate);
		void SetLinkDelay (const TimeValue& delay);

		virtual void CreateTopology();

		uint32_t GetNTerminalNodes() const;
		Ptr<Node> GetTerminalNode(int i) const;
		Ptr<Node> GetNode(uint32_t i) const;
		Ipv4Address GetTerminalInterface(int i) const;		

		virtual NodeContainer GetNodes() const;
        virtual NetDeviceContainer GetNetDevices() const;

		virtual NodeContainer GetTerminalNodes() const { return terminalNodes; }
		virtual NodeContainer GetNonTerminalNodes() const { return NodeContainer(); }
	};
}


#endif

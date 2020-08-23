#include "vl2-topology-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"

namespace ns3
{
	TypeId VL2TopologyHelper::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::VL2TopologyHelper")
			.SetParent<TopologyHelper>()
			.AddConstructor<VL2TopologyHelper>()
			.AddAttribute("NI", "The port number of intermediate switches.",
						  UintegerValue(4), MakeUintegerAccessor(&VL2TopologyHelper::m_ni), MakeUintegerChecker<uint32_t>())
            .AddAttribute("NA", "The port number of aggregate switches.",
                          UintegerValue(6), MakeUintegerAccessor(&VL2TopologyHelper::m_na), MakeUintegerChecker<uint32_t>())
            .AddAttribute("queue", "The queue size.",
                          UintegerValue(128), MakeUintegerAccessor(&VL2TopologyHelper::m_queue_size), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	TypeId VL2TopologyHelper::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	VL2TopologyHelper::VL2TopologyHelper ()
	{
		m_ni = 4;
		m_na = 6;
	}

	VL2TopologyHelper::~VL2TopologyHelper ()
	{}

	void VL2TopologyHelper::CreateTopology()
	{
		uint32_t na = m_na;
        uint32_t ni = m_ni;

        uint32_t int_switch_num = na / 2;
        uint32_t agg_switch_num = ni;
        uint32_t tor_switch_num = ni * na / 4;
        uint32_t srv_num = ni * na * 5;

        terminalNodes.Create(srv_num);
        torswitchNodes.Create(tor_switch_num);
        aggregateswitchNodes.Create(agg_switch_num);
        intermediateswitchNodes.Create(int_switch_num);

		PointToPointHelper pppSwitchSwitchHelper;

		pppSwitchSwitchHelper.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));
		pppSwitchSwitchHelper.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (5)));

        PointToPointHelper pppSwitchServerHelper;

        pppSwitchServerHelper.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
        pppSwitchServerHelper.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (5)));

		// create topology
		for (uint32_t i = 0; i < tor_switch_num; i++)
			for (uint32_t j = 0; j < 20; j++)
			{
				int from = i;
				int to = 20 * i + j;

				NetDeviceContainer link = pppSwitchServerHelper.Install(NodeContainer(torswitchNodes.Get(from), terminalNodes.Get(to)));
				switchDevices.Add(link.Get(0));
				terminalDevices.Add(link.Get(1));
			}

		for (uint32_t i = 0; i < agg_switch_num; i++)
			for (uint32_t j = 0; j < na / 2; j++)
			{
				int from = i;
				int to = (i / 2) * (na / 2) + j;

				NetDeviceContainer link = pppSwitchSwitchHelper.Install(NodeContainer(aggregateswitchNodes.Get(from), torswitchNodes.Get(to)));
				switchDevices.Add(link.Get(0));
				switchDevices.Add(link.Get(1));
			}

		for (uint32_t i = 0; i < int_switch_num; i++)
			for (uint32_t j = 0; j < agg_switch_num; j++)
			{
				int from = i;
				int to = j;

				NetDeviceContainer link = pppSwitchSwitchHelper.Install(NodeContainer(intermediateswitchNodes.Get(from), aggregateswitchNodes.Get(to)));
				switchDevices.Add(link.Get(0));
				switchDevices.Add(link.Get(1));
			}
	}

	Ptr<Node> VL2TopologyHelper::GetNode(uint32_t i) const
	{
		if (i < terminalNodes.GetN())
			return terminalNodes.Get(i);
		i -= terminalNodes.GetN();

		if (i < torswitchNodes.GetN())
			return torswitchNodes.Get(i);
		i -= torswitchNodes.GetN();

        if (i < aggregateswitchNodes.GetN())
			return aggregateswitchNodes.Get(i);
		i -= aggregateswitchNodes.GetN();

		if (i < intermediateswitchNodes.GetN())
			return intermediateswitchNodes.Get(i);

		return NULL;
	}

	uint32_t VL2TopologyHelper::GetNTerminalNodes() const
	{
		return terminalNodes.GetN();
	}

	Ptr<Node> VL2TopologyHelper::GetTerminalNode(int i) const
	{
		return terminalNodes.Get(i);
	}

	Ipv4Address VL2TopologyHelper::GetTerminalInterface(int i) const
	{
		return Ipv4Address(i);
	}

	NodeContainer VL2TopologyHelper::GetNodes() const
	{
		return NodeContainer(terminalNodes, torswitchNodes, aggregateswitchNodes, intermediateswitchNodes);
	}

    NetDeviceContainer VL2TopologyHelper::GetNetDevices() const
    {
        return NetDeviceContainer(terminalDevices, switchDevices);
    }
}

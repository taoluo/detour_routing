#include "clos-topology-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ClosTopologyHelper);

    TypeId ClosTopologyHelper::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::ClosTopologyHelper").SetParent<TopologyHelper>().AddConstructor<ClosTopologyHelper>()
			.AddAttribute("NI", "The port number of intermediate switches.",
						  UintegerValue(4), MakeUintegerAccessor(&ClosTopologyHelper::m_ni), MakeUintegerChecker<uint32_t>())
            .AddAttribute("NA", "The port number of aggregate switches.",
                          UintegerValue(6), MakeUintegerAccessor(&ClosTopologyHelper::m_na), MakeUintegerChecker<uint32_t>());		
        return tid;
    }

	TypeId ClosTopologyHelper::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	ClosTopologyHelper::ClosTopologyHelper()
	{}
	
	ClosTopologyHelper::~ClosTopologyHelper()
	{}

	void ClosTopologyHelper::CreateTopology()
	{
		uint32_t core_switch_num = m_na / 2;
		uint32_t tor_switch_num = m_ni;
		uint32_t srv_num = m_ni * m_na / 2;

		terminalNodes.Create(srv_num);
		torswitchNodes.Create(tor_switch_num);
		coreswitchNodes.Create(core_switch_num);

		PointToPointHelper pppHelper;
		pppHelper.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
        pppHelper.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (5)));

		for (uint32_t i = 0; i < tor_switch_num; i++)
			for (uint32_t j = 0; j < m_na / 2; j++)
				pppHelper.Install(NodeContainer(torswitchNodes.Get(i), terminalNodes.Get(i * m_na / 2 + j)));

		for (uint32_t i = 0; i < core_switch_num; i++)
			for (uint32_t j = 0; j < tor_switch_num; j++)
				pppHelper.Install(NodeContainer(coreswitchNodes.Get(i), torswitchNodes.Get(j)));
	}

	Ptr<Node> ClosTopologyHelper::GetNode(uint32_t i) const
	{
		if (i < terminalNodes.GetN())
			return terminalNodes.Get(i);
		i -= terminalNodes.GetN();

		if (i < torswitchNodes.GetN())
			return torswitchNodes.Get(i);
		i -= coreswitchNodes.GetN();

		if (i < coreswitchNodes.GetN())
			return coreswitchNodes.Get(i);

		return NULL;
	}

	NodeContainer ClosTopologyHelper::GetNodes() const
	{
		return NodeContainer(terminalNodes, torswitchNodes, coreswitchNodes);
	}

	uint32_t ClosTopologyHelper::GetNTerminalNodes() const
	{
		return terminalNodes.GetN();
	}

	Ipv4Address ClosTopologyHelper::GetTerminalInterface(int i) const
	{
		return Ipv4Address(i);
	}
}

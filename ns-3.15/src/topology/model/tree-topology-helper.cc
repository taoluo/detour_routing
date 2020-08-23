#include "tree-topology-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"

namespace ns3
{
	TypeId TreeTopologyHelper::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::TreeTopologyHelper")
			.SetParent<TopologyHelper>()
			.AddConstructor<TreeTopologyHelper>()
			.AddAttribute("N1", "The port number of upper level.",
						  UintegerValue(48), MakeUintegerAccessor(&TreeTopologyHelper::m_n1), MakeUintegerChecker<uint32_t>())
            .AddAttribute("N2", "The port number of upper level.",
						  UintegerValue(4), MakeUintegerAccessor(&TreeTopologyHelper::m_n2), MakeUintegerChecker<uint32_t>());
		return tid;
	}

	TypeId TreeTopologyHelper::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
	
	TreeTopologyHelper::TreeTopologyHelper ()
	{}

	TreeTopologyHelper::~TreeTopologyHelper ()
	{}

    void
    TreeTopologyHelper::SetLevel1DataRate(const DataRateValue &dataRate)
    {
        m_p2p1.SetDeviceAttribute ("DataRate", dataRate);
    }

    void
    TreeTopologyHelper::SetLevel1Delay(const TimeValue &delay)
    {
        m_p2p1.SetChannelAttribute ("Delay", delay);
    }

    void
    TreeTopologyHelper::SetLevel2DataRate(const DataRateValue &dataRate)
    {
        m_p2p2.SetDeviceAttribute ("DataRate", dataRate);
    }

    void
    TreeTopologyHelper::SetLevel2Delay(const TimeValue &delay)
    {
        m_p2p2.SetChannelAttribute ("Delay", delay);
    }

	void TreeTopologyHelper::CreateTopology()
	{
        int server_num = m_n1 * m_n2;
		int pot_switch_num = m_n2;
		int top_switch_num = 1;

		m_terminals.Create(server_num);
		m_potswitches.Create(pot_switch_num);
		m_topswitches.Create(top_switch_num);

		// create topology
		for(uint32_t i = 0; i < m_n2; i++)
        {
            // For each pot switch
            // First Create the link to servers
            for(uint32_t j = 0; j < m_n1; j++)
            {
                uint32_t terminalIndex = i * m_n1 + j;
                NetDeviceContainer link = m_p2p1.Install (NodeContainer (m_terminals.Get (terminalIndex), m_potswitches.Get (i)));
                m_terminalDevices.Add (link.Get (0));
                m_switchDevices.Add (link.Get (1));
            }
            // Then Create the link to the top switch
            NetDeviceContainer link = m_p2p2.Install (NodeContainer (m_potswitches.Get (i), m_topswitches.Get (0)));
            m_switchDevices.Add (link.Get (0));
            m_switchDevices.Add (link.Get (1));
        }
	}

	NodeContainer TreeTopologyHelper::GetNodes() const
	{
		return NodeContainer(m_terminals, m_potswitches, m_topswitches);
	}

    Ptr<Node> TreeTopologyHelper::GetNode(uint32_t i) const
    {
        return(GetNodes().Get(i));
    }

    uint32_t TreeTopologyHelper::GetNTerminalNodes() const
    {
        return(m_n1 * m_n2);
    }

    Ipv4Address TreeTopologyHelper::GetTerminalInterface(int i) const
    {
        return( Ipv4Address(i) );
    }
}

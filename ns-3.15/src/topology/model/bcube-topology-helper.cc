#include "bcube-topology-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("BCubeTopologyHelper");

// #define IsDump 1

namespace ns3
{

    NS_OBJECT_ENSURE_REGISTERED (BCubeTopologyHelper);

    TypeId
    BCubeTopologyHelper::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::BCubeTopologyHelper")
            .SetParent<TopologyHelper> ()
            .AddConstructor<BCubeTopologyHelper> ()
            ;
        return tid;
    }

	TypeId BCubeTopologyHelper::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

    BCubeTopologyHelper::BCubeTopologyHelper()
        :
        m_n (4),
        m_k (0)
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    BCubeTopologyHelper::~BCubeTopologyHelper()
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    void
    BCubeTopologyHelper::SetDataRate(const DataRateValue &dataRate)
    {
        m_p2p.SetDeviceAttribute ("DataRate", dataRate);
    }

    void
    BCubeTopologyHelper::SetDelay(const TimeValue &delay)
    {
        m_p2p.SetChannelAttribute ("Delay", delay);
    }

    void
    BCubeTopologyHelper::SetBCubeSize(const uint32_t &n, const uint32_t &k)
    {
        m_n = n;
        m_k = k;
    }

    NodeContainer
    BCubeTopologyHelper::GetTerminals()
    {
        return m_terminals;
    }

    NodeContainer
    BCubeTopologyHelper::GetSwitches()
    {
        return m_switches;
    }

    NodeContainer
    BCubeTopologyHelper::GetNodes() const
    {
        return NodeContainer( m_terminals, m_switches );
    }

    void
    BCubeTopologyHelper::CreateTopology()
    {
        uint32_t totalServer = uint32_t ( pow ( double(m_n), double(m_k+1) ) );
        m_terminals.Create( totalServer );
        uint32_t layerSwitch = uint32_t ( pow ( double(m_n), double(m_k) ) );
        m_switches.Create( layerSwitch * ( m_k + 1 ) );

#if IsDump
        std::cout << "Topology server = " << totalServer << " layer switch = " << layerSwitch << std::endl;
#endif

        // Construct links and assign IPs
        uint32_t layer, switchIndex, terminalCount;    // layer=[0,k], index=[0,layerSwitch-1]
        for ( layer = 0; layer <= m_k; layer++)
        {
            // For Every layer of switch, construct links and switches
            for ( switchIndex = 0; switchIndex < layerSwitch; switchIndex++)
            {
                // For every switch, construct
                NetDeviceContainer tempBridgeDevices;
                for ( terminalCount = 0; terminalCount < m_n; terminalCount++ )
                {
                    // std::cout << "layer=" << layer << " switch=" << switchIndex << " terminalCount=" << terminalCount;
                    // Construct ppp links
                    // Get Switch Bit
                    std::vector <uint32_t> bit = NumToBit(switchIndex);
                    // Insert 1 bit
                    bit.insert(bit.end()-layer, terminalCount);
                    // Get Terminal Num
                    uint32_t terminalIndex = BitToNum(bit);
                    NetDeviceContainer link = m_p2p.Install (NodeContainer (m_terminals.Get (terminalIndex), m_switches.Get ( layer * layerSwitch + switchIndex )));
                    m_terminalDevices.Add (link.Get (0));
                    m_switchDevices.Add (link.Get (1));
                }
            }
        }
        // Add internet stack to the terminals
        // BFSRoutingStackHelper stack;
        // stack.Install (m_terminals);
        // stack.Install (m_switches);
    }

    std::vector <uint32_t>
    BCubeTopologyHelper::NumToBit(const uint32_t &num)
    {
        // From larger to smaller
        // example: (35)4 = 203 = 2*4^2 + 0*4^1 + 3*4^0
        std::vector <uint32_t> bit;
        uint32_t x = num;
        uint32_t y;
        for( uint32_t i = 0; i < m_k; i++)
        {
            y = x % m_n;
            x = (x - y) / m_n;
            bit.insert(bit.begin(),y);
        }
        return bit;
    }

    uint32_t
    BCubeTopologyHelper::BitToNum(const std::vector <uint32_t> &bit)
    {
        uint32_t num = 0;
        for (uint32_t i = 0; i < bit.size(); i++)
        {
            num += bit[i] * uint32_t( pow ( double(m_n) , double( bit.size() - i - 1 ) ) );
        }
        return( num );
    }

	Ptr<Node> BCubeTopologyHelper::GetNode(uint32_t i) const
	{
		return GetNodes().Get(i);
	}
}// namespace ns3

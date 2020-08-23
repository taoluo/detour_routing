#include "topology-network.h"
#include "ns3/topology-helper.h"
#include "ns3/channel.h"

namespace ns3
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    TypeId TopNode::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TypeNode").SetParent<Object>().AddConstructor<TopNode>();
        return tid;
    }

    TypeId TopNode::GetInstanceTypeId (void) const
    {
        return TopNode::GetTypeId();
    }

    uint32_t TopNode::GetChildNumberInTree(uint32_t treeID) const
    {
        uint32_t num = 0;
        for (uint32_t i = 0; i < m_links.size(); i++)
        {
            Ptr<TopLink> link = m_links[i];
            if (link->GetTreeID() == treeID)
                num++;
        }

        return num;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    TypeId TopLink::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopLink").SetParent<Object>().AddConstructor<TopLink>();
        return tid;
    }

    TypeId TopLink::GetInstanceTypeId (void) const
    {
        return TopLink::GetTypeId();
    }

    TopLink::TopLink()
    {
        m_srcID = 0;
        m_destID = 0;

        m_treeID = LINK_TREE_ID_NONE;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    TypeId TopNetwork::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopNetwork").SetParent<Object>().AddConstructor<TopNetwork>();
        return tid;
    }

    TypeId TopNetwork::GetInstanceTypeId (void) const
    {
        return TopNetwork::GetTypeId();
    }

    void TopNetwork::Init(Ptr<TopologyHelper> topology_helper)
    {
        for (uint32_t i = 0; i < topology_helper->GetNNodes(); i++)
        {
            // insert the node into network
            Ptr<TopNode> toponode = CreateObject<TopNode>();
            toponode->m_id = i;

            m_nodes.push_back(toponode);

            // insert the links into the node
            Ptr<Node> node = topology_helper->GetNode(i);

            for (uint32_t j = 0; j < node->GetNDevices(); j++)
            {
                Ptr<NetDevice> dev = node->GetDevice(j);
                Ptr<Channel> channel = dev->GetChannel();

                uint32_t other_channel_id = 0;
                if (channel->GetDevice(0) == dev)
                    other_channel_id = 1;

                Ptr<NetDevice> peerdev = channel->GetDevice(other_channel_id);
                Ptr<Node> peernode = peerdev->GetNode();

                Ptr<TopLink> topolink = CreateObject<TopLink>();
                topolink->m_srcID = i;
                topolink->m_destID = peernode->GetId();

                toponode->m_links.push_back(topolink);
            }
        }
    }

    Ptr<TopNode> TopNetwork::AddNode()
    {
        Ptr<TopNode> node = CreateObject<TopNode>();
        m_nodes.push_back(node);
        return node;
    }

    Ptr<TopNode> TopNetwork::GetNode(uint32_t i)
    {
        return m_nodes[i];
    }

    uint32_t TopNetwork::GetNodeNumber() const
    {
        return m_nodes.size();
    }
}

#ifndef __TOPOLOGY_NETWORK__
#define __TOPOLOGY_NETWORK__

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/topology-helper.h"

#define LINK_TREE_ID_NONE (-1)

namespace ns3
{
    class TopNode;
    class TopLink;
    class TopNetwork;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class TopNode : public Object
    {
        friend class TopNetwork;

        std::vector< Ptr<TopLink> > m_links;
        uint32_t m_id;
    public:
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;

        uint32_t GetId() const { return m_id; }

        uint32_t GetLinkNumber() const { return m_links.size(); }

        uint32_t GetChildNumberInTree(uint32_t treeID) const;

        Ptr<TopLink> GetLink(uint32_t i) const { return m_links[i]; }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class TopLink : public Object
    {
        friend class TopNetwork;

        uint32_t m_srcID;
        uint32_t m_destID;

        uint32_t m_treeID; // the ID of the tree which the link is belonged to
    public:
        TopLink();

        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;

        uint32_t GetSrcID() const { return m_srcID; }
        uint32_t GetDestID() const { return m_destID; }

        uint32_t SetTreeID(uint32_t treeID)
        {
            uint32_t oldTreeID = m_treeID;
            m_treeID = treeID;
            return oldTreeID;
        }

        uint32_t GetTreeID() const
        {
            return m_treeID;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class TopNetwork : public Object
    {
        std::vector< Ptr<TopNode> > m_nodes;

    public:
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;

        // copy network from ns3 structures
        void Init(Ptr<TopologyHelper> topology_helper);

        Ptr<TopNode> AddNode();
        Ptr<TopNode> GetNode(uint32_t i);
        uint32_t GetNodeNumber() const;
    };
}

#endif

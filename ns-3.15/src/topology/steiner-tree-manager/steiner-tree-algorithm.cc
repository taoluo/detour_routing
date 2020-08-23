#include "steiner-tree-algorithm.h"
#include "ns3/topology-network.h"
#include <queue>
#include <set>

#include "stdio.h"

namespace ns3
{

    SteinerTreeAlgorithm::~SteinerTreeAlgorithm()
    {}

    uint32_t BFSSteinerTreeAlgorithm::CreateSteinerTrees(Ptr<TopNetwork> network, uint32_t root, const std::set<uint32_t>& receivers)
    {
        uint32_t treeNum = CreateSpanningTrees(network, root);
        PruneSpanningTrees(network, treeNum, root, receivers);
        return treeNum;
    }

    uint32_t BFSSteinerTreeAlgorithm::CreateSpanningTrees(Ptr<TopNetwork> network, uint32_t root)
    {
        std::set<uint32_t> traversed_nodes;
        std::queue<uint32_t> q;
        q.push(root);
        traversed_nodes.insert(root);

        while (q.empty() == false)
        {            
            uint32_t id = q.front();
            q.pop();

            Ptr<TopNode> node = network->GetNode(id);

            for (uint32_t i = 0; i < node->GetLinkNumber(); i++)
            {
                Ptr<TopLink> link = node->GetLink(i);

                if (traversed_nodes.find(link->GetDestID()) != traversed_nodes.end())
                    continue;
                traversed_nodes.insert(link->GetDestID());

                link->SetTreeID(1);
                q.push(link->GetDestID());
            }
        }
        
        return 1;
    }

    bool BFSSteinerTreeAlgorithm::PruneSpanningTrees(Ptr<TopNetwork> network, uint32_t treeID, uint32_t nodeID, const std::set<uint32_t>& receivers)
    {
        Ptr<TopNode> node = network->GetNode(nodeID);

        bool res = false;
        for (uint32_t i = 0; i < node->GetLinkNumber(); i++)
        {
            Ptr<TopLink> link = node->GetLink(i);
            
            if (link->GetTreeID() != treeID)
                continue;

            if (PruneSpanningTrees(network, treeID, link->GetDestID(), receivers) == false)
                link->SetTreeID(LINK_TREE_ID_NONE);
            else
                res = true;
        }

        if (res == false && receivers.find(nodeID) != receivers.end())
            res = true;
        
        return res;
    }
}

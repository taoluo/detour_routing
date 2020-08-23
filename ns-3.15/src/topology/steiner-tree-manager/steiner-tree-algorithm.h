#ifndef __STEINER_TREE_ALGORITHM__
#define __STEINER_TREE_ALGORITHM__

#include "stdint.h"
#include "ns3/ptr.h"
#include <set>

namespace ns3
{
    class TopNetwork;
    
    class SteinerTreeAlgorithm
    {        
    public:
        // Return the number of trees created.
        virtual ~SteinerTreeAlgorithm();
        virtual uint32_t CreateSteinerTrees(Ptr<TopNetwork> network, uint32_t root, const std::set<uint32_t>& receivers) = 0;
    };

    class BFSSteinerTreeAlgorithm : public SteinerTreeAlgorithm
    {
        uint32_t CreateSpanningTrees(Ptr<TopNetwork> network, uint32_t root);

        // Return whether the subtree contains one of the receivers. 
        bool PruneSpanningTrees(Ptr<TopNetwork> network, uint32_t treeID, uint32_t root, const std::set<uint32_t>& receivers);
        
    public:
        virtual uint32_t CreateSteinerTrees(Ptr<TopNetwork> network, uint32_t root, const std::set<uint32_t>& receivers);
    };
}

#endif

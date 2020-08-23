#ifndef __STEINER_TREE_MANAGER__
#define __STEINER_TREE_MANAGER__

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/topology-network.h"
#include <set>

#define STEINER_TREE_ALGORITHM_BFS 0

namespace ns3
{
    class TopNetwork;
    
    class SteinerTreeManager : public Object
    {
    public:
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;

        SteinerTreeManager();
        ~SteinerTreeManager();

        bool CreateSteinerTrees(Ptr<TopNetwork> network, uint32_t alg, uint32_t root, const std::set<uint32_t>& receivers);
    };
}

#endif

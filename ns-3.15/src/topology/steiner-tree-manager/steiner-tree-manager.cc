#include "steiner-tree-manager.h"
#include "ns3/steiner-tree-algorithm.h"
#include "ns3/topology-network.h"

namespace ns3
{
    TypeId SteinerTreeManager::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::SteinerTreeManager").SetParent<Object>().AddConstructor<SteinerTreeManager>();
        return tid;
    }

    TypeId SteinerTreeManager::GetInstanceTypeId (void) const
    {
        return SteinerTreeManager::GetTypeId();
    }

    SteinerTreeManager::SteinerTreeManager()
    {}

    SteinerTreeManager::~SteinerTreeManager()
    {}

    bool SteinerTreeManager::CreateSteinerTrees(Ptr<TopNetwork> network, uint32_t alg, uint32_t root, const std::set<uint32_t>& receivers)
    {
        SteinerTreeAlgorithm* algorithm = NULL;
        
        if (alg == STEINER_TREE_ALGORITHM_BFS)
            algorithm = new BFSSteinerTreeAlgorithm();

        if (algorithm == NULL)
            return false;

        algorithm->CreateSteinerTrees(network, root, receivers);

        delete algorithm;
        return true;
    }
}

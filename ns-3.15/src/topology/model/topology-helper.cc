#include "topology-helper.h"

namespace ns3
{
    TypeId TopologyHelper::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopologyHelper").SetParent<Object>();
        return tid;
    }

    TopologyHelper::TopologyHelper()
    {}

    TopologyHelper::~TopologyHelper()
    {}

    NetDeviceContainer TopologyHelper::GetNetDevices() const
    {
        NS_ASSERT_MSG(false, "TopologyHelper::GetNetDevice unimplemented exception!\n");
        return NetDeviceContainer();
    }

    uint32_t TopologyHelper::GetNNodes() const
    {
        return GetNodes().GetN();
    }
}

#ifndef __TOPOLOGY_HELPER__
#define __TOPOLOGY_HELPER__

#include "ns3/object.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"

namespace ns3
{
    class Node;
    class TopologyHelper : public Object
    {
    public:
        static TypeId GetTypeId (void);

        TopologyHelper();
        ~TopologyHelper();

        virtual void CreateTopology() = 0;
        virtual Ptr<Node> GetNode(uint32_t i) const = 0;
        virtual uint32_t GetNNodes() const;
        virtual NodeContainer GetNodes() const = 0;
        virtual NetDeviceContainer GetNetDevices() const;

        virtual NodeContainer GetTerminalNodes() const = 0;
        virtual NodeContainer GetNonTerminalNodes() const = 0;

        virtual uint32_t GetNTerminalNodes() const = 0;
        virtual Ipv4Address GetTerminalInterface(int i) const = 0;
    };
}

#endif

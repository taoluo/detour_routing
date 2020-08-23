#ifndef __BFS_ROUTING_STACK_HELPER__
#define __BFS_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class BFSRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		BFSRoutingStackHelper();
		~BFSRoutingStackHelper();
	};
}

#endif

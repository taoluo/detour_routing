#ifndef __CLOS_ROUTING_STACK_HELPER__
#define __CLOS_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class ClosRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		ClosRoutingStackHelper();
		~ClosRoutingStackHelper();
	};
}

#endif

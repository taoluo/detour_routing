#ifndef __VL2_ROUTING_STACK_HELPER__
#define __VL2_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class VL2RoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		VL2RoutingStackHelper();
		~VL2RoutingStackHelper();
	};
}

#endif

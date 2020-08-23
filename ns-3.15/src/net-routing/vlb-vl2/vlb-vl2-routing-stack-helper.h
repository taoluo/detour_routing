#ifndef __VLB_VL2_ROUTING_STACK_HELPER__
#define __VLB_VL2_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class VLBVL2RoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		VLBVL2RoutingStackHelper();
		~VLBVL2RoutingStackHelper();
	};
}

#endif 

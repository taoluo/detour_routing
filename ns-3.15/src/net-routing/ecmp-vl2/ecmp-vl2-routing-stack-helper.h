#ifndef __ECMP_VL2_ROUTING_STACK_HELPER__
#define __ECMP_VL2_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class ECMPVL2RoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		ECMPVL2RoutingStackHelper();
		~ECMPVL2RoutingStackHelper();
	};
}

#endif 

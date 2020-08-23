#ifndef __ECMP_ROUTING_STACK_HELPER__
#define __ECMP_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class ECMPRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		ECMPRoutingStackHelper();
		~ECMPRoutingStackHelper();
	};
}

#endif

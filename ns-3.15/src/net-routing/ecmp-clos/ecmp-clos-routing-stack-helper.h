#ifndef __ECMP_CLOS_ROUTING_STACK_HELPER__
#define __ECMP_CLOS_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class ECMPClosRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		ECMPClosRoutingStackHelper();
		~ECMPClosRoutingStackHelper();
	};
}

#endif 

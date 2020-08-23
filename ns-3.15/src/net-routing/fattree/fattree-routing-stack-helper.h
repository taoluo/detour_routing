#ifndef __FATTREE_ROUTING_STACK_HELPER__
#define __FATTREE_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class FattreeRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
        FattreeRoutingStackHelper();
		~FattreeRoutingStackHelper();

		void SetPortNumber(uint32_t n);
		void SetRoutingAlgorithm(uint32_t alg);
	};
}

#endif

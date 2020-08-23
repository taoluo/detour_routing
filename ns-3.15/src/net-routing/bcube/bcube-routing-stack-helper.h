#ifndef __BCUBE_ROUTING_STACK_HELPER__
#define __BCUBE_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class BCubeRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		BCubeRoutingStackHelper();
		~BCubeRoutingStackHelper();

		void SetBCube(uint32_t n, uint32_t k);
	};
}

#endif

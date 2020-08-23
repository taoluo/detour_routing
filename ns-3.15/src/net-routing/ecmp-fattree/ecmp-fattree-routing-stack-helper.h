#ifndef __ECMP_FATTREE_ROUTING_STACK_HELPER__
#define __ECMP_FATTREE_ROUTING_STACK_HELPER__

#include "ns3/net-routing-stack-helper.h"

namespace ns3
{
	class ECMPFattreeRoutingStackHelper : public NetRoutingStackHelper
	{
	public:
		ECMPFattreeRoutingStackHelper();
		~ECMPFattreeRoutingStackHelper();

		void SetPortNumber(uint32_t n);
	};
}

#endif 

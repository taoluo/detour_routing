#include "ns3/vl2-routing-stack-helper.h"
#include "ns3/net-routing-helper.h"
#include "ns3/ipv4.h"
#include "ns3/net-routing.h"

namespace ns3
{
	VL2RoutingStackHelper::VL2RoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::VL2RoutingProtocol");		
	}

	VL2RoutingStackHelper::~VL2RoutingStackHelper()
	{}
}

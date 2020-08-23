#include "ns3/vlb-vl2-routing-stack-helper.h"
#include "ns3/uinteger.h"

namespace ns3
{
	VLBVL2RoutingStackHelper::VLBVL2RoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::VLBVL2RoutingProtocol");
	}

	VLBVL2RoutingStackHelper::~VLBVL2RoutingStackHelper()
	{}
}

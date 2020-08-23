#include "ns3/ecmp-vl2-routing-stack-helper.h"
#include "ns3/uinteger.h"

namespace ns3
{
	ECMPVL2RoutingStackHelper::ECMPVL2RoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::ECMPVL2RoutingProtocol");
	}

	ECMPVL2RoutingStackHelper::~ECMPVL2RoutingStackHelper()
	{}
}

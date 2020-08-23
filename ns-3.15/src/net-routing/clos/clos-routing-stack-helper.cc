#include "ns3/clos-routing-stack-helper.h"
#include "ns3/net-routing-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"

namespace ns3
{
	ClosRoutingStackHelper::ClosRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::ClosRoutingProtocol");
	}
	
	ClosRoutingStackHelper::~ClosRoutingStackHelper()
	{}
}

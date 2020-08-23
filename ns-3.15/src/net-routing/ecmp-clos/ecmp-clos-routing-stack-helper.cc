#include "ns3/ecmp-clos-routing-stack-helper.h"
#include "ns3/uinteger.h"

namespace ns3
{
	ECMPClosRoutingStackHelper::ECMPClosRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::ECMPClosRoutingProtocol");
	}

	ECMPClosRoutingStackHelper::~ECMPClosRoutingStackHelper()
	{}
}

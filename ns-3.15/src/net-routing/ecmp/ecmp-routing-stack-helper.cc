#include "ns3/ecmp-routing-stack-helper.h"
#include "ns3/net-routing-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"
namespace ns3
{
	ECMPRoutingStackHelper::ECMPRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::ECMPRoutingProtocol");
	}
	
	ECMPRoutingStackHelper::~ECMPRoutingStackHelper()
	{}
}

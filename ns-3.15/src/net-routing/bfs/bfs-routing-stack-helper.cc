#include "ns3/bfs-routing-stack-helper.h"
#include "ns3/net-routing-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"
namespace ns3
{
	BFSRoutingStackHelper::BFSRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::BFSRoutingProtocol");
	}
	
	BFSRoutingStackHelper::~BFSRoutingStackHelper()
	{}
}

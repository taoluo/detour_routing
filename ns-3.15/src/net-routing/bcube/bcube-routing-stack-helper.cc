#include "ns3/bcube-routing-stack-helper.h"
#include "ns3/net-routing-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"
namespace ns3
{
	BCubeRoutingStackHelper::BCubeRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::BCubeRoutingProtocol");
	}
	
	BCubeRoutingStackHelper::~BCubeRoutingStackHelper()
	{}

	void BCubeRoutingStackHelper::SetBCube(uint32_t n, uint32_t k)
	{
		GetRoutingFactory().Set("BCube_n", UintegerValue(n));
        GetRoutingFactory().Set("BCube_k", UintegerValue(k));
	}
}

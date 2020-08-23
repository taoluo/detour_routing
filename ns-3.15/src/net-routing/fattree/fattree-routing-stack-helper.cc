#include "ns3/fattree-routing-stack-helper.h"
#include "ns3/net-routing-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"

namespace ns3
{
    FattreeRoutingStackHelper::FattreeRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::FattreeRoutingProtocol");
	}
	
	FattreeRoutingStackHelper::~FattreeRoutingStackHelper()
	{}

	void FattreeRoutingStackHelper::SetPortNumber(uint32_t n)
	{
		GetRoutingFactory().Set("N", UintegerValue(n));
	}
	
	void FattreeRoutingStackHelper::SetRoutingAlgorithm(uint32_t alg)
	{
		GetRoutingFactory().Set("RoutingAlgorithm", UintegerValue(alg));
	}
}

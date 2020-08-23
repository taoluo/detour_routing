#include "ns3/ecmp-fattree-routing-stack-helper.h"
#include "ns3/uinteger.h"

namespace ns3
{
	ECMPFattreeRoutingStackHelper::ECMPFattreeRoutingStackHelper()
	{
		GetRoutingFactory().SetTypeId ("ns3::ECMPFattreeRoutingProtocol");
	}

	ECMPFattreeRoutingStackHelper::~ECMPFattreeRoutingStackHelper()
	{}

	void ECMPFattreeRoutingStackHelper::SetPortNumber(uint32_t n)
	{
		GetRoutingFactory().Set("N", UintegerValue(n));
	}
}

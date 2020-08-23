#include "net-header.h"

namespace ns3
{
	NetHeader::NetHeader()
	{}

	NetHeader::~NetHeader()
	{}

	TypeId NetHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::NetHeader").SetParent<Header>();
		return tid;
	}
	
	TypeId NetHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
}

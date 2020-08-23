#include "fattree-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (FattreeHeader);

	TypeId FattreeHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::FattreeHeader").SetParent<NetHeader>().AddConstructor<FattreeHeader>();
		return tid;
	}
	
	TypeId FattreeHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
	
	FattreeHeader::FattreeHeader()
	{
		src = dest = 0;
		topSwitchId = 0;
		protocol = 0;
		ltime = 0;
	}
	
	FattreeHeader::~FattreeHeader()
	{}

	void FattreeHeader::Print (std::ostream &os) const
	{}
	
	uint32_t FattreeHeader::GetSerializedSize (void) const
	{
		return 3 * 4;
	}
	
	void FattreeHeader::Serialize (Buffer::Iterator start) const
	{
		Buffer::Iterator i = start;

		i.WriteU8(protocol);
		i.WriteU8(ltime);
		i.WriteU16(topSwitchId);
		i.WriteU32(src);
		i.WriteU32(dest);
	}
	
	uint32_t FattreeHeader::Deserialize (Buffer::Iterator start)
	{
		Buffer::Iterator i = start;

		protocol	= i.ReadU8();
		ltime		= i.ReadU8();
		topSwitchId = i.ReadU16();
		src			= i.ReadU32();
		dest		= i.ReadU32();		

		return GetSerializedSize();
	}
}

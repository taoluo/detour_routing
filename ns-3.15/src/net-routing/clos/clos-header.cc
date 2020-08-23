#include "ns3/clos-header.h"

namespace ns3
{
	TypeId ClosHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::ClosHeader").SetParent<NetHeader>().AddConstructor<ClosHeader>();
		return tid;
	}
	
	TypeId ClosHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	ClosHeader::ClosHeader()
	{
		src = 0;
		dest = 0;
		protocol = 0;
		ltime = 0;
		timestamp = 0;
	}
	
	ClosHeader::~ClosHeader()
	{}

	void ClosHeader::Print (std::ostream &os) const
	{}
	
	uint32_t ClosHeader::GetSerializedSize (void) const
	{
		return 18;
	}
	
	void ClosHeader::Serialize (Buffer::Iterator start) const
	{
		start.WriteU32(src);
		start.WriteU32(dest);
		start.WriteU32(core);
		start.WriteU32(timestamp);
		start.WriteU8(protocol);
		start.WriteU8(ltime);
	}
	
	uint32_t ClosHeader::Deserialize (Buffer::Iterator start)
	{
		src		  = start.ReadU32();
		dest	  = start.ReadU32();
		core	  = start.ReadU32();
		timestamp = start.ReadU32();
		protocol  = start.ReadU8();
		ltime	  = start.ReadU8();

		return GetSerializedSize();
	}	
}

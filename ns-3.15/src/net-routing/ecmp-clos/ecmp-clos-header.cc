#include "ns3/ecmp-clos-header.h"

namespace ns3
{
	TypeId ECMPClosHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::ECMPClosHeader").SetParent<NetHeader>().AddConstructor<ECMPClosHeader>();
		return tid;
	}
	
	TypeId ECMPClosHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	ECMPClosHeader::ECMPClosHeader()
	{
		src = 0;
		dest = 0;
		protocol = 0;
		ltime = 0;
		// timestamp = 0;
	}
	
	ECMPClosHeader::~ECMPClosHeader()
	{}

	void ECMPClosHeader::Print (std::ostream &os) const
	{}
	
	uint32_t ECMPClosHeader::GetSerializedSize (void) const
	{
		return 10;
	}
	
	void ECMPClosHeader::Serialize (Buffer::Iterator start) const
	{
		start.WriteU32(src);
		start.WriteU32(dest);
		// start.WriteU32(timestamp);
		start.WriteU8(protocol);
		start.WriteU8(ltime);
	}
	
	uint32_t ECMPClosHeader::Deserialize (Buffer::Iterator start)
	{
		src		  = start.ReadU32();
		dest	  = start.ReadU32();
		// timestamp = start.ReadU32();
		protocol  = start.ReadU8();
		ltime	  = start.ReadU8();

		return GetSerializedSize();
	}	
}

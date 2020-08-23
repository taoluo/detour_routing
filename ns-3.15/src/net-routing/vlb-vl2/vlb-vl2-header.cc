#include "ns3/vlb-vl2-header.h"

namespace ns3
{
	TypeId VLBVL2Header::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::VLBVL2Header").SetParent<NetHeader>().AddConstructor<VLBVL2Header>();
		return tid;
	}
	
	TypeId VLBVL2Header::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	VLBVL2Header::VLBVL2Header()
	{
		src = 0;
		dest = 0;
		protocol = 0;
		ltime = 0;
        core_index = 0;
        topdown_index = 0;
	}
	
	VLBVL2Header::~VLBVL2Header()
	{}

	void VLBVL2Header::Print (std::ostream &os) const
	{}
	
	uint32_t VLBVL2Header::GetSerializedSize (void) const
	{
		return 18;
	}
	
	void VLBVL2Header::Serialize (Buffer::Iterator start) const
	{
		start.WriteU32(src);
		start.WriteU32(dest);
		start.WriteU8(protocol);
		start.WriteU8(ltime);
        start.WriteU32(core_index);
        start.WriteU32(topdown_index);
	}
	
	uint32_t VLBVL2Header::Deserialize (Buffer::Iterator start)
	{
		src		  = start.ReadU32();
		dest	  = start.ReadU32();
		protocol  = start.ReadU8();
		ltime	  = start.ReadU8();
        core_index = start.ReadU32();
        topdown_index = start.ReadU32();

		return GetSerializedSize();
	}
}

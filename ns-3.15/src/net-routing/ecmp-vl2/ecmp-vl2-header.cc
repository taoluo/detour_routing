#include "ns3/ecmp-vl2-header.h"

#define ECMP_VL2_PATH_MAX_LENGTH 6

namespace ns3
{
	TypeId ECMPVL2Header::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::ECMPVL2Header").SetParent<NetHeader>().AddConstructor<ECMPVL2Header>();
		return tid;
	}
	
	TypeId ECMPVL2Header::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	ECMPVL2Header::ECMPVL2Header()
	{
		src = 0;
		dest = 0;
		protocol = 0;
		ltime = 0;
        // timestamp = 0;
	}
	
	ECMPVL2Header::~ECMPVL2Header()
	{}

	void ECMPVL2Header::Print (std::ostream &os) const
	{}
	
	uint32_t ECMPVL2Header::GetSerializedSize (void) const
	{
		return 16;
	}
	
	void ECMPVL2Header::Serialize (Buffer::Iterator start) const
	{
		start.WriteU32(src);
		start.WriteU32(dest);
		// start.WriteU32(timestamp);
		start.WriteU8(protocol);
		start.WriteU8(ltime);
	}
	
	uint32_t ECMPVL2Header::Deserialize (Buffer::Iterator start)
	{
		src		  = start.ReadU32();
		dest	  = start.ReadU32();
		// timestamp = start.ReadU32();
		protocol  = start.ReadU8();
		ltime	  = start.ReadU8();

		return GetSerializedSize();
	}
}

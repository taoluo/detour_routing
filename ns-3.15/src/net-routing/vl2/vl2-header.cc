#include "ns3/vl2-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (VL2Header);

	TypeId VL2Header::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::VL2Header").SetParent<NetHeader>().AddConstructor<VL2Header>();
		return tid;
	}
	
	TypeId VL2Header::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	VL2Header::VL2Header()
	{
		srcId = destId = 0;
		memset((void*)ports, 0, VL2_PATH_MAX_LENGTH);
		protocol = 0;
		ltime = 0;
	}

	VL2Header::~VL2Header()
	{}

	void VL2Header::Print (std::ostream &os) const
	{}
	
	uint32_t VL2Header::GetSerializedSize (void) const
	{
		return 16;
	}
	
	void VL2Header::Serialize (Buffer::Iterator start) const
	{
		start.WriteU32(srcId);
		start.WriteU32(destId);
		start.WriteU8(protocol);
		start.WriteU8(ltime);

		for (uint32_t i = 0; i < VL2_PATH_MAX_LENGTH; i++)
			start.WriteU8(ports[i]);
	}
	
	uint32_t VL2Header::Deserialize (Buffer::Iterator start)
	{
		srcId = start.ReadU32();
		destId = start.ReadU32();
		protocol = start.ReadU8();
		ltime = start.ReadU8();

		for (uint32_t i = 0; i < VL2_PATH_MAX_LENGTH; i++)
			ports[i] = start.ReadU8();
		
		return GetSerializedSize();
	}
}

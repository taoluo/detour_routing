#include "ns3/ecmp-fattree-header.h"

namespace ns3
{
	TypeId ECMPFattreeHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::ECMPFattreeHeader").SetParent<NetHeader>().AddConstructor<ECMPFattreeHeader>();
		return tid;
	}
	
	TypeId ECMPFattreeHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	ECMPFattreeHeader::ECMPFattreeHeader()
	{
		src = 0;
		dest = 0;
		sport = 0;
		dport = 0;
		protocol = 0;
		ltime = 0;
		// timestamp = 0;
	}
	
	ECMPFattreeHeader::~ECMPFattreeHeader()
	{}

	void ECMPFattreeHeader::Print (std::ostream &os) const
	{}
	
	uint32_t ECMPFattreeHeader::GetSerializedSize (void) const
	{
		return 14;
	}
	
	void ECMPFattreeHeader::Serialize (Buffer::Iterator start) const
	{
		start.WriteU32(src);
		start.WriteU32(dest);
		start.WriteU16(sport);
		start.WriteU16(dport);
		// start.WriteU32(timestamp);
		start.WriteU8(protocol);
		start.WriteU8(ltime);
	}
	
	uint32_t ECMPFattreeHeader::Deserialize (Buffer::Iterator start)
	{
		src		  = start.ReadU32();
		dest	  = start.ReadU32();
		sport    =  start.ReadU16();
		dport     =  start.ReadU16();
		// timestamp = start.ReadU32();
		protocol  = start.ReadU8();
		ltime	  = start.ReadU8();

		return GetSerializedSize();
	}
}

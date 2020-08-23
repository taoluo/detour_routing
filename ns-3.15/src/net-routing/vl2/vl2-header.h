#ifndef __VL2_HEADER__
#define __VL2_HEADER__

#include "ns3/net-header.h"

#define VL2_PATH_MAX_LENGTH 6

namespace ns3
{
	class VL2Header : public NetHeader
	{
	public:
		uint32_t srcId;
		uint32_t destId;
		uint8_t protocol;
		uint8_t ltime; // Living Time
		uint8_t ports[VL2_PATH_MAX_LENGTH];
		
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		VL2Header();
		~VL2Header();

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);
	};
}

#endif

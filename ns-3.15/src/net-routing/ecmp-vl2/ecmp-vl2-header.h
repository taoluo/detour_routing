#ifndef __ECMP_VL2_HEADER__
#define __ECMP_VL2_HEADER__

#include "ns3/net-header.h"

namespace ns3
{
	class ECMPVL2Header : public NetHeader
	{
		friend class ECMPVL2RoutingProtocol;
		
		uint32_t src;
		uint32_t dest;
		uint8_t protocol;
		uint8_t ltime;

		// uint32_t timestamp; // in microseconds

	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		ECMPVL2Header();
		~ECMPVL2Header();

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);
	};
}

#endif

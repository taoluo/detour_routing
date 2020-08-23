#ifndef __VLB_VL2_HEADER__
#define __VLB_VL2_HEADER__

#include "ns3/net-header.h"

namespace ns3
{
	class VLBVL2Header : public NetHeader
	{
		friend class VLBVL2RoutingProtocol;
		
		uint32_t src;
		uint32_t dest;
		uint8_t protocol;
		uint8_t ltime;

        // for VLB routing switch selection
        uint32_t core_index;
        uint32_t topdown_index;

	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		VLBVL2Header();
		~VLBVL2Header();

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);
	};
}

#endif

#ifndef __CLOS_HEADER__
#define __CLOS_HEADER__

#include "ns3/net-header.h"

namespace ns3
{
	class ClosHeader : public NetHeader
	{
		friend class ClosRoutingProtocol;
		
		uint32_t src;
		uint32_t dest;
		uint32_t core;
		uint8_t protocol;
		uint8_t ltime;

		uint32_t timestamp; // in microseconds

	public:
		static TypeId GetTypeId (void);
   		virtual TypeId GetInstanceTypeId (void) const;

		ClosHeader();
 		~ClosHeader();

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);
	};
}

#endif

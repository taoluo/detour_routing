#ifndef __ECMP_FATTREE_HEADER__
#define __ECMP_FATTREE_HEADER__

#include "ns3/net-header.h"

namespace ns3
{
	class ECMPFattreeHeader : public NetHeader
	{
		friend class ECMPFattreeRoutingProtocol;
		
		uint32_t src;
		uint32_t dest;
		uint16_t sport;
		uint16_t dport;
		uint8_t protocol;
		uint8_t ltime;

		// uint32_t timestamp; // in microseconds

	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		ECMPFattreeHeader();
		~ECMPFattreeHeader();

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);
	};
}

#endif

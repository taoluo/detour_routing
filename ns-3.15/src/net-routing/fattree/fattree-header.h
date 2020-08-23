#ifndef __FATTREE_HEADER__
#define __FATTREE_HEADER__

#include "ns3/net-header.h"

namespace ns3
{
	class FattreeHeader : public NetHeader
	{
		uint32_t src;
		uint32_t dest;
		uint16_t topSwitchId;
		uint8_t protocol;
		uint8_t ltime; // Living Time
		
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		FattreeHeader();
		~FattreeHeader();

		uint32_t GetSrc() const { return src; }
		uint32_t GetDest() const { return dest; }
		uint16_t GetTopSwitchId() const { return topSwitchId; }
		uint8_t GetProtocol() const { return protocol; }
		uint8_t GetLivingTime() const { return ltime; }

		void SetSrc(uint32_t _src) { src = _src; }
		void SetDest(uint32_t _dest) { dest = _dest; }
		void SetTopSwitchId(uint16_t _topSwitchId) { topSwitchId = _topSwitchId; }
		void SetProtocol(uint8_t _protocol) { protocol = _protocol; }
		void SetLivingTime(uint8_t _ltime) { ltime = _ltime; }

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);		
	};
}

#endif

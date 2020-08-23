#ifndef __NET_HEADER__
#define __NET_HEADER__

#include "ns3/header.h"

namespace ns3
{
	class NetHeader : public Header
	{
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		
		NetHeader();
		~NetHeader();
	};
}

#endif

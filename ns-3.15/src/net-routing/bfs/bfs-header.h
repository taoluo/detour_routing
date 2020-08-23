#ifndef __BFS_HEADER__
#define __BFS_HEADER__

#include "ns3/net-header.h"
#include <vector>

namespace ns3
{
	class BFSHeader : public NetHeader
	{
		uint8_t m_protocol;
		uint8_t m_ltime; // Living Time
        uint32_t m_src;
		uint32_t m_dest;
        uint16_t m_length;
        std::vector< uint32_t > m_path;
		
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		BFSHeader();
		~BFSHeader();

		uint32_t GetSrc() const { return m_src; }
		uint32_t GetDest() const { return m_dest; }
        uint8_t GetProtocol() const { return m_protocol; }
		uint8_t GetLivingTime() const { return m_ltime; }
        const std::vector< uint32_t >& GetPath() const { return m_path; }
        uint32_t GetNextId() const { return m_path[m_ltime]; }

        void SetSrc(uint32_t src) { m_src = src; }
		void SetDest(uint32_t dest) { m_dest = dest; }
		void SetProtocol(uint8_t protocol) { m_protocol = protocol; }
		void SetLivingTime(uint8_t ltime) { m_ltime = ltime; }
        void SetPath(const std::vector< uint32_t >& path) { m_path = path; }
      
        void Dump() const;

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);		
	};
}

#endif

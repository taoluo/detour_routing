#ifndef __ECMP_HEADER__
#define __ECMP_HEADER__

#include "ns3/net-header.h"
#include <vector>

namespace ns3
{
	class ECMPHeader : public NetHeader
	{
	public:
		uint8_t m_protocol;
		uint8_t m_ltime; // Living Time
		uint8_t m_srl;   //source route live
		uint32_t m_prio;
		uint8_t m_ecn;  //0 : disable, 1: enable, 2: marked
		uint32_t m_src;
		uint32_t m_dest;
		uint16_t m_sport;
		uint16_t m_dport;
		uint32_t m_length;
        uint32_t m_sendport;
		uint32_t m_detourcount;

		std::vector<uint32_t> m_path;
		
		enum {ECN_DIS = 0, ECN_EN = 1, ECN_MARK = 2};
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		ECMPHeader();
		~ECMPHeader();

		uint32_t GetSrc() const { return m_src; }
		uint32_t GetDest() const { return m_dest; }
		uint16_t GetSport() const {return m_sport; }
		uint16_t GetDport() const {return m_dport; }
		uint8_t GetProtocol() const { return m_protocol; }
		uint8_t GetLivingTime() const { return m_ltime; }
		uint8_t GetSrl() const {return m_srl;}
		std::vector<uint32_t>* GetPath() { return &m_path; }
		uint32_t GetNextId() const { return m_path[m_srl]; }
		uint32_t GetPrio() const {return m_prio; }
		uint8_t GetEcn() const {return m_ecn;}
        uint32_t GetSendport() const {return m_sendport;}
		uint32_t GetDetourCount() const { return m_detourcount; }

		void SetSrc(uint32_t src) { m_src = src; }
		void SetDest(uint32_t dest) { m_dest = dest; }
		void SetSport(uint16_t sport) {m_sport = sport; }
		void SetDport(uint16_t dport) {m_dport = dport;}
		void SetProtocol(uint8_t protocol) { m_protocol = protocol; }
		void SetLivingTime(uint8_t ltime) { m_ltime = ltime; }
		void SetSrl(uint8_t srl) {m_srl = srl;}
		void SetPath(const std::vector< uint32_t >& path) { m_path = path; }
		void SetPrio(uint32_t prio) {m_prio = prio; }
		void SetEcn( uint8_t ecn) {m_ecn = ecn; }
        void SetSendport (uint32_t port) {m_sendport = port; }
		void SetDetourCount(uint32_t detourcount) { m_detourcount = detourcount; }
		void Dump() const;

		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);		
	};
}

#endif

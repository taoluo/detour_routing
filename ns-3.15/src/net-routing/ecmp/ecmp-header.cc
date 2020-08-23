#include "ecmp-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ECMPHeader);

	TypeId ECMPHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::ECMPHeader").SetParent<NetHeader>().AddConstructor<ECMPHeader>();
		return tid;		
	}
	
	TypeId ECMPHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
	
	ECMPHeader::ECMPHeader()
	{
	  m_ecn = ECN_DIS;
	  m_prio = 0;
	  m_src = 0;
	  m_dest = 0;
	  m_sport = 0;
	  m_dport = 0;
	  m_protocol = 0;
	  m_ltime = 0;
	  m_detourcount = 0;
	  m_srl = 0;
      m_sendport = 0;
	}
	
	ECMPHeader::~ECMPHeader()
	{}

	void ECMPHeader::Print (std::ostream &os) const
	{}

    void ECMPHeader::Dump() const
    {
        std::cout << "Protocol=" << (uint32_t)m_protocol << " Source=" << m_src << " Dest=" << m_dest << " path=";
        for( uint32_t i = 0; i < m_path.size(); i++ )
            std::cout << m_path[i] << " ";
        std::cout << "livingTime=" << (uint32_t)m_ltime;
	std::cout << "Detour Count=" << (uint32_t)m_detourcount;
        std::cout << std::endl;
    }
	
	uint32_t ECMPHeader::GetSerializedSize (void) const
	{
	  return ( 24 + 4 + 4 + (4*m_path.size())); // mattc 12/21/2012
	}
	
	void ECMPHeader::Serialize (Buffer::Iterator start) const
	{
		Buffer::Iterator i = start;

		i.WriteU8(m_protocol);
		i.WriteU8(m_ltime);
		i.WriteU8(m_srl);
		i.WriteU32(m_prio);
		i.WriteU8(m_ecn);
		i.WriteU32(m_src);
		i.WriteU32(m_dest);
		i.WriteU16(m_sport);
		i.WriteU16(m_dport);
        i.WriteU32(m_sendport);
		i.WriteU32(m_detourcount);
		i.WriteU32(m_path.size());

		for( uint32_t j = 0; j < m_path.size(); j++ )
		{
		  i.WriteU32(m_path[j]);
		}

		//i.WriteU32(m_detour_path.size());
		//for(uint32_t j=0; j<m_detour_path.size(); j++)
		//{
		//  i.WriteU32(m_detour_path[j]);
		//}
	}
	
	uint32_t ECMPHeader::Deserialize (Buffer::Iterator start)
	{
		Buffer::Iterator i = start;

		m_protocol	= i.ReadU8();
		m_ltime		= i.ReadU8();
		m_srl           = i.ReadU8();
		m_prio          = i.ReadU32();
		m_ecn           = i.ReadU8();
		m_src           = i.ReadU32();
		m_dest		= i.ReadU32();
		m_sport         = i.ReadU16();
		m_dport         = i.ReadU16();
        m_sendport      = i.ReadU32();
		m_detourcount    = i.ReadU32();

		uint32_t length    = i.ReadU32();
		m_path.clear();
		for( uint32_t j = 0; j < length; j++ )
		{
		  m_path.push_back(i.ReadU32());
		}

		//uint32_t detour_path_len = i.ReadU32();
		//m_detour_path.clear();
		//for(uint32_t j=0; j < detour_path_len; j++)
		//{
		//  m_detour_path.push_back(i.ReadU32());
		//}

		return GetSerializedSize();
	}
}

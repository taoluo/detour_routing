#include "bfs-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (BFSHeader);

	TypeId BFSHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::BFSHeader").SetParent<NetHeader>().AddConstructor<BFSHeader>();
		return tid;		
	}
	
	TypeId BFSHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
	
	BFSHeader::BFSHeader()
	{
		m_src = 0;
        m_dest = 0;
		m_protocol = 0;
		m_ltime = 0;
	}
	
	BFSHeader::~BFSHeader()
	{}

	void BFSHeader::Print (std::ostream &os) const
	{}

    void BFSHeader::Dump() const
    {
        std::cout << "Protocol=" << (uint32_t)m_protocol << " Source=" << m_src << " Dest=" << m_dest << " path=";
        for( uint32_t i = 0; i < m_path.size(); i++ )
            std::cout << m_path[i] << " ";
        std::cout << "livingTime=" << (uint32_t)m_ltime;
        std::cout << std::endl;
    }
	
	uint32_t BFSHeader::GetSerializedSize (void) const
	{
		return ( 14 + 4 * m_path.size() );
	}
	
	void BFSHeader::Serialize (Buffer::Iterator start) const
	{
		Buffer::Iterator i = start;

		i.WriteU8(m_protocol);
		i.WriteU8(m_ltime);
		i.WriteU32(m_src);
		i.WriteU32(m_dest);
        i.WriteU32(m_path.size());
        for( uint32_t j = 0; j < m_path.size(); j++ )
        {
            i.WriteU32(m_path[j]);
        }
	}
	
	uint32_t BFSHeader::Deserialize (Buffer::Iterator start)
	{
		Buffer::Iterator i = start;

		m_protocol	= i.ReadU8();
		m_ltime		= i.ReadU8();
		m_src       = i.ReadU32();
		m_dest		= i.ReadU32();		
        uint32_t length    = i.ReadU32();
        m_path.clear();
        for( uint32_t j = 0; j < length; j++ )
        {
            m_path.push_back(i.ReadU32());
        }
		return GetSerializedSize();
	}
}

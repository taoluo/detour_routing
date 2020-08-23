#include "bcube-header.h"

namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (BCubeHeader);

	TypeId BCubeHeader::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::BCubeHeader").SetParent<NetHeader>().AddConstructor<BCubeHeader>();
		return tid;		
	}
	
	TypeId BCubeHeader::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
	
	BCubeHeader::BCubeHeader()
	{
		m_src = 0;
        m_dest = 0;
		m_protocol = 0;
		m_ltime = 0;
	}
	
	BCubeHeader::~BCubeHeader()
	{}

	void BCubeHeader::Print (std::ostream &os) const
	{}

    void BCubeHeader::Dump() const
    {
        std::cout << "Protocol=" << (uint32_t)m_protocol << " Source=" << m_src << " Dest=" << m_dest << " path=";
        for( uint32_t i = 0; i < m_path.size(); i++ )
            std::cout << m_path[i] << " ";
        std::cout << "livingTime=" << (uint32_t)m_ltime;
        std::cout << std::endl;
    }
	
	uint32_t BCubeHeader::GetSerializedSize (void) const
	{
		return ( 14 + 4 * m_path.size() );
	}
	
	void BCubeHeader::Serialize (Buffer::Iterator start) const
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
	
	uint32_t BCubeHeader::Deserialize (Buffer::Iterator start)
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

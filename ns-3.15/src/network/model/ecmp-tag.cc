#include "ecmp-tag.h"


namespace ns3
{
	NS_OBJECT_ENSURE_REGISTERED (ECMPTag);

	TypeId ECMPTag::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::ECMPTag").SetParent<Tag>().AddConstructor<ECMPTag>();
		return tid;		
	}

  TypeId ECMPTag::GetInstanceTypeId (void) const
  {
    return GetTypeId();
  }
  ECMPTag::ECMPTag(){
  }
  ECMPTag::~ECMPTag(){}
 
 uint32_t
 ECMPTag::GetSerializedSize (void) const {
   return sizeof(m_RemainByte);
 }
  /**
   * \param i the buffer to write data into.
   *
   * Write the content of the tag in the provided tag buffer.
   * DO NOT attempt to write more bytes than you requested
   * with Tag::GetSerializedSize.
   */
  void
  ECMPTag::Serialize (TagBuffer i) const
  {
    i.WriteU32(m_RemainByte);
    
  }
    
  /**
   * \param i the buffer to read data from.
   *
   * Read the content of the tag from the provided tag buffer.
   * DO NOT attempt to read more bytes than you wrote with
   * Tag::Serialize.
   */
  void
  ECMPTag::Deserialize (TagBuffer i)
  {
    m_RemainByte = i.ReadU32();
  }

  /**
   * \param os the stream to print to
   *
   * This method is typically invoked from the Packet::PrintByteTags
   * or Packet::PrintPacketTags methods.
   */
  void
  ECMPTag::Print (std::ostream &os) const
  {
    os<<"Remain Byte"<<m_RemainByte;
  }
 	
}

#ifndef ECMP_TAG_H
#define ECMP_TAG_H


#include "tag.h"

namespace ns3 {


  class ECMPTag: public  Tag
  {
  public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;

    ECMPTag();
    ~ECMPTag();
    
  virtual uint32_t GetSerializedSize (void) const;
  /**
   * \param i the buffer to write data into.
   *
   * Write the content of the tag in the provided tag buffer.
   * DO NOT attempt to write more bytes than you requested
   * with Tag::GetSerializedSize.
   */
  virtual void Serialize (TagBuffer i) const;
  /**
   * \param i the buffer to read data from.
   *
   * Read the content of the tag from the provided tag buffer.
   * DO NOT attempt to read more bytes than you wrote with
   * Tag::Serialize.
   */
  virtual void Deserialize (TagBuffer i);

  /**
   * \param os the stream to print to
   *
   * This method is typically invoked from the Packet::PrintByteTags
   * or Packet::PrintPacketTags methods.
   */
  virtual void Print (std::ostream &os) const;
  

  uint32_t
  GetRemainingByte(){return m_RemainByte;}

  void
  SetRemainingByte(uint32_t byte){m_RemainByte = byte;}

private:
  uint32_t m_RemainByte;

  };
}

#endif

#ifndef __BCUBE_PROBE_TAG_H__
#define __BCUBE_PROBE_TAG_H__

#include "ns3/packet.h"
#include "ns3/tag.h"

namespace ns3 {

class Tag;

class BCubeProbeTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  BCubeProbeTag ();

  BCubeProbeTag (uint32_t destId);

  void SetDestId (uint32_t destId) { m_destId = destId; }
  uint32_t GetDestId (void) const { return m_destId; }

  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

private:
  uint32_t m_destId;
};

}

#endif

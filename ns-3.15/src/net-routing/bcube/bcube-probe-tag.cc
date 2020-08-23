#include "bcube-probe-tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BCubeProbeTag);

TypeId
BCubeProbeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BCubeProbeTag")
    .SetParent<Tag> ()
    .AddConstructor<BCubeProbeTag> ()
  ;
  return tid;
}

TypeId
BCubeProbeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

BCubeProbeTag::BCubeProbeTag ()
  : m_destId (0)
{
}
BCubeProbeTag::BCubeProbeTag (uint32_t destId)
  : m_destId (destId)
{
}

uint32_t
BCubeProbeTag::GetSerializedSize (void) const
{
  return 4;
}

void
BCubeProbeTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_destId);
}

void
BCubeProbeTag::Deserialize (TagBuffer i)
{
  m_destId = i.ReadU32 ();
}

void
BCubeProbeTag::Print (std::ostream &os) const
{
  os << " Dest NodeId =" << m_destId;
}

} //namespace ns3

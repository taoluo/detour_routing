#ifndef __BFS_ROUTING_PROTOCOL__
#define __BFS_ROUTING_PROTOCOL__

#include "ns3/net-routing-protocol.h"
#include <map>

#include "ns3/ptr.h"
#include "ns3/nix-vector.h"
#include "ns3/object.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/channel.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv4.h"
#include "ns3/random-variable.h"

namespace ns3
{
    class BFSRoutingProtocol : public NetRoutingProtocol
	{
        typedef std::map<uint32_t, std::vector< uint32_t > > Lookup_t;
        typedef std::map<uint32_t, uint32_t> Forward_t;
        
      public:
		static TypeId GetTypeId (void);

		BFSRoutingProtocol();
		~BFSRoutingProtocol();

		uint32_t GetNAddresses(uint32_t interface) const;
		
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);
        bool FindOutputDevice (uint32_t nextId, uint32_t &index);
        bool BFS (uint32_t source, uint32_t dest, std::vector<uint32_t> &path);
        uint64_t GetTotalBytes();
      private:
        Lookup_t m_lookup;
        Forward_t m_forward;
        UniformVariable m_rand; // Uniform Random Variable
        uint64_t m_totalBytes;

		virtual uint32_t GetPathNumber() const { return 1; }
    };
}


#endif

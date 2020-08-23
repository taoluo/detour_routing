#ifndef __ECMP_ROUTING_PROTOCOL__
#define __ECMP_ROUTING_PROTOCOL__

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
#include "ns3/ecmp-header.h"
#include "ns3/tcp-header.h"

namespace ns3
{
    class ECMPRoutingProtocol : public NetRoutingProtocol
	{
        typedef std::map<uint32_t, std::vector< uint32_t > > Lookup_t;
        //typedef std::map<uint32_t, uint32_t> Forward_t;
	typedef std::map<uint32_t, std::map<uint32_t, uint32_t> > Forward_t;
        
      public:
		static TypeId GetTypeId (void);

		ECMPRoutingProtocol();
		~ECMPRoutingProtocol();

		uint32_t GetNAddresses(uint32_t interface) const;
		
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);
		void AddForwardEntry(uint32_t dest);
		bool SourceRoute(uint32_t next_id, uint32_t& index);
		bool ECMP (uint32_t dest, uint16_t hash_value, uint32_t &index);
        uint32_t GetAvailableBuffer(uint32_t dest, const ECMPHeader& header,
                                    Ptr<Node> node,
                                    std::vector<uint32_t>& node_list,
                                    uint32_t deep);
        
		bool FindBouncingDevice (uint32_t dest, const ECMPHeader& hdr, uint32_t pktsize, uint32_t &index);
		bool DetourChoice (Ptr<NetDevice> dev, uint32_t prio, uint32_t pktsize) const;
		bool ECNMark(Ptr<NetDevice> dev, ECMPHeader&  hdr, uint32_t pktsize);
		bool BFS (uint32_t source, uint32_t dest, std::vector<uint32_t> &path, uint32_t &hops);
		uint64_t GetTotalBytes();
		uint16_t hash(ECMPHeader &header, uint32_t inport);
		bool DetourNeed(Ptr<NetDevice> dev, uint32_t port, uint32_t pktsize) const;
		bool Spray (uint32_t dest, uint8_t t, uint32_t& index);
		uint32_t ReportDrop();
		double ReportThroughput();
		uint32_t ReportDetourCnt(uint64_t *pback, uint64_t *pincast);
		
		uint64_t m_backdetourcount[20];
		uint64_t m_incastdetourcount[20];
		
	        enum RoutingMode{
		  ROUTE_MOD_ECMP,
		  ROUTE_MOD_SPRAY,
		  ROUTE_MOD_SOURCE,
		};
      private:
		//Lookup_t m_lookup;
        
		UniformVariable m_rand; // Uniform Random Variable
		uint64_t m_totalBytes;
		Forward_t m_forward;
		uint32_t m_numhost;
		bool m_usebounce;
		bool m_priority;
		bool m_tracepath;
		uint32_t m_ecnthreshold;
		uint32_t m_detourthresh;
		bool  m_ecncapable;
		uint32_t m_dropcnt;
		
		RoutingMode m_mode;
		uint32_t m_robin;
		bool m_markdetour;
		double m_starttime;
		double m_forwardbytes;
		uint32_t m_TTL_limit;
		uint32_t m_TTD_limit;

		
		virtual uint32_t GetPathNumber() const { return 1; }
    };
}


#endif

#ifndef __BCUBE_ROUTING_PROTOCOL__
#define __BCUBE_ROUTING_PROTOCOL__

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
    class BCubeRoutingProtocol : public NetRoutingProtocol
	{
        /**
         * Map of Ipv4Address to NixVector
         */
/******************************
|            | Path0 | Value0 |
|  Dest IP0  |________________|
| (EndPoint) | Path1 | Value1 |
|            |________________|
|            |     ......     |
|____________|________________|
|   ......   |     ......     |
|            |                |
******************************/
		
// 1st layer
        typedef struct _Path_Value
        {
            std::vector< uint32_t > path;
            uint32_t value;
        }Path_Value_t;
// 2rd layer
        typedef std::map<uint32_t, std::vector< Path_Value_t > > Lookup_t;

        typedef std::map<uint32_t, uint32_t> NextHop_t;
    
      public:
		static TypeId GetTypeId (void);
        static double TotalSendTime;
        static double TotalRecvTime;
        static double FindOutputDeviceTime;
        
		BCubeRoutingProtocol();
		~BCubeRoutingProtocol();

		uint32_t GetNAddresses(uint32_t interface) const;
		
		Ipv4InterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const;

		void Send(Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
		void Receive(Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType);

         void FlushNixCache ();
         void FlushNixCacheForDestId ( uint32_t destId );
         std::vector< uint32_t > GetSinglePathInCache (uint32_t destId);
         bool GetPathSetFromCache( uint32_t destId, std::vector< std::vector< uint32_t > > &pathSet);
         bool InsertSinglePath(const std::vector< uint32_t > &path, uint32_t value);
         void UpdateRoutingEntry(uint32_t destId);
         bool IsSamePath(const std::vector< uint32_t > &A, const std::vector< uint32_t > &B);
         bool GeneratePath(uint32_t destId, Ptr<Packet> packet, std::vector<uint32_t> &path);
         bool FindOutputDevice (uint32_t nextId, uint32_t &index);
         std::vector < std::vector <uint32_t> > BuildPathSet(const uint32_t &src, const uint32_t &dst);
         std::vector <uint32_t> DCRouting(const uint32_t &src, const uint32_t &dst, const int32_t &i);
         std::vector <uint32_t> AltDCRouting(const uint32_t &src, const uint32_t &dst, const int32_t &i, const uint32_t &intermediate);
         std::vector <uint32_t> BCubeRouting(const uint32_t &src, const uint32_t &dst, const std::vector <uint32_t> &permutation);
         std::vector <uint32_t> NumToBit(const uint32_t &num);
         uint32_t BitToNum(const std::vector <uint32_t> &bit);
         void DumpPathSet(const std::vector < std::vector < uint32_t > > &pathSet);
         std::vector< std::vector < uint32_t > > GetPathSet(const uint32_t &src, const uint32_t &dst);
         std::vector < uint32_t > GetRandomPath(const uint32_t &src, const uint32_t &dst);
         std::vector < uint32_t > ExpandPath( const std::vector < uint32_t > &path);
         void PrintRoutingTable();
         uint32_t FindIndexForDevice(Ptr<NetDevice> dev);
         uint32_t GetAvailableBandwidth(uint32_t index);
         uint32_t GetOccupiedBandwidth(uint32_t destId);
         void ResetSampleBytes();
         uint64_t GetTotalBytes();

         uint32_t m_n;
         uint32_t m_k;
         Lookup_t m_lookup;
         NextHop_t m_nextHop;
         // Uniform Random Variable
         UniformVariable m_rand;

         uint64_t m_totalBytes;
         Time m_sampleTime;
         
         std::vector<uint64_t> m_sampleBytes;
         std::vector<uint64_t> m_currentBandwidth;
         std::map<uint32_t, uint64_t> m_sampleOccupied;
         std::map<uint32_t, uint64_t> m_occupiedBandwidth;
         std::map<uint32_t, uint32_t> m_currentPath;

		 virtual uint32_t GetPathNumber() const { return 1; }
    };
}


#endif

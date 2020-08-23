#include "ns3/simulator.h"
#include <queue>
#include <iomanip>
#include "bcube-routing-protocol.h"
#include "ns3/node.h"
#include "ns3/bcube-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/names.h"
#include "ns3/bcube-probe-tag.h"
#include "ns3/string.h"
#include "ns3/data-rate.h"
#include <string>

#define IsDump 0
#define GetTime 0

namespace ns3
{
    double BCubeRoutingProtocol::TotalSendTime = 0;
    double BCubeRoutingProtocol::TotalRecvTime = 0;
    double BCubeRoutingProtocol::FindOutputDeviceTime = 0;
    
    NS_OBJECT_ENSURE_REGISTERED (BCubeRoutingProtocol);
    
	TypeId BCubeRoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::BCubeRoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<BCubeRoutingProtocol> ()
            .AddAttribute ("BCube_n", "BCube size N",
                           UintegerValue (0),
                           MakeUintegerAccessor (&BCubeRoutingProtocol::m_n),
                           MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("BCube_k", "BCube size k",
                           UintegerValue (0),
                           MakeUintegerAccessor (&BCubeRoutingProtocol::m_k),
                           MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("SampleTime", 
                           "The time to sample the current bandwidth consumption",
                           TimeValue (Seconds (0.01)),
                           MakeTimeAccessor (&BCubeRoutingProtocol::m_sampleTime),
                           MakeTimeChecker ())
            ;
		return tid;
	}

	BCubeRoutingProtocol::BCubeRoutingProtocol()
        :
        m_totalBytes (0)
	{
	}

	BCubeRoutingProtocol::~BCubeRoutingProtocol()
	{
	}

	uint32_t
    BCubeRoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress
    BCubeRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void
    BCubeRoutingProtocol::Send (Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
#if GetTime
        // For time measyrement
        timespec res, t1, t2;
        clock_getres(CLOCK_REALTIME, &res);
        clock_gettime(CLOCK_REALTIME, &t1);
#endif
        
        BCubeHeader header;
        header.SetProtocol(protocol);
        header.SetLivingTime(0);
		header.SetSrc(source.Get());
		header.SetDest(destination.Get());
        std::vector<uint32_t> path;
        bool pathFound = GeneratePath(destination.Get(), packet, path);
        if(pathFound)
            header.SetPath(path);
        else
            std::cout << " BCubeRoutingProtocol::Send() Output Device not Found!" << std::endl;
        packet->AddHeader(header);

        uint32_t index;
        if( FindOutputDevice (path[0], index) )
        {
            Ptr<NetDevice> device = GetNode()->GetDevice (index);
#if IsDump
            std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BCubeRoutingProtocol::SendPacket() Header: ";
            header.Dump();
#endif
#if GetTime
            clock_gettime(CLOCK_REALTIME, &t2);
            double duration = (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
            BCubeRoutingProtocol::TotalSendTime += duration;
#endif

            // Do statistics
            if(m_sampleBytes.empty())
            {
                m_sampleBytes.insert(m_sampleBytes.begin(), GetNode()->GetNDevices(), 0);
                m_currentBandwidth.insert(m_currentBandwidth.begin(), GetNode()->GetNDevices(), 0);
                ResetSampleBytes();
            }
            m_sampleBytes[index] += packet->GetSize();
            m_totalBytes += packet->GetSize();

            // calculate the occupied bandwidth
            m_sampleOccupied[destination.Get()] += packet->GetSize();
        
            SendPacket(device, packet);
        }
        else
            std::cout << " BCubeRoutingProtocol::Send() Output Device not Found!" << std::endl;
	}

	void
    BCubeRoutingProtocol::Receive(Ptr<NetDevice> indev, Ptr<const Packet> p, uint16_t protocol,const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
#if GetTime
        // For time measurement
        timespec res, t1, t2;
        clock_getres(CLOCK_REALTIME, &res);
        clock_gettime(CLOCK_REALTIME, &t1);
#endif
        
        Ptr<Packet> packet = p->Copy();
        
		BCubeHeader header;
		packet->RemoveHeader(header);
#if IsDump
        std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BCubeRoutingProtocol::Receive() Header: ";
        header.Dump();
#endif
		
		if (header.GetDest() != GetNode()->GetId())
		{
			uint8_t livingTime = header.GetLivingTime() + 1;
			header.SetLivingTime(livingTime);
			packet->AddHeader(header);

            uint32_t index;
            if( FindOutputDevice (header.GetNextId(), index) )
            {
                Ptr<NetDevice> device = GetNode()->GetDevice (index);
#if IsDump
                std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BCubeRoutingProtocol::SendPacket() Header: ";
                header.Dump();
#endif
#if GetTime
                clock_gettime(CLOCK_REALTIME, &t2);
                double duration = (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
                BCubeRoutingProtocol::TotalRecvTime += duration;
#endif
                // Do statistics
                if(m_sampleBytes.empty())
                {
                    m_sampleBytes.insert(m_sampleBytes.begin(), GetNode()->GetNDevices(), 0);
                    m_currentBandwidth.insert(m_currentBandwidth.begin(), GetNode()->GetNDevices(), 0);
                    ResetSampleBytes();
                }
                m_sampleBytes[index] += packet->GetSize();
                m_totalBytes += packet->GetSize();
            
                SendPacket(device, packet);
            }
            else
                std::cout << " BCubeRoutingProtocol::Send() Output Device not Found!" << std::endl;		
		}
		else
		{
			Ptr<IpL4Protocol> protocol = GetProtocol (header.GetProtocol());

			Ipv4Header ipv4Header;
			ipv4Header.SetSource(Ipv4Address(header.GetSrc()));
			ipv4Header.SetDestination(Ipv4Address(header.GetDest()));

			Ptr<Ipv4Interface> ipv4Interface = Create<Ipv4Interface>();
            ipv4Interface->SetDevice(indev);
#if GetTime      
            clock_gettime(CLOCK_REALTIME, &t2);
            double duration = (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
            BCubeRoutingProtocol::TotalRecvTime += duration;
#endif 
			protocol->Receive(packet, ipv4Header, ipv4Interface);
		}
	}

	// =================== BCube Assistant functions ================================
    void
    BCubeRoutingProtocol::FlushNixCache ()
    {
        m_lookup.clear ();
        m_sampleBytes.clear();
        m_currentBandwidth.clear();
        m_sampleOccupied.clear();
        m_occupiedBandwidth.clear();
        m_currentPath.clear();
    }

    void
    BCubeRoutingProtocol::FlushNixCacheForDestId ( uint32_t destId )
    {
        Lookup_t::iterator iter = m_lookup.find (destId);
        if (iter != m_lookup.end ())
        {
            m_lookup.erase(iter);
        }
        std::map<uint32_t, uint64_t>::iterator it = m_occupiedBandwidth.find (destId);
        if (it != m_occupiedBandwidth.end ())
        {
            m_occupiedBandwidth.erase(it);
        }
        it = m_sampleOccupied.find (destId);
        if (it != m_sampleOccupied.end ())
        {
            m_sampleOccupied.erase(it);
        }
        std::map<uint32_t, uint32_t>::iterator it2 = m_currentPath.find (destId);
        if (it2 != m_currentPath.end ())
        {
            m_currentPath.erase(it2);
        }
    }

    std::vector< uint32_t >
    BCubeRoutingProtocol::GetSinglePathInCache (uint32_t destId)
    {
        std::vector< uint32_t > path;
        Lookup_t::iterator iter = m_lookup.find (destId);
        // not in cache
        NS_ASSERT_MSG ( iter != m_lookup.end (), "Error! No Path exist for dest!");
        // in cache
        if (iter != m_lookup.end ())
        {
            //NS_LOG_LOGIC ("Found Dest IP (EndPoint level) in cache.");
            // not in cache
            NS_ASSERT_MSG ( iter->second.size() > 0, "Error! No Path exist for dest!");
            // in cache
            uint32_t index = m_currentPath[destId];
            path = iter->second[index].path;
        }
        return path;
    }

    bool
    BCubeRoutingProtocol::GetPathSetFromCache( uint32_t destId, std::vector< std::vector< uint32_t > > &pathSet)
    {
        Lookup_t::iterator iter = m_lookup.find (destId);
        if (iter == m_lookup.end ())
        {
            return false;
        }
        pathSet.clear();
        for( uint32_t i = 0; i < iter->second.size(); i++ )
        {
            pathSet.push_back(iter->second[i].path);
        }
        return true;
    }

    bool 
    BCubeRoutingProtocol::InsertSinglePath(const std::vector< uint32_t > &path, uint32_t value)
    {
        if( path.size() == 0)
            return false;
        uint32_t destId = path[path.size()-1];
        // Get into 2rd layer, find correct index(EndPointIp) 
        Lookup_t::iterator iter = m_lookup.find (destId);
        if (iter == m_lookup.end ())
        {
            // EndPoint Ip doesn't exist; Create a new element.
            std::vector< Path_Value_t > path_value_new;
            m_lookup.insert(Lookup_t::value_type ( destId, path_value_new ));
            iter = m_lookup.find (destId);

            // Create the statistics, too
            m_occupiedBandwidth.insert(std::pair<uint32_t, uint64_t>( destId, 0));
            m_sampleOccupied.insert(std::pair<uint32_t, uint64_t>( destId, 0));
            m_currentPath.insert(std::pair<uint32_t, uint32_t>( destId, 0));
        }

        // Get into 2nd layer, find correct index
        uint32_t i;
        for ( i = 0; i < iter->second.size(); i++ )
        {
            if ( IsSamePath(iter->second[i].path, path) )
            {
                // destIp already in the routing table
                // Check if we are using this path
                if( i == m_currentPath[destId] )
                    iter->second[i].value = value + GetOccupiedBandwidth(destId);
                else
                    iter->second[i].value = value;
                break;
            }
        }
        if ( i >= iter->second.size() )
        {
            // dest IP doesn't exist, so insert this path
            Path_Value_t path_value;
            path_value.path = path;
            path_value.value = value;
            iter->second.push_back(path_value);
        }
        
        return true;
    }

    void
    BCubeRoutingProtocol::UpdateRoutingEntry(uint32_t destId)
    {
        Lookup_t::iterator iter = m_lookup.find (destId);
        if (iter != m_lookup.end ())
        {
            // Then we update the currentPath map
            uint32_t offset = m_rand.GetInteger(0, iter->second.size()-1);
            m_currentPath[destId] = offset;
            uint32_t max_value = iter->second[offset].value;
            for( uint32_t j = 1; j < iter->second.size(); j++ )
            {
                uint32_t i = (offset + j) % iter->second.size();
                if( iter->second[i].value > max_value )
                {
                    m_currentPath[destId] = i;
                    max_value = iter->second[i].value;
                }
            }
        }        
    }

    bool
    BCubeRoutingProtocol::IsSamePath(const std::vector< uint32_t > &A, const std::vector< uint32_t > &B)
    {
        if( A.size() == B.size() )
        {
            bool result = true;
            for( uint32_t i = 0; i < A.size(); i++ )
            {
                if( A[i] != B[i] )
                    result = false;
            }
            return result;
        }
        else
            return false;
    }
    
    
    bool
    BCubeRoutingProtocol::GeneratePath(uint32_t destId, Ptr<Packet> packet, std::vector<uint32_t> &path)
    {
        bool isProbe = false;
        BCubeProbeTag bcubeTag;
        if ( packet != NULL )   // When checking route, packet might be an empty pointer 
        {
            if( packet->PeekPacketTag(bcubeTag) )
            {
                isProbe = true;
            }
        }
        if ( isProbe )
        {
            // Probe Packet
#if IsDump
            std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BCubeRoutingProtocol::GeneratePath() Probe Packet Sent to Node " << bcubeTag.GetDestId() << std::endl;
#endif
            path.clear();
            path.push_back( bcubeTag.GetDestId() );
        }
        else
        {
            // Normal Packet 
            // Source Node, get path from Cache
            path = GetSinglePathInCache ( destId );
            if ( path.empty() )
            {
                //NS_LOG_ERROR ("No path to the dest: " << header.GetDestination ());
                return false;
            }
        }
        return true;
    }

    bool
    BCubeRoutingProtocol::FindOutputDevice (uint32_t nextId, uint32_t &index)
    {
#if GetTime
        // For time measurement
        timespec res, t1, t2;
        clock_getres(CLOCK_REALTIME, &res);
        clock_gettime(CLOCK_REALTIME, &t1);
#endif

        if( m_nextHop.empty() )
        {
            // First time, create the hext hop routing table
            uint32_t numberOfDevices = GetNode()->GetNDevices ();
            // scan through the net devices on the parent node
            // and then look at the nodes adjacent to them
            for (uint32_t i = 0; i < numberOfDevices; i++)
            {
                // Get a net device from the node
                // as well as the channel, and figure
                // out the adjacent net devices
                Ptr<NetDevice> localNetDevice = GetNode()->GetDevice (i);
                Ptr<Channel> channel = localNetDevice->GetChannel ();
                if (channel == 0)
                {
                    continue;
                }
                
                // this function takes in the local net dev, and channnel, and
                // writes to the netDeviceContainer the adjacent net devs
                for (uint32_t j = 0; j < channel->GetNDevices (); j++)
                {
                    Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
                    Ptr<Node> remoteNode = remoteDevice->GetNode();
                    uint32_t remoteId = remoteNode->GetId();
                    m_nextHop.insert(NextHop_t::value_type ( remoteId, i ));
                }
            }
        }
        // Now we get the next hop routing table
        bool indexfound;
        NextHop_t::iterator it = m_nextHop.find(nextId);
        if( it == m_nextHop.end() )
            indexfound = false;
        else
        {
            indexfound = true;
            index = it->second;
        }   
        
#if GetTime
        clock_gettime(CLOCK_REALTIME, &t2);
        double duration = (t2.tv_sec - t1.tv_sec)  + (float) (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
        BCubeRoutingProtocol::FindOutputDeviceTime += duration;
#endif
        
        return indexfound;
    }

    std::vector < std::vector <uint32_t> >
    BCubeRoutingProtocol::BuildPathSet(const uint32_t &src, const uint32_t &dst)
    {
        std::vector < std::vector <uint32_t> > pathSet;
        std::vector <uint32_t> A, B, C, path, expandPath;
        A = NumToBit(src);
        B = NumToBit(dst);
    
        for(int32_t i = m_k; i >= 0; i--)
        {
            if( A[i] != B[i])
            {
                path = DCRouting(src, dst, i);
            }
            else
            {
                // Random choose C as a neighbor of A at level i 
                uint32_t randInt = m_rand.GetInteger(1, m_n-1);
                C = NumToBit(src);
                C[i] = ( A[i] + randInt ) % m_n;
                path = AltDCRouting(src, dst, i, BitToNum(C));
            }
            // Expand the path to server-switch-server representation
            expandPath = ExpandPath(path);
            pathSet.push_back(expandPath);
        }
        return pathSet;
    }

    std::vector <uint32_t> 
    BCubeRoutingProtocol::DCRouting(const uint32_t &src, const uint32_t &dst, const int32_t &i)
    {
        std::vector <uint32_t> path, permutation;
        for( int32_t j = i+m_k+1; j >= i+1; j-- )
        {
            permutation.insert(permutation.begin(), j%(m_k+1));
        }
        path = BCubeRouting(src, dst, permutation);
        return path;
    }

    std::vector <uint32_t> 
    BCubeRoutingProtocol::AltDCRouting(const uint32_t &src, const uint32_t &dst, const int32_t &i, const uint32_t &intermediate)
    {
        std::vector <uint32_t> path, permutation;
        for( int32_t j = i+m_k; j >= i; j-- )
        {
            permutation.insert(permutation.begin(), j%(m_k+1));
        }
        path = BCubeRouting(intermediate, dst, permutation);
        path.insert(path.begin(), intermediate);
        return path;
    }

    std::vector <uint32_t> 
    BCubeRoutingProtocol::BCubeRouting(const uint32_t &src, const uint32_t &dst, const std::vector <uint32_t> &permutation)
    {
        std::vector <uint32_t> path, A, B, I_Node;
        A = NumToBit(src);
        B = NumToBit(dst);
        I_Node = NumToBit(src);
        for(int32_t i = m_k; i >= 0; i--)
        {
            if( A[ permutation[i] ] != B[ permutation[i] ])
            {
                I_Node[ permutation[i] ] = B[ permutation[i] ];
                path.push_back( BitToNum(I_Node) );
            }
        }
        return path;
    }

    std::vector <uint32_t> 
    BCubeRoutingProtocol::NumToBit(const uint32_t &num)
    {
        // From larger to smaller
        // example: 35 = 2*4^2 + 0*4^1 + 3*4^0 = (203)4
        std::vector <uint32_t> bit;
        uint32_t x = num;
        uint32_t y;
        for( uint32_t i = 0; i <= m_k; i++)
        {
            y = x % m_n;
            x = (x - y) / m_n;
            bit.insert(bit.begin(),y);
        }
        return bit;
    }

    uint32_t
    BCubeRoutingProtocol::BitToNum(const std::vector <uint32_t> &bit)
    {
        uint32_t num = 0;
        for (uint32_t i = 0; i < bit.size(); i++)
        {
            num += bit[i] * uint32_t( pow ( double(m_n) , double( bit.size() - i - 1 ) ) );
        }
        return( num );
    }

    void
    BCubeRoutingProtocol::DumpPathSet(const std::vector < std::vector < uint32_t > > &pathSet)
    {
        for( uint32_t i = 0; i < pathSet.size(); i++)
        {
            std::cout << "Path " << i << ": Bits( ";
            for(uint32_t j = 0; j < pathSet[i].size()/2; j++)
            {
                // Switch Node
                uint32_t server_num = uint32_t( pow ( double(m_n), double(m_k + 1) ) );
                uint32_t switch_index = pathSet[i][j*2] - server_num;
                uint32_t each_layer_num = uint32_t( pow ( double(m_n), double(m_k) ) );
                uint32_t layer = switch_index / each_layer_num;
                uint32_t index = pathSet[i][j*2] % each_layer_num;
                std::vector < uint32_t > switch_bit = NumToBit(index);
                std::cout << "<" << layer << ",";
                for(uint32_t k = 1; k < switch_bit.size(); k++)
                    std::cout << switch_bit[k] ;
                std::cout << "> ";
                // Server Node
                std::vector < uint32_t > server_bit = NumToBit(pathSet[i][j*2+1]);
                for(uint32_t k = 0; k < server_bit.size(); k++)
                    std::cout << server_bit[k] ;
                std::cout << " ";
            }
            std::cout << ") Num( ";
            for(uint32_t j = 0; j < pathSet[i].size(); j++)
            {
                std::cout << pathSet[i][j] << " ";
            }
            std::cout << ")" << std::endl;
        }
    }

    std::vector< std::vector < uint32_t > >
    BCubeRoutingProtocol::GetPathSet(const uint32_t &src, const uint32_t &dst)
    {
        std::vector< std::vector < uint32_t > > pathSet;
        pathSet = BuildPathSet(src, dst);
        return pathSet;
    }

    std::vector < uint32_t > 
    BCubeRoutingProtocol::GetRandomPath(const uint32_t &src, const uint32_t &dst)
    {
        std::vector< std::vector < uint32_t > > pathSet;
        pathSet = BuildPathSet(src, dst);
        uint32_t randInt = m_rand.GetInteger(0, pathSet.size()-1);
        return pathSet[randInt];
    }

    std::vector < uint32_t > 
    BCubeRoutingProtocol::ExpandPath( const std::vector < uint32_t > &path)
    {
        // Input path is constructed by server-server hops
        // Expand to server-switch-server hops
        std::vector < uint32_t > A, B, result;
        A = NumToBit(GetNode()->GetId());
        for (uint32_t i = 0; i < path.size(); i++)
        {
            B = NumToBit(path[i]);
            std::vector < uint32_t > C;  // switch bits
            uint32_t switch_layer = 0;
            uint32_t switch_count, switch_index, switch_id;
            for (uint32_t j = 0; j <= m_k; j++)
            {
                // Each server is k+1 bits
                if ( A[j] == B[j] )
                {
                    C.push_back( A[j] );
                }
                else
                {
                    switch_layer = m_k - j;
                }
            }
            switch_count = BitToNum(C);
            switch_index = switch_layer * uint32_t( pow ( double(m_n), double(m_k) ) ) + switch_count;
            switch_id = switch_index + uint32_t( pow ( double(m_n), double(m_k + 1) ) );
            // Save the path
            result.push_back(switch_id);
            result.push_back(path[i]);
            
            // For next loop
            A = NumToBit(path[i]);
        }
        return result;
    }

    void
    BCubeRoutingProtocol::PrintRoutingTable()
    {
        std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BCubeRoutingProtocol::PrintRoutingTable():" << std::endl;
        for( Lookup_t::iterator iter = m_lookup.begin(); iter != m_lookup.end(); iter++ )
        {
            std::cout << "    " << iter->first << " Occupied " << GetOccupiedBandwidth(iter->first) << " Current Path " << m_currentPath[iter->first] << std::endl;
            for( uint32_t i = 0; i < iter->second.size(); i++ )
            {
                std::cout << "        ";
                for( uint32_t j = 0; j < iter->second[i].path.size(); j++)
                    std::cout << iter->second[i].path[j] << " ";
                std::cout << "(" << iter->second[i].value << ")" << std::endl;
            }
        }
    }

    uint32_t
    BCubeRoutingProtocol::FindIndexForDevice(Ptr<NetDevice> dev)
    {
        for( uint32_t i = 0; i < GetNode()->GetNDevices(); i++ )
        {
            if( GetNode()->GetDevice(i) == dev )
                return i;
        }
        std::cout << "Fatal Error: NetDevice Not Found!" << std::endl;
        return 0;
    }
    
    uint32_t
    BCubeRoutingProtocol::GetAvailableBandwidth(uint32_t nextId)
    {
        // Do statistics
        if(m_sampleBytes.empty())
        {
            m_sampleBytes.insert(m_sampleBytes.begin(), GetNode()->GetNDevices(), 0);
            m_currentBandwidth.insert(m_currentBandwidth.begin(), GetNode()->GetNDevices(), 0);
            ResetSampleBytes();
        }
            
        uint32_t index;
        if( FindOutputDevice (nextId, index) )
        {
            Ptr<NetDevice> netDevice = GetNode()->GetDevice(index);
            StringValue str;
            netDevice->GetAttribute(std::string("DataRate"), str);
            DataRate bandwidth(str.Get());
            uint32_t bandwidthInMbps = (bandwidth.GetBitRate() - m_currentBandwidth[index])/1000000;
            return ( bandwidthInMbps );
        }
        else
        {
            std::cout << "Fatal Error! GetAvailableBandwidth: destination doesn't exist!" << std::endl;
            return 0;
        }
    }

    uint32_t
    BCubeRoutingProtocol::GetOccupiedBandwidth(uint32_t destId)
    {
        return (uint32_t)( m_occupiedBandwidth[destId]/1000000 );
    }
  
    void
    BCubeRoutingProtocol::ResetSampleBytes()
    {
        for( uint32_t i = 0; i < GetNode()->GetNDevices(); i++ )
        {
            m_currentBandwidth[i] = uint64_t( (double)m_sampleBytes[i] * 8 / m_sampleTime.GetSeconds() );
            m_sampleBytes[i] = 0;
        }
        std::map<uint32_t, uint64_t>::iterator it;
        for( it = m_occupiedBandwidth.begin(); it != m_occupiedBandwidth.end(); it++ )
        {
            it->second = uint64_t( (double)m_sampleOccupied[it->first] * 8 / m_sampleTime.GetSeconds() );
            m_sampleOccupied[it->first] = 0;
        }
        
        Simulator::Schedule (m_sampleTime, &BCubeRoutingProtocol::ResetSampleBytes, this);
    }

    uint64_t
    BCubeRoutingProtocol::GetTotalBytes()
    {
        return m_totalBytes;
    }
}


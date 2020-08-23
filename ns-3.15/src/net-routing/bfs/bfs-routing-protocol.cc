#include "ns3/simulator.h"
#include <queue>
#include <iomanip>
#include "ns3/node.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/names.h"
#include "bfs-routing-protocol.h"
#include "bfs-header.h"
#include "ns3/tcp-header.h"

#define IsDump 0
#define UseBounce 0
namespace ns3
{
    NS_OBJECT_ENSURE_REGISTERED (BFSRoutingProtocol);
    
	TypeId BFSRoutingProtocol::GetTypeId(void)
	{
		static TypeId tid = TypeId ("ns3::BFSRoutingProtocol")
			.SetParent<NetRoutingProtocol> ()
			.AddConstructor<BFSRoutingProtocol> ()
            ;
		return tid;
	}

	BFSRoutingProtocol::BFSRoutingProtocol()
        :
        m_totalBytes (0)
	{
	}

	BFSRoutingProtocol::~BFSRoutingProtocol()
	{
	}

	uint32_t
    BFSRoutingProtocol::GetNAddresses(uint32_t interface) const
	{
		return 1;
	}

	Ipv4InterfaceAddress
    BFSRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
	{
		return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
	}

	void
    BFSRoutingProtocol::Send (Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
	{
		// if (GetNode()->GetId() == 0)
		// {
		// 	TcpHeader tcphdr;
		// 	packet->PeekHeader(tcphdr);

		// 	if (tcphdr.GetDestinationPort() == 10)
		// 		printf("%lf, %d, \n", Simulator::Now().GetSeconds(), tcphdr.GetSequenceNumber().GetValue());
		// }
		
		BFSHeader header;
        header.SetProtocol(protocol);
        header.SetLivingTime(0);
		header.SetSrc(source.Get());
		header.SetDest(destination.Get());
        // Search for path in the lookup table
        Lookup_t::iterator iter = m_lookup.find (destination.Get());
        if (iter == m_lookup.end ())
        {
            // Path not found in the lookup table, create a path
            std::vector<uint32_t> path;
            bool pathFound = BFS(source.Get(), destination.Get(), path);
            if(pathFound)
            {
                // Insert into the lookup table
                m_lookup.insert(Lookup_t::value_type(destination.Get(), path));
                iter = m_lookup.find(destination.Get());
            }
            else
                std::cout << " BFSRoutingProtocol::Send() Output Device not Found!" << std::endl;
        }
        header.SetPath(iter->second);
        packet->AddHeader(header);

        uint32_t index;
        if( FindOutputDevice (header.GetNextId(), index) )
        {
            Ptr<NetDevice> device = GetNode()->GetDevice (index);
#if IsDump
            std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BFSRoutingProtocol::SendPacket() Header: ";
            header.Dump();
#endif
            m_totalBytes += packet->GetSize();
            SendPacket(device, packet);
        }
        else
            std::cout << " BFSRoutingProtocol::Send() Output Device not Found!" << std::endl;
	}

	void
    BFSRoutingProtocol::Receive(Ptr<NetDevice> indev, Ptr<const Packet> p, uint16_t protocol,const Address &from, const Address &to, NetDevice::PacketType packetType)
	{
        Ptr<Packet> packet = p->Copy();
		BFSHeader header;
		packet->RemoveHeader(header);
 #if IsDump
        std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BFSRoutingProtocol::Receive() Header: ";
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
	      //std::cout<<GetNode()->GetId()<<": "<<header.GetNextId()<<" at "<<index<<std::endl;
                Ptr<NetDevice> device = GetNode()->GetDevice (index);
#if IsDump
                std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BFSRoutingProtocol::SendPacket() Header: ";
                header.Dump();
#endif
                m_totalBytes += packet->GetSize();
#if UseBounce
		Ptr<Packet> bounce_packet = p->Copy();
		uint32_t idx;
		BFSHeader bounce_header;
 
                if (!SendPacket(device, packet))
		{
	     
		  bounce_packet->RemoveHeader(bounce_header);
		  //std::cout<<"test "<<bounce_header.GetNextId()<<std::endl;
		  if (bounce_header.GetLivingTime())
		  {
		    uint8_t ltime = bounce_header.GetLivingTime()-1;
		    bounce_header.SetLivingTime(ltime);
		    bounce_packet->AddHeader(bounce_header);
		   
		    if ( !FindOutputDevice (bounce_header.GetNextId(), idx))
		    {
		       std::cout<<GetNode()->GetId()<<" Cannot find back device "<<bounce_header.GetNextId()<<std::endl;
		    }
		  }
		  else
		  {
		    std::vector< uint32_t> def_path(bounce_header.GetPath());
		    def_path.insert(def_path.begin(),31);
		    def_path.insert(def_path.begin(),32);
		    idx = 3;
		    bounce_header.SetPath(def_path);
		    
		    uint8_t ltime = bounce_header.GetLivingTime()+1;
		    bounce_header.SetLivingTime(ltime);
		    bounce_packet->AddHeader(bounce_header);
		   
		    
		    std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") BFSRoutingProtocol::SendPacket() Header: ";
                bounce_header.Dump();
		  }

		 
		  Ptr<NetDevice> backdevice = GetNode()->GetDevice (idx);
		  if (!SendPacket(backdevice, bounce_packet))
		    std::cout<<GetNode()->GetId()<<" Backward fail"<<std::endl;
		  else
		    std::cout<<GetNode()->GetId()<<" Backward success"<<std::endl;	   
		 
	     
		}
#else
		SendPacket(device, packet);
#endif
		  
            }
            else
                std::cout << " BFSRoutingProtocol::Send() Output Device not Found!" << std::endl;		
		}
		else
		{
			Ptr<IpL4Protocol> protocol = GetProtocol (header.GetProtocol());

			Ipv4Header ipv4Header;
			ipv4Header.SetSource(Ipv4Address(header.GetSrc()));
			ipv4Header.SetDestination(Ipv4Address(header.GetDest()));

			Ptr<Ipv4Interface> ipv4Interface = Create<Ipv4Interface>();
			protocol->Receive(packet, ipv4Header, ipv4Interface);
		}
		
		// if (GetNode()->GetId() == 0)
		// {
		// 	TcpHeader tcphdr;
		// 	packet->PeekHeader(tcphdr);

		// 	if (tcphdr.GetSourcePort() == 10)
		// 		printf("%lf, , %d\n", Simulator::Now().GetSeconds(), tcphdr.GetAckNumber().GetValue());
		// }
	}

	// =================== BFS Assistant functions ================================

    bool
    BFSRoutingProtocol::FindOutputDevice (uint32_t nextId, uint32_t &index)
    {
        bool indexfound = false;
        Forward_t::iterator iter = m_forward.find (nextId);
        if (iter == m_forward.end ())
        {
            // Output device not found in the forwarding table, insert
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
                    // if this node is the next hop node
                    if (remoteNode->GetId() == nextId)
                    {
                        index = i;
                        m_forward.insert(Forward_t::value_type(nextId, i));
                        indexfound = true;
                        break;
                    }
                }
                if (indexfound)
                    break;
            }
        }
        else
        {
            index = iter->second;
            indexfound = true;
        }
        return indexfound;
    }

    bool
    BFSRoutingProtocol::BFS (uint32_t source, uint32_t dest, std::vector<uint32_t> &path)
    {
        std::queue<uint32_t> greyNodeList;  // discovered nodes with unexplored children

        // Initialize the parent vector for recovering the path
        uint32_t numberOfNodes = NodeList::GetNNodes();
        std::vector<uint32_t> parentVector;
        parentVector.reserve (numberOfNodes);
        parentVector.insert (parentVector.begin (), numberOfNodes, 0); 

        // Maintain a history vector
        std::vector<bool> history;
        history.reserve (numberOfNodes);
        history.insert (history.begin (), numberOfNodes, false);
        
        // Add the source node to the queue, set its parent to itself 
        greyNodeList.push (source);
        history[source] = true;
        parentVector[source] = source;

        // BFS loop
        bool pathFound = false;
        while (greyNodeList.size () != 0)
        {
            uint32_t current = greyNodeList.front ();
            if (current == dest) 
            {
                pathFound = true;
                break;
            }
            // Iterate over the current node's adjacent nodes
            // and push them into the queue
            Ptr<Node> currNode = NodeList::GetNode(current);

            //Shuffle the sequence of the devices
            std::vector<uint32_t> temp, sequence;
            temp.reserve(currNode->GetNDevices());
            sequence.reserve(currNode->GetNDevices());
            for(uint32_t i = 0; i < currNode->GetNDevices(); i++)
                temp.push_back(i);
            while(temp.size() > 0)
            {
                uint32_t index = m_rand.GetInteger(0, temp.size()-1);
                sequence.push_back(temp[index]);
                temp.erase(temp.begin()+index);
            }
            for (uint32_t i = 0; i < currNode->GetNDevices(); i++)
            {
                // Figure out the adjacent node ( we assume here we use ppp link)
                Ptr<NetDevice> localNetDevice = currNode->GetDevice (sequence[i]);

                // make sure that we can go this way
                if (!(localNetDevice->IsLinkUp ()))
                {
                    continue;
                }
                Ptr<Channel> channel = localNetDevice->GetChannel ();
                if (channel == 0)
                { 
                    continue;
                }
                for( uint32_t j = 0; j < channel->GetNDevices(); j++ )
                {
                    if( channel->GetDevice(j) != localNetDevice )
                    {
                        Ptr<NetDevice> remoteNetDevice = channel->GetDevice(j);
                        uint32_t remote = remoteNetDevice->GetNode()->GetId();
                        if( !history[remote] )
                        {
                            parentVector[remote] = current;
                            history[remote] = true;
                            greyNodeList.push (remote);
                        }
                    }
                }
            }
            // Pop off the head grey node.  We have all its children.
            // It is now black.
            greyNodeList.pop ();
        }
        if( pathFound )
        {
            // Trace back in the parent vector to recover the path
            path.clear();
            path.insert( path.begin(), dest );
            uint32_t temp = parentVector[dest];
            while( temp != source )
            {
                path.insert( path.begin(), temp );
                temp = parentVector[temp];
            }
            return true;
        }
        else
            return false;
    }

    uint64_t
    BFSRoutingProtocol::GetTotalBytes()
    {
        return m_totalBytes;
    }
}


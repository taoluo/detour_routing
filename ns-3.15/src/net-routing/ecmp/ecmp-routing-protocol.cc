#include "ns3/simulator.h"
#include <queue>
#include <iomanip>
#include "ns3/node.h"
#include "ns3/ipv4-interface.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/names.h"
#include "ecmp-routing-protocol.h"
#include "ecmp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/ipv4-header.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
//#include "ns3/ecmp-tag.h"


#define IsDump 0
//#define UseBounce 1


int64_t global_time = 0;


namespace ns3
{
  NS_OBJECT_ENSURE_REGISTERED (ECMPRoutingProtocol);
  
  TypeId ECMPRoutingProtocol::GetTypeId(void)
  {
    static TypeId tid = TypeId ("ns3::ECMPRoutingProtocol")
      .SetParent<NetRoutingProtocol> ()
      .AddConstructor<ECMPRoutingProtocol> ()
      .AddAttribute("NumHost", "Num of Hosts.",
                    UintegerValue(16), MakeUintegerAccessor(&ECMPRoutingProtocol::m_numhost), MakeUintegerChecker<uint32_t>())
      .AddAttribute("UseBounce", "xx",
                    BooleanValue(false), MakeBooleanAccessor(&ECMPRoutingProtocol::m_usebounce), MakeBooleanChecker())
      .AddAttribute("ECNThreshold", "xx", 
                    UintegerValue(20*1500), MakeUintegerAccessor(&ECMPRoutingProtocol::m_ecnthreshold), MakeUintegerChecker<uint32_t>())  //percentage
      .AddAttribute("DetourThreshold", "xx", 
                    UintegerValue(20*1500), MakeUintegerAccessor(&ECMPRoutingProtocol::m_detourthresh), MakeUintegerChecker<uint32_t>())  //percentage
      .AddAttribute("ECNCapable", "xx",
                    BooleanValue(false), MakeBooleanAccessor(&ECMPRoutingProtocol::m_ecncapable), MakeBooleanChecker())
      .AddAttribute("EnablePriority", "xx",
                    BooleanValue(false), MakeBooleanAccessor(&ECMPRoutingProtocol::m_priority), MakeBooleanChecker())
      .AddAttribute("RouteMode", "xx",
                    EnumValue(ROUTE_MOD_ECMP), MakeEnumAccessor(&ECMPRoutingProtocol::m_mode),
                    MakeEnumChecker(ROUTE_MOD_ECMP, "ECMP",
                                    ROUTE_MOD_SPRAY, "spray",
                                    ROUTE_MOD_SOURCE, "source"))
      .AddAttribute("MarkDetourPath", "xx",
                    BooleanValue(false), MakeBooleanAccessor(&ECMPRoutingProtocol::m_markdetour), MakeBooleanChecker())
      .AddAttribute("TracePath", "Enables hop capture at each switch and counts packet detours",
                    BooleanValue(false), MakeBooleanAccessor(&ECMPRoutingProtocol::m_tracepath), MakeBooleanChecker())
      .AddAttribute("TTLLimit", "xx", 
                    UintegerValue(255), MakeUintegerAccessor(&ECMPRoutingProtocol::m_TTL_limit), MakeUintegerChecker<uint32_t>()) //TTL Limit
      .AddAttribute("TTDLimit", "xx", 
                    UintegerValue(100000), MakeUintegerAccessor(&ECMPRoutingProtocol::m_TTD_limit), MakeUintegerChecker<uint32_t>()) //TTD Limit

      
      ;
    
    return tid;
  }
  
  ECMPRoutingProtocol::ECMPRoutingProtocol()
    :
    m_totalBytes (0),
    m_dropcnt (0),
    m_robin (0),
    m_starttime (0.0),
    m_forwardbytes (0.0)
  {
    for(uint32_t ii = 0; ii < 20; ii++) {
      m_incastdetourcount[ii] = 0;
      m_backdetourcount[ii] = 0;
    }
  }
  
  ECMPRoutingProtocol::~ECMPRoutingProtocol()
  {
  }
  
  uint32_t
  ECMPRoutingProtocol::GetNAddresses(uint32_t interface) const
  {
    return 1;
  }
  
  Ipv4InterfaceAddress
  ECMPRoutingProtocol::GetAddress(uint32_t interface, uint32_t addressIndex) const
  {
    return Ipv4InterfaceAddress(Ipv4Address(GetNode()->GetId()), Ipv4Mask::GetZero());
  }
  
  void
  ECMPRoutingProtocol::Send (Ptr<Packet> packet, Ipv4Address source, Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)
  {

    
    ECMPHeader header;
    header.SetProtocol(protocol);
    header.SetLivingTime(0);
    header.SetSrc(source.Get());
    header.SetDest(destination.Get());
    
    TcpHeader tcphdr;
    packet->RemoveHeader(tcphdr);
    header.SetSport(tcphdr.GetSourcePort());
    header.SetDport(tcphdr.GetDestinationPort());

    /*
    if (packet->GetSize() != 0) {
      ECMPTag tag;
      packet->PeekPacketTag(tag);
      uint32_t flowsize = tag.GetRemainingByte();
      std::cout<<"GetSize: "<<flowsize<<std::endl;
    }
    */


    
    packet->AddHeader(tcphdr);

    if (m_ecncapable)
      header.SetEcn(ECMPHeader::ECN_EN);
    else
      header.SetEcn(ECMPHeader::ECN_DIS);
    
    
    
    header.SetDetourCount(0);  //init
    
    if (m_priority)
      {
	//test prio
	//std::cout<<"[ecmp] Send: test prio"<<std::endl;
	if ( header.GetSport() == 10 || header.GetDport() == 10 ||header.GetSport() == 10 || header.GetDport() == 20 )  //test
	  {
	    //std::cout<<"prio hi"<<std::endl;
	    header.SetPrio(1);
	  }
	else
	  header.SetPrio(0);
      } 
  
    packet->AddHeader(header);

    if (m_tracepath) {
      std::cout<<Simulator::Now().GetSeconds()<<" [SEND] "<<header.GetSrc()<<":"<<header.GetSport()<<"->"
               <<header.GetDest()<<":"<<header.GetDport()<<" SIZE "<<packet->GetSize()<<std::endl;
    }

    //std::cout<<"startsend"<<std::endl;      
    SendPacket(GetNode()->GetDevice(0), packet);
    
    
    // Search for path in the lookup table
    /*
      Lookup_t::iterator iter = m_lookup.find (destination.Get());
      if (iter == m_lookup.end ())
      {
      // Path not found in the lookup table, create a path
      std::vector<uint32_t> path;
      bool pathFound = ECMP(source.Get(), destination.Get(), path);
      if(pathFound)
      {
      // Insert into the lookup table
      m_lookup.insert(Lookup_t::value_type(destination.Get(), path));
      iter = m_lookup.find(destination.Get());
      }
      else
      std::cout << " ECMPRoutingProtocol::Send() Output Device not Found!" << std::endl;
      }
      header.SetPath(iter->second);
      packet->AddHeader(header);
            
      uint32_t index;
      if( FindOutputDevice (header.GetNextId(), index) )
      {
      Ptr<NetDevice> device = GetNode()->GetDevice (index);
      #if IsDump
      std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") ECMPRoutingProtocol::SendPacket() Header: ";
      header.Dump();
      #endif
      m_totalBytes += packet->GetSize();
      SendPacket(device, packet);
      }
      else
      std::cout << " ECMPRoutingProtocol::Send() Output Device not Found!" << std::endl;
    */
    
    
  }

  void
  ECMPRoutingProtocol::Receive(Ptr<NetDevice> indev, Ptr<const Packet> p, uint16_t protocol,const Address &from, const Address &to, NetDevice::PacketType packetType)
  {
    Ptr<Packet> packet = p->Copy();
    ECMPHeader ECMPhdr;
    //TcpHeader tcphdr;
    packet->RemoveHeader(ECMPhdr);
    //packet->AddHeader(tcphdr);

    //std::cout<<"receive"<<std::endl;
#if IsDump
    std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") ECMPRoutingProtocol::Receive() Header: ";
    header.Dump();
#endif
                
    if (ECMPhdr.GetDest() != GetNode()->GetId())
      {
	//If *this* node is not the current destination
          
	//increment Living Time
	uint8_t livingTime = ECMPhdr.GetLivingTime() + 1;
	if (livingTime > m_TTL_limit) {
	  
	  std::cout<<Simulator::Now().GetSeconds()
		   <<" [SW] "<<GetNode()->GetId()
		   << " Drop[TTL] " << ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
		   <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" "
		   <<" SIZE "<< packet->GetSize()
		   <<" TTD "<< ECMPhdr.GetDetourCount()<<" TTL "<<uint32_t(ECMPhdr.GetLivingTime());
	  std::vector<uint32_t> * detourPath = ECMPhdr.GetPath();
	  std::cout << " Path: ";
	  
	  for(uint32_t i=0; i<detourPath->size(); i++)
	    {
	      std::cout << detourPath->at(i) << " ";
	    }
	    std::cout << std::endl;
	    
	   
	  return;
	}
	ECMPhdr.SetLivingTime(livingTime);

          
            
	if(m_tracepath)
          {
            
            std::vector<uint32_t> * detourPath = ECMPhdr.GetPath();
            if(detourPath->size() == 0)
              {
                //this must be the first hop switch so sent the sender
                uint32_t sender = ECMPhdr.GetSrc();
                detourPath->push_back(sender);
              }
            detourPath->push_back(GetNode()->GetId());
            //std::cout << "foobar: " << detourPath->size() << std::endl;
          }
          
	uint32_t shortest_path_index = 0, detour_path_index = 0;
	bool detour_flag = 0;
	bool detour_fail = 0;
	uint16_t hash_value;
	hash_value = hash(ECMPhdr, indev->GetIfIndex());
	bool ps = 0;
	if (m_mode == ROUTE_MOD_ECMP)
	  ps = ECMP (ECMPhdr.GetDest(), hash_value, shortest_path_index);
	else if (m_mode == ROUTE_MOD_SPRAY)
	  ps = Spray(ECMPhdr.GetDest(), ECMPhdr.GetLivingTime(), shortest_path_index);

	/*
          else if (m_mode == ROUTE_MOD_SOURCE)  //use for test now
          {
	  uint8_t srl = ECMPhdr.GetSrl();
	  if (srl < 2 && ECMPhdr.GetSrc() < 11)
	  {
	  //std::cout<<GetNode()->GetId()<<"->"<<ECMPhdr.GetNextId()<<std::endl;
	  ps = SourceRoute(ECMPhdr.GetNextId(), shortest_path_index);
                
	  if (ps)
	  ECMPhdr.SetSrl(srl+1);
                
	  }
	  else
	  ps = ECMP (ECMPhdr.GetDest(), hash_value, shortest_path_index);
          }
	*/
          
	if (ps)
    {

#if IsDump
            std::cout << Simulator::Now().GetSeconds() << " Node(" << GetNode()->GetId() << ") ECMPRoutingProtocol::SendPacket() Header: ";
            header.Dump();
#endif
            m_totalBytes += packet->GetSize();
            Ptr<NetDevice> device;
            //ECNMark(GetNode()->GetDevice(index), ECMPhdr, pakcet->GetSize());
            packet->AddHeader(ECMPhdr);
            
          if (m_usebounce)
	      {
            if ( (m_priority && DetourChoice(GetNode()->GetDevice (shortest_path_index), ECMPhdr.GetPrio(), packet->GetSize()))
                 || (!m_priority && (DetourNeed( GetNode()->GetDevice (shortest_path_index),shortest_path_index,  packet->GetSize()))) )   
            {
              //if we get here then we have commited to bouncing I think

              //bounce
                
        
              packet->RemoveHeader(ECMPhdr);  //we need to reset the ECMPhdr
              uint32_t detourCount = ECMPhdr.GetDetourCount() + 1;  //increment detour count by one
              ECMPhdr.SetDetourCount(detourCount);
              //test
              /*
                if (m_tracepath) {
                std::cout<<Simulator::Now().GetSeconds()
                <<" [SW] "<<GetNode()->GetId()
                << " Detour " << ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
                <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" "
                <<" SIZE "<< packet->GetSize()
                <<" TTD "<< ECMPhdr.GetDetourCount()<<" TTL "<< (int)ECMPhdr.GetLivingTime()<<std::endl;
                }
            */
            
		    packet->AddHeader(ECMPhdr);


		    //TTD drop packet
		    if (detourCount > m_TTD_limit) {
		      std::cout<<Simulator::Now().GetSeconds()
			       <<" [SW] "<<GetNode()->GetId()
			       << " Drop[TTD] " <<" "<<ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
			       <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" "
			       <<" SIZE "<< packet->GetSize()
			       <<" TTD "<< ECMPhdr.GetDetourCount()<<" TTL "<< (int)ECMPhdr.GetLivingTime()<<std::endl;;
		      return;
		    }


		    //test
		    //if (ECMPhdr.GetSrc() == 0 && ECMPhdr.GetDest() == 15)
		    //  std::cout<<"bounce high pro"<<std::endl;
		    //std::cout<<"Bouncing from "<<index;
		    //detour_path_index = shortest_path_index;
		    if (!FindBouncingDevice(ECMPhdr.GetDest(), ECMPhdr, packet->GetSize(), detour_path_index))
		      {
                //std::cout<<GetNode()->GetId()<<": drop due to no space to detour"<<std::endl;
                //return;
                detour_path_index = shortest_path_index;
                detour_fail = 1;
		      }
		    //bounce to other port
		    //std::cout<<" to "<<index<<std::endl;
            
		    detour_flag = 1;
		  }


		packet->RemoveHeader(ECMPhdr);
              
		if (detour_flag) // has detour?
		  {
		    //std::cout<<"detour: "<<shortest_path_index<<" "<<detour_path_index<<std::endl;
		    //bool flag;
		    device = GetNode()->GetDevice (detour_path_index);
            ECMPhdr.SetSendport(detour_path_index);
		    if (m_markdetour)
		      ECNMark(GetNode()->GetDevice(detour_path_index), ECMPhdr, packet->GetSize());
		    else
		      {
			//std::cout<<"short path"<<std::endl;
			ECNMark(GetNode()->GetDevice(shortest_path_index), ECMPhdr, packet->GetSize()); 
		      }
		  }
		else
		  {
		    device = GetNode()->GetDevice (shortest_path_index);
            ECMPhdr.SetSendport(shortest_path_index);
		    ECNMark(GetNode()->GetDevice(shortest_path_index), ECMPhdr, packet->GetSize());
		    //std::cout<<"shortest"<<shortest_path_index<<std::endl;
                
		  }
             
		packet->AddHeader(ECMPhdr);

       
		//report crossbar
		uint32_t num_pkt_short = 0, detoured_pkt_short = 0, num_pkt_detour = 0, detoured_pkt_detour = 0;
		Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(GetNode()->GetDevice(shortest_path_index));
		dev->ReportQueueInfo (num_pkt_short, detoured_pkt_short);

		dev = DynamicCast<PointToPointNetDevice>(GetNode()->GetDevice(detour_path_index));
		dev->ReportQueueInfo (num_pkt_detour, detoured_pkt_detour);

		bool send_fail = 0;
              
		if (!SendPacket(device, packet))
		  {
		    send_fail = 1;
		    if (detour_fail == 1) {
		      //std::cout<<"Drop for no space to detour: "<<GetNode()->GetId()<<" "<<ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
		      //       <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" "
		      //       <<packet->GetSize()<<" "<<ECMPhdr.GetDetourCount()<<std::endl;
		      ;
		    } else {
		      std::cout<<GetNode()->GetId()<<" Send fail"<<std::endl;
		      exit(1);
		    }
		  }

		if (m_tracepath)  {

          if (Simulator::Now().GetMilliSeconds() >= global_time)
          {
           
            std::cout<<Simulator::Now().GetMilliSeconds() << " link ";
            uint32_t numberOfNodes = NodeList::GetNNodes();
            
            for (uint32_t i = 0 ; i < numberOfNodes; ++i)
            {
              if( i < m_numhost)
                continue;

              //test

              /*
              if(i == 128)
                {
                  for (uint32_t k = 0; k < NodeList::GetNode(i)->GetNDevices(); ++k)
                    {
                  Ptr<NetDevice> netdev = NodeList::GetNode(i)->GetDevice(k);
                  Ptr<Channel> channel = netdev->GetChannel ();
                  if (channel == 0)
                    {
                      continue;
                    }
              
                  for (uint32_t j = 0; j < channel->GetNDevices (); j++)
                    {
                      if (channel->GetDevice(j) != netdev)
                        {
                          Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
                          Ptr<Node> remoteNode = remoteDevice->GetNode();
                          std::cout<<k<<"port to "<<remoteNode->GetId()<<"; ";
                        }
                    }
                    }

                  exit(1);
                }
              */

              
              Ptr<Node> pNode = NodeList::GetNode(i);
              for(uint32_t j = 0; j < pNode->GetNDevices(); ++j)
              {
                
                Ptr<NetDevice> pNetdev = pNode->GetDevice(j);
                Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(pNetdev);
                uint32_t pktcnt = 0;
                dev->CyclePktCount(pktcnt);
                std::cout<<"("<<i<<","<<j<<","<<pktcnt<<") ";
              }
            }
            //if(global_time == 2)
            // exit(1);
            global_time += 1;
             std::cout<<std::endl;
          }
         
          
		  std::cout<<Simulator::Now().GetSeconds()<<" [SW] "<<GetNode()->GetId()<<" "<<ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
			   <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" SIZE "
			   <<packet->GetSize()<<" DetourCnt "<<ECMPhdr.GetDetourCount();
		  if (send_fail)
		    std::cout<<" Drop "<<indev->GetIfIndex()<<"=>"<<shortest_path_index
			     <<"("<<num_pkt_short<<","<<detoured_pkt_short<<")";
		  else if (detour_flag)
            {
              std::vector<uint32_t> node_list;
              uint32_t onehopbuffer = GetAvailableBuffer(ECMPhdr.GetDest(), ECMPhdr, GetNode(), node_list, 1);
              uint32_t ori_size_1hop = node_list.size();
              node_list.clear();
              uint32_t twohopbuffer =  GetAvailableBuffer(ECMPhdr.GetDest(), ECMPhdr, GetNode(), node_list, 2);
              uint32_t ori_size_2hop = node_list.size();

		    std::cout<<" Detour "<<indev->GetIfIndex()<<"=>"<<shortest_path_index
			     <<"("<<num_pkt_short<<","<<detoured_pkt_short<<")"<<"=>"
                     <<detour_path_index<<"("<<num_pkt_detour<<","<<detoured_pkt_detour<<")"
                     << " buf "<<onehopbuffer<<","<<twohopbuffer
                     << " neighbor "<<ori_size_1hop<<","<<ori_size_2hop;
            }
		  else
		    std::cout<<" Enqueue "<<indev->GetIfIndex()<<"=>"<<shortest_path_index
			     <<"("<<num_pkt_short<<","<<detoured_pkt_short<<")";
                
		  std::cout<<std::endl;
		}
	      }
            else
	      {
		packet->RemoveHeader(ECMPhdr);
		device = GetNode()->GetDevice (shortest_path_index);
        ECMPhdr.SetSendport(shortest_path_index);
		ECNMark(GetNode()->GetDevice(shortest_path_index), ECMPhdr, packet->GetSize());

		packet->AddHeader(ECMPhdr);

		//report crossbar
		uint32_t num_pkt_short = 0, detoured_pkt_short = 0;
		Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(GetNode()->GetDevice(shortest_path_index));
		dev->ReportQueueInfo (num_pkt_short, detoured_pkt_short);
		bool drop_flag = 0;
              
		if(!SendPacket(device, packet))
		{
          drop_flag = 1;
          m_dropcnt++;
		}

		if (m_tracepath)
        {              
		  std::cout<<Simulator::Now().GetSeconds()<<" [SW] "<<GetNode()->GetId()<<" "<<ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
                   <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()
                   <<" SIZE "<<packet->GetSize();
		  if(drop_flag)
		    std::cout<<" Drop"<<indev->GetIfIndex()<<"=>"<<shortest_path_index<<"("<<num_pkt_short<<")";
		  else
		    std::cout<<" Enqueue"<<indev->GetIfIndex()<<"=>"<<shortest_path_index<<"("<<num_pkt_short<<")";
		  std::cout<<std::endl;
		}
          }    
    }
	else
	  std::cout << " ECMPRoutingProtocol::Send() Output Device not Found!" << std::endl;          
    }
        
    else
    {
            
      if(m_tracepath)
	  {
	    std::vector<uint32_t> * detourPath = ECMPhdr.GetPath();
        
	    std::cout<<Simulator::Now().GetSeconds()
                 << " [RECV] " << ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
                 <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" "
                 <<" SIZE "<< (packet->GetSize()+ ECMPhdr.GetSerializedSize())
                 <<" DetourCnt "<< ECMPhdr.GetDetourCount();
              
	    std::cout << " Path: ";
	    for(uint32_t i=0; i<detourPath->size(); i++)
	    {
          std::cout << detourPath->at(i) << " ";
        }
	    std::cout << ECMPhdr.GetDest() << std::endl;
              
	  }

#if 0
      //measure throughput
      if (m_starttime == 0.0)
      {
        m_starttime = Simulator::Now().GetSeconds();
        m_forwardbytes = 0.0;
      }
      m_forwardbytes += packet->GetSize();// + ECMPhdr.GetSerializedSize();
#endif
      
            
            
      //std::cout<<Simulator::Now().GetSeconds()<<" [RECV] "<<ECMPhdr.GetSrc()<<":"<<ECMPhdr.GetSport()<<"->"
      //       <<ECMPhdr.GetDest()<<":"<<ECMPhdr.GetDport()<<" SIZE "<<packet->GetSize()+ECMPhdr.GetSerializedSize()<<std::endl;
      
            

      Ptr<IpL4Protocol> protocol = GetProtocol (ECMPhdr.GetProtocol());
            
      Ipv4Header ipv4Header;
      ipv4Header.SetSource(Ipv4Address(ECMPhdr.GetSrc()));
      ipv4Header.SetDestination(Ipv4Address(ECMPhdr.GetDest()));
      
      if (ECMPhdr.GetEcn()== ECMPHeader::ECN_MARK)
        ipv4Header.SetEcn(Ipv4Header::CE);
      
      Ptr<Ipv4Interface> ipv4Interface = Create<Ipv4Interface>();
      protocol->Receive(packet, ipv4Header, ipv4Interface);
    }
  }

  // =================== ECMP Assistant functions ================================

  bool
  ECMPRoutingProtocol::Spray (uint32_t dest, uint8_t t, uint32_t& index)
  {
    uint32_t m_n = 0;
    for (uint32_t i = 0; ;++i)
      {
        if (i * i * i / 4 == m_numhost)
	  {
	    m_n = i;
	    break;
	  }
      }
      
    if (t >= 3 ) //will modify this when increase our size
      {
        uint32_t srv_num = m_n * m_n * m_n / 4;
        uint32_t pot_switch_num = m_n * m_n;
        uint32_t id = GetNode()->GetId();
        
        if (id < srv_num + pot_switch_num / 2) //edge
          index = dest % (m_n / 2);
        else if (id < srv_num + pot_switch_num)  //aggregate
          index = (dest % (m_n * m_n / 4)) / (m_n / 2);
        else    //core
          index = dest / (m_n * m_n / 4);
      }
    else
      {
          
	//devid = hash(header, device->GetIfIndex()) % (m_n / 2) + m_n / 2;
          
	index = m_robin + m_n / 2;
	m_robin++;
	if (m_robin == m_n / 2)
	  m_robin = 0;
      }

    //if (GetNode()->GetId()== 17 && index > 1)
    // {
    //std::cout<<"robin "<<m_robin<<" output "<<index<<std::endl;
    //}
      
    return 1;
      
  }


  
  bool
  ECMPRoutingProtocol::SourceRoute (uint32_t dest, uint32_t &index)
  {
    bool indexfound = false;
    uint32_t minhops = 1000;
    uint32_t candid = 0;

    Forward_t::iterator iter = m_forward.find (dest);
    if (iter == m_forward.end ())
      {
	AddForwardEntry(dest);
	iter = m_forward.find (dest);
	if (iter == m_forward.end ())
	  return false;
      }
    //seach existing flow entry
        
    for (std::map<uint32_t, uint32_t>::iterator it = iter->second.begin(); it != iter->second.end(); it++)
      {
	if (it->second == minhops)
	  candid++;
	else if (it->second < minhops)
	  {
	    minhops = it->second;
	    candid = 1;
	  }
      }
          
     
    for (std::map<uint32_t, uint32_t>::iterator it = iter->second.begin(); it != iter->second.end(); it++)
      {   
	if (it->second == minhops)
	  {     
	    index = it->first;
	    indexfound = true;
	    break;
	  }
      }
 
    return indexfound;
 
  }


  
  
  bool
  ECMPRoutingProtocol::ECMP (uint32_t dest, uint16_t hash_value, uint32_t &index)
  {
    bool indexfound = false;
    uint32_t minhops = 1000;
    uint32_t candid = 0;
    //Forward_t::iterator iter = m_forward.find (nextId);
    //std::cout<<"test"<<std::endl;
    Forward_t::iterator iter = m_forward.find (dest);
    if (iter == m_forward.end ())
      {
	AddForwardEntry(dest);
	iter = m_forward.find (dest);
	if (iter == m_forward.end ())
	  return false;
      }
    //seach existing flow entry
        
    for (std::map<uint32_t, uint32_t>::iterator it = iter->second.begin(); it != iter->second.end(); it++)
      {
	if (it->second == minhops)
	  candid++;
	else if (it->second < minhops)
	  {
	    minhops = it->second;
	    candid = 1;
	  }
      }
          
    uint32_t pos = hash_value % candid;
    for (std::map<uint32_t, uint32_t>::iterator it = iter->second.begin(); it != iter->second.end(); it++)
      {   
	if (it->second == minhops)
	  {
	    if (pos == 0)
	      {
		index = it->first;
		indexfound = true;
		break;
	      }
	    else
	      pos--;
	  }
      }
          
    return indexfound;
  }

  //continuously try every port 
  bool
  ECMPRoutingProtocol::FindBouncingDevice(uint32_t dest, const ECMPHeader& header, uint32_t pktsize, uint32_t &index)
  {
    Forward_t::iterator iter = m_forward.find (dest);
    if (iter == m_forward.end ())
      {
        //std::cout<<"cannot find bouncing device"<<std::endl;
        std::cout<<"no entry"<<std::endl;
        exit(1);
        //return 0;
      }
    else
      {
        std::vector<uint32_t>temp, sequence;
        temp.reserve(GetNode()->GetNDevices());
        sequence.reserve(GetNode()->GetNDevices());
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i)
          temp.push_back(i);
        while (temp.size() >0 )
	  {
	    uint32_t pos = m_rand.GetInteger(0, temp.size()-1);
	    sequence.push_back(temp[pos]);
	    temp.erase(temp.begin()+pos);
	  }


        //PRIO
        if (m_priority)
	  {
	    for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i)
	      {
		Ptr<NetDevice> localNetDevice = GetNode()->GetDevice (sequence[i]);
              
		if (DetourChoice(localNetDevice, header.GetPrio(), pktsize))
		  continue;
          
		Ptr<Channel> channel = localNetDevice->GetChannel ();
		if (channel == 0)
		  {
		    continue;
		  }
              
		for (uint32_t j = 0; j < channel->GetNDevices (); j++)
		  {
		    if (channel->GetDevice(j) != localNetDevice)
		      {
			Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
			Ptr<Node> remoteNode = remoteDevice->GetNode();
                      
			if (remoteNode->GetId() >= m_numhost)
			  {
			    //std::cout<<"Bounce from "<<GetNode()->GetId()<<" to "<<remoteNode->GetId()<<std::endl;
			    index = sequence[i];
			    return 1;
			  }                                       
		      }
		  }
	      }
	  }

        //MAX
        //detour to switch only
        for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i)
	  {
	    Ptr<NetDevice> localNetDevice = GetNode()->GetDevice (sequence[i]);
          
	    if (DetourNeed(localNetDevice, sequence[i], pktsize))
	      continue;
          
	    Ptr<Channel> channel = localNetDevice->GetChannel ();
	    if (channel == 0)
	      {
		continue;
	      }
          
	    for (uint32_t j = 0; j < channel->GetNDevices (); j++)
	      {
		if (channel->GetDevice(j) != localNetDevice)
		  {
		    Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
		    Ptr<Node> remoteNode = remoteDevice->GetNode();
                  
		    if (remoteNode->GetId() >= m_numhost)
		      {
			//std::cout<<"Bounce from "<<GetNode()->GetId()<<" to "<<remoteNode->GetId()<<std::endl;
			index = sequence[i];
			return 1;
		      }                                   
		  }
	      }
	  }


        
        //detour to host with low priority
        /*
	  for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i)
	  {
          Ptr<NetDevice> localNetDevice = GetNode()->GetDevice (sequence[i]);
          
          if (DeviceQueueExceed(localNetDevice, pktsize))
	  continue;
          
          Ptr<Channel> channel = localNetDevice->GetChannel ();
          if (channel == 0)
	  {
	  continue;
	  }
          
          for (uint32_t j = 0; j < channel->GetNDevices (); j++)
	  {
	  if (channel->GetDevice(j) != localNetDevice)
	  {
	  Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
	  Ptr<Node> remoteNode = remoteDevice->GetNode();
                  
	  //if (remoteNode->GetId() >= m_numhost)
	  //{
	  //std::cout<<"Bounce from "<<GetNode()->GetId()<<" to "<<remoteNode->GetId()<<std::endl;
	  index = sequence[i];
	  return 1;
	  //}                                       
	  }
	  }
	  }
        */
        
        
        //std::cout<<"Cannot find bouncing device"<<std::endl;
        return 0;
      }

  }



    uint32_t GetAvailableDeviceBuffer(Ptr<Node> NetNode)
  {
    uint32_t available = 0;
      for (uint32_t i = 0; i < NetNode->GetNDevices(); ++i)
	  {
	    Ptr<NetDevice> Netdev = NetNode->GetDevice (i);
        Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(Netdev);
        Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
        uint32_t maxq, qlen;
        queue->GetQueueLength(qlen, maxq);
        available += (maxq - qlen);
      }
      return available;
  }

  
  uint32_t
  ECMPRoutingProtocol::GetAvailableBuffer(uint32_t dest, const ECMPHeader& header, Ptr<Node> node, std::vector<uint32_t>& node_list,
                                          uint32_t deep)
  {
    uint32_t buffer = 0;
    
    Forward_t::iterator iter = m_forward.find (dest);
    if (iter == m_forward.end ())
      {
        //std::cout<<"cannot find bouncing device"<<std::endl;
        std::cout<<"no entry"<<std::endl;
        exit(1);
        //return 0;
      }
    else
      {
        std::vector<uint32_t>temp, sequence;
        temp.reserve(node->GetNDevices());
        sequence.reserve(node->GetNDevices());
        for (uint32_t i = 0; i < node->GetNDevices(); ++i)
          temp.push_back(i);
        while (temp.size() >0 )
	  {
	    uint32_t pos = m_rand.GetInteger(0, temp.size()-1);
	    sequence.push_back(temp[pos]);
	    temp.erase(temp.begin()+pos);
	  }


        //MAX
        //detour to switch only
        for (uint32_t i = 0; i < node->GetNDevices(); ++i)
	  {
	    Ptr<NetDevice> localNetDevice = node->GetDevice (sequence[i]);
          
	    //if (DetourNeed(localNetDevice, sequence[i], pktsize))
	    //  continue;
          
	    Ptr<Channel> channel = localNetDevice->GetChannel ();
	    if (channel == 0)
	    {
          continue;
        }
          
	    for (uint32_t j = 0; j < channel->GetNDevices (); j++)
	    {
          if (channel->GetDevice(j) != localNetDevice)
          {
		    Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
		    Ptr<Node> remoteNode = remoteDevice->GetNode();

            
		    if (remoteNode->GetId() >= m_numhost )
		    {
              //std::cout<<"Bounce from "<<GetNode()->GetId()<<" to "<<remoteNode->GetId()<<std::endl;
              //index = sequence[i];
              //return 1;
              std::vector<uint32_t>::iterator it;
              bool counted = false;
              for (it = node_list.begin(); it != node_list.end(); ++it)
              {
                if (*it == remoteNode->GetId())
                  {
                        counted = true;
                        break;
                      }
              }

                if (counted == true)
                  continue;


                buffer += GetAvailableDeviceBuffer(remoteNode);
                node_list.push_back(remoteNode->GetId());
                
                uint32_t deep_curr = deep - 1;
                if (deep_curr > 0 )
                {
                  buffer += GetAvailableBuffer(dest, header, remoteNode, node_list, deep_curr);
                }
		      }                                   
		  }
	      }
	  }
        return buffer;
      }

  }

  


  
  bool
  ECMPRoutingProtocol::BFS (uint32_t source, uint32_t dest, std::vector<uint32_t> &path, uint32_t &hops)
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

    // ECMP loop
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
	if (source == dest)
	  hops = 0;
	else
	  hops = 1;
	path.clear();
	path.insert( path.begin(), dest );
	uint32_t temp = parentVector[dest];
	//std::cout<<"source = "<<source<<": "<<dest<<" ";
          
	while( temp != source )
	  {
	    hops++;
	    path.insert( path.begin(), temp );
	    temp = parentVector[temp];
	    //std::cout<<temp<<" ";
	  }
	//std::cout<<std::endl;
	return true;
      }
    else
      return false;
  }

  void
  ECMPRoutingProtocol::AddForwardEntry(uint32_t dest)
  {
    // Output device not found in the forwarding table, insert
    uint32_t numberOfDevices = GetNode()->GetNDevices ();
    // scan through the net devices on the parent node
    // and then look at the nodes adjacent to them
    std::map<uint32_t, uint32_t> routes;
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
	    if (channel->GetDevice(j) != localNetDevice)
	      {
		Ptr<NetDevice> remoteDevice = channel->GetDevice (j);
		Ptr<Node> remoteNode = remoteDevice->GetNode();
		std::vector<uint32_t> path;                   
		uint32_t hops;
		// if this node is the next hop node
		if (BFS(remoteNode->GetId(), dest, path, hops))
		  routes.insert(std::map<uint32_t, uint32_t>::value_type(i, hops + 1));
	      }
	  }
      }
      
    m_forward.insert(Forward_t::value_type(dest,routes));

    //static int num = 0;
    //std::cout<<"new entry "<<++num<<std::endl;
    //std::cout<<"hash = "<<hash_value<<" candid = "<<candid<<" output port = "<<index<<std::endl;   
     
      
    //display the routing hops
    /*
      std::cout<<"new entry"<<std::endl;
      Forward_t::iterator jj = m_forward.find(dest);
      for (std::map<uint32_t, uint32_t>::iterator it = jj->second.begin(); it != jj->second.end(); it++)
      {
      std::cout<<it->first<<" "<<it->second<<std::endl;
      }
    */
      
    //exit(1);
  }


  

  uint64_t
  ECMPRoutingProtocol::GetTotalBytes()
  {
    return m_totalBytes;
  }


  
  uint16_t ECMPRoutingProtocol::hash(ECMPHeader& header, uint32_t inport)
  {
    uint16_t value = 0;
    // uint64_t now = Simulator::Now().GetNanoSeconds();

    value ^= (header.m_src & 0x0000FFFF);
    value ^= (header.m_src & 0xFFFF0000) >> 16;
    value ^= header.m_sport;
    value ^= header.m_dport;
    value ^= (header.m_dest & 0x0000FFFF);
    value ^= (header.m_dest & 0xFFFF0000) >> 16;
    //value += 1;
    //std::cout<<header.m_src<<" "<<header.m_dest<<" "<<header.m_sport<<" "<<header.m_dport<<std::endl;
    //if (m_type == ECMP_FATTREE_PACKET_LEVEL)
    //  {
    //    return rand();
    // value ^= now & 0x000000000000FFFF;
    // value ^= (now & 0x00000000FFFF0000) >> 16;
    // value ^= (now & 0x0000FFFF00000000) >> 32;
    // value ^= (now & 0xFFFF000000000000) >> 48;
    // value ^= (header.timestamp & 0x0000FFFF);
    // value ^= (header.timestamp & 0xFFFF0000) >> 16;
    // }
                
    return value ^ inport;
    //return 0;
  }

  bool ECMPRoutingProtocol::DetourNeed (Ptr<NetDevice> Netdev, uint32_t port, uint32_t pktsize) const
  {
    //bool needdetour = false;
    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(Netdev);
    //check buffer overflow
    if(dev->CheckBufferOverflow (port, pktsize))
      //needdetour = true;
      return true;
 
    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
    uint32_t maxq, qlen;
    queue->GetQueueLength(qlen, maxq);
    //std::cout<<qlen<<" "<<pktsize<<" "<<qlen+pktsize<<" ? "<<maxq<<std::endl;
    //return queue->Overflow(pktsize);
    if ((qlen + pktsize + 10) >= m_detourthresh)
      //needdetour = true;
      return true;

    
    //return needdetour;
    return false;
    
  }


  
  bool ECMPRoutingProtocol::DetourChoice (Ptr<NetDevice> Netdev, uint32_t prio, uint32_t pktsize) const
  {
    uint32_t qlen, qmax;
      
    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(Netdev);
    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
    queue->GetQueueLength(qlen, qmax);
    //std::cout<<"qlen = "<<qlen<<", qmax = "<<qmax<<std::endl;
    if ((qlen >= m_detourthresh && prio == 0) || (qlen + pktsize + 10 >= qmax) )
      return 1;
    else
      return 0;
  }
 
  
  /*
    bool ECMPRoutingProtocol::DetourChoice (Ptr<NetDevice> Netdev, uint32_t prio) const
    {
    uint32_t qlen, qmax;
    UniformVariable rand;
    uint32_t high[2] = {50*1500,80*1500};
    uint32_t low[2] = {20*1500,50*1500};
    if (prio >= sizeof(high)/sizeof(uint32_t))
    {
    std::cout<<"out of prio range"<<std::endl;
    return 1;
    }
      
    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(Netdev);
    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
    queue->GetQueueLength(qlen, qmax);
    //std::cout<<"qlen = "<<qlen<<", qmax = "<<qmax<<std::endl;
    
    uint32_t prob;
    if (qlen > high[prio])
    prob = 1000;
    else if (qlen < low[prio])
    prob = 0;
    else
    {
    prob = (uint32_t)(double(qlen - low[prio]) / (high[prio] - low[prio]) * 1000);
    }
    uint32_t randnum = rand.GetInteger(0,1000);
    //std::cout<<randnum<<" "<<prob<<std::endl;

    if (randnum < prob)
    {
    //std::cout<<"bounce"<<std::endl;
    return 1;   //bounce
    }
    else
    return 0;   //do not bounce
    }
 
  */
  
  bool
  ECMPRoutingProtocol::ECNMark(Ptr<NetDevice> Netdev, ECMPHeader& ECMPhdr, uint32_t pktsize)
  {
    uint32_t qlen, qmax;
    if ( ECMPhdr.GetEcn() != ECMPHeader::ECN_EN )
      {
	return 0;
      }
    else
      {
	Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(Netdev);
	Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
	queue->GetQueueLength(qlen, qmax);
	//std::cout<<"qlen = "<<qlen<<", qmax = "<<qmax<<m_ecnthreshold<<std::endl;
	//std::cout<<"size"<<ECMPhdr.Get
	if ( qlen + pktsize + 10 >= m_ecnthreshold)  // slight mod since we cutoff the max buffer 
	  {
	    //uint8_t flags = tcphdr.GetFlags();
	    //tcphdr.SetFlags(flags | TcpHeader::ECE);
	    ECMPhdr.SetEcn(ECMPHeader::ECN_MARK);
	    //std::cout<<"[ECMP]ECN Mark "<< m_ecnthreshold<<" "<<qlen+pktsize+10<<std::endl;
	    return 1;
	  }

	return 0;
      }
  }

  uint32_t
  ECMPRoutingProtocol::ReportDetourCnt(uint64_t *pback, uint64_t *pincast)
  {

    for (uint32_t ii = 0; ii < 20; ++ii) {
      pback[ii] = m_backdetourcount[ii];
      pincast[ii] = m_incastdetourcount[ii];
    }

    return 1;
  }
  
  uint32_t
  ECMPRoutingProtocol::ReportDrop()
  {
    //std::cout<<"total packet drops = "<<m_dropcnt<<std::endl;
    uint32_t drop = m_dropcnt;
    m_dropcnt = 0;
    return drop;
  }

  double
  ECMPRoutingProtocol::ReportThroughput()
  {
    double timenow = Simulator::Now().GetSeconds();
    double th = m_forwardbytes * 8 / (timenow - m_starttime);
    return th;
  }

 



  
}



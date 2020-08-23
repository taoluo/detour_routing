#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/net-routing-module.h"
#include "ns3/topology-module.h"
#include "ns3/bfs-routing-stack-helper.h"
#include "ns3/bulk-send-application.h"
#include "ns3/ecmp-fattree-routing-stack-helper.h"
#include "ns3/packet-sink.h"
#include "sys/stat.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/random-variable.h"
#include "ns3/random-variable-stream.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <set>

//#include "ns3/tcp-newreno.h"
//#include "ns3/drop-tail-queue.h"

using namespace ns3;
using namespace std;



//Global variables
#define INCAST_QPS 10000
#define INTER_ARRIVAL 120
#define INCAST_DEGREE 40
#define INCAST_SIZE 20
#define SWITCH_BUFFER 100
#define OVERSUBSCRIPTION 1




void
Foo(int round)
{
  
  std::cout<<"=====round  "<<round<<"====="<<std::endl;
}

void
Goo(int num)
{
  std::cout<<std::endl<<"+++++++++++++++++++++++"<<std::endl<<"     flow num = "
	   <<num<<std::endl<<"++++++++++++++++++++++++++"<<std::endl<<std::endl;
}

void
DumpStatus(NodeContainer& host, NodeContainer& sw)
{
  //fattree->GetNode(destId)->GetObject<ECMPRoutingProtocol>()->ReportDrop();
  //dump switch status
  uint32_t drop = 0;
  for(uint32_t i = 0; i < sw.GetN(); ++i)
  {
    uint32_t tmp = sw.Get(i)->GetObject<ECMPRoutingProtocol>()->ReportDrop();
    drop += tmp;
  }
  std::cout<<"Drop = "<<drop<<std::endl;

  //host
  //based on counter in Node.cc and TcpSockBase, maybe not the right place
  uint32_t TO = 0, FR = 0, TOflow, FRflow;
  uint32_t sum_TO = 0, sum_FR = 0, sum_TOflow = 0, sum_FRflow = 0;
  for (uint32_t i = 0; i < host.GetN(); ++i)
  {
    TO = 0;
    FR = 0;
    host.Get(i)->ReportStatus(TO, FR, TOflow, FRflow);
    sum_TO += TO;
    sum_FR += FR;
    sum_TOflow += TOflow;
    sum_FRflow += FRflow;
  }
  std::cout<<"TO = "<<sum_TO<<" FR = "<<sum_FR<<" TOflow = "<<sum_TOflow<<" FRflow = "<<sum_FRflow<<std::endl;
}

void DumpDetourCount(NodeContainer& host)
{
  
  //fattree->GetNode(destId)->GetObject<ECMPRoutingProtocol>()->ReportDrop();
  //dump switch status
  uint64_t incast_count[20];
  uint64_t back_count[20];
  uint64_t pincast[20];
  uint64_t pback[20];
  
  for (uint32_t ii = 0; ii < 20; ii++){
    incast_count[ii] = 0;
    back_count[ii] = 0;
  }
    
  for(uint32_t i = 0; i < host.GetN(); ++i)
  {
    //std::cout<<" test"<<std::endl;
    host.Get(i)->GetObject<ECMPRoutingProtocol>()->ReportDetourCnt(pback, pincast);
    for (uint32_t ii = 0; ii < 20; ii++){
      incast_count[ii] += *(pincast + ii);
      back_count[ii] += *(pback + ii);
    } 
  }
  //std::cout<<" test1"<<std::endl;
   for (uint32_t ii = 0; ii < 20; ii++){
     std::cout<<ii<<" "<<incast_count[ii]<<" "<<back_count[ii]<<std::endl;
   }
}


void
printsize(int size)
{
  std::cout<<std::endl<<"+++++++++++++++++++++++"<<std::endl<<"     flow size = "
	   <<size<<std::endl<<"++++++++++++++++++++++++++"<<std::endl<<std::endl;
}

void
printth(Ptr<Node> host)
{
  double th = host->GetObject<ECMPRoutingProtocol>()->ReportThroughput();
  std::cout<<Simulator::Now().GetSeconds()<<" Throughput = "<<th/1e6<<" Mbps"<<std::endl;
}

typedef std::vector< boost::any > AnyVector;

int signaling();
//int incast_exp();
int shortflow(Ptr<FattreeTopologyHelper> fattree);
int incast(Ptr<FattreeTopologyHelper> fattree, uint32_t, uint32_t, double);
int background(Ptr<FattreeTopologyHelper> fattree, double, uint32_t, double);
int shape_trace();
int generate_trace(uint32_t, double, double);
int large();
int incast_exp(Ptr<FattreeTopologyHelper> fattree, uint32_t num_node, uint32_t group_size, double stoptime);
int fairness();


int main()
{
  generate_trace(16, 1, 1);  //num_node, rate, load
  //shape_trace();
  //signaling();
  //large();
  //fairness();
}







int large()
{

  RngSeedManager::SetRun (0);
  
  //TCP
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1440));               //MTU
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));                   //Number of Delay ACK (1 for disable DelACK)
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));
  Config::SetDefault("ns3::RttEstimator::MinRTO", TimeValue (Seconds (0.01)));           // minRTO (10ms for DataCenter)
  Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue (Seconds (0.01)));
  Config::SetDefault("ns3::TcpSocket::ConnTimeout", TimeValue(Seconds(0.0004)));       //assume incast as pre-connected
    
  Config::SetDefault("ns3::TcpNewReno::EnableECN", BooleanValue(false));                 //ECN capability in end-host stack, conflict with DCTCP
  Config::SetDefault("ns3::TcpNewReno::EnableDCTCP", BooleanValue(true));               //DCTCP capability in end-hxfost stack, conflict with ECN


  //Switch buffer

  Config::SetDefault("ns3::ECMPRoutingProtocol::NumHost", UintegerValue(128));
  Config::SetDefault("ns3::DropTailQueue::MaxBytes", UintegerValue (SWITCH_BUFFER * 1500));        // max queue size in switch port
  Config::SetDefault("ns3::ECMPRoutingProtocol::DetourThreshold", UintegerValue (SWITCH_BUFFER * 1500));        // detour threshold in switch port
  Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));    // queueu size on BYTES or PACKET
  Config::SetDefault("ns3::PointToPointNetDevice::DynamicBuffer", BooleanValue(false));    // whether use dyanmic buffer, if so, set queue size to infi
  
  


  //Switch forward
  Config::SetDefault("ns3::TcpNewReno::EnableFastRecovery", BooleanValue (false));       // TCP Fastrecovery
  Config::SetDefault("ns3::ECMPRoutingProtocol::UseBounce", BooleanValue (true));         //Enable detouring
  Config::SetDefault("ns3::ECMPRoutingProtocol::EnablePriority", BooleanValue (false));   //true: random detouring for differing priority
                                                                                          //false: detouring when buffer overflow
  Config::SetDefault("ns3::ECMPRoutingProtocol::ECNCapable", BooleanValue(true));        //sender ecn capability
  Config::SetDefault("ns3::ECMPRoutingProtocol::RouteMode", EnumValue(ECMPRoutingProtocol::ROUTE_MOD_ECMP));  //Routing protocol: ECMP, source routing, spray
  Config::SetDefault("ns3::ECMPRoutingProtocol::MarkDetourPath", BooleanValue(false));        //whether detour path or otherwise shortest path
  Config::SetDefault("ns3::ECMPRoutingProtocol::TracePath", BooleanValue(true)); 

  //Config::SetDefault("ns3::ECMPRoutingProtocol::TTLLimit", UintegerValue(24));   //TTL
  //Config::SetDefault("ns3::ECMPRoutingProtocol::TTDLimit", UintegerValue(1));   //TTD
  //App
  Config::SetDefault("ns3::Application::Desync",TimeValue(Seconds(0.00005)));
  
  //std::cout<<"minRTO=0.01"<<std::endl;
  
  Ptr<FattreeTopologyHelper> fattree = CreateObject<FattreeTopologyHelper>();
  fattree->SetAttribute("N", UintegerValue(8));
  fattree->SetAttribute("LinkSpeed", StringValue("1Gbps"));

  if (OVERSUBSCRIPTION)
    //fattree->CreateTopologyOversubscription(OVERSUBSCRIPTION, SWITCH_BUFFER, 5);
    fattree->CreateTopologyOversubscription(OVERSUBSCRIPTION, SWITCH_BUFFER, SWITCH_BUFFER);
  else
    fattree->CreateTopology();
  
  ECMPRoutingStackHelper stack;

  
  //BFSRoutingStackHelper stack;
  stack.Install(fattree->GetNodes());

  //void (*pdetour)(NodeContainer&) =  DumpDetourCount;
  //Simulator::Schedule (Seconds(150.0), pdetour, fattree->GetTerminalNodes());
  
  //void (*pTh)(Ptr<Node>) = printth;


  //generate short flows
  //shortflow(fattree);


  
  //generate background traffic
  //0.164, 0.25,0.5, 2
  
  background(fattree, 20.0/INTER_ARRIVAL, 128, 1); //load, node_num, stoptime
  
  //generate incast flows
  incast(fattree, 128, INCAST_DEGREE, 1);  //node_num, group, stoptime

  //incast_exp(fattree, 128, 40, 400);
  
  //uint32_t sourceId = 0;
  //uint32_t destId = 15;

  
  Simulator::Run ();
  Simulator::Destroy ();

  
  
  //clean up
  //for(map<uint32_t,AnyVector*>::iterator ii=receivers.begin(); ii!=receivers.end(); ++ii)
  //{
  //  AnyVector* traces = (*ii).second;
  //  delete traces;
  //}

  return 0;
}



int fairness()
{
  
  RngSeedManager::SetRun (100);
  
  //TCP
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1440));               //MTU
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));                   //Number of Delay ACK (1 for disable DelACK)
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));
  Config::SetDefault("ns3::RttEstimator::MinRTO", TimeValue (Seconds (0.01)));           // minRTO (10ms for DataCenter)
  Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue (Seconds (0.01)));
  Config::SetDefault("ns3::TcpSocket::ConnTimeout", TimeValue(Seconds(0.01)));       //assume incast as pre-connected
    
  Config::SetDefault("ns3::TcpNewReno::EnableECN", BooleanValue(false));                 //ECN capability in end-host stack, conflict with DCTCP
  Config::SetDefault("ns3::TcpNewReno::EnableDCTCP", BooleanValue(true));               //DCTCP capability in end-host stack, conflict with ECN
  Config::SetDefault("ns3::ECMPRoutingProtocol::NumHost", UintegerValue(128));

  //Switch
  Config::SetDefault("ns3::DropTailQueue::MaxBytes", UintegerValue (100 * 1500));        // max queue size in switch port
  Config::SetDefault("ns3::ECMPRoutingProtocol::DetourThreshold", UintegerValue (100 * 1500));        // detour threshold in switch port
  Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));    // queueu size on BYTES or PACKET
  Config::SetDefault("ns3::TcpNewReno::EnableFastRecovery", BooleanValue (false));       // TCP Fastrecovery
  Config::SetDefault("ns3::ECMPRoutingProtocol::UseBounce", BooleanValue (true));         //Enable detouring
  Config::SetDefault("ns3::ECMPRoutingProtocol::EnablePriority", BooleanValue (false));   //true: random detouring for differing priority
                                                                                          //false: detouring when buffer overflow
  Config::SetDefault("ns3::ECMPRoutingProtocol::ECNCapable", BooleanValue(true));        //sender ecn capability
  Config::SetDefault("ns3::ECMPRoutingProtocol::RouteMode", EnumValue(ECMPRoutingProtocol::ROUTE_MOD_ECMP));  //Routing protocol: ECMP, source routing, spray
  Config::SetDefault("ns3::ECMPRoutingProtocol::MarkDetourPath", BooleanValue(false));        //whether detour path or otherwise shortest path
  //Config::SetDefault("ns3::ECMPRoutingProtocol::TracePath", BooleanValue(true)); 

  //Config::SetDefault("ns3::ECMPRoutingProtocol::TTLLimit", UintegerValue(24));   //TTL
  //Config::SetDefault("ns3::ECMPRoutingProtocol::TTDLimit", UintegerValue(1));   //TTD
  //App
  Config::SetDefault("ns3::Application::Desync",TimeValue(Seconds(0.00005)));
  
  //std::cout<<"minRTO=0.01"<<std::endl;
  
  Ptr<FattreeTopologyHelper> fattree = CreateObject<FattreeTopologyHelper>();
  fattree->SetAttribute("N", UintegerValue(8));
  fattree->SetAttribute("LinkSpeed", StringValue("1Gbps"));
  
  fattree->CreateTopology();
  
  ECMPRoutingStackHelper stack;

  

  stack.Install(fattree->GetNodes());

  //uint32_t num_flow = 2000;
  uint32_t flownum = 16;
  uint32_t fsize = 10 * 1024 * 1024;
  double start_time =0.001;
  UniformVariable rand;

  uint32_t source_node[128];

  for (uint32_t ii = 0; ii < 128; ++ii) {
    source_node[ii] = ii;
  }

  // shuffle the array
  for (int ii = sizeof(source_node)/sizeof(uint32_t); ii > 1;) {
    uint32_t n = rand.GetInteger(0, ii-1);
    --ii;
    uint32_t tmp = source_node[n];
    source_node[n] = source_node[ii];
    source_node[ii] = tmp;
  }

  for (uint32_t ii = 0; ii < 128; ii+=2 ) {
   
    uint32_t    src = source_node[ii];
    uint32_t    dest = source_node[ii+1];

    uint32_t step = 0;
    if (step == 0) {
      BulkSendHelper SFbulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(dest), 30));
      SFbulkApp.SetAttribute("SendSize", UintegerValue(8*1024));
      PacketSinkHelper SFsinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 30));
      ApplicationContainer SFserverApps = SFsinkApp.Install (fattree->GetNode(dest));
      
      SFbulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix flow size
      ApplicationContainer SclientApps;
      for (uint32_t jj = 0; jj < flownum; ++jj) {
        SclientApps.Add( SFbulkApp.Install (fattree->GetNode(src)));
      }
      
      SclientApps.Start (Seconds (start_time));    
      SclientApps.Stop (Seconds (2000.0));
      
      SFserverApps.Start (Seconds (0.0));
      SFserverApps.Stop (Seconds(2000));
      
      step += 1;
    }

    if (step == 1) {

      BulkSendHelper DFbulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(src), 30));
      DFbulkApp.SetAttribute("SendSize", UintegerValue(8*1024));
      PacketSinkHelper DFsinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 30));
      ApplicationContainer DFserverApps = DFsinkApp.Install (fattree->GetNode(src));
      
      DFbulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix flow size
      ApplicationContainer DclientApps;
      for (uint32_t jj = 0; jj < flownum; ++jj) {
        DclientApps.Add( DFbulkApp.Install (fattree->GetNode(dest)));
      }
      
    
      DclientApps.Start (Seconds (start_time));
      DclientApps.Stop (Seconds (2000.0));
    
      DFserverApps.Start (Seconds (0.0));
      DFserverApps.Stop (Seconds(2000));
    }
  
  }
  
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}

int incast_exp(Ptr<FattreeTopologyHelper> fattree, uint32_t num_node, uint32_t group_size, double stoptime)
{

  void (*pFoo)(int) = Foo;
  void (*pGoo)(int) = Goo;
  void (*pSt)(NodeContainer&, NodeContainer&) = DumpStatus;
  void (*pfz)(int) = printsize;

  

  //uint32_t num_node = 128;
  //uint32_t group_size = 40;
  uint32_t source_node[num_node];

  for (uint32_t ii = 0; ii < num_node; ++ii) {
    source_node[ii] = ii;
  }

  //std::cout<<sizeof(source_node)<<std::endl;
  //exit(1);
  uint32_t flowsize[] = {20 * 1024};  
  //uint32_t num_vary[]={group_size};
  //uint32_t num_vary[] = {8,16,24,32,40,48,56,64,72,80,88,96,104,112,120};
  uint32_t num_vary[]={80};
  //uint32_t source_node[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  //double step = 0.01;
  //double step = 0.002/(num_node / group_size);  //13.25ms, 48 flows,
  double step = 1.0/1;
  double start = 0.0001;
  double time = start;  //do NOT be 0
  uint32_t avr = 300;
  //std::cout<<step<<" "<<avr<<std::endl;
  UniformVariable rand;

  if ((avr * step) * sizeof(num_vary)/sizeof(uint32_t) > (stoptime - start)) {
    std::cout<<"exp time too short"<<std::endl;
    exit(1);
  }

  //uint32_t dest_idx = 0;
  //Generate Incast Traffic
  //varying flowsize
  for (uint32_t seq = 0; seq < sizeof(flowsize)/sizeof(uint32_t); ++seq)
  {
    uint32_t fsize = flowsize[seq];
    
    //varying flow number
    for (uint32_t round = 0; round < sizeof(num_vary)/sizeof(uint32_t); ++round)
      {
	for (uint32_t avr_i = 0; avr_i < avr; ++avr_i)   //Averaging
	{

	  uint32_t dest_idx = rand.GetInteger(0, sizeof(source_node)/sizeof(uint32_t) - 1);
	  uint32_t dest = source_node[dest_idx];
	  //dest_idx++;
	  //if (dest_idx == num_node)
	  //dest_idx = 0;

	  BulkSendHelper SFbulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(dest), 20));
	  SFbulkApp.SetAttribute("SendSize", UintegerValue(8*1024));
	  PacketSinkHelper SFsinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 20));
	  ApplicationContainer SFserverApps = SFsinkApp.Install (fattree->GetNode(dest));
	  
	  SFbulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix flow size
	  ApplicationContainer clientApps;
	  
	  // shuffle the array
	  for (int ii = sizeof(source_node)/sizeof(uint32_t); ii > 1;) {
	    uint32_t n = rand.GetInteger(0, ii-1);
	    --ii;
	    uint32_t tmp = source_node[n];
	    source_node[n] = source_node[ii];
	    source_node[ii] = tmp;
	  }


	  uint32_t temp = num_vary[round];
	  uint32_t source_idx = 0;
	 
	  

	  //fix total amount of traffic
	  //uint32_t fsize = tsize / temp;
	  //bulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix total size


	  while(temp) {
	      uint32_t sourceId = source_node[source_idx];
   
	      if (sourceId != dest) {
		clientApps.Add( SFbulkApp.Install (fattree->GetNode(sourceId)));
		temp--;
	      }
	      
	      source_idx++;
	      source_idx %= (sizeof(source_node) / sizeof(uint32_t));
	  }

	  
	  clientApps.Start (Seconds (time));
	  time += step;
	  clientApps.Stop (Seconds (stoptime));

	  SFserverApps.Start (Seconds (0.0));
	  SFserverApps.Stop (Seconds(stoptime));

	  Simulator::Schedule (Seconds(time),pSt,fattree->GetTerminalNodes(), fattree->GetNonTerminalNodes());
	  Simulator::Schedule( Seconds(time),pFoo, avr_i);  //print one round
	 
	}
	Simulator::Schedule(Seconds(time), pGoo, num_vary[round]); //print
      }
   
    
    Simulator::Schedule(Seconds(time), pfz, flowsize[seq]); //print flow size
  }
  return 0;
}

/*
int signaling()
{

  RngSeedManager::SetRun (0);
  
  //TCP
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1440));               //MTU
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));                   //Number of Delay ACK (1 for disable DelACK)
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(10));
  Config::SetDefault("ns3::RttEstimator::MinRTO", TimeValue (Seconds (0.3)));           // minRTO (10ms for DataCenter)
  Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue (Seconds (0.01)));  
  Config::SetDefault("ns3::TcpNewReno::EnableECN", BooleanValue(false));                 //ECN capability in end-host stack, conflict with DCTCP
  Config::SetDefault("ns3::TcpNewReno::EnableDCTCP", BooleanValue(false));               //DCTCP capability in end-host stack, conflict with ECN
  
  //Switch
  Config::SetDefault("ns3::DropTailQueue::MaxBytes", UintegerValue (100000 * 1500));        // max queue size in switch port
  Config::SetDefault("ns3::ECMPRoutingProtocol::DetourThreshold", UintegerValue (100 * 1500));        // detour threshold in switch port
  Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));    // queueu size on BYTES or PACKET
  Config::SetDefault("ns3::TcpNewReno::EnableFastRecovery", BooleanValue (true));       // TCP Fastrecovery
  Config::SetDefault("ns3::ECMPRoutingProtocol::UseBounce", BooleanValue (false));         //Enable detouring
  Config::SetDefault("ns3::ECMPRoutingProtocol::EnablePriority", BooleanValue (false));   //true: random detouring for differing priority
                                                                                          //false: detouring when buffer overflow
  Config::SetDefault("ns3::ECMPRoutingProtocol::ECNCapable", BooleanValue(false));        //sender ecn capability
  Config::SetDefault("ns3::ECMPRoutingProtocol::RouteMode", EnumValue(ECMPRoutingProtocol::ROUTE_MOD_ECMP));  //Routing protocol: ECMP, source routing, spray
  Config::SetDefault("ns3::ECMPRoutingProtocol::MarkDetourPath", BooleanValue(false));        //whether detour path or otherwise shortest path
  
  //App
  Config::SetDefault("ns3::Application::Desync",TimeValue(Seconds(0.00005)));


  
  Ptr<FattreeTopologyHelper> fattree = CreateObject<FattreeTopologyHelper>();
  fattree->SetAttribute("N", UintegerValue(4));
  fattree->SetAttribute("LinkSpeed", StringValue("1Gbps"));
  fattree->CreateTopology();
  
  ECMPRoutingStackHelper stack;
  
  //BFSRoutingStackHelper stack;
  stack.Install(fattree->GetNodes());

  //void (*pTh)(Ptr<Node>) = printth;


  //generate short flows
  //shortflow(fattree);

  //trace();
  
  //generate background traffic
  background(fattree);
  
  //generate incast flows
  incast(fattree);

  // uint32_t sourceId = 0;
  //uint32_t destId = 15;

  
  Simulator::Run ();
  Simulator::Destroy ();

  
  
  //clean up
  //for(map<uint32_t,AnyVector*>::iterator ii=receivers.begin(); ii!=receivers.end(); ++ii)
  //{
  //  AnyVector* traces = (*ii).second;
  //  delete traces;
  //}

  return 0;
}
*/


/* 
int incast_exp()
{
  //Debug trace
  //LogComponentEnable ("BulkSendApplication", LOG_ALL);
  //LogComponentEnable ("PacketSink", LOG_ALL);
  //LogComponentEnable ("TcpSocketBase", LOG_ALL);
  //LogComponentEnable ("TcpL4Protocol", LOG_ALL);
  //LogComponentEnable ("Ipv4L3Protocol", LOG_ALL);
  
  //TCP
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1500));               //MTU
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));                   //Number of Delay ACK (1 for disable DelACK)
  Config::SetDefault("ns3::RttEstimator::MinRTO", TimeValue (Seconds (0.1)));           // minRTO (10ms for DataCenter)
  Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue (Seconds (0.01)));  
  Config::SetDefault("ns3::TcpNewReno::EnableECN", BooleanValue(false));                 //ECN capability in end-host stack, conflict with DCTCP
  Config::SetDefault("ns3::TcpNewReno::EnableDCTCP", BooleanValue(false));               //DCTCP capability in end-host stack, conflict with ECN
  
  //Switch
  Config::SetDefault("ns3::DropTailQueue::MaxBytes", UintegerValue (20000 * 1500));        // max queue size in switch port
  Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(Queue::QUEUE_MODE_BYTES));    // queueu size on BYTES or PACKET
  Config::SetDefault("ns3::TcpNewReno::EnableFastRecovery", BooleanValue (true));       // TCP Fastrecovery
  Config::SetDefault("ns3::ECMPRoutingProtocol::UseBounce", BooleanValue (false));         //Enable detouring
  Config::SetDefault("ns3::ECMPRoutingProtocol::EnablePriority", BooleanValue (false));   //true: random detouring for differing priority
                                                                                          //false: detouring when buffer overflow
  Config::SetDefault("ns3::ECMPRoutingProtocol::ECNCapable", BooleanValue(false));        //sender ecn capability
  Config::SetDefault("ns3::ECMPRoutingProtocol::Enablespray", BooleanValue(false));        //whether pakcet spraying or otherwise flow-based ECMP
  Config::SetDefault("ns3::ECMPRoutingProtocol::MarkDetourPath", BooleanValue(false));        //whether detour path or otherwise shortest path
  
  //App
  Config::SetDefault("ns3::Application::Desync",TimeValue(Seconds(0.00005)));


  
  Ptr<FattreeTopologyHelper> fattree = CreateObject<FattreeTopologyHelper>();
  fattree->SetAttribute("N", UintegerValue(4));
  fattree->CreateTopology();
  
  ECMPRoutingStackHelper stack;

  //BFSRoutingStackHelper stack;
  stack.Install(fattree->GetNodes());

  // uint32_t sourceId = 0;
  uint32_t destId = 15;
 
  BulkSendHelper bulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(destId), 80));
  bulkApp.SetAttribute("SendSize", UintegerValue(8*1024));

  
  PacketSinkHelper sinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));

  
  

  ApplicationContainer serverApps = sinkApp.Install (fattree->GetNode(destId));
   //ApplicationContainer clientApps = bulkApp.Install (fattree->GetNode(sourceId));
  
  //uint32_t MultiConn = 1;


  
  

  
  //uint32_t flowsize[] = {1024, 4* 1024, 8 * 1024,16 * 1024, 32*1024, 64 * 1024, 128 * 1024};
  uint32_t totalsize[] = {32*1024};
  //uint32_t totalsize = 8 * 1024*1024;
  //uint32_t num_vary[] = {8,16,24,32,40,48,56,64,72};
  //uint32_t num_vary[]={1,2,4,8,16,32,64,128,256,512};
  uint32_t num_vary[]={30};

  double step = 2;
  double time = 1;  //do NOT be 0
   
  //varying flowsize
  for (uint32_t seq = 0; seq < sizeof(totalsize)/sizeof(uint32_t); ++seq)
  {
    
    uint32_t tsize = totalsize[seq];   
    
    bulkApp.SetAttribute("MaxBytes", UintegerValue(tsize));  //fix flow size
   
    ApplicationContainer clientApps[10 * sizeof(num_vary)/sizeof(uint32_t)];
    void (*pFoo)(int) = Foo;
    void (*pGoo)(int) = Goo;
    void (*pSt)(NodeContainer&, NodeContainer&) = DumpStatus;
    void (*pfz)(int) = printsize;
  
    
    uint32_t avr = 10;
    //varying flow number
    for (uint32_t round = 0; round < sizeof(num_vary)/sizeof(uint32_t); ++round)
      {
	for (uint32_t avr_i = 0; avr_i < avr; ++avr_i)   //Averaging
	{
	  uint32_t appid = round * avr + avr_i;
	  uint32_t temp = num_vary[round];
	  uint32_t sourceId = 0;

	  //fix total amount of traffic
	  //uint32_t fsize = tsize / temp;
	  //bulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix total size
   
	  while(temp)
	    {
	      clientApps[appid].Add( bulkApp.Install (fattree->GetNode(sourceId)));
	      sourceId++;
	      if(sourceId == 15)
		sourceId = 0;
	      temp--;
	    }
	  clientApps[appid].Start (Seconds (time));
	  time += step;
	  clientApps[appid].Stop (Seconds (time));
	  Simulator::Schedule (Seconds(time),pSt,fattree->GetTerminalNodes(), fattree->GetNonTerminalNodes());
	  Simulator::Schedule( Seconds(time),pFoo, avr_i);  //print one round
	}
	Simulator::Schedule(Seconds(time), pGoo, num_vary[round]); //print
      }

    Simulator::Schedule(Seconds(time), pfz, totalsize[seq]); //print flow size
  }
  // clientApps.Start (Seconds (0.0));
  // clientApps.Stop (Seconds (20));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (10 * sizeof (num_vary)/ sizeof(uint32_t)*step * (sizeof (totalsize)/sizeof(uint32_t)) + 1));


  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
#endif
*/



int shortflow(Ptr<FattreeTopologyHelper> fattree)
{
  BulkSendHelper SFbulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(14), 10));
  SFbulkApp.SetAttribute("SendSize", UintegerValue(8*1024));

  PacketSinkHelper SFsinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 10));
  
  ApplicationContainer SFserverApps = SFsinkApp.Install (fattree->GetNode(14));
   //ApplicationContainer clientApps = bulkApp.Install (fattree->GetNode(sourceId));
  


  uint32_t flowsize[] = {14 * 1024};  
  uint32_t num_vary[]={1};

  uint32_t source_node[]={ 12 };
  //double step = 0.01;

 
  double step = 0.15;
  double delay = 0.015;
  ExponentialVariable rand(delay);

  //double tst = 0;
  //for (int ii = 0; ii < 1000; ++ii) {
  //tst += rand.GetValue();
  //std::cout<<tst<<" "<<ii * step<<std::endl;
  //}
  //exit(1);
  double start = 0.01;
  double time[3] = {start, start, start};  //do NOT be 0
  uint32_t avr = 100;

  //Generate Incast Traffic
  //varying flowsize
  for (uint32_t seq = 0; seq < sizeof(flowsize)/sizeof(uint32_t); ++seq)
  {
    uint32_t fsize = flowsize[seq];
    SFbulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix flow size
    //ApplicationContainer clientApps[avr * sizeof(num_vary)/sizeof(uint32_t)];
    
    //varying flow number
    for (uint32_t round = 0; round < sizeof(num_vary)/sizeof(uint32_t); ++round)
      {
	for (uint32_t avr_i = 0; avr_i < avr; ++avr_i)   //Averaging
	{
	  //uint32_t appid = round * avr + avr_i;
	  uint32_t temp = num_vary[round];
	  uint32_t source_idx = 0;
	  uint32_t sourceId = source_node[source_idx];

	  
   
	  while(temp)
	    {
	      ApplicationContainer clientApps;
	      clientApps.Add( SFbulkApp.Install (fattree->GetNode(sourceId)));
	      
	      source_idx++;
	      if(source_idx == sizeof(source_node) / sizeof(uint32_t))
		source_idx = 0;
	      sourceId = source_node[source_idx];
	      temp--;

	      //time[temp] += rand.GetValue();
	      //std::cout<<time<<std::endl;
	      //std::cout<<time[temp]+rand.GetValue()<<std::endl;
	      clientApps.Start (Seconds(time[temp] + rand.GetValue()));
	      time[temp] += step;
	      
	      clientApps.Stop (Seconds(20));
	      
	    }
	  
	  
	  
	  //Simulator::Schedule (Seconds(time),pSt,fattree->GetTerminalNodes(), fattree->GetNonTerminalNodes());
	  //Simulator::Schedule( Seconds(time),pFoo, avr_i);  //print one round
	}
	//Simulator::Schedule(Seconds(time), pGoo, num_vary[round]); //print
      }

    //Simulator::Schedule(Seconds(time), pfz, flowsize[seq]); //print flow size
  }
  // clientApps.Start (Seconds (0.0));
  // clientApps.Stop (Seconds (20));
  SFserverApps.Start (Seconds (0.0));
  SFserverApps.Stop (Seconds(20) );

  return 0;
}


int generate_trace(uint32_t node_num, double rate, double load)  // rate = 1 or 10
{
  double starttime = 0.0001;
  double stoptime = 150;
  double current = starttime;
  std::ofstream outfile("scratch/test/large_topo_dctcp_trace/trace.1");
  double meanFlowSize = 1623 * 1460;
  double lamda = rate * load * 1000000000 / (meanFlowSize * 8 / 1440 * 1500) / (node_num - 1); 

  uint32_t flow_arry[6]= {0,0,0,0,0,0};


  ExponentialVariable Exrand(1/lamda);
  UniformVariable Urand;

  EmpiricalVariable Emrand;
  double var = 1440;

  
  Emrand.CDF( 6 * var,  0);
  Emrand.CDF( 6 * var,  0.15);
  Emrand.CDF( 13 * var, 0.2);
  Emrand.CDF( 19 * var, 0.3);
  Emrand.CDF( 33 * var, 0.4);
  Emrand.CDF( 53 * var, 0.53);
  Emrand.CDF( 133 * var, 0.6);
  Emrand.CDF( 667 * var, 0.7);
  Emrand.CDF( 1333 * var, 0.8);
  Emrand.CDF( 3333 * var, 0.9);
  Emrand.CDF(6667 * var,  0.97);
  Emrand.CDF(20000 * var, 1);
  
  /*
  Emrand.CDF( 6 * var,  0);
  Emrand.CDF( 6 * var,  0.25);
  Emrand.CDF( 13 * var, 0.3);
  Emrand.CDF( 19 * var, 0.4);
  Emrand.CDF( 33 * var, 0.6);
  Emrand.CDF( 53 * var, 0.73);
  Emrand.CDF( 133 * var, 0.8);
  Emrand.CDF( 667 * var, 0.85);
  Emrand.CDF( 1333 * var, 0.9);
  //Emrand.CDF( 3333 * var, 0.9);
  Emrand.CDF(6667 * var,  0.97);
  Emrand.CDF(20000 * var, 1);
  */


  /*
    //128 node exp1-5
  Emrand.CDF( 6 * var,  0);
  Emrand.CDF( 6 * var,  0.4);
  Emrand.CDF( 13 * var, 0.5);
  Emrand.CDF( 19 * var, 0.6);
  Emrand.CDF( 33 * var, 0.7);
  Emrand.CDF( 53 * var, 0.75);
  Emrand.CDF( 133 * var, 0.8);
  Emrand.CDF( 667 * var, 0.85);
  Emrand.CDF( 1333 * var, 0.9);
  //Emrand.CDF( 3333 * var, 0.9);
  Emrand.CDF(6667 * var,  0.97);
  Emrand.CDF(20000 * var, 1);
  */
  
  double avr_flow_size = 0.0;
  uint32_t numflow = 0;

  UniformVariable rand;

  //independently
  for (uint32_t ii = 0; ii < node_num; ++ii) {
    for (uint32_t jj = 0; jj < node_num; ++jj) {
      if (ii == jj)
	continue;

      current = starttime;

      do {
	double interval = Exrand.GetValue();
	current += interval;

	if (current > stoptime)
	  break;

	numflow += 1;

	uint32_t flowsize = Emrand.GetInteger();

	//shape, insert more short flows


    //exp13-40
    /*
	  if (flowsize > 110000 && flowsize < 1100000) {
	  //if (flowsize > 100000){
	  if (rand.GetValue(0,1) < 0.8)
	    flowsize /= 100;
	  //else
	  //flowsize *= 5;
	} else if (flowsize > 1100000) {
	  if (rand.GetValue(0,1) < 0.8)
	    flowsize /= 1000;
	  //else
	  //flowsize *= 5;
	}
	*/
	

      //exp1-5
	/*
	if (flowsize > 100000 && flowsize < 10000000){
	  if (rand.GetValue(0,1) < 0.8)
	    flowsize /= 100;
	}
	*/
	
	double f_size = 1*1024;
	uint32_t pos_i = 0;
	while (flowsize > f_size) {
	  f_size *= 10;
	  pos_i++;
	  if (pos_i == 5)
	    break;
	}
	flow_arry[pos_i]++;
	
	avr_flow_size += flowsize;
	
	outfile<<"stats: "<<current<<" start grp "<<ii<<" "
	       <<jj<<" fid 1 "<<flowsize<<" bytes"<<std::endl;
      } while(1);
    }
  }
    


  
  //central
  /*
  do {
    double interval = Exrand.GetValue();
    current += interval;

    if (current > stoptime)
      break;

    numflow += 1;
    
    uint32_t src = Urand.GetInteger(0,node_num - 1);
    uint32_t dst = Urand.GetInteger(0,node_num - 1);
    while (src == dst)
       dst = Urand.GetInteger(0,node_num - 1);
    
    uint32_t flowsize = Emrand.GetInteger();

    //shape, insert more short flows
    if (flowsize > 100000 && flowsize < 10000000) {
      if (rand.GetValue(0,1) < 0.8)
	flowsize /= 100;
    }
    
    double f_size = 10*1024;
    uint32_t pos_i = 0;
    while (flowsize > f_size) {
      f_size *= 10;
      pos_i++;
      if (pos_i == 4)
	break;
    }
    flow_arry[pos_i]++;
 
    avr_flow_size += flowsize;
    
    outfile<<"stats: "<<current<<" start grp "<<src<<" "
	   <<dst<<" fid 1 "<<flowsize<<" bytes"<<std::endl;
  }while(1);
  */
  
  for (uint32_t ii = 0; ii < 6; ii++)
    std::cout<<flow_arry[ii]<<" ";
  
  std::cout<<std::endl;   
  std::cout<<numflow<<" "<<avr_flow_size/numflow<<std::endl;
  
  return 0;
}



int shape_trace()
{
  map<uint32_t, AnyVector*> receivers;
  double startover = -1;
  std::ifstream infile("scratch/test/fixedflowsize-dctcp-traces/trace.1");
  //std::ifstream infile("scratch/test/fixedflowsize-dctcp-traces/trace_made");
  std::ofstream outfile("scratch/test/fixedflowsize-dctcp-traces/trace_made");
  std::string line;

  //std::cout<<"test"<<std::endl;
  if (!infile.good()) {
    std::cout<<"Open trace file fail!"<<std::endl;
    exit(1);
  }

  uint32_t count_size[6] = {0,0,0,0,0,0};
  
  
  double last_time = 0;
  double total_bytes = 0;
  startover = -1;
  while (std::getline(infile, line))
    {
      std::istringstream iss(line);
      //std::cout<<"test"<<std::endl;
      vector<string> strs;
      boost::split(strs, line, boost::is_any_of(" "));
      
      double startTime = boost::lexical_cast<double>(strs[1]);
  
      if (startover == -1)
	startover = startTime;
      startTime = (startTime - startover);

	//uint sender = atoi(strs[4].c_str());
	//uint receiver = atoi(strs[5].c_str());

      double time_shift = 3.3230657;
      for (uint l = 0; l < 10; ++l){
	  
	uint bytes = atoi(strs[8].c_str());	
	UniformVariable rand;
	
	outfile<<strs[0]<<" "<<startTime + l * time_shift<<" ";
	for (int ii = 2; ii < 8; ii++)
	    outfile<<strs[ii]<<" ";
	if (bytes > 100000 && bytes < 10000000) {
	  if (rand.GetValue(0,1) < 0.8)
	    bytes = bytes / 100;
	}
	
	//else if (bytes < 100000)
	  //if (rand.GetValue(0,1) < 0.5)
	  //bytes = bytes / 10;

	if (last_time < (startTime + l * time_shift)) {
	  last_time = (startTime + l * time_shift);
	  //std::cout<<last_time<<std::endl;
	}

	
	total_bytes += bytes;
	int jj = 0;
	int po = bytes;
	while(po > 1000  ){
	  jj += 1;
	  po = po / 10;
	}
	count_size[jj]++;
	outfile <<bytes<<" "<<strs[9]<<std::endl;
      }
    }

  for (uint32_t ii = 0; ii < 6; ++ii)
    std::cout<<count_size[ii]<<" ";
  std::cout<<std::endl;

  //std::cout<<last_time<<std::endl;
  //std::cout<<total_bytes<<" "<<last_time - startover<<std::endl;;
  //double load = (total_bytes  / 1440 * 1500 * 8 ) / (last_time - startover) /  1000000000 / 16;
  //std::cout<<load <<std::endl;
  return 0;
}

int background(Ptr<FattreeTopologyHelper> fattree, double load, uint32_t t_size, double stoptime)
{
  
  //uint32_t max_count = 0;
  //uint32_t max_receiver;
  //double load = 1;
  //double length = 5;
  
  map<uint32_t, AnyVector*> receivers;
  double startover = 0;
  std::ifstream infile;
  //std::ifstream infile("scratch/test/fixedflowsize-dctcp-traces/trace.1");
  if (t_size == 16)
    infile.open("scratch/test/fixedflowsize-dctcp-traces/trace_made", ifstream::in);
  else if (t_size == 128) {
    infile.open("scratch/test/large_topo_dctcp_trace/seq_trace_128.1", ifstream::in);
    std::cout<<"seq_trace_128.1"<<std::endl;
  }  else if (t_size == 432) {
    infile.open("scratch/test/large_topo_dctcp_trace/seq_trace_432.1", ifstream::in);
    std::cout<<"seq_trace_432.1"<<std::endl;
  }  else if (t_size == 1024) {
    infile.open("scratch/test/large_topo_dctcp_trace/seq_trace_1024.1", ifstream::in);
    std::cout<<"seq_trace_1024.1"<<std::endl;
  }  else {
    std::cout<<"topology size error"<<std::endl;
    exit(1);
  }

  //std::ofstream outfile("scratch/test/fixedflowsize-dctcp-traces/trace_made");
  std::string line;

  //std::cout<<"test"<<std::endl;
  if (!infile.good()) {
    std::cout<<"Open trace file fail!"<<std::endl;
    exit(1);
  }
  while (std::getline(infile, line))
  {
    std::istringstream iss(line);
    //std::cout<<"test"<<std::endl;
    vector<string> strs;
    
    boost::split(strs, line, boost::is_any_of(" "));

    double startTime = boost::lexical_cast<double>(strs[1]);
    if (startover == -1)
      startover = startTime;
    startTime = (startTime - startover) / load;


    if ( startTime > stoptime )
      break;
    
    uint sender = atoi(strs[4].c_str());
    uint receiver = atoi(strs[5].c_str());
    uint bytes = atoi(strs[8].c_str());
        
    //<<strs[0]<<strs[1]<<strs[0]<<strs[1]<<strs[0]<<strs[1]<<strs[0]<<strs[1]
    //std::cout << "start time: " << startTime << " sender: " << sender << " receiver: " << receiver << " bytes: " << bytes << std::endl;
    //exit(1);

    if(receivers.count(receiver) == 0){
      AnyVector* traces = new AnyVector;      
      receivers.insert(std::pair<uint32_t,AnyVector*>(receiver,traces));
    }

    AnyVector* traces = receivers[receiver];  //traces for a given receiver
    
    AnyVector trace;
    trace.push_back(startTime);
    trace.push_back(sender);
    trace.push_back(bytes);

    traces->push_back(trace);
  }
  infile.close();
  //exit(1);

  // Set up receivers
  
  PacketSinkHelper sinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));
  ApplicationContainer serverApps;
  for(map<uint32_t,AnyVector*>::iterator ii=receivers.begin(); ii!=receivers.end(); ++ii)
  {
    uint32_t receiver = (*ii).first;
    serverApps.Add(sinkApp.Install (fattree->GetNode(receiver)));
  }
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (30));


  
  //Set up senders
   
  for(map<uint32_t,AnyVector*>::iterator ii=receivers.begin(); ii!=receivers.end(); ++ii)
  {
    uint32_t receiver = (*ii).first;
    AnyVector* traces = (*ii).second;

    //if (traces->size() > max_count){
    //  max_count = traces->size();
    //  max_receiver = receiver;
    //}
    
    //std::cout << sender << "->" << receiver << ": " << packetCount << std::endl;
    
    BulkSendHelper bulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(receiver), 80));
    bulkApp.SetAttribute("SendSize", UintegerValue(8*1024));  //application buffer size

    for(uint i=0; i<traces->size(); i++){
        
       boost::any traceObj = traces->at(i);
       AnyVector trace = boost::any_cast<AnyVector>(traceObj);
    
       boost::any startTimeObj = trace[0];
       boost::any senderObj = trace[1];
       boost::any bytesObj = trace[2];

       double startTime = boost::any_cast<double>(startTimeObj);
       uint sender = boost::any_cast<uint>(senderObj);
       uint32_t bytes = boost::any_cast<uint>(bytesObj);

       bulkApp.SetAttribute("MaxBytes", UintegerValue(bytes));
       
       ApplicationContainer clientApps;
       clientApps.Add(bulkApp.Install (fattree->GetNode(sender)));
       
       clientApps.Start (Seconds (startTime));
       clientApps.Stop (Seconds (30));
    }
    
  }
  
  //std::cout<<max_receiver<<" "<<max_count<<std::endl;

  
  //clean up
  for(map<uint32_t,AnyVector*>::iterator ii=receivers.begin(); ii!=receivers.end(); ++ii)
  {
    AnyVector* traces = (*ii).second;
    delete traces;
  }

  return 0 ;
}

int incast(Ptr<FattreeTopologyHelper> fattree, uint32_t num_node, uint32_t group_size, double stoptime)
{

  //void (*pFoo)(int) = Foo;
  void (*pGoo)(int) = Goo;
  //void (*pSt)(NodeContainer&, NodeContainer&) = DumpStatus;
  void (*pfz)(int) = printsize;

  uint32_t base_node = num_node;
  while(base_node < group_size)
    base_node *= 2;
  

  //uint32_t num_node = 128;
  //uint32_t group_size = 40;
  uint32_t source_node[base_node];

  for (uint32_t ii = 0; ii < base_node; ++ii) {
    source_node[ii] = ii;
  }

  //std::cout<<sizeof(source_node)<<std::endl;
  //exit(1);
  uint32_t flowsize[] = {INCAST_SIZE * 1024};  
  uint32_t num_vary[]={group_size};

  //uint32_t source_node[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  //double step = 0.01;
  //double step = 0.002/(num_node / group_size);  //13.25ms, 48 flows,
  double step = 1.0/INCAST_QPS;
  double start = 0.0001;
  double time = start;  //do NOT be 0
  uint32_t avr = (stoptime - start)/step;
  //std::cout<<step<<" "<<avr<<std::endl;
  UniformVariable rand;

  //uint32_t dest_idx = 0;
  //Generate Incast Traffic
  //varying flowsize
  for (uint32_t seq = 0; seq < sizeof(flowsize)/sizeof(uint32_t); ++seq)
  {
    uint32_t fsize = flowsize[seq];
    
    //varying flow number
    for (uint32_t round = 0; round < sizeof(num_vary)/sizeof(uint32_t); ++round)
    {
      for (uint32_t avr_i = 0; avr_i < avr; ++avr_i)   //Averaging
      {

        uint32_t dest_idx = (rand.GetInteger(0, sizeof(source_node)/sizeof(uint32_t) - 1)) % num_node;
        uint32_t dest = source_node[dest_idx] % num_node;
        //std::cout<<"dest "<<dest_idx<<" "<<dest<<std::endl;
        //dest_idx++;
        //if (dest_idx == num_node)
        //dest_idx = 0;

        BulkSendHelper SFbulkApp("ns3::TcpSocketFactory", InetSocketAddress(fattree->GetTerminalInterface(dest), 20));
        SFbulkApp.SetAttribute("SendSize", UintegerValue(8*1024));
        PacketSinkHelper SFsinkApp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 20));
        //std::cout<<"test "<<dest<<std::endl;
        ApplicationContainer SFserverApps = SFsinkApp.Install (fattree->GetNode(dest));

        //std::cout<<"test "<<std::endl;
        SFbulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix flow size
        ApplicationContainer clientApps;
        
        // shuffle the array
        for (int ii = sizeof(source_node)/sizeof(uint32_t); ii > 1;) {
          uint32_t n = (rand.GetInteger(0, ii-1) );
          --ii;
          uint32_t tmp = source_node[n];
          source_node[n] = source_node[ii];
          source_node[ii] = tmp;
        }

        //std::cout<<"tet"<<std::endl;
        uint32_t temp = num_vary[round];
        uint32_t source_idx = 0;
	 
	  

	  //fix total amount of traffic
	  //uint32_t fsize = tsize / temp;
	  //bulkApp.SetAttribute("MaxBytes", UintegerValue(fsize));  //fix total size

        //std::cout<<"te"<<std::endl;
        while(temp) {
	      uint32_t sourceId = source_node[source_idx] % num_node;
   
	      if (sourceId != dest) {
            //std::cout<<"source "<<sourceId<<std::endl;
            clientApps.Add( SFbulkApp.Install (fattree->GetNode(sourceId)));
            temp--;
	      }
	      
	      source_idx++;
	      source_idx %= (sizeof(source_node) / sizeof(uint32_t));
        }

	  
        clientApps.Start (Seconds (time));
        time += step;
        clientApps.Stop (Seconds (30));
        
        SFserverApps.Start (Seconds (0.0));
        SFserverApps.Stop (Seconds(30));

        //Simulator::Schedule (Seconds(time),pSt,fattree->GetTerminalNodes(), fattree->GetNonTerminalNodes());
        //Simulator::Schedule( Seconds(time),pFoo, avr_i);  //print one round
        
      }
      Simulator::Schedule(Seconds(time), pGoo, num_vary[round]); //print
    }
   
    
    Simulator::Schedule(Seconds(time), pfz, flowsize[seq]); //print flow size
  }
  // clientApps.Start (Seconds (0.0));
  // clientApps.Stop (Seconds (20));

  //SFserverApps.Start (Seconds (0.0));
  //SFserverApps.Stop (Seconds (avr * sizeof (num_vary)/ sizeof(uint32_t)* step * (sizeof (flowsize)/sizeof(uint32_t)) + start) );
  
  //std::cout<<max_receiver<<" "<<max_count<<std::endl;
  return 0;
}

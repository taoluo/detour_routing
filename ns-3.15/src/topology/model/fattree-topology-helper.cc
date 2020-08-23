#include "fattree-topology-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/net-routing.h"
#include "ns3/uinteger.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/drop-tail-queue.h"
namespace ns3
{
	TypeId FattreeTopologyHelper::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::FattreeTopologyHelper")
			.SetParent<TopologyHelper>()
			.AddConstructor<FattreeTopologyHelper>()
			.AddAttribute("N", "The port number.",
				      UintegerValue(4), MakeUintegerAccessor(&FattreeTopologyHelper::m_n), MakeUintegerChecker<uint32_t>())
		        .AddAttribute("LinkSpeed", "linkspeed",
				      StringValue("1Gbps"), MakeStringAccessor(&FattreeTopologyHelper::m_linkspeed), MakeStringChecker())
			.AddAttribute("queue", "The queue size.",
						  UintegerValue(128), MakeUintegerAccessor(&FattreeTopologyHelper::m_queue_size), MakeUintegerChecker<uint32_t>());
		     
			
		return tid;
	}

	TypeId FattreeTopologyHelper::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}
	
	FattreeTopologyHelper::FattreeTopologyHelper ()
	{}

	FattreeTopologyHelper::~FattreeTopologyHelper ()
	{}

  
	void FattreeTopologyHelper::CreateTopology()
	{
		uint32_t n = m_n;
		
		int srv_num = n * n * n / 4;
		int pot_switch_num = n * n;
		int top_switch_num = n * n / 4;

		terminalNodes.Create(srv_num);
		potswitchNodes.Create(pot_switch_num);
		topswitchNodes.Create(top_switch_num);

		PointToPointHelper pppHelper;

		pppHelper.SetDeviceAttribute ("DataRate", StringValue(m_linkspeed));
		pppHelper.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (5)));

		// create topology
		for (int i = 0; i < srv_num; i++)
		{
			int from = i;
			int to = i / (n / 2);

			NetDeviceContainer link = pppHelper.Install(NodeContainer(terminalNodes.Get(from), potswitchNodes.Get(to)));
			//Ptr<PointToPointNetDevice> host = link.Get(0);
			//Ptr<DropTailQueue> queue = host->GetQueue();
			//queue->SetAttribute("MaxBytes", UintegerValue(20000*1500));
			terminalDevices.Add(link.Get(0));
			switchDevices.Add(link.Get(1));
		}

		for (uint32_t i = 0; i < n; i++)
			for (uint32_t j = 0; j < n / 2; j++)
				for (uint32_t k = 0; k < n / 2; k++)
				{
					int from = i * n / 2 + j;
					int to = i * n / 2 + k + n * n / 2;

					NetDeviceContainer link = pppHelper.Install(NodeContainer(potswitchNodes.Get(from), potswitchNodes.Get(to)));
					switchDevices.Add(link.Get(0));
					switchDevices.Add(link.Get(1));
				}

		for (uint32_t i = 0; i < n; i++)
			for (uint32_t j = 0; j < n / 2; j++)
				for (uint32_t k = 0; k < n / 2; k++)
				{
					int from = pot_switch_num / 2 + i * n / 2 + j;
					int to = j * n / 2 + k;
					NetDeviceContainer link = pppHelper.Install(NodeContainer(potswitchNodes.Get(from), topswitchNodes.Get(to)));
					switchDevices.Add(link.Get(0));
					switchDevices.Add(link.Get(1));
				}

		for (int i = 0; i < srv_num; i++)
		{
		    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(terminalNodes.Get(i)->GetDevice(0));
		    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
		    queue->SetAttribute("MaxBytes", UintegerValue(20000*1500));
		}		
	}

  
  void FattreeTopologyHelper::CreateTopologyOversubscription(int oversub, int torbuffer, int corebuffer)
	{
		uint32_t n = m_n;
		
		int srv_num = n * n * n / 4;
		int pot_switch_num = n * n;
		int top_switch_num = n * n / 4;

		terminalNodes.Create(srv_num);
		potswitchNodes.Create(pot_switch_num);
		topswitchNodes.Create(top_switch_num);

		PointToPointHelper pppHelper;

		pppHelper.SetDeviceAttribute ("DataRate", StringValue(m_linkspeed));
		pppHelper.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (5)));

		// create topology
		for (int i = 0; i < srv_num; i++)
		{
           //leaf
			int from = i;
			int to = i / (n / 2);

			NetDeviceContainer link = pppHelper.Install(NodeContainer(terminalNodes.Get(from), potswitchNodes.Get(to)));
			//Ptr<PointToPointNetDevice> host = link.Get(0);
			//Ptr<DropTailQueue> queue = host->GetQueue();
			//queue->SetAttribute("MaxBytes", UintegerValue(20000*1500));
			terminalDevices.Add(link.Get(0));
			switchDevices.Add(link.Get(1));
		}


        switch(oversub){
        case 1: pppHelper.SetDeviceAttribute ("DataRate", StringValue("1Gbps"));break;
        case 2: pppHelper.SetDeviceAttribute ("DataRate", StringValue("500Mbps"));break;
        case 3: pppHelper.SetDeviceAttribute ("DataRate", StringValue("333Mbps"));break;
        case 4: pppHelper.SetDeviceAttribute ("DataRate", StringValue("250Mbps"));break;
        case 8: pppHelper.SetDeviceAttribute ("DataRate", StringValue("125Mbps"));break;
        }
		for (uint32_t i = 0; i < n; i++)
			for (uint32_t j = 0; j < n / 2; j++)
				for (uint32_t k = 0; k < n / 2; k++)
				{
					int from = i * n / 2 + j;
					int to = i * n / 2 + k + n * n / 2;

					NetDeviceContainer link = pppHelper.Install(NodeContainer(potswitchNodes.Get(from), potswitchNodes.Get(to)));
					switchDevices.Add(link.Get(0));
					switchDevices.Add(link.Get(1));
				}

		for (uint32_t i = 0; i < n; i++)
			for (uint32_t j = 0; j < n / 2; j++)
				for (uint32_t k = 0; k < n / 2; k++)
				{
					int from = pot_switch_num / 2 + i * n / 2 + j;
					int to = j * n / 2 + k;
					NetDeviceContainer link = pppHelper.Install(NodeContainer(potswitchNodes.Get(from), topswitchNodes.Get(to)));
					switchDevices.Add(link.Get(0));
					switchDevices.Add(link.Get(1));
				}

        //assign large queue length for host nodes
		for (int i = 0; i < srv_num; i++)
		{
		    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(terminalNodes.Get(i)->GetDevice(0));
		    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
		    queue->SetAttribute("MaxBytes", UintegerValue(20000*1500));
		}
        
        for (int i = 0; i < pot_switch_num / 2; i++)
		{
		    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(potswitchNodes.Get(i)->GetDevice(0));
		    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
		    queue->SetAttribute("MaxBytes", UintegerValue(torbuffer * 1500));
		}

        for (int i = pot_switch_num / 2; i < pot_switch_num; i++)
		{
		    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(potswitchNodes.Get(i)->GetDevice(0));
		    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
		    queue->SetAttribute("MaxBytes", UintegerValue(corebuffer * 1500));
		}

        for (int i = 0; i < top_switch_num; i++)
		{
		    Ptr<PointToPointNetDevice> dev = DynamicCast<PointToPointNetDevice>(topswitchNodes.Get(i)->GetDevice(0));
		    Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue>(dev->GetQueue());
		    queue->SetAttribute("MaxBytes", UintegerValue(corebuffer * 1500));
		}	
	}


  
	Ptr<Node> FattreeTopologyHelper::GetNode(uint32_t i) const
	{
		if (i < terminalNodes.GetN())
			return terminalNodes.Get(i);
		i -= terminalNodes.GetN();

		if (i < potswitchNodes.GetN())
			return potswitchNodes.Get(i);
		i -= potswitchNodes.GetN();

		if (i < topswitchNodes.GetN())
			return topswitchNodes.Get(i);

		return NULL;
	}

	uint32_t FattreeTopologyHelper::GetNTerminalNodes() const
	{
		return terminalNodes.GetN();
	}

	Ptr<Node> FattreeTopologyHelper::GetTerminalNode(int i) const
	{
		return terminalNodes.Get(i);
	}

	Ipv4Address FattreeTopologyHelper::GetTerminalInterface(int i) const
	{
		return Ipv4Address(i);
	}

	NodeContainer FattreeTopologyHelper::GetNodes() const
	{
		return NodeContainer(terminalNodes, potswitchNodes, topswitchNodes);
	}

	

    NetDeviceContainer FattreeTopologyHelper::GetNetDevices() const
    {
        return NetDeviceContainer(terminalDevices, switchDevices);
    }
}

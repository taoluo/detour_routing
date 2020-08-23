#include "torus-topology-helper.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/point-to-point-remote-channel.h"
#include "ns3/object-factory.h"
#include "ns3/ptr.h"
#include "ns3/queue.h"
#include "ns3/channel.h"

#include <vector>

namespace ns3
{
	TypeId TorusTopologyHelper::GetTypeId (void)
	{
		static TypeId tid = TypeId ("ns3::TorusTopologyHelper")
			.SetParent<TopologyHelper>()
			.AddConstructor<TorusTopologyHelper>()
			.AddAttribute("N", "The port number.",
						  UintegerValue(4), MakeUintegerAccessor(&TorusTopologyHelper::m_n), MakeUintegerChecker<uint32_t>())
			.AddAttribute("K", "The port number.",
						  UintegerValue(4), MakeUintegerAccessor(&TorusTopologyHelper::m_k), MakeUintegerChecker<uint32_t>())
			.AddAttribute("queue", "The queue size.",
						  UintegerValue(128), MakeUintegerAccessor(&TorusTopologyHelper::m_queue_size), MakeUintegerChecker<uint32_t>());
			
		return tid;
	}

	TypeId TorusTopologyHelper::GetInstanceTypeId (void) const
	{
		return GetTypeId();
	}

	TorusTopologyHelper::TorusTopologyHelper ()
	{
		queueFactory.SetTypeId ("ns3::DropTailQueue");
		deviceFactory.SetTypeId ("ns3::PointToPointNetDevice");
		channelFactory.SetTypeId ("ns3::PointToPointChannel");
	}

	TorusTopologyHelper::~TorusTopologyHelper ()
	{}

	void TorusTopologyHelper::SetLinkRate(const DataRateValue& rate)
	{
		deviceFactory.Set("DataRate", rate);
	}
	
	void TorusTopologyHelper::SetLinkDelay(const TimeValue& delay)
	{
		channelFactory.Set("Delay", delay);
	}

	void TorusTopologyHelper::CreateTopology()
	{
		uint32_t nodeNum = (int)pow(m_n, m_k);
		
		terminalNodes.Create(nodeNum);
		
		for (uint32_t j = 0; j < m_k; j++)
		{
			std::vector< Ptr<PointToPointChannel> > channels;
			
			for (uint32_t i = 0; i < terminalNodes.GetN(); i++)
				channels.push_back(channelFactory.Create<PointToPointChannel> ());

			uint32_t srvid = 0;
			uint32_t base = (uint32_t) pow ((double)m_n, j);

			for (uint32_t i = 0; i < terminalNodes.GetN(); i++)
			{
				uint32_t linkid = i / m_n * m_n + (i + m_n - 1) % m_n;

				Ptr<Node> node = terminalNodes.Get(srvid);
				
				Ptr<PointToPointNetDevice> device = deviceFactory.Create<PointToPointNetDevice>();
				device->SetAddress(Mac48Address::Allocate());
				device->SetQueue(queueFactory.Create<Queue> ());
				node->AddDevice(device);

				device->Attach(channels[linkid]);

				uint32_t part1 = srvid / (base * m_n) * base + srvid % base;
				uint32_t part2 = srvid / base % m_n;

				part2++;
				if (part2 == m_n)
				{
					part1++;
					part2 = 0;
				}

				srvid = part1 / base * (base * m_n) + part2 * base + part1 % base;				
			}

			srvid = 0;
			for (uint32_t i = 0; i < terminalNodes.GetN(); i++)
			{
				uint32_t linkid = i;

				Ptr<Node> node = terminalNodes.Get(srvid);
				
				Ptr<PointToPointNetDevice> device = deviceFactory.Create<PointToPointNetDevice>();
				device->SetAddress(Mac48Address::Allocate());
				device->SetQueue(queueFactory.Create<Queue> ());
				node->AddDevice(device);

				device->Attach(channels[linkid]);

				uint32_t part1 = srvid / (base * m_n) * base + srvid % base;
				uint32_t part2 = srvid / base % m_n;

				part2++;
				if (part2 == m_n)
				{
					part1++;
					part2 = 0;
				}

				srvid = part1 / base * (base * m_n) + part2 * base + part1 % base;				
			}			
		}
	}

	Ptr<Node> TorusTopologyHelper::GetNode(uint32_t i) const
	{
		if (i < terminalNodes.GetN())
			return terminalNodes.Get(i);

		return NULL;
	}

	uint32_t TorusTopologyHelper::GetNTerminalNodes() const
	{
		return terminalNodes.GetN();
	}

	Ptr<Node> TorusTopologyHelper::GetTerminalNode(int i) const
	{
		return terminalNodes.Get(i);
	}

	Ipv4Address TorusTopologyHelper::GetTerminalInterface(int i) const
	{
		return Ipv4Address(i);
	}

	NodeContainer TorusTopologyHelper::GetNodes() const
	{
		return terminalNodes;
	}

    NetDeviceContainer TorusTopologyHelper::GetNetDevices() const
    {
        return terminalDevices;
    }	
}

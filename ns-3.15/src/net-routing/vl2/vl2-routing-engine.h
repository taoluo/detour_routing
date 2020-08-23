#ifndef __VL2_ROUTING_ENGINE__
#define __VL2_ROUTING_ENGINE__

#include "ns3/simple-ref-count.h"
#include "ns3/vl2-header.h"
#include "ns3/ptr.h"

namespace ns3
{
	class Packet;
	
	class VL2RoutingEngine : public SimpleRefCount<VL2RoutingEngine>
	{
	protected:
		uint32_t m_ni;
		uint32_t m_na;
		
	public:
		VL2RoutingEngine(uint32_t ni, uint32_t na);
		virtual ~VL2RoutingEngine();

		virtual void FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header) = 0;
	};

	// --------------------------------------------------------------------------------------------------------------------------------
	
	class VL2RandomRoutingEngine : public VL2RoutingEngine
	{
	public:
		VL2RandomRoutingEngine(uint32_t ni, uint32_t na);
		~VL2RandomRoutingEngine();

		virtual void FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header);
	};
	
	// --------------------------------------------------------------------------------------------------------------------------------
	
	class VL2RoundRobinRoutingEngine : public VL2RoutingEngine
	{
		uint32_t index1;
		uint32_t index2;
		uint32_t index3;
	public:
		VL2RoundRobinRoutingEngine(uint32_t ni, uint32_t na);
		~VL2RoundRobinRoutingEngine();

		virtual void FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header);		
	};

	class VL2SequentialRoundRobinRoutingEngine : public VL2RoutingEngine
	{
		uint32_t index1;
		uint32_t index2;
		uint32_t index3;
	public:
		VL2SequentialRoundRobinRoutingEngine(uint32_t ni, uint32_t na);
		~VL2SequentialRoundRobinRoutingEngine();

		virtual void FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header);
	};

    // --------------------------------------------------------------------------------------------------------------------------------

	class VL2VirtualRandomRoutingEngine : public VL2RoutingEngine
	{
	public:
		VL2VirtualRandomRoutingEngine(uint32_t ni, uint32_t na);
		~VL2VirtualRandomRoutingEngine();

		virtual void FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header);
	};

	// --------------------------------------------------------------------------------------------------------------------------------
	
	class VL2PathTagRoutingEngine : public VL2RoutingEngine
	{
	public:
		VL2PathTagRoutingEngine(uint32_t ni, uint32_t na);
		~VL2PathTagRoutingEngine();

		virtual void FillRoutingPath(Ptr<Packet> packet, uint32_t src, uint32_t dest, VL2Header& header);		
	};
}

#endif 

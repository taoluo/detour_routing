#ifndef __ROBIN_INDEX__
#define __ROBIN_INDEX__

// --------------------------------------------------------------------------------
// Packet level index
// --------------------------------------------------------------------------------

#define ROBIN_INDEX_RANDOM								0
#define ROBIN_INDEX_ROUND_ROBIN							1
#define ROBIN_INDEX_DIGITAL_REVERSAL_ROUND_ROBIN		2
#define ROBIN_INDEX_PATH_TAG							3

// --------------------------------------------------------------------------------
// Flow level index
// --------------------------------------------------------------------------------
#define ROBIN_INDEX_INIT_RANDOM							4


#include "stdint.h"
#include "ns3/simple-ref-count.h"
#include "ns3/ptr.h"
#include "ns3/tag.h"
#include <vector>

namespace ns3
{
	class Packet;
	
	class Index : public SimpleRefCount<Index>
	{
	protected:
		uint32_t m_n;
		
	public:
		Index(uint32_t n);
		virtual ~Index();

		operator uint32_t();

		virtual uint32_t GetNext(Ptr<Packet>) = 0;
	};

	// ---------------------------------------------------------------------------------------------------
	class RandomIndex : public Index
	{
		uint32_t m_base;
	public:
		RandomIndex(uint32_t base, uint32_t n);
		~RandomIndex();

		virtual uint32_t GetNext(Ptr<Packet>);
	};

	// ---------------------------------------------------------------------------------------------------
	class InitRandomIndex : public Index
	{
		uint32_t m_value;
	public:
		InitRandomIndex(uint32_t base, uint32_t n);
		~InitRandomIndex();

		virtual uint32_t GetNext(Ptr<Packet>);
	};	

	// ---------------------------------------------------------------------------------------------------

	class RoundRobinIndex : public Index
	{
		uint32_t m_index;
	public:
		RoundRobinIndex(uint32_t n);
		~RoundRobinIndex();

		virtual uint32_t GetNext(Ptr<Packet>);
	};

	// ---------------------------------------------------------------------------------------------------

	class RandomRobinIndex : public Index
	{
		uint32_t m_index;
		uint32_t* m_indexes;

	public:
		RandomRobinIndex(uint32_t base, uint32_t n);
		~RandomRobinIndex();

		virtual uint32_t GetNext(Ptr<Packet>);
	};

	// ---------------------------------------------------------------------------------------------------

	class BinaryReversalRobinIndex : public Index
	{
		uint32_t m_index;
		uint32_t m_bitCount;

		uint32_t getReversed(uint32_t value) const;

	public:
		BinaryReversalRobinIndex(uint32_t);
		~BinaryReversalRobinIndex();

		virtual uint32_t GetNext(Ptr<Packet>);
	};

	// ---------------------------------------------------------------------------------------------------

	class DigitalReversalRobinIndex : public Index
	{
		uint32_t m_base;
		uint32_t m_digitCount;
		uint32_t m_index;

	public:
		DigitalReversalRobinIndex(uint32_t, uint32_t, bool randomStart = false);
		~DigitalReversalRobinIndex();

		virtual uint32_t GetNext(Ptr<Packet>);
	};

	// ---------------------------------------------------------------------------------------------------
	class PathTag : public Tag
	{
		uint32_t m_path;
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;

		PathTag() { m_path = 0; }
		PathTag(uint32_t path) { m_path = path; }
		~PathTag() {}

		uint32_t GetPath() const { return m_path; }
		void SetPath(uint32_t path) { m_path = path; }

		virtual uint32_t GetSerializedSize (void) const
		{
			return 4;
		}

		virtual void Serialize (TagBuffer i) const
		{
			i.WriteU32(m_path);
		}

		virtual void Deserialize (TagBuffer i)
		{
			m_path = i.ReadU32();
		}

		virtual void Print (std::ostream &os) const
		{}
	};

	
	class PathTagIndex : public Index
	{
		std::vector<uint32_t> m_index_set;
		
	public:
		PathTagIndex(uint32_t);
		~PathTagIndex();
		
		virtual uint32_t GetNext(Ptr<Packet>);

		uint32_t GetPathNumber() const { return m_n; }
	};
}

#endif 

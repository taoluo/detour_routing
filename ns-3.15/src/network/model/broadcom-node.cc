/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation, INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: George F. Riley<riley@ece.gatech.edu>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include <iostream>
#include <fstream>
#include "node.h"
#include "node-list.h"
#include "net-device.h"
#include "application.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/global-value.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/broadcom-node.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("BroadcomNode");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BroadcomNode);

TypeId 
BroadcomNode::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::BroadcomNode")
		.SetParent<Object> ()
		.AddConstructor<BroadcomNode> ();
	return tid;
}


BroadcomNode::BroadcomNode()
{
	m_maxBufferBytes=2000000; //9MB
	m_usedTotalBytes=0;
    //std::cout<<"initiate broadcom"<<std::endl;
	for (uint i=0;i<pCnt;i++)
	{
		m_usedIngressPortMinBytes[i]=0;
		m_usedIngressPortSharedBytes[i]=0;
		m_usedIngressPortMinExceed[i]=0;
		m_usedIngressPortSharedExceed[i]=0;
		m_usedEgressPortBytes[i]=0;
		for (uint j=0;j<qCnt;j++)
		{
			m_usedIngressPGMinBytes[i][j]=0;
			m_usedIngressPGSharedBytes[i][j]=0;
			m_usedIngressPGMinExceed[i][j]=0;
			m_usedIngressPGSharedExceed[i][j]=0;
			m_usedEgressQMinBytes[i][j]=0;
			m_usedEgressQSharedBytes[i][j]=0;
			m_pause_remote[i][j]=false;
		}
	}
	for (uint i=0;i<4;i++)
	{
		m_usedIngressSPBytes[i]=0;
		m_usedIngressSPExceed[i]=0;
		m_usedEgressSPBytes[i]=0;
	}

	m_usedIngressSPSharedBytes=0;
	m_usedIngressSPSharedExceed=0;

	m_usedIngressHeadroomBytes=0;


	//ingress params
	m_buffer_cell_limit_sp=2000*1500; //ingress sp buffer threshold p.120
	m_buffer_cell_limit_sp_shared=2000*1500; //ingress sp buffer shared threshold p.120, nonshare -> share
	m_pg_min_cell=1500; //ingress pg guarantee p.121					---1
	m_port_min_cell=1500; //ingress port guarantee						---2
	m_pg_shared_limit_cell=20*1500; //max buffer for an ingress pg			---3	PAUSE
	m_port_max_shared_cell=4800*1500; //max buffer for an ingress port		---4	PAUSE
	m_pg_hdrm_limit=1000*1500; //ingress pg headroom
	m_port_max_pkt_size=100*1500; //ingress global headroom
	//still needs reset limits..
	m_port_min_cell_off = 4700*1500;
	m_pg_shared_limit_cell_off = m_pg_shared_limit_cell-2*1500;


	//egress params
	m_op_buffer_shared_limit_cell=m_maxBufferBytes; //per egress sp limit
	m_op_uc_port_config_cell=m_maxBufferBytes; //per egress port limit
	m_q_min_cell=1*1500;
	m_op_uc_port_config1_cell=m_maxBufferBytes;
	
	//qcn
	m_pg_qcn_threshold=60*1500;
	m_pg_qcn_threshold_max=60*1500;
	m_pg_qcn_maxp = 0.1;

	//dynamic threshold
	m_dynamicth = false;

	m_pg_shared_alpha_cell=1.0/16;
	m_port_shared_alpha_cell=128;   //not used for now. not sure whether this is used on switches
	m_pg_shared_alpha_cell_off_diff=16;
	m_port_shared_alpha_cell_off_diff=16;

	m_log_start = 2.1;
	m_log_end = 2.2;
	m_log_step = 0.00001;

}


BroadcomNode::~BroadcomNode ()
{}

bool 
BroadcomNode::CheckIngressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	if (m_usedTotalBytes+psize>m_maxBufferBytes)  //buffer full, usually should not reach here.
	{
		std::cout<<"WARNING: Drop because ingress buffer full\n";
		exit(1);
		return false;
	}
	if (m_usedIngressPGMinBytes[port][qIndex]+psize>m_pg_min_cell && m_usedIngressPortMinBytes[port]+psize>m_port_min_cell) // exceed guaranteed, use share buffer
	{
		//after pg/port share limit reached, do packets go to service pool buffer or headroom?
		if (m_usedIngressPGSharedBytes[port][qIndex]+psize>m_pg_shared_limit_cell					//exceed pg share limit, use headroom
			|| m_usedIngressPortSharedBytes[port]+psize>m_port_max_shared_cell				//exceed port share limit, use headroom
			|| (m_usedIngressSPBytes[GetIngressSP(port,qIndex)] + psize > m_buffer_cell_limit_sp	// exceed SP buffer limit and...
			&& m_usedIngressSPSharedBytes + psize > m_buffer_cell_limit_sp_shared)					// exceed shared sp buffer, use headroom
		   )
		{
			if (m_usedIngressHeadroomBytes + psize > m_pg_hdrm_limit) // exceed headroom space
			{
				std::cout<<"WARNING: Drop because ingress headroom full\n";
				return false;
			}
		}
	}
	return true;
}




bool 
BroadcomNode::CheckEgressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
  //std::cout<<"checkegreee "<<port<<" "<<qIndex<<" "<<psize<<std::endl;
  //std::cout<<m_usedEgressSPBytes[GetEgressSP(port,qIndex)]<<" "<<m_op_buffer_shared_limit_cell<<std::endl;
  //std::cout<<m_usedEgressPortBytes[port]<<" "<<m_op_uc_port_config_cell<<std::endl;
  //std::cout<<m_usedEgressQSharedBytes[port][qIndex]<<" "<<m_op_uc_port_config1_cell<<std::endl;
  if (m_usedEgressSPBytes[GetEgressSP(port,qIndex)]+psize>m_op_buffer_shared_limit_cell)  //exceed the sp limit
	{
      std::cout<<m_usedEgressSPBytes[GetEgressSP(port,qIndex)]<<" "<<m_op_buffer_shared_limit_cell<<std::endl;
      std::cout<<"WARNING: Drop because egress SP buffer full"<<std::endl;
		return false;
	}
    //std::cout<<m_op_uc_port_config_cell<<std::endl;
	if (m_usedEgressPortBytes[port]+psize>m_op_uc_port_config_cell)	//exceed the port limit
	{
      std::cout<<m_usedEgressPortBytes[port]<<" "<<m_op_uc_port_config_cell<<std::endl;
      std::cout<<"WARNING: Drop because egress Port buffer full\n";
		return false;
	}
    //std::cout<<"checkegreee2"<<std::endl;
    
	if (m_usedEgressQSharedBytes[port][qIndex]+psize>m_op_uc_port_config1_cell) //exceed the queue limit
	{
      std::cout<<m_usedEgressQSharedBytes[port][qIndex]<<" "<<m_op_uc_port_config1_cell<<std::endl;
      std::cout<<"WARNING: Drop because egress Q buffer full\n";
		return false;
	}
    //std::cout<<"checkegreee3"<<std::endl;
	return true;
}


void
BroadcomNode::UpdateIngressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	m_usedTotalBytes += psize; //count total buffer usage

	/*
	uint32_t PGexceed=0;
	for (uint32_t i=0;i<32;i++)
		for (uint32_t j=0;j<8;j++)
			PGexceed+=m_usedIngressPGSharedExceed[i][j];

	uint32_t Portexceed=0;
	for (uint32_t i=0;i<32;i++)
		Portexceed+=m_usedIngressPortSharedExceed[i];
			
	std::cout<<m_usedTotalBytes<<"\tPort:"<<port<<"\tQindex:"<<qIndex<<"\tExceed:"<<m_usedIngressPGSharedExceed[port][qIndex]<<"\tHeadroom Used:"<<m_usedIngressHeadroomBytes<<"\tPG exceed:"<<PGexceed<<"\tPort exceed:"<<Portexceed<<"\n";
	*/

	if (m_usedIngressPGMinBytes[port][qIndex]+psize<m_pg_min_cell) //use guaranteed pg buffer
	{
		m_usedIngressPGMinBytes[port][qIndex]+=psize;
		return;
	}
	else
	{
		m_usedIngressPGMinExceed[port][qIndex]++;
	}
	
	if (m_usedIngressPortMinBytes[port]+psize<m_port_min_cell) //use guaranteed port buffer
	{
		// if the packet is using port_min, is it also counted for pg?
		m_usedIngressPortMinBytes[port]+=psize;
		return;
	}
	else
	{
		m_usedIngressPortMinExceed[port]++;
	}

	//begin to use shared buffer
	if (m_usedIngressPGSharedBytes[port][qIndex]+psize<m_pg_shared_limit_cell)								//doesn't exceed pg share limit
	{
		if (m_usedIngressPortSharedBytes[port]+psize<m_port_max_shared_cell)								//doesn't exceed port share limit
		{
			if (m_usedIngressSPBytes[GetIngressSP(port,qIndex)]+psize < m_buffer_cell_limit_sp)				//doesn't exceed sp limit
			{
				m_usedIngressSPBytes[GetIngressSP(port,qIndex)]+=psize;
				m_usedIngressPortSharedBytes[port]+=psize;
				m_usedIngressPGSharedBytes[port][qIndex]+=psize;
				return;
			}
			else
			{
				m_usedIngressSPExceed[GetIngressSP(port,qIndex)] ++;
				if (m_usedIngressSPSharedBytes+psize<m_buffer_cell_limit_sp_shared)							//exceeds sp limit bu not sp share limit
				{
					m_usedIngressSPSharedBytes+=psize;
					m_usedIngressPortSharedBytes[port]+=psize;
					m_usedIngressPGSharedBytes[port][qIndex]+=psize;
					return;
				}
				else																						//exceeds sp share limit, use headroom
				{
					m_usedIngressHeadroomBytes += psize;
					//std::cout<<"Exceed SP pg#" << qIndex << "share limit!    "<<m_usedIngressHeadroomBytes<<"\n";
					//std::cout<<m_usedIngressSPSharedBytes<<"\t"<<m_usedIngressSPBytes[GetIngressSP(port,qIndex)]<<"\n";
					m_usedIngressSPSharedExceed ++;
					return;
				}
			}
		}
		else																								//exceeds port share limit, use headroom
		{
			m_usedIngressHeadroomBytes += psize;
			//std::cout<<"Exceed Port pg#" << qIndex << "share limit!    "<<m_usedIngressHeadroomBytes<<"\n";
			m_usedIngressPortSharedExceed[port] ++;
			return;
		}
	}
	else																									//exceeds pg share limit, use headroom
	{

		m_usedIngressHeadroomBytes += psize;
		//std::cout<<"Exceed PG pg#" << qIndex << "share limit!    "<<m_usedIngressHeadroomBytes<<"\n";
		m_usedIngressPGSharedExceed[port][qIndex] ++;
		return;
	}
	return;
}

void 
BroadcomNode::UpdateEgressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{

  //std::cout<<"BEFORE ADD:"<<psize<<" "<<m_usedEgressQSharedBytes[port][qIndex]<<"\tTotal:"<<m_usedTotalBytes<<"\n";
  if (m_usedEgressQMinBytes[port][qIndex]+psize<m_q_min_cell)	//guaranteed
  {
    m_usedEgressQMinBytes[port][qIndex]+=psize;
    return;
  }
  else
  {
    m_usedEgressQSharedBytes[port][qIndex]+=psize;
    m_usedEgressPortBytes[port]+=psize;
    m_usedEgressSPBytes[GetEgressSP(port,qIndex)]+=psize;
    
    //std::cout<<"ADD:"<<psize<<" "<<m_usedEgressQSharedBytes[port][qIndex]<<"\tTotal:"<<m_usedTotalBytes<<"\n";
    
  }

  return;
}

void
BroadcomNode::RemoveFromIngressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	m_usedTotalBytes -= psize;

		//std::cout<<m_usedTotalBytes<<"\tPort:"<<port<<"\tqIndex:"<<qIndex<<"\tPG exceed:"<<m_usedIngressPGSharedExceed[port][qIndex]<<"\n";

	if (m_usedIngressPGMinExceed[port][qIndex]==0)																		//doesn't exceed pg min
	{
		//std::cout<<"!!!!!!!!!!!!\t"<<port<<"\t"<<qIndex<<"\n";
		m_usedIngressPGMinBytes[port][qIndex]-=psize;
		return;
	}
	else
	{
		m_usedIngressPGMinExceed[port][qIndex]--;
	}

	if (m_usedIngressPortMinExceed[port]==0)																		//doesn't exceed port min
	{
		//std::cout<<"@@@@@@@@@@@@\t"<<port<<"\t"<<qIndex<<"\n";
		m_usedIngressPortMinBytes[port]-=psize;
		return;
	}
	else
	{
		m_usedIngressPortMinExceed[port]--;
	}

	if (m_usedIngressPGSharedExceed[port][qIndex]>0)																//exceeds pg share limit, remove from headroom
	{

		m_usedIngressHeadroomBytes -= psize;
		//std::cout<<"Remove from headroom for pg#" << qIndex <<"!           " << m_usedIngressHeadroomBytes<<"\n";
		m_usedIngressPGSharedExceed[port][qIndex] --;
		return;
	}
	if (m_usedIngressPortSharedExceed[port]>0)																		//exceeds port share limit, remove from headroom
	{
		m_usedIngressHeadroomBytes -= psize;
		m_usedIngressPortSharedExceed[port] --;
		return;
	}
	if (m_usedIngressSPSharedExceed>0)																				//exceeds sp share limit, remove from headroom
	{
		m_usedIngressHeadroomBytes -= psize;
		m_usedIngressSPSharedExceed --;
		return;
	}
	if (m_usedIngressSPExceed[GetIngressSP(port,qIndex)]>0)															//exceeds sp limit, remove from shared sp buffer
	{
		m_usedIngressSPSharedBytes-=psize;
		m_usedIngressPortSharedBytes[port]-=psize;
		m_usedIngressPGSharedBytes[port][qIndex]-=psize;
		m_usedIngressSPExceed[GetIngressSP(port,qIndex)]--;
		return;
	}
	else																											//nothing special
	{
		m_usedIngressSPBytes[GetIngressSP(port,qIndex)]-=psize;
		m_usedIngressPortSharedBytes[port]-=psize;
		m_usedIngressPGSharedBytes[port][qIndex]-=psize;
	}
	return;
}

void 
BroadcomNode::RemoveFromEgressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	//m_usedEgressPGBytes[qIndex] -= psize;
	//m_usedEgressPortBytes[port] -= psize;
	//m_usedEgressSPBytes[GetIngressSP(port,qIndex)] -= psize;


	if (m_usedEgressQSharedBytes[port][qIndex]>0)
	{
      //std::cout<<"BEFORE REMOVE:"<<psize<<" "<<m_usedEgressQSharedBytes[port][qIndex]<<" "<<m_usedEgressSPBytes[GetIngressSP(port,qIndex)]<<"\tTotal:"<<m_usedTotalBytes<<"\n";
      if(m_usedEgressQSharedBytes[port][qIndex] >= psize)
      {
    
		m_usedEgressQSharedBytes[port][qIndex] -= psize;
		m_usedEgressPortBytes[port] -= psize;
		m_usedEgressSPBytes[GetIngressSP(port,qIndex)] -= psize;

	
      }
      else
      {
        uint32_t count = m_usedEgressQSharedBytes[port][qIndex];
        m_usedEgressQSharedBytes[port][qIndex] = 0;
  		m_usedEgressQMinBytes[port][qIndex]-=(psize - count);
		m_usedEgressPortBytes[port] -= count;
		m_usedEgressSPBytes[GetIngressSP(port,qIndex)] -= count;
        
      }
      //std::cout<<"REMOVE:"<<psize<<" "<<m_usedEgressQSharedBytes[port][qIndex]<<" "<<m_usedEgressSPBytes[GetIngressSP(port,qIndex)]<<"\tTotal:"<<m_usedTotalBytes<<"\n";
	}
	else
	{
		m_usedEgressQMinBytes[port][qIndex]-=psize;
	}


	return;
}

void
BroadcomNode::GetPauseClasses(uint32_t port, uint32_t qIndex, bool pClasses[])
{
	if (m_dynamicth)
	{
		/*
		if (m_usedIngressPortSharedBytes[port]>m_port_shared_alpha_cell*(m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit))
		{
			std::cout<<"PAUSE Port:"<<port<<"\n";
			for (int i=0;i<qCnt;i++)
			{
				pClasses[i]=true;
			}
			return;
		}
		else
		{
			for (int i=0;i<qCnt;i++)
			{
				pClasses[i]=false;
				if (m_usedIngressPGSharedBytes[port][i]>m_pg_shared_alpha_cell*(m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit))
				{
					std::cout<<"PAUSE Port:"<<port<<"\tPG:"<<i<<"\tIngress PG buffer:"<<int(m_usedIngressPGSharedBytes[port][i]/1430.0)<<"\tTotal left:"<<int((m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit)/1430.0)<<"\tTotal used:"<<int(m_usedTotalBytes/1430.0)<<"\n";
					//std::cout<<"PAUSE Port:"<<port<<"\tQueue:"<<i<<"\n";
					pClasses[i]=true;
				}
			}
		}
		*/
		for (uint i=0;i<qCnt;i++)
		{
			pClasses[i]=false;

			if (m_usedIngressPGSharedBytes[port][i]==0)
				continue;

			if (m_usedIngressPGSharedBytes[port][i]>m_pg_shared_alpha_cell*((double)m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit))
			{
				std::cout<<Simulator::Now().GetSeconds()<<"\tPAUSE Port:"<<port<<"\tPG:"<<i<<"\tIngress PG buffer:"<<int(m_usedIngressPGSharedBytes[port][i]/1430.0)<<"\tShared buffer left:"<<int(((double)m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit)/1430.0)<<"\tTotal used:"<<int(m_usedTotalBytes/1430.0)<<"\n";
				pClasses[i]=true;
			}
		}
	}
	else
	{
		if (m_usedIngressPortSharedExceed[port]>0)					//pause the whole port
		{
			//std::cout<<"PAUSE Port:"<<port<<"\n";
			for (uint i=0;i<qCnt;i++)
			{
				pClasses[i]=true;
			}
			return;
		}
		else
		{
			for (uint i=0;i<qCnt;i++)
			{
				pClasses[i]=false;
			}
		}
		if (m_usedIngressPGSharedExceed[port][qIndex]>0)
		{
			//std::cout<<"PAUSE Port:"<<port<<"\tPG:"<<qIndex<<"\tIngress PG buffer:"<<int(m_usedIngressPGSharedBytes[port][qIndex]/1430.0)<<"\n";
			pClasses[qIndex]=true;
			//std::cout<<"PAUSE\tPort:"<<port<<"\tqindex:"<<qIndex<<"\tUsed PG buffer:"<<m_usedIngressPGSharedBytes[port][qIndex]<<"\tUsed Ingress Headroom:"<<m_usedIngressHeadroomBytes<<"\n";
		}
	}
	return;
}


bool
BroadcomNode::GetResumeClasses(uint32_t port, uint32_t qIndex)
{
	if (m_dynamicth)
	{
		/*
		if (m_usedIngressPGSharedBytes[port][qIndex]<m_pg_shared_alpha_cell*(m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit)-m_pg_shared_alpha_cell_off_diff
			&& m_usedIngressPortSharedBytes[port]<m_port_shared_alpha_cell*(m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit)-m_port_shared_alpha_cell_off_diff)
		{
			return true;
		}
		*/
		if (m_usedIngressPGSharedBytes[port][qIndex]<m_pg_shared_alpha_cell*((double)m_maxBufferBytes-m_usedTotalBytes-m_pg_hdrm_limit)-m_pg_shared_alpha_cell_off_diff)
		{
			return true;
		}
	}
	else
	{
		if (m_usedIngressPGSharedBytes[port][qIndex]<m_pg_shared_limit_cell_off 
			&& m_usedIngressPortSharedBytes[port]<m_port_min_cell_off)
		{
			return true;
		}
	}

	return false;
}

uint32_t 
BroadcomNode::GetIngressSP(uint32_t port,uint32_t pgIndex)
{
	return 0;
}

uint32_t 
BroadcomNode::GetEgressSP(uint32_t port,uint32_t qIndex)
{
	return 0;
}

bool
BroadcomNode::ShouldSendCN(uint32_t indev, uint32_t ifindex, uint32_t qIndex)
{
	//if (m_usedIngressPGSharedBytes[indev][qIndex]>m_pg_qcn_threshold)
	//	return true;

	//for fluid model
	/*
	if (Simulator::Now().GetSeconds()>m_log_start && Simulator::Now().GetSeconds()<m_log_end)
	{
		std::ofstream f;
		f.open("/tmp/ramdisk/test.txt",std::ios::app);
		f<<Simulator::Now().GetSeconds()<<"\t"<<m_usedEgressPortBytes[ifindex]<<"\n";
		f.close();
		m_log_start += m_log_step;
	}
	*/

	//operate on egress port
	if (m_usedEgressPortBytes[ifindex]>m_pg_qcn_threshold_max)
	{
		return true;
	}
	else if (m_usedEgressPortBytes[ifindex]>m_pg_qcn_threshold && m_pg_qcn_threshold!=m_pg_qcn_threshold_max)
	{
		double p = 1.0 * (m_usedEgressPortBytes[ifindex]-m_pg_qcn_threshold) / (m_pg_qcn_threshold_max - m_pg_qcn_threshold) * m_pg_qcn_maxp;
		if (UniformVariable(0,1).GetValue()<p)
			return true;
	}
	return false;
}


void 
BroadcomNode::SetBroadcomParams(
	uint32_t buffer_cell_limit_sp, //ingress sp buffer threshold p.120
	uint32_t buffer_cell_limit_sp_shared, //ingress sp buffer shared threshold p.120, nonshare -> share
	uint32_t pg_min_cell, //ingress pg guarantee p.121					---1
	uint32_t port_min_cell, //ingress port guarantee						---2
	uint32_t pg_shared_limit_cell, //max buffer for an ingress pg			---3	PAUSE
	uint32_t port_max_shared_cell, //max buffer for an ingress port		---4	PAUSE
	uint32_t pg_hdrm_limit, //ingress pg headroom
	uint32_t port_max_pkt_size, //ingress global headroom
	uint32_t q_min_cell,	//egress queue guaranteed buffer
	uint32_t op_uc_port_config1_cell, //egress queue threshold
	uint32_t op_uc_port_config_cell, //egress port threshold
	uint32_t op_buffer_shared_limit_cell, //egress sp threshold
	uint32_t q_shared_alpha_cell,
	uint32_t port_share_alpha_cell,
	uint32_t pg_qcn_threshold)
{
	m_buffer_cell_limit_sp = buffer_cell_limit_sp;
	m_buffer_cell_limit_sp_shared = buffer_cell_limit_sp_shared;
	m_pg_min_cell = pg_min_cell;
	m_port_min_cell = port_min_cell;
	m_pg_shared_limit_cell = pg_shared_limit_cell;
	m_port_max_shared_cell = port_max_shared_cell;
	m_pg_hdrm_limit = pg_hdrm_limit;
	m_port_max_pkt_size = port_max_pkt_size;
	m_q_min_cell = q_min_cell;
	m_op_uc_port_config1_cell = op_uc_port_config1_cell;
	m_op_uc_port_config_cell = op_uc_port_config_cell;
	m_op_buffer_shared_limit_cell = op_buffer_shared_limit_cell;
	m_pg_shared_alpha_cell = q_shared_alpha_cell;
	m_port_shared_alpha_cell = port_share_alpha_cell;
	m_pg_qcn_threshold = pg_qcn_threshold;
}

uint32_t 
BroadcomNode::GetUsedBufferTotal()
{
	return m_usedTotalBytes;
}

void 
BroadcomNode::SetDynamicThreshold()
{
	m_dynamicth = true;
	//using dynamic threshold, we don't respect the static thresholds anymore
	m_pg_shared_limit_cell = m_maxBufferBytes;
	m_port_max_shared_cell = m_maxBufferBytes;
	m_buffer_cell_limit_sp_shared =  m_maxBufferBytes;
	m_buffer_cell_limit_sp = m_maxBufferBytes;
	return;
}

void 
BroadcomNode::SetMarkingThreshold(uint32_t kmin, uint32_t kmax, double pmax)
{
	m_pg_qcn_threshold = kmin*1500;
	m_pg_qcn_threshold_max = kmax*1500;
	m_pg_qcn_maxp = pmax;
}


} // namespace ns3

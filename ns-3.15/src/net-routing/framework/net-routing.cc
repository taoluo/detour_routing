/*
 * Copyright (c) 2009 The Georgia Institute of Technology
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
 * Authors: Josh Pelkey <jpelkey@gatech.edu>
 */

#include <queue>
#include <iomanip>

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-list-routing.h"

#include "net-routing.h"
#include <stdio.h>

NS_LOG_COMPONENT_DEFINE ("NetRouting");

namespace ns3
{
    NS_OBJECT_ENSURE_REGISTERED (NetRouting);

    TypeId NetRouting::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::NetRouting").SetParent<Ipv4RoutingProtocol>().AddConstructor<NetRouting>();
        return tid;
    }

    NetRouting::NetRouting ()
    {
        m_totalNodes = 0;

        NS_LOG_FUNCTION_NOARGS ();
    }

    NetRouting::~NetRouting ()
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    void NetRouting::SetIpv4 (Ptr<Ipv4> ipv4)
    {
        NS_ASSERT (ipv4 != 0);
        NS_ASSERT (m_ipv4 == 0);
        NS_LOG_DEBUG ("Created NetRoutingProtocol");

        m_ipv4 = ipv4;
    }

    void NetRouting::DoDispose ()
    {
        NS_LOG_FUNCTION_NOARGS ();

        m_node = 0;
        m_ipv4 = 0;

        Ipv4RoutingProtocol::DoDispose ();
    }


    void NetRouting::SetNode (Ptr<Node> node)
    {
        NS_LOG_FUNCTION_NOARGS ();
        m_node = node;
    }

    Ptr<Ipv4Route> NetRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    {
		Ptr<Ipv4Route> rtentry = Create<Ipv4Route>();
		rtentry->SetSource(Ipv4Address(m_node->GetId()));
		rtentry->SetDestination(header.GetDestination());
		rtentry->SetGateway(header.GetDestination());
		rtentry->SetOutputDevice(m_node->GetDevice(0));

        return rtentry;
    }

    bool NetRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                   LocalDeliverCallback lcb, ErrorCallback ecb)
    {
        return false;
    }

    void NetRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
    {}

    // virtual functions from Ipv4RoutingProtocol
    void NetRouting::NotifyInterfaceUp (uint32_t i)
    {
    }
    void NetRouting::NotifyInterfaceDown (uint32_t i)
    {
    }
    void NetRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
    {
    }
    void NetRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
    {
    }
} // namespace ns3

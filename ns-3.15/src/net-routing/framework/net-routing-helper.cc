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

#include "net-routing-helper.h"
#include "ns3/net-routing.h"

namespace ns3 {

    NetRoutingHelper::NetRoutingHelper ()
    {
        m_agentFactory.SetTypeId("ns3::NetRouting");
    }

    NetRoutingHelper::NetRoutingHelper (const NetRoutingHelper &o)
        : m_agentFactory (o.m_agentFactory)
    {
    }

    NetRoutingHelper* NetRoutingHelper::Copy (void) const 
    {
        return new NetRoutingHelper (*this); 
    }

    void NetRoutingHelper::SetAttribute (std::string name, const AttributeValue &value)
    {
        m_agentFactory.Set (name, value);
    }

    Ptr<Ipv4RoutingProtocol> NetRoutingHelper::Create (Ptr<Node> node) const
    {
        Ptr<NetRouting> agent = m_agentFactory.Create<NetRouting> ();
        agent->SetNode (node);
        node->AggregateObject (agent);
  
        return agent;
    }    
} // namespace ns3

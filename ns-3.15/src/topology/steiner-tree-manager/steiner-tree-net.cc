#include "steiner-tree-net.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <map>
#include <stack>
#include "assert.h"
#include <algorithm>
#include "steiner-tree-bcube.h"
#include "steiner-tree-torus.h"
#include "steiner-tree-fattree.h"

NetNode::NetNode(Network* network, int type, bool cachingAbility)
{
    // update node information
    m_nReserved[0] = 0;
    m_nReserved[1] = 0;
    m_nReserved[2] = 0;
    m_nReserved[3] = 0;

    m_bCachingAbility = cachingAbility;
    m_bIsDestNode = false;
    m_nID = network->m_Nodes.size();
    m_nType = type;

    // update network information
    m_network = network;
    if (type == NODE_TYPE_SERVER)
        network->m_nServerNumber++;
    else if (type == NODE_TYPE_SWITCH)
        network->m_nSwitchNumber++;
    else
        assert(false);

    network->m_Nodes.push_back(this);
}

void NetNode::AddLinks(NetLink* inlink, NetLink* outlink)
{
    assert(m_OutLinks.size() == m_InLinks.size());

    inlink->SetDestID(m_nID, m_InLinks.size());
    outlink->SetSrcID(m_nID, m_OutLinks.size());

    m_InLinks.push_back(inlink);
    m_OutLinks.push_back(outlink);
}


static bool debug_mode = false;

NetLink::NetLink(Network* network) : m_Network(network) 
{
    Init(); 
    m_id = network->m_links.size(); 
    network->m_links.push_back(this);
}

void NetLink::SetSrcID(int srcID, int port)
{
    m_nSrcID = srcID; 
    m_nSrcPort = port;
    //m_Network->m_Nodes[srcID]->m_OutLinks.push_back(this); 
}

void NetLink::SetDestID(int destID, int port)
{
    m_nDstID = destID;
    m_nDstPort = port;
    // m_Network->m_Nodes[destID]->m_InLinks.push_back(this); 
}

void Network::AddDestID(int destID)
{
    m_destIDs.insert(destID);
    m_Nodes[destID]->m_bIsDestNode = true;
}

void Network::AddFailureLink(int id)
{
    m_links[id]->m_bIsBroken = true;
}

int Network::GetDestNodeNum() const
{
    return m_destIDs.size();
}

void Network::ClearAllNodesReservations()
{
    for each(NetNode* node in m_Nodes)
    {
        node->m_nReserved[0] = 0;
        node->m_nReserved[1] = 0;
        node->m_nReserved[2] = 0;
        node->m_nReserved[3] = 0;
    }
}

void Network::ClearAllLinksReservations()
{
    for each(NetLink* link in m_links)
    {
        link->m_nReserved[0] = 0;
        link->m_nReserved[1] = 0;
        link->m_nReserved[2] = 0;
        link->m_nReserved[3] = 0;
    }
}


int Network::GetTreeDepth(int srcID, int treeID)
{
    int maxdepth = 0;

    ClearAllLinksReservations();

    for each(int id in m_destIDs)
    {
        int port = -1;

        std::stack<NetLink*> links;

        int depth = 0;
        do
        {
            NetNode* node = m_Nodes[id];

            assert(node->m_fib[treeID].find(port) != node->m_fib[treeID].end());

            port = node->m_fib[treeID][port];

            if (port == -1)
            {
                depth = 1;
                break;
            }

            NetLink* link = node->m_InLinks[port];

            if (link->m_nReserved[0] != 0)
            {
                depth = link->m_nReserved[0];
                break;
            }

            id = link->GetSrcID();
            port = link->GetSrcPort();
            links.push(link);
        } while(true);

        while (links.empty() == false)
        {
            NetLink* link = links.top(); 
            links.pop();

            link->m_nReserved[0] = depth++;
        }

        maxdepth = max(maxdepth, depth);
    }

    return maxdepth;
}

int Network::GetBranchNodeNumber(int root, int srcID, int treeID)
{
    int res = 0;
    
    NetNode* node = m_Nodes[srcID];
    if(node->m_nReserved[0])
        return 0;
    node->m_nReserved[0] = 1;

    int no_branch = 0;
    for each(NetLink* link in node->m_OutLinks)
    {
        if(link->GetTreeID() == treeID)
        {
            res += GetBranchNodeNumber(root, link->GetDestID(), treeID);
            no_branch++;
        }
    }

    if(node->m_bIsDestNode == false && root != srcID && no_branch > 1)
        res++;

    return res;
}

int Network::GetSteinerTreeNodes(int root, int srcID, int treeID, std::set<int>& tree_nodes)
{
    int node_number = 0;

    NetNode* node = m_Nodes[srcID];
    if(node->m_nReserved[0])
        return 0;
    node->m_nReserved[0] = 1;
    
    if(node->m_bIsDestNode == false && srcID != root)
    {
        tree_nodes.insert(srcID);
        node_number += 1;
    }

    for each(NetLink* link in node->m_OutLinks)
    {
        if(link->GetTreeID() == treeID)
            node_number += GetSteinerTreeNodes(root, link->GetDestID(), treeID, tree_nodes);
    }

    return node_number;
}


bool Network::ConstructSteinerTree(int srcID, int treeID)
{
    for each(int destid in m_destIDs)
    {
        NetNode* node = m_Nodes[destid];
        int currid = destid;
        bool next = false;
        int port = -1;

        while(currid != srcID)
        {
            for each(NetLink* inlink in node->m_InLinks)
            {
                if(inlink->GetTreeID() != treeID)
                    continue;

                if(inlink->m_nReserved[0] != 0)
                    next = true;
                else
                {
                    inlink->m_nReserved[0] = treeID;
                    currid = inlink->GetSrcID();
                }

                // printf("%5d:  %d: %d->%d (%d)\n", inlink->m_id, inlink->GetDestID(), port, inlink->GetDstPort(), inlink->GetSrcID());
                node->m_fib[treeID][port] = inlink->GetDstPort();
                port = inlink->GetSrcPort();

                break;
            }

            if(next)
                break;

            node = m_Nodes[currid];
            //printf("%d\n", currid);
        }

        if (node->m_nID == srcID)
            node->m_fib[treeID][port] = -1;
    }

    return true;
}

int Network::ConstructSteinerTree(int srcID, bool multiple)
{
    if (multiple == true)
    {
        BuildSpanningTrees(srcID);

        ClearAllNodesReservations(); 
        ClearAllLinksReservations();

        for(int i = 0; i < m_spanTreeIDs.size(); i++)
            ConstructSteinerTree(srcID, m_spanTreeIDs[i]);

        for each(NetLink* link in m_links)
        {
            link->SetTreeID(link->m_nReserved[0]);
            link->m_nReserved[0] = 0;
        }
    }
    else
    {
        ClearAllNodesReservations();
        ClearAllLinksReservations();
        if (BFSRepairSteinerTree(srcID, 1))
            m_spanTreeIDs.push_back(1);
    }

    return m_spanTreeIDs.size();
}

void Network::VisualizeGraph(int srcID, int treeID, const char* append)
{
    char script_name[1024];
    char out_name[1024];
    
    sprintf(script_name, "%s.dot", m_name.c_str());
    sprintf(out_name, "%s_%s.jpg", m_name.c_str(), append);

    FILE* fp = fopen(script_name, "w");
    fprintf(fp, "digraph G{\n");

    for(DestIDs::iterator it = m_destIDs.begin(); it != m_destIDs.end(); it++)
    {
        const NetNode* node = m_Nodes[*it];
        if(node->m_nType == NODE_TYPE_SWITCH)
            fprintf(fp, "\"%s\"[shape=box]\n", node->getName());
        else if(node->m_nType == NODE_TYPE_SERVER)
        {
            if((*it) == srcID)
                fprintf(fp, "\"%s\"[color=limegreen, style=filled]\n", node->getName());
            else if(node->m_bIsDestNode)
                fprintf(fp, "\"%s\"[color=purple, style=filled]\n", node->getName());
            else
                fprintf(fp, "\"%s\"", node->getName());
        }
    }

    for(int i = 0; i < m_links.size(); i++)
    {
        const NetLink* link = m_links[i];

        if(link->GetTreeID() != treeID && treeID != 0)
            continue;

        if(link->m_bIsBroken == false)
            fprintf(fp, "\"%s\"->\"%s\"[color = %s, label=\"%d\"]\n", m_Nodes[link->GetSrcID()]->getName(), 
                m_Nodes[link->GetDestID()]->getName(), link->getLinkColor(), i);
        else
            fprintf(fp, "\"%s\"->\"%s\"[color = %s, style=dashed, label=\"%d\"]\n", m_Nodes[link->GetSrcID()]->getName(), 
                m_Nodes[link->GetDestID()]->getName(), link->getLinkColor(), i);
    }

    fprintf(fp, "\n}");
    fclose(fp);

    CStringA cmd_line;
    cmd_line.Format("dot -Tjpg %s -o %s", script_name, out_name);
    system(cmd_line);
}

void Network::FreeSpanningTreeLinks(int treeID)
{
    for each(NetLink* link in m_links)
        if(link->GetTreeID() == treeID)
            link->SetTreeID(0);
}

double Network::GetAvgTreeDepth(int srcID)
{
    if (m_spanTreeIDs.empty())
        return 0;

    int depth_sum = 0;
    for each(int spanning_tree_id in m_spanTreeIDs)
    {
        // ClearAllNodesReservations();
        ClearAllLinksReservations();
        depth_sum += GetTreeDepth(srcID, spanning_tree_id);
    }

    return depth_sum * 1.0 / m_spanTreeIDs.size();
}

int Network::GetMaxTreeDepth(int srcID)
{
    int max_depth = 0;
    for each(int spanning_tree_id in m_spanTreeIDs)
    {
        ClearAllNodesReservations();
        int depth = GetTreeDepth(srcID, spanning_tree_id);
        if(max_depth < depth)
            max_depth = depth;
    }

    return max_depth;
}

int Network::GetTotalSteinerTreeNodeNumber(int srcID)
{
    std::set<int> spainer_tree_node_ids; 
    for each(int spanning_tree_id in m_spanTreeIDs)
    {
        ClearAllNodesReservations();
        GetSteinerTreeNodes(srcID, srcID, spanning_tree_id, spainer_tree_node_ids);
    }

    return spainer_tree_node_ids.size();
}

double Network::GetAverageSteinerNodeNumber(int srcID)
{
    if (m_spanTreeIDs.empty())
        return 0;

    int no_stnode_sum = 0;
    for each(int spanning_tree_id in m_spanTreeIDs)
    {
        std::set<int> stainer_tree_node_ids; 
        ClearAllNodesReservations();
        GetSteinerTreeNodes(srcID, srcID, spanning_tree_id, stainer_tree_node_ids);

        no_stnode_sum += stainer_tree_node_ids.size();
    }

    return no_stnode_sum * 1.0 / m_spanTreeIDs.size();
}

int Network::GetTotalBranchNodeNumber(int srcID)
{
    if (m_spanTreeIDs.empty())
        return 0;

    int res = 0;
    for each(int tree_id in m_spanTreeIDs)
    {
        ClearAllNodesReservations();
        res += GetBranchNodeNumber(srcID, srcID, tree_id);
    }

    return res;
}

double Network::GetAverageBranchNodeNumber(int srcID)
{
    if (m_spanTreeIDs.empty())
        return 0;

    int branch_node_sum = 0;
    for each(int tree_id in m_spanTreeIDs)
    {
        ClearAllNodesReservations();
        branch_node_sum += GetBranchNodeNumber(srcID, srcID, tree_id);
    }

    return branch_node_sum * 1.0 / m_spanTreeIDs.size();
}

int Network::GetNumberOfCompleteSteinerTree() const
{
    return m_spanTreeIDs.size();
}

void Network::PrepareToRepairSteinerTrees()
{
    broken_tree_ids.clear();
    for each(NetLink* link in m_links)
        if(link->m_bIsBroken && link->GetTreeID() != 0)
            broken_tree_ids.insert(link->GetTreeID());

    m_tree_links.clear();
    for each(int treeID in m_spanTreeIDs)
    {
        std::vector<NetLink*> links;
        links.reserve(m_links.size());
        m_tree_links[treeID] = links;
    }

    for each(NetLink* link in m_links)
    {
        int treeID = link->GetTreeID();
        if(treeID != 0)
            m_tree_links[treeID].push_back(link);
    }
}

bool Network::RepairSteinerTrees(int srcID)
{
    int no_total = m_spanTreeIDs.size();
    int no_broken = broken_tree_ids.size();
    int bound = GetUpperBoundOfSteinerTreeNumber(srcID);

    std::set<int> m_complete_trees;

    m_complete_trees.clear();
    for each(int tree_id in m_spanTreeIDs)
    {
        if(broken_tree_ids.find(tree_id) == broken_tree_ids.end())
            m_complete_trees.insert(tree_id);
    }
    
    for(int tree_index = 0; tree_index < no_total; tree_index++)
    {
        if(broken_tree_ids.find(m_spanTreeIDs[tree_index]) == broken_tree_ids.end())
            continue;

        ClearAllNodesReservations();

        int treeID = m_spanTreeIDs[tree_index];
        if(m_complete_trees.size() >= bound || BFSRepairSteinerTree(srcID, treeID) == false)
            FreeSpanningTreeLinks(treeID);
        else
            m_complete_trees.insert(treeID);
    }

    m_spanTreeIDs.clear(); 
    for each(int treeID in m_complete_trees)
        m_spanTreeIDs.push_back(treeID);

    return true;
}

bool Network::VerifySteinerTrees(int srcID)
{
    for each(int treeID in m_spanTreeIDs)
    {
        ClearAllNodesReservations();
        int no_dest_in_steiner_tree = GetNumberOfTargetNodesInSubtree(srcID, treeID);
        
        if(no_dest_in_steiner_tree == m_destIDs.size())
        {
            if(debug_mode)
                printf("Tree %d verified success.\n", treeID);
        }
        else
        {
            if(debug_mode)
                printf("Tree %d verified failed (%d != %d).\n", treeID, no_dest_in_steiner_tree, m_destIDs.size());
            
            VisualizeGraph(srcID, treeID, "invalid");

            ClearAllNodesReservations();
            return false;
        }
    }
    return true;
}

int Network::GetUpperBoundOfSteinerTreeNumber(int srcID) const
{
    int min_indegree = 0;

    NetNode* node = m_Nodes[srcID];
    for each(NetLink* link in node->m_OutLinks)
        if(link->m_bIsBroken == false)
            min_indegree += 1;

    for each(int destID in m_destIDs)
    {
        NetNode* node = m_Nodes[destID];

        int no_valid_link = 0;
        for each(NetLink* link in node->m_InLinks)
            if(link->m_bIsBroken == false)
                no_valid_link += 1;

        min_indegree = min(no_valid_link, min_indegree);
    }

    return min_indegree;
}

int Network::GetNetworkNodeNumber(int srcID, bool bClear)
{
    ClearAllNodesReservations();

    std::deque<int> queue;
    queue.push_back(srcID);

    int no_nodes = 1;
    m_Nodes[srcID]->m_nReserved[0] = true;

    while(queue.empty() == false)
    {
        int nodeid = queue.front();
        queue.pop_front();

        NetNode* node = m_Nodes[nodeid];
        
        for each(NetLink* link in node->m_OutLinks)
        {
            if(link->m_bIsBroken == true)
                continue;

            int peerid = link->GetDestID();
            NetNode* peernode = m_Nodes[peerid];

            if(peernode->m_nReserved[0] == true)
                continue;
            peernode->m_nReserved[0] = true;

            no_nodes++;
            queue.push_back(peerid);
        }
    }

    return no_nodes;
}

bool Network::IsNetworkConnected(int srcID)
{
    return (GetNetworkNodeNumber(srcID) == m_Nodes.size());
}

bool Network::BFSRepairSteinerTree(int srcID, int treeID)
{
    const int max_link_no = 4096;
    NetLink* idleLinks[max_link_no];
    int no_idleLinks = 0;
    NetLink* halfIdleLinks[max_link_no];
    int no_halfIdleLinks = 0;

    std::deque<int> curr_nodes;
    curr_nodes.push_back(srcID);

    NetNode* node = m_Nodes[srcID];
    node->m_nReserved[0] = treeID;

    int no_dest_left = m_destIDs.size();

    while(curr_nodes.empty() == false)
    {
        int nodeID = curr_nodes.front();
        curr_nodes.pop_front();

        NetNode* node = m_Nodes[nodeID];

        if(node->m_bIsDestNode == true)
            no_dest_left--;

        if(no_dest_left == 0)
            break;

        Links& outlinks = node->m_OutLinks;

        no_idleLinks = 0;
        no_halfIdleLinks = 0;

        for each(NetLink* link in outlinks)
        {
            if(link->m_bIsBroken == true)
                continue;

            if(link->GetTreeID() == 0 || link->GetTreeID() == treeID)
            {
                if((link->m_revlink->GetTreeID() == 0 || link->m_revlink->GetTreeID() == treeID) && link->m_revlink->m_bIsBroken == false)
                {
                    idleLinks[no_idleLinks++] = link;
                    link->SetTreeID(0);
                    link->m_revlink->SetTreeID(0);
                }
                else
                    halfIdleLinks[no_halfIdleLinks++] = link;
            }
        }

        // first, we use all the links that are previously used
        for(int i = 0; i < no_idleLinks; i++)
        {
            NetLink* link = idleLinks[i];

            int peer_id = link->GetDestID();
            NetNode* peer_node = m_Nodes[peer_id];
            if(peer_node->m_nReserved[0] == treeID)
                continue;
            peer_node->m_nReserved[0] = treeID;
            link->m_nReserved[1] = treeID;

            curr_nodes.push_back(peer_id);
        }

        // then, we use the other links
        for(int i = 0; i < no_halfIdleLinks; i++)
        {
            NetLink* link = halfIdleLinks[i];

            int peer_id = link->GetDestID();
            NetNode* peer_node = m_Nodes[peer_id];
            if(peer_node->m_nReserved[0] == treeID)
                continue;
            peer_node->m_nReserved[0] = treeID;
            link->m_nReserved[1] = treeID;

            curr_nodes.push_back(peer_id);
        }
    }

    for each(NetNode* node in m_Nodes)
        node->m_fib[treeID].clear();

    const std::vector<NetLink*>& tree_links = m_tree_links[treeID];
    for each(NetLink* link in tree_links)
        link->SetTreeID(0);

    m_Nodes[srcID]->m_nReserved[1] = treeID;
    // reconstruct steiner tree
    for each(int destid in m_destIDs)
    {
        NetNode* node = m_Nodes[destid];

        int port = -1;
        while(true)
        {
            int prev_id = -1;
            bool connected = false;

            node->m_nReserved[1] = treeID;

            for each(NetLink* link in node->m_InLinks)
            {
                if(link->m_nReserved[1] != treeID)
                    continue;

                node->m_fib[treeID][port] = link->GetDstPort();
                port = link->GetSrcPort();

                connected = true;

                if (link->m_nReserved[3] == treeID)
                    break;

                link->SetTreeID(treeID);
                link->m_nReserved[3] = treeID;

                prev_id = link->GetSrcID();
                break;
            }

            if (node->m_nID == srcID)
                break;

            if (prev_id == -1 && connected == false)
                return false;

            if (prev_id == -1 && connected == true)
                break;

            node = m_Nodes[prev_id];
        }

        if (node->m_nID == srcID)
            node->m_fib[treeID][port] = -1;
    }

    return true;
}

int Network::GetNumberOfTargetNodesInSubtree(int srcID, int treeID)
{
    int no_target_nodes = 0;

    NetNode* node = m_Nodes[srcID];
    if(node->m_nReserved[0] != 0)
        return 0;
    node->m_nReserved[0] = 1;

    if(node->m_bIsDestNode)
        no_target_nodes = 1;

    for each(NetLink* link in m_Nodes[srcID]->m_OutLinks)
    {
        if(link->GetTreeID() == treeID && link->m_bIsBroken == false)
            no_target_nodes += GetNumberOfTargetNodesInSubtree(link->GetDestID(), treeID);
    }

    return no_target_nodes;
}

double Network::ImproveQualityOfTree(int srcID, int treeID, bool parentLink, int inport, int& upport, int& no_flows)
{
    NetNode* node = m_Nodes[srcID];

    std::vector<int> outlinkIDs;
    
    for (int i = 0; i < node->m_OutLinks.size(); i++)
    {
        NetLink* link = node->m_OutLinks[i];
        if (link->GetTreeID() == treeID)
        {
            if (link->m_revlink->m_bIsBroken == false && (link->m_revlink->GetTreeID() == 0 || link->m_revlink->GetTreeID() == treeID))
                outlinkIDs.push_back(i);
        }
    }

    for (int i = 0; i < node->m_OutLinks.size(); i++)
    {
        NetLink* link = node->m_OutLinks[i];
        if (link->GetTreeID() == treeID)
        {
            if ((link->m_revlink->m_bIsBroken == false && (link->m_revlink->GetTreeID() == 0 || link->m_revlink->GetTreeID() == treeID)) == false)
                outlinkIDs.push_back(i);
        }
    }

    if (node->m_bCachingAbility == true)
    {
        double quality = 1.0;

        for (int i = 0; i < outlinkIDs.size(); i++)
        {
            NetLink* link = node->m_OutLinks[outlinkIDs[i]];

            if (link->GetTreeID() == treeID && m_links[link->GetDestID()]->m_nReserved[0] == 0)
            {
                m_links[link->GetDestID()]->m_nReserved[0] = 1;

                int downport = -2;
                int child_flows = 0;
                double subtree_quality = ImproveQualityOfTree(link->GetDestID(), treeID, false, link->GetSrcPort(), downport, child_flows);
                quality = min(quality, subtree_quality);
            }
        }

        if (parentLink)
            upport = inport;

        no_flows = 1;
        return quality;
    }
    else
    {
        double quality = 0.0;

        int sub_flows = 0;
        double sub_quality = 1.0;
        double min_quality = 1.0;

        double total_flows = 0;

        int prevport = (node->m_bCachingAbility == false ? -2 : -1);

        for (int i = 0; i < outlinkIDs.size(); i++)
        {
            NetLink* link = node->m_OutLinks[outlinkIDs[i]];

            if (link->GetTreeID() == treeID && m_links[link->GetDestID()]->m_nReserved[0] == 0)
            {
                if (prevport != -2)
                    node->m_fib[treeID][link->GetSrcPort()] = prevport;

                m_links[link->GetDestID()]->m_nReserved[0] = 1;

                bool needUpLink = (parentLink == true || i < outlinkIDs.size() - 1);

                int downport = -2;
                int child_flows = 0;
                double subtree_quality = ImproveQualityOfTree(link->GetDestID(), treeID, needUpLink, link->GetDstPort(), downport, child_flows);

                if (prevport == -2)
                {
                    total_flows += sub_flows;
                    min_quality = min(min_quality, sub_quality);

                    sub_quality = 1.0;
                    sub_flows = 0;
                }

                sub_flows = max(sub_flows, child_flows);
                sub_quality = min(sub_quality, subtree_quality);

                if (needUpLink == true && downport != -2 && ((link->m_revlink->GetTreeID() == 0 || link->m_revlink->GetTreeID() == treeID)) && link->m_revlink->m_bIsBroken == false)
                {
                    link->m_revlink->SetTreeID(treeID);
                    // quality = min(quality, subtree_quality);

                    NetNode* downnode = m_Nodes[link->GetDestID()];
                    downnode->m_fib[treeID][link->m_revlink->GetSrcPort()] = downport;

                    prevport = link->m_revlink->GetDstPort();
                }
                else
                {
                    total_flows += sub_flows;
                    min_quality = min(min_quality, sub_quality);

                    sub_quality = 1.0;
                    sub_flows = 0;

                    prevport = -2;
                }
            }
        }

        total_flows += sub_flows;
        min_quality = min(min_quality, sub_quality);

        if (min_quality * total_flows <= 1.0)
            quality = min_quality;
        else
            quality = min_quality * 1.0 / (min_quality * total_flows);

        no_flows = total_flows;

        if (parentLink)
            upport = prevport;
    
        return quality;
    }   
}

double Network::ImproveQualityOfTrees(int srcID)
{
    double totalQuality = 0;
    for (int i = 0; i < m_spanTreeIDs.size(); i++)
    {   
        for (int i = 0; i < m_links.size(); i++)
            m_links[i]->m_nReserved[0] = 0;

        int downport, no_flows;
        totalQuality += ImproveQualityOfTree(srcID, m_spanTreeIDs[i], false, -1, downport, no_flows);
    }

    return totalQuality;
}

void Network::PrintFIB(int treeID) const
{
    for each (NetNode* node in m_Nodes)
    {
        if (node->m_fib[treeID].empty() == true)
            continue;

        printf("Node %s: \n", node->getName());
        for (NetNode::FIB::iterator it = node->m_fib[treeID].begin(); it != node->m_fib[treeID].end(); it++)
        {
            if (it->second == -1)
                printf("%d -> %d (local)\n", it->first, it->second);
            else
                printf("%d -> %d (%d)\n", it->first, it->second, node->m_InLinks[it->second]->GetSrcID());
        }
    }
}

int Network::GetNumberOfLinksInTree(int treeID) const
{
    int no_links = 0;
    for each (NetLink* link in m_links)
    {
        if (treeID != 0 && link->GetTreeID() == treeID)
            no_links++;
        else if (treeID == 0 && link->GetTreeID() != 0)
            no_links++;
    }

    return no_links;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "steiner-tree-bcube.h"
#include <assert.h>
#include <deque>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BCubeNode::BCubeNode(BCube* bcube, int type, bool cachingAbility) : NetNode(bcube, type, cachingAbility)
{}

BCubeNode::~BCubeNode()
{}

int BCubeNode::GetNeighborSwitchID(int port)
{
    assert(m_nType == NODE_TYPE_SERVER);

    BCube* bcube = (BCube*)m_network;
    int n = bcube->m_n;
    int k = bcube->m_k;

    int base = port * (int)pow((double)n, k);

    int mod = (int)pow((double)n, port);
    int offset = m_nID / (mod * n) * mod + m_nID % mod;

    return bcube->GetServerNumber() + base + offset;
}

int BCubeNode::GetNeighborServerID(int port)
{
    assert(m_nType == NODE_TYPE_SWITCH);

    BCube* bcube = (BCube*)m_network;
    int n = bcube->m_n;
    int k = bcube->m_k;

    int localID = m_nID - bcube->GetServerNumber();

    int dim = localID / (int)pow((double)n, k);

    int base = dim * (int)pow((double)n, k);
    int offset = localID - base;

    int mod = (int)pow((double)n, dim);
    return offset / mod * (mod * n) + port * mod + offset % mod;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BCube::Init()
{
    int serverNum = (int)pow((double)m_n, m_k + 1);
    int switchNum = (m_k + 1) * (int)pow((double)m_n, m_k);

    for (int i = 0; i < serverNum; i++)
        BCubeNode* node = new BCubeNode(this, NODE_TYPE_SERVER, true);

    for (int i = 0; i < switchNum; i++)
        BCubeNode* node = new BCubeNode(this, NODE_TYPE_SWITCH, false);
    return;
}

void BCube::BuildNetwork()
{
    for(int i = 0; i < GetServerNumber(); i++)
    {
        BCubeNode *node = (BCubeNode *)m_Nodes[i]; 
        
        for (int j = 0; j <= m_k; j++)
        {
            int switchid = node->GetNeighborSwitchID(j);
            BCubeNode* switchNode = (BCubeNode*)m_Nodes[switchid];

            NetLink* uplink = new NetLink(this);
            NetLink* downlink = new NetLink(this);
            uplink->m_revlink = downlink;
            downlink->m_revlink = uplink;

            node->AddLinks(downlink, uplink);
            switchNode->AddLinks(uplink, downlink);
        }
    }
}

void BCube::BuildSpanningTrees(int srcID)
{
    if(srcID >= GetServerNumber() || srcID < 0)
    {
        fprintf(stderr, "%s %d: srcID %d outofrange\n", srcID);
        exit(0);
    }

    for (int i = 0; i <= m_k; i++)
    {
        m_spanTreeIDs.push_back(i + 1);

        std::deque<int> deqServers;
        deqServers.push_back(srcID); 

        for (int j = i; j <= i + m_k; j++)
        {
            int level = j % (m_k + 1); 

            std::deque<int> deqTemp; 
            std::deque<int>::iterator itd; 

            for(itd = deqServers.begin(); itd!=deqServers.end(); itd++)
            {
                int id = (*itd);
                int switchID = ((BCubeNode*)m_Nodes[id])->GetNeighborSwitchID(level);

                this->m_Nodes[id]->m_OutLinks[level]->SetTreeID(i + 1); 

                BCubeNode *swnode = (BCubeNode*)m_Nodes[switchID]; 

                for (int n = 0; n < m_n; n++)
                {
                    int neighborid = swnode->GetNeighborServerID(n); 
                    if(neighborid!=id)
                    {

                        deqTemp.push_back(neighborid);
                        swnode->m_OutLinks[n]->SetTreeID(i + 1); 
                    }
                }
            }

            if(deqServers.size()==1)
                deqServers.pop_back();

            std::deque<int>::const_iterator itc; 
            for(itc=deqTemp.begin(); itc!=deqTemp.end(); itc++)
                deqServers.push_back(*itc); 
                        
            deqTemp.clear();
        }

        for(int id1 = 0; id1 < GetServerNumber(); id1++)
        {
            if(id1 == srcID)
                continue; 

            int shift = (int)pow((double)m_n, i);

            if (((id1 / shift) % m_n) != ((srcID / shift) % m_n))
                continue;

            int id2 = ((id1 / shift + m_n - 1) % m_n + id1 / shift / m_n * m_n) * shift + id1 % shift;

            BCubeNode* node1 = (BCubeNode*)m_Nodes[id1];
            BCubeNode* node2 = (BCubeNode*)m_Nodes[id2];

            node1->m_InLinks[i]->SetTreeID(i + 1); 
            node2->m_OutLinks[i]->SetTreeID(i + 1); 
        }
    }
}


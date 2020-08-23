#include "steiner-tree-fattree.h"
#include "assert.h"

FattreeNode::FattreeNode(Fattree* fattree, int type, bool cachingAbility) : NetNode(fattree, type, cachingAbility)
{}

FattreeNode::~FattreeNode()
{}

Fattree::Fattree(int n, int k) : Network("Fattree")
{
    m_n = n;
}

Fattree::~Fattree()
{
}

void Fattree::Init()
{
    int n = m_n;

    int srv_num = n * n * n / 4;
    int pot_switch_num = n * n;
    int top_switch_num = n * n / 4;

    for (int i = 0; i < srv_num + pot_switch_num + top_switch_num; i++)
    {
        if (i < srv_num)
            new FattreeNode(this, NODE_TYPE_SERVER, true);
        else
            new FattreeNode(this, NODE_TYPE_SWITCH, false);
    }
}

void Fattree::BuildNetwork()
{
    int n = m_n;

    int srv_num = n * n * n / 4;
    int pot_switch_num = n * n;
    int top_switch_num = n * n / 4;

    for (int i = 0; i < srv_num; i++)
    {
        int from = i;
        int to = srv_num + i / (n / 2);

        NetLink* uplink = new NetLink(this);
        NetLink* downlink = new NetLink(this);

        uplink->m_revlink = downlink;
        downlink->m_revlink = uplink;

        NetNode* downnode = m_Nodes[from];
        NetNode* upnode = m_Nodes[to];

        downnode->AddLinks(downlink, uplink);
        upnode->AddLinks(uplink, downlink);
    }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n / 2; j++)
            for (int k = 0; k < n / 2; k++)
            {
                int from = srv_num + i * n / 2 + j;
                int to = srv_num + i * n / 2 + k + n * n / 2;

                NetLink* uplink = new NetLink(this);
                NetLink* downlink = new NetLink(this);

                uplink->m_revlink = downlink;
                downlink->m_revlink = uplink;

                NetNode* downnode = m_Nodes[from];
                NetNode* upnode = m_Nodes[to];

                downnode->AddLinks(downlink, uplink);
                upnode->AddLinks(uplink, downlink);
            }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n / 2; j++)
            for (int k = 0; k < n / 2; k++)
            {
                int from = srv_num + pot_switch_num / 2 + i * n / 2 + j;
                int to = srv_num + pot_switch_num + j * n / 2 + k;

                NetLink* uplink = new NetLink(this);
                NetLink* downlink = new NetLink(this);

                uplink->m_revlink = downlink;
                downlink->m_revlink = uplink;

                NetNode* downnode = m_Nodes[from];
                NetNode* upnode = m_Nodes[to];

                downnode->AddLinks(downlink, uplink);
                upnode->AddLinks(uplink, downlink);
            }
}

void Fattree::BuildSpanningTrees(int srcID)
{
    ClearAllNodesReservations();
    ClearAllLinksReservations();
    if (BFSRepairSteinerTree(srcID, 1))
        m_spanTreeIDs.push_back(1);
}

#pragma once

#include "steiner-tree-net.h"

class BCube; 

struct BCubeNode: public NetNode
{
public:
    BCubeNode(BCube* bcube, int type, bool cachingAbility);
    ~BCubeNode();

    int GetNeighborSwitchID(int port);
    int GetNeighborServerID(int port);
};

class BCube : public Network
{
    friend struct BCubeNode;

public:
    BCube(int n, int k) : Network("BCube")
    {
        m_n = n;
        m_k = k;
    }

    ~BCube() {}

public:
    int     m_n;
    int     m_k; 

protected:

public:
    void    Init();
    void    BuildNetwork();
    void    BuildSpanningTrees(int srcID);
};

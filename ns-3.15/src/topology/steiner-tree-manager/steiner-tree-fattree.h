#pragma once

#include "steiner-tree-net.h"
#include <vector>

class Fattree;

struct FattreeNode: public NetNode
{
public:
    FattreeNode(Fattree* fattree, int type, bool cachingAbility);
    ~FattreeNode();
};

class Fattree : public Network
{
    friend struct FattreeNode;

    int m_n;

public:
    Fattree(int n, int k);
    ~Fattree();

    virtual void Init();
    virtual void BuildNetwork();
    void BuildSpanningTrees(int srcID);
};

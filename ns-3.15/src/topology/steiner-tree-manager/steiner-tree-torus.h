
#pragma once


#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <deque>

using namespace std; 

#include "steiner-tree-net.h"

class Torus;

struct TorusNode: public NetNode
{
    Torus* m_torus;
public:
    TorusNode(Torus* torus);
    ~TorusNode();

    vector<int>     m_vectorP;
    vector<int>     m_vectorQ;
    vector<int>     m_vectorR;

public:
    void Print();

    int ArrayToID(char *arrayID);
    void IDToArray(int id, char*arrayID); 

    int GetNeighborID(int dim, int direction);
    void CalculatePQR(TorusNode *srBCubeNode);

    void ColorLink(int dim, int direction, int color);

    void Clear()
    {
        NetNode::Clear();
        m_vectorP.clear();
        m_vectorQ.clear();
        m_vectorR.clear();
    }

    virtual CStringA getName() const;
};

class Torus : public Network
{
    friend TorusNode;

public:
    Torus(int n, int k) : Network("Torus")
    {
        sm_n = n;
        sm_k = k;
    }
    ~Torus(){}

public:
    int     sm_n; 
    int     sm_k; 

    void Init();

    void BuildNetwork();

    void BuildSpanningTrees(int srcID);
};

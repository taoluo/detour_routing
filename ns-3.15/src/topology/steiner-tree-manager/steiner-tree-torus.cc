#include "steiner-tree-torus.h"
#include <math.h>

TorusNode::~TorusNode()
{
}

CStringA TorusNode::getName() const
{
    CStringA name;
    name = "<";
    for(int i = 0; i < m_torus->sm_k - 1; i++)
    {
        CStringA tmp;
        tmp.Format("%d, ", m_arrayID[i]);
        name += tmp;
    }
    CStringA tmp;
    tmp.Format("%d>", m_arrayID[m_torus->sm_k - 1]);
    name += tmp;
    return name;
}


TorusNode::TorusNode(Torus* torus) : NetNode(torus, NODE_TYPE_SERVER, true)
{
    m_torus = torus;

    memset(this->m_arrayID, 0, MAXIDLENGTH);

    int tmp = m_nID; 
    for(int i = 0; i < m_torus->sm_k; i++)
    {
        m_arrayID[i] = tmp % m_torus->sm_n;
        tmp = tmp / m_torus->sm_n;
    }

    //for(int i = 0; i < m_torus->sm_k * 2; i++)
    //{
    //  m_OutLinks.push_back(new NetLink(m_torus));
    //  m_torus->m_links.push_back(m_OutLinks[i]);
    //}

    m_vectorP.reserve(m_torus->sm_k);
    m_vectorQ.reserve(m_torus->sm_k);
    m_vectorR.reserve(m_torus->sm_k);
}

void TorusNode::Print()
{
    printf("nodeid: %d, arrayID ", m_nID);
    for(int i=m_torus->sm_k-1; i>=0; i--)
        printf("%d", m_arrayID[i]);
    printf("\n");

    for(int dim=0; dim<m_torus->sm_k; dim++)
    {
        printf("\t neighbor -%d = %d, color=%d\n", dim+1, this->m_OutLinks[2*dim]->GetDestID(), this->m_OutLinks[2*dim]->GetTreeID());
        printf("\t neighbor +%d = %d, color=%d\n", dim+1, this->m_OutLinks[2*dim+1]->GetDestID(), this->m_OutLinks[2*dim+1]->GetTreeID());
    }

    vector<int>::const_iterator cit; 
    printf("\t Vector P: ");
    for (cit= this->m_vectorP.begin(); cit!=this->m_vectorP.end(); cit++)
        printf("%d ", *cit);
    printf("\n");

    printf("\t Vector Q: ");
    for (cit= this->m_vectorQ.begin(); cit!=this->m_vectorQ.end(); cit++)
        printf("%d ", *cit);
    printf("\n");

    printf("\t Vector R: ");
    for (cit= this->m_vectorR.begin(); cit!=this->m_vectorR.end(); cit++)
        printf("%d ", *cit);
    printf("\n");

    printf("\n");
}

int TorusNode::ArrayToID(char *arrayID)
{
    int res = 0; 
    int mul=1; 
    for (int i = 0; i < m_torus->sm_k; i++)
    {
        res += mul*arrayID[i];
        mul *= m_torus->sm_n; 
    }
    return res; 
}

void TorusNode::IDToArray(int id, char*arrayID)
{
    int tmp = id; 
    for(int i=0; i < m_torus->sm_k; i++)
    {
        arrayID[i]= tmp% m_torus->sm_n;
        tmp /= m_torus->sm_n;
    }
}

int TorusNode::GetNeighborID(int dim, int direction)
{
    int weight = 1;
    int neighborID = m_nID;

    int n = m_torus->sm_n;
    for(int i = 0; i < dim; i++)
        weight *= n;

    if(direction == 1)
    {
        if(m_arrayID[dim] < n - 1)
            return neighborID + weight;
        else
            return neighborID - weight * (n - 1);
    }
    else if(direction == -1)
    {
        if(m_arrayID[dim] > 0)
            return neighborID - weight;
        else
            return neighborID + weight * (n - 1);
    }
}

void TorusNode::CalculatePQR(TorusNode *srBCubeNode)
{
    if(m_nID == srBCubeNode->m_nID)
        return; 

    char tmpArray[MAXIDLENGTH]={0};
    for(int i = 0; i < m_torus->sm_k; i++)
    {
        tmpArray[2 * i] = -1 * (i + 1);
        tmpArray[2 * i + 1] = (i + 1);
    }

    int thresh = (int) floor( m_torus->sm_n / 2.0);
    
    for(int i = 0; i < m_torus->sm_k; i++)
    {
        int srcVal = srBCubeNode->m_arrayID[i];
        int myVal = this->m_arrayID[i];

        int d = (myVal - srcVal + m_torus->sm_n) % m_torus->sm_n; 
        if (d <= thresh && d > 0 )
        {
            this->m_vectorP.push_back(-1 * (i + 1));
            this->m_vectorQ.push_back(i + 1);
            tmpArray[2 * i] = 0;
            tmpArray[2 * i + 1] = 0;
        }

        int d2 = (srcVal - myVal + m_torus->sm_n) % m_torus->sm_n;
        if (d2 < thresh && d > 0)
        {
            this->m_vectorP.push_back(i + 1);
            this->m_vectorQ.push_back(-1 * (i + 1));
            tmpArray[2 * i + 1] = 0;
            tmpArray[2 * i] = 0;
        }
    }

    for(int i = 0; i < m_torus->sm_k; i++)
    {
        if(tmpArray[2 * i] != 0)
            m_vectorR.push_back(tmpArray[2 * i] ); 

        if(tmpArray[2 * i + 1] != 0)
            m_vectorR.push_back(tmpArray[2 * i + 1]);
    }
}

void TorusNode::ColorLink(int dim, int direction, int color)
{
    int treeID = 0;
    if (color > 0)
        treeID = 2 * color - 1;
    else
        treeID = 2 * -color;

    if(direction==1)
        this->m_OutLinks[2*dim+1]->SetTreeID(treeID);
    else if(direction ==-1)
        this->m_OutLinks[2*dim]->SetTreeID(treeID);
    else
    {
        fprintf(stderr, "ColorLink Error!\n");
        exit(0);
    }
    return;
}

void Torus::Init()
{
    int num=1;
    for(int i=0; i< sm_k; i++)
        num *= sm_n;

    for(int i = 0; i < num; i++)
        new TorusNode(this);

    m_spanTreeIDs.reserve(sm_k * 2);
}

void Torus::BuildNetwork()
{
    for(int j = 0; j < sm_k; j++)
    {
        std::vector<NetLink*> decLinks;
        std::vector<NetLink*> incLinks;

        for(int i = 0; i < m_Nodes.size(); i++)
        {
            NetLink* declink = new NetLink(this);
            NetLink* inclink = new NetLink(this);
            declink->m_revlink = inclink;
            inclink->m_revlink = declink;

            decLinks.push_back(declink);
            incLinks.push_back(inclink);
        }

        int srvid = 0;
        int base = (int)pow((double)sm_n, j);
        for(int i = 0; i < m_Nodes.size(); i++)
        {
            int linkid = i / sm_n * sm_n + (i + sm_n - 1) % sm_n;
            m_Nodes[srvid]->AddLinks(incLinks[linkid], decLinks[linkid]);

            int part1 = srvid / (base * sm_n) * base + srvid % base;
            int part2 = srvid / base % sm_n;

            part2++;
            if (part2 == sm_n)
            {
                part1++;
                part2 = 0;
            }

            srvid = part1 / base * (base * sm_n) + part2 * base + part1 % base;
        }

        srvid = 0;
        for(int i = 0; i < m_Nodes.size(); i++)
        {
            int linkid = i;
            m_Nodes[srvid]->AddLinks(decLinks[linkid], incLinks[linkid]);

            int part1 = srvid / (base * sm_n) * base + srvid % base;
            int part2 = srvid / base % sm_n;

            part2++;
            if (part2 == sm_n)
            {
                part1++;
                part2 = 0;
            }

            srvid = part1 / base * (base * sm_n) + part2 * base + part1 % base;
        }
    }
}

void Torus::BuildSpanningTrees(int srcID)
{
    for(int i = 0; i < sm_k; i++)
    {
        m_spanTreeIDs.push_back(2 * i + 1);
        m_spanTreeIDs.push_back(2 * i + 2);
    }

    for(int i = 0; i < m_Nodes.size(); i++)
    {
        TorusNode *node = (TorusNode*)m_Nodes[i];
        node->CalculatePQR( (TorusNode*)m_Nodes[srcID]);
    }

    for(int i = 0; i < m_Nodes.size(); i++)
    {
        if(i == srcID)
            continue; 

        TorusNode *node = (TorusNode*)m_Nodes[i];

        int pLen = node->m_vectorP.size();
        for(int j = 0; j < pLen; j++)
        {
            int color = node->m_vectorP[j]; 
            int move = node->m_vectorP[(j + 1) % pLen];

            int dim = abs(move);
            int direction;
            if(move > 0)
                direction =1;
            else if(move < 0)
                direction=-1;
            else
            {
                fprintf(stderr, "cannot reach, move=0!\n");
                exit(0);
            }

            int neighborID = node->GetNeighborID(dim - 1, direction);
            ((TorusNode*)m_Nodes[neighborID])->ColorLink(dim - 1, -1 * direction, color);
        }

        pLen = node->m_vectorQ.size();
        for(int j = 0; j < pLen; j++)
        {
            int move = node->m_vectorQ[j];

            int dim = abs(move);
            int direction;
            if(move > 0)
                direction = 1;
            else if (move < 0)
                direction = -1;
            else
            {
                fprintf(stderr, "cannot reach, move=0!\n");
                exit(0);
            }
            
            int neighborID = node->GetNeighborID(dim - 1, direction);
            ((TorusNode*)m_Nodes[neighborID])->ColorLink(dim - 1, -1 * direction, move);
        }

        pLen = node->m_vectorR.size();
        for(int j = 0; j < pLen; j++)
        {
            int move = node->m_vectorR[j];

            int dim = abs(move);
            int direction;
            if(move > 0)
                direction = 1;
            else if (move < 0)
                direction = -1;
            else
            {
                fprintf(stderr, "cannot reach, move=0!\n");
                exit(0);
            }
            
            int neighborID = node->GetNeighborID(dim - 1, direction);
            ((TorusNode*)m_Nodes[neighborID])->ColorLink(dim - 1, -1 * direction, move);
        }
    }
}

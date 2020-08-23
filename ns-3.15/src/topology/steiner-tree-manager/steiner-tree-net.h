#pragma once

#include <set>
#include <map>
#include <vector>
#include <list>
#include <stdio.h>
#include <time.h>

#include <string>

#define     MAXIDLENGTH     10

#define     NODE_TYPE_SERVER        0
#define     NODE_TYPE_SWITCH        1

class Network;

struct NetLink
{
protected:
    Network* m_Network;

public:
    int m_id;

    NetLink(Network* network);

    void Init()
    {
        m_nSrcID = -1; 
        m_nDstID = -1; 
        m_nSrcPort = -1;
        m_nDstPort = -1;

        m_bIsBroken = 0;
        m_nTreeID = 0; 

        m_revlink = NULL;
        
        m_nReserved[0] = 0;
        m_nReserved[1] = 0;
        m_nReserved[2] = 0;
        m_nReserved[3] = 0;
    }

    void Clear()
    {
        m_bIsBroken = 0;
        m_nTreeID = 0;

        m_nReserved[1] = 0;
        m_nReserved[2] = 0;
        m_nReserved[3] = 0;
    }

    virtual ~NetLink() {}

    void SetSrcID(int srcID, int port);
    void SetDestID(int destID, int port);

    int GetSrcID() const { return m_nSrcID; }
    int GetDestID() const { return m_nDstID; }

    int GetSrcPort() const { return m_nSrcPort; }
    int GetDstPort() const { return m_nDstPort; }

    const char* getLinkColor() const 
    {
        const char* colors[] = {"grey", "green", "gold4", "blue", "brown"};
        const char* colors2[] = {"grey", "blue", "hotpink", "blue", "brown"};

        if(m_nTreeID >= 0)
            return colors[m_nTreeID];
        else
            return colors2[-m_nTreeID];
    }

public:
    int     m_bIsBroken;
    int     m_nReserved[4];
    NetLink* m_revlink;

    void SetTreeID(int treeID)
    {
        m_nTreeID = treeID; 
    }

    int GetTreeID() const { return m_nTreeID; }

private: 
    int     m_nSrcID; 
    int     m_nDstID;

    int     m_nSrcPort;
    int     m_nDstPort;

    int     m_nTreeID;
};

struct NetNode
{
protected:
    Network* m_network;
public:
    NetNode(Network* network, int type, bool cachingAbility);

    virtual void Clear()
    {
        m_nReserved[0] = 0;
        m_nReserved[1] = 0;
        m_nReserved[2] = 0;
        m_nReserved[3] = 0;
        m_bIsDestNode = false;
    }

    virtual ~NetNode() {}

    virtual void Print() {}

public:
    /**
     * sm_nType=0, server
     * sm_nType=1, switch
     */
    int     m_nType; 
    
    int     m_nID;
    char    m_arrayID[MAXIDLENGTH]; 

    typedef std::vector<NetLink*> Links;
    Links   m_OutLinks;
    Links   m_InLinks;

    void    AddLinks(NetLink* inlink, NetLink* outlink);  // please use this function to contruct the network.

    typedef std::map<int, int> FIB;
    FIB     m_fib[16];

    bool    m_bIsDestNode;
    bool    m_bCachingAbility;

    // This variable is only used in the link failure repairing algorithm
    int     m_nReserved[4];

    virtual std::string getName() const
    {
        char namebuf[1024];
        
        std::string name;
        if(m_nType == NODE_TYPE_SERVER)
        {
            sprintf(namebuf, "server %d", m_nID);
            name = namebuf;
        }
        else if(m_nType == NODE_TYPE_SWITCH)
        {
            sprintf(namebuf, "switch %d", m_nID);
            name = namebuf;
        }
        return name;
    }
};

class Network
{
    friend struct NetLink;
    friend struct NetNode;

    std::string m_name;

    int     m_nServerNumber;
    int     m_nSwitchNumber; 

protected:

    typedef std::set<int> DestIDs;
    DestIDs m_destIDs;

    typedef std::vector<NetLink*> Links;
    Links m_links;

    typedef std::vector<NetNode*> Nodes;
    Nodes m_Nodes;

    typedef std::vector<int> SpanningTreeIDs;
    SpanningTreeIDs m_spanTreeIDs;

    /************************************************************************************************************/
    /* Tree construction algorithm functions                                                                    */
    /************************************************************************************************************/

    // return true if the tree whose root is srcID contains dest nodes 
    bool ConstructSteinerTree(int srcID, int treeID);

    /************************************************************************************************************/
    /* Tree repair algorithm functions                                                                          */
    /************************************************************************************************************/
    bool BFSRepairSteinerTree(int srcID, int treeID);


    int GetNumberOfTargetNodesInSubtree(int srcID, int treeID);
    int GetTreeDepth(int srcID, int treeID);
    int GetSteinerTreeNodes(int rootID, int srcID, int treeID, std::set<int>& tree_nodes);
    int GetBranchNodeNumber(int rootID, int srcID, int treeID);
    
    int GetNetworkNodeNumber(int srcID, bool bClear = true);

    void ClearAllNodesReservations();
    void ClearAllLinksReservations();
    void FreeSpanningTreeLinks(int treeID);

    double ImproveQualityOfTree(int srcID, int treeID, bool parentLink, int inport, int& upport, int& no_flows);

    std::set<int> broken_tree_ids;
    std::map<int, std::vector<NetLink*> > m_tree_links;
public:
    Network(const char* name) : m_name(name), m_nServerNumber(0), m_nSwitchNumber(0) {}
    virtual ~Network() 
    {
        for each(NetNode* node in m_Nodes)
            delete node;
        for each(NetLink* link in m_links)
            delete link;
    }

    void Clear()
    {
        for each(NetNode* node in m_Nodes)
            node->Clear();
        for each(NetLink* link in m_links)
            link->Clear();

        m_destIDs.clear();
        m_spanTreeIDs.clear();
    }

    virtual void Init() = 0;
    virtual void BuildNetwork() = 0;
    virtual void BuildSpanningTrees(int srcID) = 0;

    /************************************************************************************************************/
    /* Generating destination, failure links                                                                    */
    /************************************************************************************************************/

    int GetDestNodeNum() const;

    void AddDestID(int destID);
    void AddFailureLink(int id);

    /************************************************************************************************************/
    /* Visualization                                                                                            */
    /************************************************************************************************************/

    // Generate dot script
    void    VisualizeGraph(int srcID, int treeID, const char* append);

    /************************************************************************************************************/
    /* Operations                                                                                               */
    /************************************************************************************************************/

    // Construct steiner tree from spanning tree
    int ConstructSteinerTree(int srcID, bool multiple);

    // Repair steiner trees
    void PrepareToRepairSteinerTrees();
    bool RepairSteinerTrees(int srcID);

    double ImproveQualityOfTrees(int srcID);

    /************************************************************************************************************/
    /* Basic Metrics                                                                                            */
    /************************************************************************************************************/

    std::string getName() const { return m_name; }

    int GetServerNumber() const { return m_nServerNumber; }
    int GetSwitchNumber() const { return m_nSwitchNumber; }

    int GetNodeNumber() const { return m_Nodes.size(); }
    int GetLinkNumber() const { return m_links.size(); }
        
    // Get the depth of the steiner tree
    double GetAvgTreeDepth(int srcID);
    int GetMaxTreeDepth(int srcID);

    // Get the node number of steiner tree
    int GetTotalSteinerTreeNodeNumber(int srcID);
    double GetAverageSteinerNodeNumber(int srcID);

    // Get the branch node number of steiner tree
    int GetTotalBranchNodeNumber(int srcID);
    double GetAverageBranchNodeNumber(int srcID);

    int GetNumberOfCompleteSteinerTree() const;
    int GetUpperBoundOfSteinerTreeNumber(int srcID) const;

    int GetNumberOfLinksInTree(int treeID) const;

    void PrintFIB(int treeID) const;

    bool VerifySteinerTrees(int srcID);
    bool IsNetworkConnected(int srcID);

    int GetSteinerTreeNumber() const { return m_spanTreeIDs.size(); }

    NetNode* GetNode(int id) const { return m_Nodes[id]; }
    NetLink* GetLink(int id) const { return m_links[id]; }
};

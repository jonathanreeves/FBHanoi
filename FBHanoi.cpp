#include <iostream>
#include <vector>
#include <list>

#include <cstring> // for memset
#include <cstdlib> // for atoi
#include <cstdio>  // for getchar

#define MAX_NUMBER_BUF 128

//==============================================================================
// 
// Class declaration for the directed/undirected "graph"
//
//==============================================================================
class Graph
{
public:
    typedef enum
    {
        kVertexColor_White,
        kVertexColor_Grey,
        kVertexColor_Black,
    } VertexColor;

    typedef struct Edge
    {
        struct Edge *next;
        int vertexIndex;
    } Edge;

    typedef struct Vertex
    {
        VertexColor color;
        int index;
        int distance;
        Edge *edgeList;
        std::vector<int> state;
        struct Vertex *predecessor;
        int lastMove[2];
    } Vertex;

    Graph(int numDisks, int numPegs) : numDisks(numDisks), numPegs(numPegs), numVertices(0) { }

    ~Graph() { Cleanup(); }

    Vertex *GetVertex(std::vector<int>& state);
    int BuildAndExplore(std::vector<int>& startState, std::vector<int>& endState);
    void Cleanup(void);

    int numDisks;
    int numPegs;
    int numVertices;
    std::list<Vertex> vtxList;
};

//==============================================================================
//
// Graph Cleanup
//
// destroy any objects created/allocated during graph building
//
//==============================================================================
void Graph::Cleanup(void)
{
    std::list<Vertex>::iterator iter;
    for (iter = vtxList.begin(); iter != vtxList.end(); iter++)
    {
        Edge *cur = (*iter).edgeList;
        while (cur)
        {
            Edge *next = cur->next;
            delete cur;
            cur = next;
        }
    }
    vtxList.clear();
    numVertices = 0;
}

//==============================================================================
//
// Get Vertex
//
// look up a vertex object given its "state"
//
//==============================================================================
Graph::Vertex *Graph::GetVertex(std::vector<int>& state)
{
    // TODO: ideally this should be implemented with hashing to be more
    // efficient, but this is just to get it done:
    std::list<Vertex>::iterator iter;
    for (iter = vtxList.begin(); iter != vtxList.end(); iter++)
    {
        Vertex *vtx = &(*iter);
        if (std::equal(vtx->state.begin(), vtx->state.end(), state.begin()))
        {
            return vtx;
        }
    }

    // if we made it here, this vertex doesn't exist. Create it.
    Vertex newVtx;
    memset(&newVtx, 0, sizeof(Vertex));

    newVtx.color = kVertexColor_White;
    newVtx.state = state;
    newVtx.index = numVertices++;
    vtxList.push_back(newVtx);

    return &vtxList.back();
}

//==============================================================================
//
// Peg Has Smaller Disk
//
// Utility function for looking seeing if a particular peg has a smaller disk
// on it, given a "state" array. This is used while trying to figure out what
// valid neighbor states exist for a given state.
//
//==============================================================================
static bool PegHasSmallerDisk(std::vector<int>& state, int diskRad, int peg)
{
    for (int i = diskRad-1; i >= 0; i--)
    {
        if (state[i] == peg)
        {
            return true;
        }
    }
    return false;
}

//==============================================================================
//
// Disk Not Smallest On Peg
//
// Utility function for looking seeing if a particular disk is or is not the
// smallest on a peg, given a "state" array. This is used while trying to
// figure out what valid neighbor states exist for a given state.
//
//==============================================================================
static bool DiskNotSmallestOnPeg(std::vector<int>& state, int diskRad)
{
    for (int i = diskRad-1; i >= 0; i--)
    {
        if (state[i] == state[diskRad])
        {
            return true;
        }
    }
    return false;
}

//==============================================================================
//
// Build And Explore
//
// This is a standard "Breadth First Search" algorithm for an undirected graph,
// but where the neighbor vertices and adjacent edges are actually calculated
// on the fly using a couple of utility functions to determine what valid
// neighbor states exist.
//
//==============================================================================
int Graph::BuildAndExplore(std::vector<int>& startState, std::vector<int>& endState)
{
    // make this the first vertex:
    Cleanup();

    // for each disk d in the state
    //    if the disk is not the smallest on its peg, continue
    //    for each peg p not equal to the peg the d is on
    //        if (p not occupied by smaller disk)
    //            move disk d to peg p (creating new state and vertex)
    //            make an edge to the new state
    Vertex *startVtx = GetVertex(startState);
    startVtx->color = kVertexColor_Grey;

    // populate the list with the first node
    std::list<Vertex *> bfsList;
    bfsList.push_back(startVtx);

    while (!bfsList.empty())
    {
        Vertex *curVtx = bfsList.front();
        bfsList.pop_front();

        // calculate all neighbors for this vertex
        std::vector<int> curState = curVtx->state;
        for (int diskRad = 0; diskRad < curState.size(); diskRad++)
        {
            // in order for this disk to be moveable, it must be the smallest
            // on its peg
            if (DiskNotSmallestOnPeg(curState, diskRad))
            {
                continue;
            }
            for (int peg = 0; peg < numPegs; peg++)
            {
                if (peg == curState[diskRad] ||
                        PegHasSmallerDisk(curState, diskRad, peg))
                {
                    continue;
                }
            
                std::vector<int> newState = curState;
                newState[diskRad] = peg;

                Vertex *newVtx = GetVertex(newState);
                if (newVtx)
                {
                    // add edges pointing between them
                    Edge *edge1 = new Edge;
                    edge1->vertexIndex = newVtx->index;
                    edge1->next = curVtx->edgeList;
                    curVtx->edgeList = edge1;

                    Edge *edge2 = new Edge;
                    edge2->vertexIndex = curVtx->index;
                    edge2->next = newVtx->edgeList;
                    newVtx->edgeList = edge2;

                    if (newVtx->color == kVertexColor_White)
                    {
                        newVtx->predecessor = curVtx;
                        newVtx->distance = curVtx->distance+1;
                        newVtx->color = kVertexColor_Grey;
                        newVtx->lastMove[0] = curState[diskRad]+1;
                        newVtx->lastMove[1] = peg+1;
                        bfsList.push_back(newVtx);
                    }
                }
            }
        }

        curVtx->color = kVertexColor_Black;
        if (std::equal(curVtx->state.begin(), curVtx->state.end(), endState.begin()))
        {
            return curVtx->distance;
        }
    }
}


//==============================================================================
//
// Get Next Integer
//
// Read a whitespace delimited string from STDIN and convert it to an integer.
//
//==============================================================================
int GetNextInt(void)
{
    char buf[MAX_NUMBER_BUF];
    int bufIndex = 0;

    do
    {
        buf[bufIndex++] = getchar();
    } while (buf[bufIndex-1] != ' ' && buf[bufIndex-1] != '\n');
    buf[bufIndex-1] = 0;

    return atoi(buf);
}

//==============================================================================
//
// Print State
//
// Print a formatted configuration of the pegs
//
//==============================================================================
void PrintState(std::vector<int>& state)
{
    std::cout << "state = " << std::endl << "    ";
    for (int i = 0; i < state.size()-1; i++)
    {
        std::cout << state[i]+1 << ' ';
    }
    std::cout << state[state.size()-1]+1 << std::endl;
}


//==============================================================================
//
// Print Move
//
// print a formatted "move" as it exists in the vertex data
//
//==============================================================================
void PrintMove(Graph::Vertex *vtx)
{
    std::cout << vtx->lastMove[0] << " " << vtx->lastMove[1] << std::endl;
}

//==============================================================================
//
// main
//
//==============================================================================
int main(int argc, char **argv)
{
    int numDisks = GetNextInt();
    int numPegs = GetNextInt();
    std::vector<int> startState;
    std::vector<int> endState;

    for (int i = 0; i < numDisks; i++)
    {
        startState.push_back(GetNextInt()-1);
    }

    for (int i = 0; i < numDisks; i++)
    {
        endState.push_back(GetNextInt()-1);
    }

    Graph *graph = new Graph(numDisks, numPegs);

    int numMoves = graph->BuildAndExplore(startState, endState);
    std::cout << "num moves = " << numMoves << std::endl;

    Graph::Vertex *vtx = graph->GetVertex(endState);
    std::list<Graph::Vertex *> forwardList;
    for (int i = 0; i < numMoves; i++)
    {
        forwardList.push_front(vtx);
        vtx = vtx->predecessor;
    }

    std::cout << numMoves << std::endl;
    std::list<Graph::Vertex *>::iterator iter;
    for (iter = forwardList.begin(); iter != forwardList.end(); iter++)
    {
        PrintMove(*iter); 
    }

    delete graph;

    return 0;
}

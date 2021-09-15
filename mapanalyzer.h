#ifndef MAPANALYZER_H
#define MAPANALYZER_H

#include <string>
#include <vector>
#include <map>



class MapEdge
{
    private:
        int destination;
        double cost;

    public:
    // constructor
    MapEdge();

    // init with destination and cost
    MapEdge(int dest, double cost);

    int getDest();

    double getCost();
    
    void setCost(double n);
    
    std::string toString();
};

class MapNode
{
// instance variables
    private:
        int nodeNum;
        std::vector<MapEdge*>* edges;

    /**
     * Constructor for objects of class MapNode
     */
    public:
    MapNode();
    
    MapNode(int n);

    // check if an edge with given destination exist
    bool edgeExist(int k);
    
    // add an edge given destination and cost
    void addEdge(int k, double w);
    
    // return the edge with the given destination
    MapEdge* getEdge(int k);
    
    // return distance with this node
    double getDistWith(int k);
    
    // return the actual node object
    
    std::vector<MapEdge*>* getEdges();
    
    // return the current node number
    int getNodeNum();
    
    // toString method
    std::string toString();
};


// template <typename T>
class MapAnalyzer {

    public:
        MapAnalyzer();
        MapAnalyzer(std::string filename, int start, int end, double limit);
        void addNode(int n);
        bool nodeExist(int n); 
        bool addEdge(int k1, int k2, double w);
        MapNode* getNode(int n);
        void setStartEnd(int start, int end);
        int getStart();
        int getEnd();
        int indexOf(std::vector<int> list, int target);
        double distBtw(int node1, int node2, double limit);
        std::vector<int> getNodesWithinMain(double limit, int curr);
        void setVisited(int n);
        std::vector<int> removeDuplicate(std::vector<int> vec);
        bool hasVisited(int n);
        bool contains(std::vector<int> v, int n);
        void clearTempData();
        std::vector<int> getNodesWithin(double limit, int curr);
        std::vector<int> getNeighbors(int curr);
        int getMinLimit(std::vector<int> nodes, std::vector<double> distance);
        void DijkstraExit(double limit, int curr);
        std::vector<int> showPathFrom(int start, int end);
        void DjikstraPathfinding(double limit, int curr, int end);
        double getTotalDistance(double limit);
        void printVector(std::vector<int> v);
        void printVector(std::vector<bool> v);
        void printVector(std::vector<double> v);
        void printNodes(std::vector<MapNode*> v);
        int getDjiPathLength();
        double getTotalDistance();
        bool inMap(int n);

    private:
        std::vector<MapNode*>* nodes;
        std::vector<int> nodesNearby;
        std::vector<double> distance;
        std::vector<int> prev;
        std::vector<int> djiPath;
        std::vector<bool> visited;
        std::map<int, bool> visitMap;

        double limit; // 99 * 10000
        
        int start;
        int end;
        int cameFrom;
        bool finish;

        
        
    
};

#endif
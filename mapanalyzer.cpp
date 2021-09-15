#include <iostream>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <set>

#include "mapanalyzer.h"

using namespace std;




MapEdge::MapEdge()
{
    // initialise instance variables
}

// init with destination and cost
MapEdge::MapEdge(int dest, double cost) {
    this->destination = dest;
    this->cost = cost;
}

int MapEdge::getDest() {
    return destination;
}

double MapEdge::getCost() {
    return cost;
}

void MapEdge::setCost(double n) {
    cost = n;
}

string MapEdge::toString() {
    return "\ndestination: " + to_string(destination) + "\ncost: " + to_string(cost);
}




MapNode::MapNode()
{
    // initialise instance variables
    this->edges = new vector<MapEdge*>();
    this->nodeNum = -1;
}

MapNode::MapNode(int n)
{
    // initialise instance variables
    this->edges = new vector<MapEdge*>();
    this->nodeNum = n;
}

// check if an edge with given destination exist
bool MapNode::edgeExist(int k) {
    // empty list
    if (edges->size() == 0) {
        return false;
    }
    // check each edge
    for (int i = 0; i < edges->size(); i++) {
        if (edges->at(i)->getDest() == k) {
            return true;
        }
    }
    return false;
}

// add an edge given destination and cost
void MapNode::addEdge(int k, double w) {
    if (edgeExist(k)) {
        getEdge(k)->setCost(w);
    } else {
        edges->push_back(new MapEdge(k, w));
    }
}

// return the edge with the given destination
MapEdge* MapNode::getEdge(int k) {
    // empty list
    if (edges->size() == 0) {
        return NULL;
    }
    // check each edge
    for (int i = 0; i < edges->size(); i++) {
        if (edges->at(i)->getDest() == k) {
            return edges->at(i);
        }
    }
    return NULL;
}

// return distance with this node
double MapNode::getDistWith(int k) {
    if (!edgeExist(k)) {
        return -1;
    }
    return getEdge(k)->getCost();
}

// return the actual node object

vector<MapEdge*>* MapNode::getEdges() {
    return edges;
}

// return the current node number
int MapNode::getNodeNum() {
    // if (nodeNum == NULL) {
    //     return -1;
    // }
    return nodeNum;
}

// toString method
string MapNode::toString() {
    string out = string("\n---------------------------\n") + "Node# " + to_string(nodeNum) + "\nlist of edges:";
    // out += "\n---------------------------\n" + "Node# " + to_string(nodeNum) + "\nlist of edges:";
    for (int i = 0; i < edges->size(); i++) {
        out += edges->at(i)->toString();
    }
    return out;
}

MapAnalyzer::MapAnalyzer(){}

MapAnalyzer::MapAnalyzer(string filename, int start, int end, double limit) {

    this->nodes = new vector<MapNode*>();

    this->limit = limit;

    finish = false;
    // ofstream mapReader;
    // mapReader.open ("map.txt");

    // // read all the items


    // mapReader.close();

    string line;
    ifstream myfile (filename);
    vector<string> node_dist; // temp vector for values read from map.txt
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            istringstream iss(line);
            copy(istream_iterator<string>(iss),
                istream_iterator<string>(),
                back_inserter(node_dist));
            // cout << line << '\n';

        }
        myfile.close();
    }

    else cout << "Unable to open file"; 

    // process the filled vector
    // for (int i = 0; i < node_dist.size(); i++) {
    //     cout << node_dist[i] << '\n';
    // }

    // build the map according to map.txt
    while (node_dist.size() > 0) {

        // read in the values
        double dist = stod(node_dist.back());
        
        node_dist.pop_back();
        int node2 = stoi(node_dist.back());
        node_dist.pop_back();
        int node1 = stoi(node_dist.back());
        node_dist.pop_back();
     
        addNode(node1);
        addNode(node2);
        // printNodes(*nodes);
        addEdge(node1, node2, dist);
        // cout << "distance is " << distBtw(node1, node2, 10000) << endl;
    }

    // set start and end location
    setStartEnd(start, end);

}

// add node
void MapAnalyzer::addNode(int n) {
    if (nodeExist(n)) {
        return;
    } else {
        MapNode* add = new MapNode(n);
        nodes->push_back(add);
    }
}

// check existance
bool MapAnalyzer::nodeExist(int n) {
    for (int i = 0; i < nodes->size(); i++) {
        if (nodes->at(i)->getNodeNum() == n) {
            return true;
        }
    }
    return false;
}

// add an edge with three parameters
bool MapAnalyzer::addEdge(int k1, int k2, double w) {
    if (getNode(k1) == NULL || getNode(k2) == NULL) { // node does not exist
        return false;
    } else if (getNode(k1)->getEdge(k2) != NULL) { // repeated edge
        return false;
    } else if (w == 0) { // edge length is 0
        return false;
    } else {
        getNode(k1)->addEdge(k2, w);
        getNode(k2)->addEdge(k1, w);
        return true;
    }
}

// get node that has n as node number
MapNode* MapAnalyzer::getNode(int n) {
    for (int i = 0; i < nodes->size(); i++) {
        if (nodes->at(i)->getNodeNum() == n) {
            return nodes->at(i);
        }
    }
    return NULL;
}

// set start (patient) and end (this hospital) node
void MapAnalyzer::setStartEnd(int start, int end) {
    this->start = start;
    this->end = end;
}

// return index of a vector element
int MapAnalyzer::indexOf(vector<int> list, int target) {
    auto it = find(list.begin(), list.end(), target);
 
    // If element was found
    if (it != list.end())
    {
     
        // calculating the index
        // of target
        int index = it - list.begin();
        // cout << index << endl;
        return index;
    }
    else {
        // If the element is not
        // present in the vector
        // cout << "-1" << endl;
        return -1;
    }


}

// get distance in between 2 nodes and show unconnectivity
// using limit + 1(can't reach)
double MapAnalyzer::distBtw(int node1, int node2, double limit) {
    if (getNode(node1)->getEdge(node2) == NULL) {
        return limit + 1;
    } else {
        return getNode(node1)->getEdge(node2)->getCost();
    }
}

// return nodes within a certain range recursively,
// important in Dji Algorithm
vector<int> MapAnalyzer::getNodesWithinMain(double limit, int curr) {
    if (getNode(curr) == NULL) {
        return {};
    }
    vector<int> nodeList;
    // set visit
    setVisited(curr);
    // add all nodes that haven't been visited and is smaller than the limit
    for (int i = 0; i < getNode(curr)->getEdges()->size(); i++) {
        if (getNode(curr)->getEdges()->at(i)->getCost() <= limit
        && !hasVisited(getNode(curr)->getEdges()->at(i)->getDest())
        ) {
            nodeList.push_back(getNode(curr)->getEdges()->at(i)->getDest());
        }
    }
    // if there are more nodes around they will also be processed recursively
    if (nodeList.size() > 0) {
        vector<int> addList;
        for (int i = 0; i < nodeList.size(); i++) {
            if (limit - getNode(curr)->getDistWith(nodeList.at(i)) >= 0) {
                vector<int> temp = getNodesWithinMain(limit - 
                getNode(curr)->getDistWith(nodeList.at(i)), nodeList.at(i));
                addList.insert(addList.end(), temp.begin(), temp.end());
            }
        }
        nodeList.insert(nodeList.end(), addList.begin(), addList.end()); // replace addAll method
    }
    // remove the node itself and any duplicate nodes
    vector<int> output = removeDuplicate(nodeList);
    output.erase(remove(output.begin(), output.end(), curr), output.end()); // replace remove method
    // output.remove((Integer)curr);
    return output;
}

//setter
void MapAnalyzer::setVisited(int n) {
    visitMap[n] = true;
}

// getters
int MapAnalyzer::getStart() {
    return start;
}

int MapAnalyzer::getEnd() {
    return end;
}

vector<int> MapAnalyzer::removeDuplicate(vector<int> vec) {
    set<int> s( vec.begin(), vec.end() );
    vec.assign( s.begin(), s.end() );
    return vec;
}

// don't use in getMinLimit!! not the same visited vector
bool MapAnalyzer::hasVisited(int n) {
    map<int, bool>::iterator it = visitMap.find(n);
    return it != visitMap.end();
}

bool MapAnalyzer::contains(vector<int> v, int n) {
    // vector<int>::iterator it = array.find(n);
    return find(v.begin(), v.end(), n) != v.end();
}

// clear visited map
void MapAnalyzer::clearTempData() {
    visitMap.clear(); 
}

// the last method plus clean up the visited info
vector<int> MapAnalyzer::getNodesWithin(double limit, int curr) {
    vector<int> output;
    output = getNodesWithinMain(limit, curr);
    clearTempData();
    // cout << "nodes within " << limit << " of " << curr << " is: " << endl;
    // printVector(output);
    return output;
}

// get all neighbors within a certain range
vector<int> MapAnalyzer::getNeighbors(int curr) {
    if (getNode(curr) == NULL) {
        cout << "No such node: " << curr << endl;
        return {};
    }
    vector<int> out;
    for (int i = 0; i < getNode(curr)->getEdges()->size(); i++) {
        out.push_back(getNode(curr)->getEdges()->at(i)->getDest());
    }
    return out;
}

// get the minimum length among all the distances within range
int MapAnalyzer::getMinLimit(std::vector<int> nodes, std::vector<double> distance) {

    // int j = 0;
    // while (visited.at(j)) {
    //     j++;
    // }

    // rewritten in 2021, now lighter and better!
    // initialize the comparison to the first item that has not been visited
    double shortest = limit + 1;
    int shortestIndex = -1;
    for (int i = 0; i < distance.size(); i++) {
        if (distance.at(i) < shortest && !visited.at(i)) {
            shortest = distance.at(i);
            shortestIndex = i;
        }
    }
    return nodes.at(shortestIndex);
}

// Dijkstra's algorithm to exit, we will have a complete list of distance
// to all the vertices within limit from current vertex
void MapAnalyzer::DijkstraExit(double limit, int curr) {
    nodesNearby.push_back(curr);
    vector<int> temp = getNodesWithin(limit, curr);
    nodesNearby.insert(nodesNearby.end(), temp.begin(), temp.end());

    // cout << "nodesNearby is: " << endl;
    // printVector(nodesNearby);
    
    // initialize all nodes
    // the first node is current node (distance = 0)
    distance.push_back(0);
    prev.push_back(-1);
    visited.push_back(false);
    for (int i = 0; i < nodesNearby.size() - 1; i++) {
        distance.push_back(limit + 1);
        prev.push_back(-1);
        visited.push_back(false);
    }
    
    // mark and process the nearest v
    for (int i = 0; i < nodesNearby.size(); i++) {
        int v = getMinLimit(nodesNearby, distance);
        visited.at(indexOf(nodesNearby, v)) = true;
        if (distance.at(indexOf(nodesNearby, v)) == limit + 1) {
            cout << "Can't reach" << endl;
            return;
        } else {
            vector<int> nList = getNeighbors(v);
            // cout << "neighbors of " << v << " are: " << endl;
            // printVector(nList);
            
            // analyze all neighbors
            for (int j = 0; j < nList.size(); j++) {
                int w = nList.at(j);

                // if not contains means node not within limit
                if (contains(nodesNearby, w)) {
                    if (distance.at(indexOf(nodesNearby, w)) > 
                    distance.at(indexOf(nodesNearby, v)) + distBtw(v, w, limit)) {
                        
                        distance.at(indexOf(nodesNearby, w)) =  
                        distance.at(indexOf(nodesNearby, v)) + distBtw(v, w, limit);
                        prev.at(indexOf(nodesNearby, w)) = v;

                    }
                }
                
            }
        }
    }
    //clearTempData();
}

// this path shows the list of nodes going from start to end (start not
// included in the list)
vector<int> MapAnalyzer::showPathFrom(int start, int end) {
    vector<int> result;
    int i = indexOf(nodesNearby, end);
    int cnt = 0; 
    // form the shortest path
    result.push_back(end);

    // form the path using prev node info
    // cout << "prev vector is: " << endl;
    // printVector(prev);
    // cout << "nodesNearby vector is: " << endl;
    // printVector(nodesNearby);
    // cout << "distance vector is: " << endl;
    // printVector(distance);
    // cout << "visited vector is: " << endl;
    // printVector(visited);

    // check all nodes leading to 
    while (prev.at(i) != start && cnt <= nodes->size()) {
        int temp = prev.at(i);
        result.push_back(temp);
        i = indexOf(nodesNearby, temp);
        cnt++;
    }
    result.push_back(start);
    reverse(result.begin(), result.end());
    //cout << "Dji path found:  "  << endl;
    //printVector(result);
    //cout << "distance is " << distance.at(indexOf(nodesNearby, end)) << endl;
    return result;
}

// FINAL METHOD to be used to find path distance
double MapAnalyzer::getTotalDistance() {
    DjikstraPathfinding(limit, start, end);
    return distance.at(indexOf(nodesNearby, end));
}

// arbitrary start end
void MapAnalyzer::DjikstraPathfinding(double limit, int curr, int end) {
    DijkstraExit(limit, curr);
    djiPath = showPathFrom(curr, end);
    
}

// new: fixed start end
// void MapAnalyzer::DjikstraPathfinding(double limit) {
//     DijkstraExit(limit, start);
//     djiPath = showPathFrom(start, end);
//     printVector(djiPath);
// }

// print out vector content, debug methods
void MapAnalyzer::printVector(vector<int> v) {
    for (vector<int>::iterator i = v.begin(); i != v.end(); ++i) {
        cout << *i << ' ';
    }
    cout << endl;
}

void MapAnalyzer::printVector(vector<bool> v) {
    for (vector<bool>::iterator i = v.begin(); i != v.end(); ++i) {
        cout << *i << ' ';
    }
    cout << endl;
}

void MapAnalyzer::printVector(vector<double> v) {
    for (vector<double>::iterator i = v.begin(); i != v.end(); ++i) {
        cout << *i << ' ';
    }
    cout << endl;
}

// debug method
void MapAnalyzer::printNodes(vector<MapNode*> v) {
    for (vector<MapNode*>::iterator i = v.begin(); i != v.end(); ++i) {
        cout << (*i)->getNodeNum() << ' ';
        
    }
    cout << endl;
}

// return length (NUMBER OF STEPS)
int MapAnalyzer::getDjiPathLength() {
    return djiPath.size();
}

bool MapAnalyzer::inMap(int n) {
    // cout << "n is: " << n << endl;
    for (int i = 0; i < nodes->size(); i++) {
        // cout << "node num is: " << nodes->at(i)->getNodeNum() << endl;
        if (nodes->at(i)->getNodeNum() == n) {
            return true;
        }
    }
    return false;
}
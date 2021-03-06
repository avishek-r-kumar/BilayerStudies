#ifndef _VERTEX_H
#define _VERTEX_H

#include <vector>
#include <algorithm>
#include <string>

using namespace std; 

class Vertex
{
  
  Vertex* BFSHelper(double xFind, double yFind, int searchDepth, std::vector<Vertex*>& fifo);
 public:
  float x; 
  float y; 
  float z; 
  int index;
  string type; 
  int atomno; 
  Vertex();
  std::vector<Vertex*> edges; 
  std::vector<vector<Vertex*> > rings;
  Vertex(int type,float xIn, float yIn, float zIn);
  Vertex(Vertex &v);

  void AddEdge(Vertex* edge);
  int RemoveEdge(double xVert, double yVert);
  int RemoveEdge(Vertex* edge);
  int RemoveSingleEdge(Vertex* edge);
  void ClearConnection();
  void ModifyData(double xNew, double yNew, double zNew);

  //Ring Counter Functions 
  Vertex* BFS(double xFind, double yFind, int searchDepth);
  void findCyclesToSelf(int maxCycleSize, std::vector<std::vector<Vertex*> > &cycleList);
  void CountCyclesLocally(int depth, std::vector<std::vector<Vertex*> >&allCycles);
  void AddRing(std::vector<Vertex*> cycle); 

};

#endif 

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <cassert>

#include "vertex.h"
#include "graph.h"
#include "ringarea.h"
#include "testcases.h"

//global variables 
Graph bilayer; 
std::vector<std::vector<Vertex*> > allCycles;
std::vector<std::vector<Vertex*> > sortedCycles; 
const int ringmax = 12; 
int countBucket[ringmax];
float areaBucket[ringmax]; 
std::vector<std::vector<int> > stack; 

/*
  read_xyz: reads a file in the xyz format omiting oxygen 
  atoms  
  @param: file, xyz file 
*/
void read_xyz(char *file, Graph &bilayer,bool Debug=false)
{
  int numatoms;
  char buffer[3];
  char atom[2]; 
  float x,y,z;
  FILE *in; 
  in = fopen(file,"r");
  if(in == NULL)
    {
      std::cerr << "Cannot open file: " << file << std::endl; 
      exit(1);
    }
  fscanf(in, "%d", &numatoms);
  fscanf(in, "%s\n",buffer);
  if(Debug)
    {
      printf("%d\n",numatoms);
      printf("%s\n",buffer);
    }
  while(4 == fscanf(in,"%s %f %f %f\n",atom,&x,&y,&z) )
    {
      if(Debug)
	printf("%s %.15f %.15f %.15f\n",atom,x,y,z);
      if (atom[0] == 'O')
	{
	  continue; 
	}
      else
	{
	  Vertex* pos = new Vertex(14,x,y,z); 
	  bilayer.vertices.push_back(pos); 
	}
      
    }
  fclose(in);
  
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    bilayer.vertices[i]->index = i; 
  
}//read_xyz()


/*
  Connect atoms within a certain distance 
 */
void connectAtoms(Graph &bilayer,float dist,int Debug=0)
{
  
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    {
      for(unsigned int j = i+1; j < bilayer.vertices.size(); j++)
	{
	  float x_ij = bilayer.vertices[i]->x - bilayer.vertices[j]->x; 
	  float y_ij = bilayer.vertices[i]->y - bilayer.vertices[j]->y;
	  float z_ij = bilayer.vertices[i]->z - bilayer.vertices[j]->z;
	  float r = sqrt( x_ij*x_ij + y_ij*y_ij + z_ij*z_ij ); 
	  
	  if(Debug)
	    {
	      std::cout << "Vertex: " << i << " " << bilayer.vertices[i]->x << " " << bilayer.vertices[i]->y << " " << bilayer.vertices[i]->z << std::endl; 
	      std::cout << "Vertex: " << j << " " << bilayer.vertices[j]->x << " " << bilayer.vertices[j]->y << " " << bilayer.vertices[j]->z << std::endl; 
	      std::cout << "x_ij " << x_ij << std::endl; 
	      std::cout << "y_ij " << y_ij << std::endl; 
	      std::cout << "z_ij " << z_ij << std::endl; 
	      std::cout << "r: " << r << std::endl; 
	      
	    }
	  if(r > 0.0 && r < dist)
	    bilayer.vertices[i]->AddEdge(bilayer.vertices[j]); 
	    

	}//j loop 
    }//i loop
}//connectatoms

/*
  MakeHoney, Output coordinates to be visualized in Mathematica
  @param bilayer, Graph object containing vertices 
  @param nfile, array of characters for name of output file
 */
void MakeHoney(Graph& bilayer, string nfile="honeycomb1.m")
{
  FILE *outFile; 
  outFile = fopen(nfile.c_str() , "w");
  fprintf(outFile, "Graphics[{Black");


  for(unsigned int j =0; j < bilayer.vertices.size(); j++)
    {
      for(unsigned int i =0; i < bilayer.vertices[j]->edges.size(); i++)
        {
          
	  fprintf(outFile, ", \nLine[{{%f,%f},{%f,%f}}]", bilayer.vertices[j]->x,
		  bilayer.vertices[j]->y,
		  bilayer.vertices[j]->edges[i]->x,
		  bilayer.vertices[j]->edges[i]->y);
	  
        }
    }

  fprintf(outFile, "\n}]");
  fclose(outFile);
}

void fillCountBucket(int countBucket[], std::vector <std::vector<Vertex*> > &allCycles)
{
  std::vector <int> list;
  for(unsigned int i =0; i<12; i++) countBucket[i] =0;
  for(unsigned int i =0; i<allCycles.size(); i++) countBucket[allCycles[i].size()]++;
  for(unsigned int i =0; i<12; i++) list.push_back(countBucket[i]);
  
  stack.push_back(list);
  list.clear();
  
  // std::cout << "Rings Statistics" << std::endl;                                                                                                                                                                                                                                
  // for(int i =0; i < 12; i++) std::cout << i << " rings: " << countBucket[i] << std::endl;                                                                                                                                                                                      
  int ring_sum =0;
  for(int i =0; i < 12; i++) { ring_sum += i*countBucket[i];}
  int sum =0;
  for(int i = 0; i < 12; i++){sum += countBucket[i];}
  //std::cout << "RING SUM " << ring_sum << std::endl;                                                                                                                                                                                                                            
  //std::cout << "SUM " << sum << std::endl;                                                                                                                                                                                                                                      
  double average = (double) ring_sum/sum;
  //std::cout << "AVERAGE " << average << std::endl;                                                                                                                                                                                                                                
  FILE* count;
  
  count = fopen( "countBucket.dat", "w");
  fprintf(count, "Ring Statistics\n");
  for(int i =0; i < 12; i++) fprintf(count, "%d RINGS: %d\n", i, countBucket[i]);
  fprintf(count, "RING SUM: %d\n",ring_sum);
  fprintf(count, "SUM: %d\n", sum);
  fprintf(count, "AVERAGE: %f\n",average);
  fprintf(count, "\n");
  fclose(count);

}//fillCountBucket()

/*
  cycleDump,Outputs Rings in Mathematica format
  @param allCycles, vector of vector of Vertex objects containing rings
*/
void cycleDump(std::vector <std::vector<Vertex*> > &allCycles)
{
  FILE* cycle;
  cycle = fopen("cycleList.dat" ,"w");
  
  for(unsigned int i = 0; i<12; i++)
    {
      fprintf(cycle,"\n");
      fprintf(cycle, "RING SIZE %d\n" , i);
      for(unsigned int j =0; j < allCycles.size(); j++)
        {
	  std::vector <Vertex*> Ring = allCycles[j];
          if(Ring.size() != i) continue;
          for(unsigned int k =0; k<Ring.size(); k++) 
	    fprintf(cycle, "Circle[{%f,%f},0.2],\n",Ring[k]->x,Ring[k]->y);
          fprintf(cycle,"\n");
        }
      fprintf(cycle, "\n");
    }
  
  fclose(cycle);
}//cycleDump()

void polygonGraphics(std::vector <std::vector<Vertex*> > &allCycles)
{
  unsigned int minRing = 4; 
  unsigned int maxRing = 10; 

  const char *colors[11];
  colors[4]="Blue";
  colors[5]="Black";
  colors[6]="Yellow";
  colors[7]="Green";
  colors[8]="Red";
  colors[9]="Purple";
  colors[10]="Cyan";

  FILE* poly; 
  poly = fopen("poly.m", "w"); 
  fprintf(poly, "Graphics[{\n"); 
  fprintf(poly, "EdgeForm[Thick]\n"); 
  bool first;

  while( minRing <= maxRing )
    {
      first=true;
      fprintf(poly,","); 
      fprintf(poly,colors[minRing]);
      fprintf(poly,",\n"); 
      
      fprintf(poly, "Polygon[{\n"); 
      for(unsigned int i = 0; i < allCycles.size(); i++)
	{
	  if ( allCycles[i].size() != minRing ) continue; 

	  if (first)
	    {
	      fprintf(poly," ");
	      first=false;
	    }
	  else
	    {
	      fprintf(poly,","); 
	    }
	  fprintf(poly,"{ "); 
	  for(unsigned int j = 0; j < allCycles[i].size(); j++)
	    {
	      if ( j != 0)
		fprintf(poly,","); 
	      fprintf(poly, "{%f,%f}",allCycles[i][j]->x, allCycles[i][j]->y); 
	    }//j loop over vertices in cycle
	  fprintf(poly, "}\n"); 
	}//i loop over cycle
      fprintf(poly, "}]\n");//closes up Polygon
      minRing += 1; 
    }//for color of rings 

  fprintf(poly, "}]\n");//closes of Graphics
  fprintf(poly, "Export[\"poly.pdf\",%]"); 

  fclose(poly); 
}//PolygonGraphics()


/*
  AddRings to vertices. Good for looking for superrings,
  rings that are made of smaller rings. 
 */
void AddRings(vector <vector<Vertex*> > &allCycles)
{
  for(unsigned int i = 0; i < allCycles.size(); i++)
    {
      for(unsigned int j = 0; j < allCycles[i].size(); j++)
	allCycles[i][j]->AddRing(allCycles[i]); 
	
    }
}//AddRings()

/*
  secondSort,Deletes rings that are combination of others from the ring list.
  @param bilayer, bilayer object
  @param allCycles, vector of vector of Vertex* containing ring list 
  @param i, int of the vertex
*/
void secondSort(Graph &bilayer, std::vector<std::vector<Vertex*> > &allCycles, int i)
{
  for(unsigned int k = 0; k < bilayer.vertices[i]->rings.size(); k++) //iterate through ring list of vertex i to find a ring greater than seven  
    {
      if(bilayer.vertices[i]->rings[k].size() < 7)
	continue; 
      std::vector <Vertex*> kCycle = bilayer.vertices[i]->rings[k]; 
      for(unsigned int l = 0; l< bilayer.vertices[i]->rings.size(); l++)//iterate through all rings of the vertex to find smaller rings that are apart of the greater 
	{
	  if(bilayer.vertices[i]->rings[l].size() >= kCycle.size() )
	    continue; 
	  std::vector <Vertex*> lCycle = bilayer.vertices[i]->rings[l]; 
	  int num_matches = 0; 
	  for(unsigned int m = 0; m < lCycle.size(); m++)//iterate through the vertices of the smaller ring to see if they are in the larger ring
	    {
	      for(unsigned int n = 0; n < kCycle.size(); n++)
		{
		  if(lCycle[m] == kCycle[n])
		    num_matches++; 
		}//n loop over the vertices of the kCycle 
	    }//m loop over the vertices of lCycle 
	  if(num_matches > 3)
	    {
	      for(unsigned int o = 0; o < allCycles.size(); o++)
		{
		  if(kCycle == allCycles[o])
		    allCycles.erase(allCycles.begin() + o); 
		}//i loop over the CycleList 
	    }
	}// l loop over the ring list 
    }//k loop over the rings looking for rings greater than nine specific vertex 
}//secondSort()

/*
  Caclulates second moment from ring distribution. 
 */
float secondmoment()
{
  int sum = 0;  
  float avgringsize = 0.0;  
  float moment2 = 0.0; 

  for(int i = 0; i < ringmax; i++)
    sum += countBucket[i];
      
  for(int i = 0; i< ringmax; i++)
    avgringsize += i*((float)countBucket[i]/sum); 

  for(int i = 0; i < ringmax; i++)
    moment2 += (i-avgringsize)*(i-avgringsize)*((float)countBucket[i]/sum); 
      
  return moment2; 

}//secondmoment()

void ringstatsOut(int countBucket[], string nfile ="ringhist.dat")
{
  FILE *out; 

  out = fopen(nfile.c_str(),"w"); 

  fprintf(out,"%f\n",secondmoment()); 
  for(int i = 4; i < ringmax; i++)
    fprintf(out,"%d %d\n",i,countBucket[i]); 

  fclose(out); 
}//ringstatsOut()

void areastatsOut(float areaBucket[], float areabndlength, string nfile="areahist.dat")
{
  FILE *out; 
  
  out = fopen(nfile.c_str(),"w");
  
  fprintf(out,"%f\n",areabndlength);
  for(int i = 4; i < ringmax; i++)
    fprintf(out,"%d %f\n",i,areaBucket[i]); 

  fclose(out); 
  
}//areastatsOut()

//ringstatsOut
int main(int argc, char *argv[])
{
  
  if ( argc < 2 )
    {
      std::cerr << "Not enough input arguments: " << argc << std::endl; 
      exit(1);  
    }
  
  string out = "honeycomb1.m";

  read_xyz(argv[1],bilayer);
  std::cout << "Making Connections" << std::endl; 
  connectAtoms(bilayer,2.1);
  
  std::cout << "Making output" << std::endl; 

  bilayer.vertices[969]->AddEdge(bilayer.vertices[967]); //arbitrary connection  
  MakeHoney(bilayer); 

  //start counting cycles 
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    bilayer.vertices[i]->CountCyclesLocally(allCycles); 
  //  bilayer.FirstSort(allCycles); 
  AddRings(allCycles);
  std::cout << "Sorting through the Rings Now" << std::endl; 
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    secondSort(bilayer,allCycles,i); 

  //Ring Statistics 
  fillCountBucket(countBucket,allCycles); 
  cycleDump(allCycles); 
  std::cout << "mu2 = " << secondmoment() << std::endl; 
    
  
  float area; 
  
  for(int i = 0; i < ringmax; i++) areaBucket[i] = 0.0; 


  for(unsigned int i = 0; i < allCycles.size(); i++)
      {
	area += ringArea(allCycles[i],areaBucket,ringmax); 
	sortedCycles.push_back(ringSort(allCycles[i]));
      }

    polygonGraphics(sortedCycles); 
    float bndlength = avgbnd_length(bilayer); 

    float areasum = 0.; 

    for(int i = 0; i < ringmax; i++)
      areasum += areaBucket[i]; 

    std::cout << "area: " <<  area  << std::endl; 
    std::cout << "average of area bucket " << areasum << std::endl; 
    std::cout << "average bond length " << avgbnd_length(bilayer) << std::endl; 
    std::cout << "area/avgbondlength*2 " << area/( bndlength*bndlength ) << std::endl; 
    
    for(int i = 0; i < ringmax; i++)
      areaBucket[i] /= bndlength*bndlength;

    ringstatsOut(countBucket); 
    areastatsOut(areaBucket,area/(bndlength*bndlength)); 

    return 0; 
}//main()

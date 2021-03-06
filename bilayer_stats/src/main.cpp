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
#include "tinyxml2.h"
#include "aboav.h"

//global variables 
Graph bilayer; 
std::vector<std::vector<Vertex*> > allCycles;
std::vector<std::vector<Vertex*> > sortedCycles; 
const int ringmax = 12; 
int countBucket[ringmax];
double aboavBucket[ringmax]; 
std::vector<std::vector<double> > aboavStack; 
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
  if(fscanf(in, "%d", &numatoms))
    {
      std::cout << "Number of atoms: " << numatoms << std::endl; 
    }
  else
    {
      std::cerr << "ERROR reading number of atoms" << std::endl; exit(1); 
    }

  if(fscanf(in, "%s\n",buffer))
    

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
	  //continue;
	  bilayer.vertices.push_back(new Vertex(8,x,y,z)); 
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

void read_connect(char *file, Graph &bilayer)
{
  FILE *in; 
  int i,j,nconnect, nconcount=0; 

  in = fopen(file,"r"); 
  if( NULL == in )
    {
      std::cerr << "Cannot open file: " << file << std::endl; 
      exit(1); 
    }
  if(fscanf(in,"%d",&nconnect)) 
    {
      std::cout << "Number of manual connections: " << nconnect << std::endl; 
    }
  else
    {
      std::cerr << "Error reading number of connections" << std::endl; 
      exit(1); 
    }

  while( 2 == fscanf(in,"%d %d\n",&i,&j) )
    {
      //std::cout << "Making connection between i: " << i << " and j: " << j << std::endl; 
      bilayer.vertices[i]->AddEdge(bilayer.vertices[j]); 
      nconcount++; 
    }
  fclose(in); 

  if( nconnect != nconcount)
    {
      std::cerr << "The number of connections: " << nconnect << " . Does not match the number read: " << nconcount << std::endl;
      exit(1); 
    }


}//read_connect()


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
void MakeHoney(Graph& bilayer, string nfile="bilayer", float rmax =10.0)
{
  FILE *outFile; 
  string extm="_structure.m"; 
  string extpdf = "_structure.pdf";
  string file = nfile + extm; 
  float xdist = 0.0; 
  float ydist = 0.0; 
  float r = 0.0; 
  
  outFile = fopen(file.c_str(), "w");


  fprintf(outFile, "Graphics[{Black");


  for(unsigned int j =0; j < bilayer.vertices.size(); j++)
    {
      for(unsigned int i =0; i < bilayer.vertices[j]->edges.size(); i++)
        {
	  xdist = bilayer.vertices[j]->x - bilayer.vertices[j]->edges[i]->x; 
	  ydist = bilayer.vertices[j]->y - bilayer.vertices[j]->edges[i]->y; 
	  r = sqrt(xdist*xdist + ydist*ydist); 
	  if(r < rmax )
	    {

	      fprintf(outFile, ", \nLine[{{%f,%f},{%f,%f}}]", bilayer.vertices[j]->x,
		      bilayer.vertices[j]->y,
		      bilayer.vertices[j]->edges[i]->x,
		      bilayer.vertices[j]->edges[i]->y);
	  
	    }
        }
    }

  fprintf(outFile, "\n}]");
  fprintf(outFile,"\nExport[\"%s\",%%]",(nfile+extpdf).c_str()); 
  fclose(outFile);
}



void fillCountBucket(int countBucket[], std::vector <std::vector<Vertex*> > &allCycles,string nfile="bilayer")
{

  string extdat ="_ringCount.dat"; 
  string file = nfile + extdat; 
  
  std::vector <int> list;
  for(int i =0; i<ringmax; i++) countBucket[i] =0;
  for(unsigned int i =0; i<allCycles.size(); i++) countBucket[allCycles[i].size()]++;
  for(int i =0; i<ringmax; i++) list.push_back(countBucket[i]);
  
  stack.push_back(list);
  list.clear();
  

  int ring_sum =0;
  for(int i =0; i < ringmax; i++) { ring_sum += i*countBucket[i];}
  int sum =0;
  for(int i = 0; i < ringmax; i++){sum += countBucket[i];}

  double average = (double) ring_sum/sum;

  FILE* count;
  
  count = fopen(file.c_str(), "w");
  fprintf(count, "Ring Statistics\n");
  for(int i =0; i < ringmax; i++) fprintf(count, "%d RINGS: %d\n", i, countBucket[i]);
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

void polygonGraphics(std::vector <std::vector<Vertex*> > &allCycles, string nfile="bilayer")
{
  string ext ="_poly.m";
  string extpdf="_poly.pdf"; 
  string file = nfile+ext; 
   

  unsigned int minRing = 4; //4 
  unsigned int maxRing = 10; //10

  const char *colors[11]; //11
  colors[4]="Blue"; //4
  colors[5]="Black"; //5
  colors[6]="Yellow"; //6
  colors[7]="Green"; //7
  colors[8]="Red"; //8
  colors[9]="Purple"; //9
  colors[10]="Cyan"; //10

  FILE* poly; 
  poly = fopen(file.c_str(), "w"); 
  fprintf(poly, "Graphics[{\n"); 
  fprintf(poly, "EdgeForm[Thick]\n"); 
  bool first;

  while( minRing <= maxRing )
    {
      first=true;
      fprintf(poly,","); 
      fprintf(poly,"%s",colors[minRing]); 
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
      minRing += 1; //1 
    }//for color of rings 

  fprintf(poly, "}]\n");//closes of Graphics
  fprintf(poly, "Export[\"%s\",%%]",(nfile+extpdf).c_str()); 

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
  RemoveRings to vertices. Good for removing the wrong rings
  after a sort and adding the right ones. 
 */
void RemoveRings(Graph &bilayer)
{
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    {
      bilayer.vertices[i]->rings.clear(); 
    }
}

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
      if(bilayer.vertices[i]->rings[k].size() < 7) //7
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
	  if(num_matches > 3) //3
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

void ringstatsOut(int countBucket[], string nfile ="bilayer")
{
  string ext = "_ringhist.dat"; 
  string file = nfile+ext; 
    
  FILE *out; 

  out = fopen(file.c_str(),"w"); 

  fprintf(out,"%f\n",secondmoment()); 
  for(int i = 4; i < ringmax; i++)
    fprintf(out,"%d %d\n",i,countBucket[i]); 

  fclose(out); 
}//ringstatsOut()

void areastatsOut(float areaBucket[], float areabndlength, string nfile="bilayer")
{

  string ext ="_areahist.dat";
  string file = nfile+ext; 
  FILE *out; 
  
  out = fopen(file.c_str(),"w");
  fprintf(out,"%f\n",areabndlength);
  for(int i = 4; i < ringmax; i++)
    fprintf(out,"%d %f\n",i,areaBucket[i]); 

  fclose(out); 
  
}//areastatsOut()

void readParameters(char *nfile,float &bondlength, string &basename, float &a, float &b)
{
  tinyxml2::XMLDocument doc; 
  doc.LoadFile(nfile); 

  bool distbond = atoi( doc.FirstChildElement("root")->FirstChildElement("distbond")->GetText() ); 
  if(distbond)
    bondlength = atof( doc.FirstChildElement("root")->FirstChildElement("bondlength")->GetText() );
  else
    bondlength = 0; 

  basename = doc.FirstChildElement("root")->FirstChildElement("basename")->GetText(); 
  bool pbc = atoi( doc.FirstChildElement("root")->FirstChildElement("pbc")->GetText() );

  if(pbc)
    {
      std::cout << "Using PBC conditions " << std::endl; 
      a = atof(doc.FirstChildElement("root")->FirstChildElement("latticex")->GetText()); 
      b = atof(doc.FirstChildElement("root")->FirstChildElement("latticey")->GetText());
      std::cout << "a: " << a << " " << "b: " << b << std::endl; 
    }
  else
    {
      a = 0.;
      b = 0.; 
    }

  std::cout << "bondlength: " << bondlength << std::endl; 
  std::cout << "basename: " << basename << std::endl; 
  
}//readParameters()

/*
  Output the vertices of the rings 
  @param nfile: file to be output 
  @param allCycles the vector of vector of rings
*/
void outputRings(string nfile, std::vector <vector<Vertex*> > &allCycles)
{
  
  string ext ="_ringlist.dat";
  string file = nfile+ext; 
  FILE *out; 
  out = fopen(file.c_str(),"w");
  
  for(unsigned int i = 0; i < allCycles.size(); i++)
    {
      for(unsigned int j = 0; j < allCycles[i].size(); j++)
	fprintf(out,"%d ",allCycles[i][j]->index); 
      fprintf(out,"\n"); 
    }
  fclose(out); 

}//outputRings()

//ringstatsOut
int main(int argc, char *argv[])
{
  
  if ( argc < 3 )
    {
      std::cerr << "Not enough input arguments: " << argc << std::endl; 
      std::cout << "./main.e coordinates.xyz coordinates.xml coordinates.con" << std::endl; 
      exit(1);  
    }
  
  float bondlength; 
  string basename; 
  float areasum = 0.; 
  float bndlength;
  float bndlength2; 
  float deviation; 
  float latticex, latticey; 
  int depth = ringmax - 1; 
  //string out = "honeycomb1.m";

  //Debug 
  std::cout << "Number of input arguments is: " << argc << std::endl; 
  for(int i = 0; i < argc; i++)
    std::cout << "argv[" << i << "]: " << argv[i] << std::endl; 
 

  read_xyz(argv[1],bilayer);
  readParameters(argv[2],bondlength,basename,latticex,latticey); 
  
  if(bondlength > 0)
    {
      std::cout << "Making Connections Based On Distance" << std::endl; 
      connectAtoms(bilayer,bondlength);
    }
  if(argc == 4)
    {
      std::cout << "Making manual connections" << std::endl; 
      read_connect(argv[3],bilayer); 
      MakeHoney(bilayer,basename); 
    }
  
  //Remove Ring 
  std::cout << "x: " << bilayer.vertices[971]->x << "y: " << bilayer.vertices[971]->y << std::endl; 
  std::cout << "Before:Number of connections to 971: " << bilayer.vertices[971]->edges.size() << std::endl; 
  bilayer.vertices[971]->RemoveEdge(bilayer.vertices[152]); 
  std::cout << "After:Number of connections to 971: " << bilayer.vertices[971]->edges.size() << std::endl; 
 
 //start counting cycles 
  std::cout << "Counting Rings" << std::endl; 
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    bilayer.vertices[i]->CountCyclesLocally(depth,allCycles); 
  
  AddRings(allCycles);
  std::cout << "Sorting through the Rings Now" << std::endl; 
  for(unsigned int i = 0; i < bilayer.vertices.size(); i++)
    secondSort(bilayer,allCycles,i); 

  //Ring Statistics 
  fillCountBucket(countBucket,allCycles,basename); 
  cycleDump(allCycles); 
  std::cout << "mu2 = " << secondmoment() << std::endl; 
    
  
   
  for(int i = 0; i < ringmax; i++) areaBucket[i] = 0.0; 

  FILE *ring;
  std::vector<Vertex*> cycle; 
  string ext = "_ringdist.dat";
  ring = fopen((basename+ext).c_str(),"w"); 
  for(unsigned int i = 0; i < allCycles.size(); i++)
      {
	cycle = ringSort(allCycles[i]); 
	fprintf(ring,"%d %f\n",allCycles[i].size(),ringArea(cycle,areaBucket,latticex,latticey) ); 
	sortedCycles.push_back(ringSort(allCycles[i]));
      }
  fclose(ring); 

  
  PolygonPBC(sortedCycles); 
  polygonGraphics(sortedCycles,basename); 
  bndlength = avgbnd_length(bilayer,latticex,latticey); 
  bndlength2 = avgbnd_lengthtwo(bilayer,latticex,latticey); 
  deviation = sqrt(bndlength2-bndlength*bndlength); 

  for(int i = 0; i < ringmax; i++)
    areasum += areaBucket[i]; 
  
  std::cout << "sum of area bucket " << areasum << std::endl; 
  std::cout << "average bond length " << avgbnd_length(bilayer,latticex,latticey) << std::endl; 
  std::cout << "average bond lenth squared" << avgbnd_lengthtwo(bilayer,latticex,latticey) << std::endl; 
  std::cout << "deviation bond length: " << deviation << std::endl; 
  std::cout << "area/avgbondlength*2 " << areasum/( bndlength*bndlength ) << std::endl; 
  
  for(int i = 0; i < ringmax; i++)
    areaBucket[i] /= bndlength*bndlength;
  
  ringstatsOut(countBucket,basename); 
  areastatsOut(areaBucket,areasum/(bndlength*bndlength),basename); 
  outputRings(basename,sortedCycles); 
  outputConnect(bilayer,latticex,latticey); 
  //Running Aboav function 
  RemoveRings(bilayer); 
  AddRings(allCycles);
  Aboav(allCycles,aboavBucket,aboavStack,ringmax); 

  
  return 0; 
}//main()

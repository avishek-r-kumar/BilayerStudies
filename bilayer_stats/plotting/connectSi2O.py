#!/usr/bin/env python
"""
Forms connectivity table for a silicon bilayer, by connecting the Si to O. 
./connectSi2O.py XYZ DIST RING

XYZ:    XYZ File with coordinates 
DIST:   Distance that the oxygens need to be within the silicons
RING:   Ring list  
"""

import sys
import numpy as np 

def read_xyz(nfile, Verbose=False):
    """
    Reads Files in xyz format and returns the number of atoms 
    and a list of atom types and a numpy array of coordinates.
    
    read_xyz(nfile, Verbose=False):

    inputs:
    nfile: xyz file
    Verbose: Verbose output
    
    returns:
    natom: number of atoms
    title: title of coordinates
    atoms: list of atoms
    coordinates: a (natom,3) numpy array of coordinates 
    """
    coordinates=[]
    atoms=[]
    nread = 0
    
    infile = open(nfile,"r")
   
    natom = int(infile.readline())
    title = infile.readline()
        
    for line in infile:
        atoms.append(line.strip('\n').split()[0])
        coordinates.append(line.strip('\n').split()[1:])
        nread+=1
        if(Verbose):
            print atoms[-1]
            print coordinates[-1]
        
    infile.close()
    assert natom==nread, "The number of atoms listed, %d, does not match the number of atoms read,%d."%(natom,nread)
    
    return natom, title, atoms, np.array(coordinates,dtype=float)

def connectSi2O(atom,coordinates,rmax=1.25,Verbose=False):
    """
    connect Si to O

    connectSi2O(atom,coordinates,rmax=1.25,Verbose=False)
    
    inputs:
    atom: list of atom types, Si or O
    coordinates: (n,3) numpy array of coordinates
    rmax: the maximum distance for the Si O bond 
    Verbose: Verbose output 

    return:
    connect: numpy array of connectivity table 
    """
    connect=[]
    for i in range(len(coordinates)):
        if(Verbose): print "atom:%s x:%f y:%f z:%f"%(atom[i],coordinates[i][0],coordinates[i][1],coordinates[i][2])
        if atom[i] == "Si":
            if(Verbose): print "Found a Si"
            for j in range(len(coordinates)):
                if atom[j] == "O":
                    if(Verbose): print "Found a O"
                    rij = coordinates[i] - coordinates[j]
                    r = np.sum(rij*rij)
                    if(r < rmax): connect.append([i,j])
    connect.append([963,2382])
    connect.append([296,2382])
    connect.append([969,2383])
    connect.append([967,2383])
    return np.array(connect,dtype=int)

def outputConnect(basename,connect): 
    """
    outputConnect, output the connectivity table
    
    outputConnect(basename,connect)

    inputs:
    basename: basename of the file
    connect: numpy array of connectivity 
    """
    print "Output Connectivity Table:%s"%(basename)+".con"
    outfile=open(basename+".con","w")
    outfile.write("%d\n"%len(connect))
    for con in connect:
        outfile.write("%d %d\n"%(con[0],con[1]))
    outfile.close()

def readRings(filename,verbose=False):
    """
    reads in the Ringlist
    
    readRings(filename,verbose=False)
    
    inputs:
    filename
    verbose

    returns:
    list of rings. 
    """
    rings = []
    infile = open(filename,"r")
    for line in infile:
        rings.append( np.array(line.strip('\n').split()[:],dtype=int) )
        if(verbose):
            print rings[-1]
    infile.close()
    
    return rings

def ORings(ring,connect):
    """
    Documentation coming soon
    """
    oring=[]
    for i in range(len(ring)):
        atomi = ring[i]
        if i == (len(ring) - 1):
            atomj = ring[0]
        else:
            atomj = ring[i+1]
        #print "%d %d"%(atomi,atomj)
        olist=[]
        for i in range(len(connect)):
            if(connect[i][0] == atomi or connect[i][0] == atomj):
                #print "Found A Match"
                #print connect[i]
                #print connect[i][1]
                olist.append(connect[i][1])
        #print olist     
        for x in olist:
            if(olist.count(x) == 2):
                #print "Found Bridging Oxygen:%d"%x
                oring.append(x)
                break 
    #print oring
    #print ring 
    for i in range(len(ring)):
        print "%d "%ring[i],
        print "%d "%oring[i],
    print ""


def main():

    basename=sys.argv[1].split('/')[-1].split('.')[0]

    print "Reading the Input File"
    natom, title, atom, coordinates = read_xyz(sys.argv[1])

    print "Looking for connections"
    connect = connectSi2O(atom,coordinates)
    
    outputConnect(basename,connect)

    rings=readRings(sys.argv[3])
    
    for ring in rings:
        ORings(ring,connect)


if __name__ == "__main__" and len(sys.argv) == 4:
    main()
else:
    print "Not Enough Input Arguements: %d"%len(sys.argv)
    print __doc__

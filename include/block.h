//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <block.h>
//
// Intrpret the bookshelf files and convert bookshelf to OA format
//
// Author: Wang Sifei
// History: 2012/02/12 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _BLOCK_H_WSF_
#define _BLOCK_H_WSF_
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<cmath>
#include<vector>
#include<map>
#include<assert.h>
#include<pwd.h>
#include<stdlib.h>
#include<cstring>

#include"enum.h"
#include"instTerm.h"
#include"net.h"
#include"inst.h"
#include"shape.h"
#include"row.h"
#include"module.h"
#include"layer.h"

#include "base.h"
#include "design.h"

using namespace std;

class ltstr
{
public:
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

class Block
{
public:
	inline Block() : bloat(1) {
		//cout<<"now interpret the lefdef file"<<endl;
		netMapIndex = new map<const char*, long, ltstr>;

	};
	inline ~Block() {
		if(netMapIndex) {
			delete netMapIndex;
			//cout<<"free netMapIndex"<<endl;
		}
		for(unsigned long i=0; i<modules.size(); i++) {
			if(modules[i]) {
				delete modules[i];
			}
		}
		for(unsigned long i=0; i<nodes.size(); i++) {
			if(nodes[i]) {
				delete nodes[i];
			}
		}
		//cout<<"free nodes[]"<<endl;
		for(unsigned long i=0; i<nets.size(); i++) {
			if(nets[i]) {
				delete nets[i];
			}
		}
		//cout<<"free nets[]"<<endl;
		for(unsigned long i=0; i<rows.size(); i++) {
			if(rows[i]) {
				delete rows[i];
			}
		}
		//cout<<"free layers[]"<<endl;
		for(unsigned long i=0; i<layers.size(); i++) {
			if(layers[i]) {
				delete layers[i];
			}
		}
		//cout<<"free rows[]"<<endl;
		for(unsigned long i=0; i<nonRectangularNodes.size(); i++) {
			if(nonRectangularNodes[i]) {
				delete nonRectangularNodes[i];
			}
		}

		//=============hjy==================
		//delete[]nonRectangularNodes;
		//cout<<"finished interpret"<<endl;
	};
public :
	//******************************************************************
	//                io interface for LEFDEF
	//******************************************************************
	void parseLefDef(string techFile, string cellFile, string defFile);
	void loadTechLef(string techFile);
	void loadCellLef(string cellFile);
	void loadDef(string defFile);
	void reloadDef(string defFile);
	void insertBlocks();
	void saveDef(string defFile, string originDef);
	void saveDefForRDP(string defFile, string originDef);
	//*******************************************************************
	//				  io interface for OA
	//*******************************************************************
	void parseOA(oaBlock *oaBlk);
	void saveOA(oaBlock *oaBlk,Block* block);

	//*******************************************************************
	//				  io interface for bookshelf
	//*******************************************************************
	void parseBookShelf(string file);
	void loadPl(const char *pl);
	void savePl(const char *filename, const char *oldPlFile);
	void savePlAccurate(const char *filename, const char *oldPlFile);
	void loadNets( const char *netsFile);
	void loadNodePl(const char *plFile,const char *nodeFile);
	void loadScl(const char *sclFile);
	void loadShapes(const char *shapesFile);
	void splitNonRectNode();
	//*******************************************************************

	void writeName();
	void clearName();

public:
	//check functions
	void checkNodeMapIndex();
	void checkNetMapIndex();
	void checkNonRectMapIndex();
	void checkShapes();
	void checkNodes();
	void checkNets();
	void checkInstTerms();
	void checkRows(); 
public:
	inline int getBloatSize() {
		return bloat;
	};
	inline void setBloatSize(int b) {
		bloat = b;
	};
	inline vector<myNet*>& getNets() {
		return nets;
	}
	inline vector<Inst*>& getInsts() {
		return nodes;
	}
	inline vector<myRow*>& getRows() {
		return rows;
	}
	inline vector<Shape*>& getShapes() {
		return nonRectangularNodes;
	}
	inline vector<Layer*>& getLayers() {
		return layers;
	}
	inline unsigned long getNumNodes() {
		return numTotalNodes;
	}
	inline unsigned long getNumNets() {
		return numTotalNets;
	}
	inline unsigned long getNumRows() {
		return numTotalRows;
	}
	inline unsigned long getNumLayers(){
		return layers.size();
	}
	//=======hjy20130619=========
	inline double getTargetUtil()
	{
		return targetUtil;
	}
	inline void setTargetUtil(double t)
	{
		targetUtil=t;
	}
	inline void setMaxDisplacement(long m)
	{
		maxDisplacement=m;
	}
	inline long getMaxDisplacement()
	{
		return maxDisplacement;
	}

	long getLayerIndex(string name){
		return layerMapIndex[name.c_str()];
	}

private:
	vector<myNet*> nets;
	vector<Inst*> nodes;
	vector<myRow*> rows;
	vector<Module*> modules;
	vector<Shape*> nonRectangularNodes;
	vector<Layer*> layers;

	map<const char*, long, ltstr> nodeMapIndex;
	map<const char*, long, ltstr> nonRectMapIndex;
	map<const char*, long, ltstr> moduleMapIndex;
	map<const char*, long, ltstr> *netMapIndex;
	map<const char*, long, ltstr> *instMapMaster;
	map<const char*, long, ltstr> layerMapIndex;

	unsigned long numTotalNodes;
	unsigned long numTotalTerminals;
	unsigned long numTotalNets;
	unsigned long numTotalPins;
	unsigned long numTotalFixed;
	unsigned long numTotalMoved;
	unsigned long numTotalRows;
	unsigned long numNonRectangularNodes;
	double targetUtil;//hjy20130619
	long maxDisplacement;//hjy20130619

private:
	int bloat;

public:
	//for debug
	string defFile;
};

#endif

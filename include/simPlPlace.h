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
// History: 2012/02/21 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************
#ifndef SIMPLPLACE_H
#define SIMPLPLACE_H

#include<queue>
#include"place.h"
#include"inst.h"
#include"net.h"
#include"instTerm.h"
#include"enum.h"
#include"RLRegion.h"
#include <util.h>
#include <cgsolver.h>
#include <assert.h>
#include <stdlib.h>
#include "test.h"

//#include "cholmod.h"

void amgpcg(int n, double *a, int *ja, int *ia, double *b, double *x,
		int maxIter, double tol, int initValue);

int fpCG(int * row, int * col, double *val, int numVals, int numRows,
		double *bb, double *temp);

void testCG();

void classicCG(int n, double *a, int *ja, int *ia, double *b, double *x,
		int maxIter, double tol, int initValue);

int cholmodSolve3(int dim, int nnz, int* Ap, int*Ai, double* Ax, double* b,
		double*sol);
int cholmod(int dim, double *a, int *ja, int *ia, double *b, int job);
int iccgSolve(int *row, int *col, double *val, int nnz, int dim, double *b,
		double *sol);

class Triple {
public:
	inline Triple() {
	}
	;
	inline Triple(long r, long c, double v) :
		row(r), column(c), element(v) {
	}
	;
	inline ~Triple() {
	}
	;
public:
	friend bool operator>(Triple n1, Triple n2) {
		return n1.column < n2.column;
	}
	;
public:
	long row;
	long column;
	double element;
};

struct RectLevel {
	RLRegion* region;
	int level;
};

struct InstPos {
	Inst* inst;
	myPoint oldPos;
};

class SimPlPlace: public myPlacement {
public:
	inline SimPlPlace(Block *topBlock) :
		myPlacement(topBlock) {
		//cout<<"Now begin global placement..."<<endl;
		//clock();
		setBlockXYWH();
	}
	;
	inline ~SimPlPlace() {
		for (long i = 0; i < gridNumX; ++i) {
			for (long j = 0; j < gridNumY; ++j) {
				if (bins[i][j]) {
					delete bins[i][j];
				}
			}
		}
	}
	;
public:
	void iPlace(int strategy);
	void randomDisperse();
	void purposeDisperse();
	void setBlockXYWH();
	void filterInstsAndNets();

	void buildB2B();
	void buildClique(); // add by ZhouQ
	void buildHybrid();
	double calWeight(Inst* instA, Inst* instB, long num, bool isVertical,
			long netWeight);
	void insertElement(long row, long column, double element, bool isVertical);
	void compressMatrixX(int type, int dim);
	void compressMatrixY(int type, int dim);
	void linearSolveX(int type, int dim);
	void linearSolveY(int type, int dim);
	void linearSolverX();
	void linearSolverY();
	void freeB2B();
	void updateB2B(int type, double weight);
public:
	void gPlace();
	void guiFile(const char* fname);
	void guiRect(const char* fname,  vector<Triple>& overf, long x1, long x2, long y1, long y2);
	void guiDensityMap(const char* fname);
	void guiClustersGroup(const char* fname, vector<RLRegion*>& clus , vector<Triple>& overf);//add by Wuwy
	void guiClustersGroupMatlab(const char* fname, const char* densityfname, vector<RLRegion*>& clus , vector<Triple>& overf);
	void guiRectGroup(const char* fname, vector<Rect>& rects);
	void guiRect1(const char* fname, long x1,long x2, long y1, long y2);
	void roughLegalization(long h, long v, long step);
	void setDensFactor();
	double getDensFactor();
	void buildGrids();
	void getBinsUsage(long step);
	void findOverfilledBins();
	RLRegion* getExpansionRegion(Triple& overfilledBin, long step);
	RLRegion* getExpansionRegion(vector<RLRegion*>& hotSpot);
	double getRectCellArea(long lx, long ly, long rx, long ry);
	double getRectAvailableArea(long lx, long ly, long rx, long ry);
	double getRectCellAreaI(long lx, long ly, long rx, long ry);
	double getRectCellAreaJ(long lx, long ly, long rx, long ry);
	RLRegion* getCluster(Triple& overfilledBin, long h, long v, long step);
	void getCellsInCluster(RLRegion* cluster);
	void reOrder(); //add by ZhouQ
	void saveOldPos();
	void restoreOldPos();  // add by Junjie Ke
	void saveUpperBoundPos(vector<myPoint>& pos);  // add by Junjie Ke
	void restoreUpperBoundPos(vector<myPoint>& pos);  // add by Junjie Ke

	void leftExtend(double& area, double& areaCell, double& areaAvailable, vector<bool>& extend,
  			vector<myPoint>& overfilledPoint, long& gridX_L, long& gridY_T,
			long& gridY_B);
	void rightExtend(double& area, double& areaCell, double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridX_R, long& gridY_T,
			long& gridY_B);
	void topExtend(double& area, double& areaCell, double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridY_T, long& gridX_L,
			long& gridX_R);
	void bottomExtend(double& area, double& areaCell, double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridY_B, long& gridX_L,
			long& gridX_R);

	void leftExtend_Bounding(double& area, double& areaCell,
			double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridX_L, long& gridY_T,
			long& gridY_B);
	void rightExtend_Bounding(double& area, double& areaCell,
			double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridX_R, long& gridY_T,
			long& gridY_B);
	void topExtend_Bounding(double& area, double& areaCell,
			double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridY_T, long& gridX_L,
			long& gridX_R);
	void bottomExtend_Bounding(double& area, double& areaCell,
			double& areaAvailable, vector<bool>& extend,
			vector<myPoint>& overfilledPoint, long& gridY_B, long& gridX_L,
			long& gridX_R);

	myPoint getBary2(long gridX_L, long gridX_R, long gridY_B, long gridY_T);
	myPoint getBary(vector<myPoint>& overfilledPoint);

	void lookAheadLegalize(long h, long v, long step);
	void lookAheadLegalizePOLAR(long step);
	void updateBinsInCluster(RLRegion* cluster);
	void findHotSpot(vector<vector<RLRegion*> >& hotSpots);
	void divide(const RLRegion* r, const bool vertical, vector<RLRegion*>& Q_next);
	void moveCellsToBin(RLRegion* bin, long step);
	void cellDistribution(RLRegion* rect, long step);
	void linearDiffusion(RLRegion* rect, long step);
	void diffusion(RLRegion* rect, int level);
	void getStripsUsage(vector<long>& borders, vector<Inst*>& obstacles,
			vector<double>& whiteSpace, long boundary1, long boundary2,
			int level, double& totalWS);
	void cellShifting(vector<long> borders, vector<Inst*> targetCells,
			vector<double> whiteSpace, int level, double totalWhiteSpace,
			double Cb);
	void linearScaling(vector<Inst*> targetCells, long begin, long end,
			long boundaryL, long boundaryH, int level);
	void flattenRL();// if the curcirt has few fixed instance, use a more simple flatten rough legalization method instead
	void globalRefine();// the refinement technique presented in POLAR
	void globalRefine1();
	void reOrder(vector<Inst*>& insts, bool isDirectX);
	void setNetCenterOfInst();

	void placeAtBinCenter();
	void centerDiffusion(RLRegion* cluster);

	void netBasedCellInflation();// bloating cells in nets that passes OF gcells.
	void binBasedCellInflation(); //bloating cells in congested gcells
	void congestionEstimate(string file);
	void bloating(myNet* net, long dem, long cap, bool hori);
	void restore();

	void constraintMove(double alpha);

	void setNetCenterAndCong();
	void netBasedMovementH();
	void netBasedMovementV();

	void pinDenBasedInflation(double alpha);
	void blockInflation(double ratio);
	double getTotalCellArea();
	long getTotalPinNum();
	double getAvailableArea();

	void loadSol(string f1, string f2);//for debug

	void checkOrder(vector<Inst*> v, bool horizontal); // for debug
	void setAverageNodeArea();
	inline void setTol(double tt, double ot) {
		overfillTol = ot;
		targetTol = tt;
	}
	;
	inline void setAverageNodeArea(double a) {
		averageNodeArea = a;
	}
	;
public:
	void dPlace();

public:
	void checkMatrix();
	void printOrder();

public:
	inline void setVisible(bool v) {
		visible = v;
	}

private:
	long blockW;
	long blockH;
	long blockX;
	long blockY;

private:
	struct ITLIN_IO *itlin_ioctl;
	vector<Inst*> preValidNodes;
	vector<Inst*> validNodes;
	vector<myNet*> validNets;
	vector<Inst*> inValidNodes;

	double averageNodeArea;
	double averageNodeWidth;
	double averageNodeHeight;

	long numMoveNodes;
	long numValidNodes;
	long numInvalidNodes;
	long numStarNets; // number of star nets when building hybrid net model. nets with more than 3 pins.

	vector<vector<Triple> > B2B_MatrixX;
	vector<vector<Triple> > B2B_MatrixY;
	vector<double> diagV;
	vector<double> Bx;
	vector<double> By;

	vector<long> Xp;
	vector<long> Xi;
	vector<double> Xx;
	vector<double> Xsolve;
	vector<long> diagX; //for updateB2B

	vector<long> Yp;
	vector<long> Yi;
	vector<double> Yx;
	vector<double> Ysolve;
	vector<long> diagY; //for updateB2B

private:
	//vector<InstPos> moveSortedNodesX;
	//vector<InstPos> moveSortedNodesY;
	//vector<Inst*> clusterObstacle;    //all fixed cells in one cluster
	//vector<InstPos> moveSortedNodes;
	//vector<InstPos> fixedSortedNodes;
	vector<Triple> overfilledBins;
	queue<RectLevel> subRegions;
	double overfilledFactor;
	vector<vector<RLRegion*> > bins;
	vector<myPoint> oldPos;

	vector<long> gridCoordX;
	vector<long> gridCoordY;

	long gridNumX;
	long gridNumY;
	long gridSize;
	long gcellWidth;
	long gcellHeight;

	double overfillTol;
	double targetTol;
	int maxDiffusionLevel;

	bool simplified; //if the circuit has few fixed objects, simplify the process of getting LAL cluster

	long **capH;
	long **capV;

private:
	bool visible; //for gui
	//vector<Rect> subCluster;  //for debug

private:
	long numCluster;

public:
	void setClusternum(){
		numCluster = 0;
	}
	long getClusternum(){
		return numCluster; // for debug
	}

};


#endif

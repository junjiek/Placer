#ifndef _DATIL_PLACE_H_
#define _DATIL_PLACE_H_

#include <map>
#include <vector>
#include "block.h"
#include "inst.h"
#include "net.h"
#include "row.h"
#include "instTerm.h"
#include "rect.h"
#include "point.h"
#include "place.h"
#include "ldpUtility.h"
#include "ldplaceRow.h"
#include "dplace.h"
#include "simPlPlace.h"

const bool FINAL = true;
const long BENEFIT_THRESHOLD = 0;
const long WORSEN_THRESHOLD = 0;
const long SWAP_WEIGHT1 = 500000;
const long SWAP_WEIGHT2 = 0;
const long RANGE = 1100;
const long RANGE_NUM = 15;
class Ldplace: public myPlacement {
private:
	Block *block;

	vector<Inst *> movValInsts;
	vector<Inst *> movInvInsts;
//	vector<Inst *>& macros;
//	vector<Inst *>& stdCells;
	long numRow;
	unsigned long numInst;
	long numFixed;//numMovableInst, Inst;
	LdplaceRow ** ldpRows;
	
//	long numMacro, numStdCell;
//	long numTerm;
//	map<const myRow*, UInt4> rowIndexMap;
	
//used
    long rowHeight;
	long siteWidth;
//used
	long rowAreaLeft, rowAreaRight, rowAreaBottom, rowAreaTop;

	//add by ZhouQ
	double abu2,abu5,abu10,abu20;
	//int abu2Last,abu5Last,abu10Last,abu20Last;
	double abu2Last,abu5Last,abu10Last,abu20Last;
		int gridNumX;
		int gridNumY;
		int gridSizeX;
		int gridSizeY;
		vector<long> gridCoordX;
		vector<long> gridCoordY;
		double** gridDensity;
		double** gridCellArea;
		double** gridAreaAvailable;
		//vector<double> binDensity;
		vector<Triple> binDensity;

	
	//long **siteMap;

	/*
	 *  coefficient for the Bezier Curve
	 */
//	long binominalSize;
//	long *binomialCoefficient;

public:
//	vector<vector<Inst *> > instOfRows;


public:
	Ldplace(Block *blk);
	~Ldplace();

	/*
	 * main entry of ldplace
	 */
	void run();
	void dplace();
	void initLdprows(vector<myRow *>& rows);
	void initSubrow(vector<Inst*>& insts, vector<myRow *>& rows);
	void initInstofSubrow();
	void initLdplace(vector<myRow *>& rows,  vector<Inst*>& insts);
	void legalize();
	void check();
	void preOptimize();
	void postOptimize();

	/*
	 * 	Basic functions
	 */
	void checkInstNum();
	void showDesign();
	bool isMacro(Inst *inst);
	long getRowIndex(Inst *inst);
	long getRowIndex(Inst *inst, size_t &subRowIndex);
//used
	long getRowIndexOccupied(long y, bool bottom);
	long getRowIndexIn(long y,bool bottom);
	bool checkOverlap(Inst *a, Inst *b);
	mySubRow * getSubRow(Inst *inst);
	void getInstOfRows();
	bool isFixedInst(Inst *inst);


	bool isRLMPin(Inst *inst);
	// insert a inst into instOfRow, sorted by its low-left coordinate
	void insert(Inst *inst, long rowIndex);
	void getSite(Inst *inst, long &rowIndex, long &siteIndex);
	void getSite(Inst *inst, long &rowIndexBegin, long &rowIndexEnd,
			long &siteIndexBegin, long &siteIndexEnd);




	/*
	 *  HPWL Optimization Functions
	 */
	void singleSegmentClustering();
	inline void getSpaces(mySubRow* subRowNow, list<Inst*>::iterator k, long& origSpcPre, long& origSpcMid,
			                long& origSpcPost,long& origBoundPre, long& origBoundPost, bool type);
	//inline void getSpaces(mySubRow* subRowNow, list<Inst*>::iterator k, long& origSpcPre, long& origSpcMid,
				                //long& origSpcPost, bool type);
    inline void getPenalty(unsigned long instWidth, long& mid, long& p1, long& p2);
//	void verticalSwap(Inst* instOrig, long& rowIndex, long& boxLeft, long& boxRight);
    void swapInRow(long* origInfor, long& rwoIndex, mySubRow* & subOrigNow,
    		       list<Inst*>::iterator& k, bool& done, bool vertical);
    void swapInCongest(long* origInfor, long& rowIndex, mySubRow* & subOrigNow,
                            list<Inst*>::iterator& k, bool& done);
	//void GVSwap(long iterator);
    void GVSwap();
    void CongestSwap(long iterator);
	void VerticalSwap();
	bool test();
	void localReOrdering(long con);
	void getOptimalBox(Inst *inst, Rect& optBox);
	void getBBox(InstTerm& instTerm, Rect& bbox);
	mySubRow *findOptimalSubRow(Inst *inst, Rect optimalBox);
	mySubRow *findCellToSwap(Inst *inst, Rect optimalBox, Inst *swapInst);
	inline long getHPWL(Inst *inst);
	inline long getHPWL(Inst inst);//hjy20130608
	inline void getOrigin(InstTerm& instTerm, myPoint& pnt);
	inline void getBBox( myNet* & net, Rect& bbox);
	void WorsenSwap();
	void WorsenInRow(long* origInfor, long& rowIndex, mySubRow* & subOrigNow,
			                list<Inst*>::iterator& k, bool& done, bool vertical);
	//add by ZhouQ
	void initBins();
	void rebuildBins();
	long getScaledHPWL(Inst* inst);
	long getPenaltyD(Inst* inst, long oldX, long oldY);
	void updateDensity(Inst* inst, long oldX, long oldY);
	inline double getDensity(long x, long y);
	double getDensityPenalty(Inst* inst, long newX, long newY);
	void updateABU(double newDensity, double oldDensity);
	void buildABU();
	double getABUpenalty();
	void checkABU();
	inline int getGridX(long x);
	inline int getGridY(long y);
	void cellRestore();
	void cellBloating(double rate);
	void relaxation();
	void relaxationV(long rangeX, long rangeY);
	void mendDisp();
	void GVSwap_WL(long iteration);
	void swapInRow_WL(long* origInfor, long& rwoIndex, mySubRow* & subOrigNow,
	    		       list<Inst*>::iterator& k, bool& done, bool vertical);
	void swapInRow_Density(long* origInfor, long& rowIndex, mySubRow* & subOrigNow,
		    		       list<Inst*>::iterator& k, bool& done);
	Inst* getPerm1(int idx,Inst* &Group,const long left,const long right);
	void binRefineH();
	void binRefineV();
	void binSpaceRefine();
	void assignToSites();

	//for debug
	void checkDisp();
	void checkBinDensity();
	void checkRowDensity();
	void checkOverlap();
public:
	void guiFile_DP(const char *fname);
//===============================hjy20130606=========================
	//Inst* finalPerm(int idx,Inst* &group,const int size,Inst* &pBuf,const long left,const long right);
	Inst*  getPerm(int idx, Inst* &Group,const int size,Inst* &pBuf,const long left,const long right);
	//int FullPermutation(Inst* Group,const int size,const int N,const long left,const long right,Inst** &oriGroup);
//============================================================================
	void guiRectGroup(const char *fname, vector<Rect>& rects);
private:
	bool visible_DP;
public:
	inline void setVisible_DP(bool v)
	{
				visible_DP=v;
	}
protected:
    Rect Area;
    long blockX;
    long blockY;
    long blockW;
    long blockH;
    vector<Inst*>insts;
 //   vector<myRow*>rows;
   // vector<myNet*>nets;
//===========================================
};

#endif /* LDPLACE_H_ */

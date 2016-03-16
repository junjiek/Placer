#ifndef _DPLACE_H_
#define _DPLACE_H_

#include "simPlPlace.h"
#include "ldpUtility.h"
#include <set>
#include <map>

enum
{
    fdplcSiteFreeType = 0,
    fdplcSiteInstType = 1,
    fdplcSiteBlockageType = 2
};
//==================hjy20130416=====================

enum
{
    fdplcValidPlace = 0,
    fdplcOverlapPlace = 1,
    fdplcOverlapWithFixedPlace = 2,
    fdplcOverflowPlace = 4,
    fdplcOutsidePlace = 8,
    fdplcUnmatchSitePlace = 16,
    fdplcOverlapWithBlockagePlace = 32
};

#ifndef ABS
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

//==================================================

struct Line
{
public:
    inline static bool leftCmp(const Line &line1, const Line &line2)
    {
	return line1.x < line2.x;
    }
    inline static bool rightCmp(const Line &line1, const Line &line2)
    {
	return (line1.x + line1.w) < (line2.x + line2.w);
    }
public:
    int x; //the start coordinate
    int w; //the width
};

template<class ValType>
class IndexSort
{
public:
    static bool compare(const IndexSort &v1, const IndexSort &v2);
public:
    size_t index;
    ValType val;
};

template<class ValType>
inline bool IndexSort<ValType>::compare(const IndexSort<ValType> &v1, 
				   const IndexSort<ValType> &v2)
{
    return v1.val < v2.val;
}

class fdplSiteContainer
{
public:
    fdplSiteContainer() : rows(0), rowSiteIndex(0), curIndex(0){}
    void addSite(myRow *row, unsigned long siteIndex);
    myRow *getNext(unsigned long &siteIndex);
    myRow *first(unsigned long &siteIndex);
    myRow *last(unsigned long &siteIndex);
    //===================hjy20130402====================
    unsigned long getCurIndex();
    //=================================================
    unsigned long size();
    void reset();
    void clear();
private:
    vector<myRow*> rows;
    vector<unsigned long> rowSiteIndex;
    unsigned long curIndex;

};

struct fdplFreeSite
{
    unsigned long start;
    unsigned long end;
    unsigned long width;
};

union fdplSiteObject
{
    Inst *inst;
};

struct fdplSiteContent
{
    fdplSiteContent();
    unsigned long type;
    fdplSiteObject obj;

};

class fdplInstSort
{
public:
    fdplInstSort();
    fdplInstSort(Inst *cell);
    static bool Compare(const fdplInstSort &inst1, const fdplInstSort &inst2);
public:
    Inst *inst;
    double wl;
    double widthCur;//hjy20130508
};

class fdplDetailPlace : public myPlacement
{
public:
    fdplDetailPlace(Block *blk);// : myPlacement(blk) {};
    inline ~fdplDetailPlace() {
	    if (instOfRows.size() > 0)
	    {
		    delete[] instOfRows[0];
		    delete[] instOutRows[0];
	    }
    }
//===================hjy20130115add===================
public:
	void guiFile_LG(const char* fname);
	//==================================================
public:
    myRow *getSite(myPoint &origin, unsigned long &siteIndex);
    unsigned long getSite(Inst *inst, fdplSiteContainer &siteCon);
    unsigned long getSite(myRow *src, unsigned long index, myRow *dst);

    unsigned long getFreeSite(myRow *row, vector<fdplFreeSite> &freeSites,
			unsigned long maxNumSites = 1000000);
    long getRowDist(myRow *row1, myRow *row2);
    myRow *getNextRow(myRow *row);
    myRow *getPreRow(myRow *row);
    unsigned long getInvalidInsts(vector<Inst*> &res, bool isRemove = false, 
	    bool isUnique = false);

    void init();
    void format();
    void clear();
    void removeInst(Inst *inst);
    unsigned long insertInstToRow(Inst *inst, myRow *row, unsigned long siteIndex);
    unsigned long insert(Inst *inst);
    void insertFixed(Inst *inst);
    void legalize();
public:
    myPoint getROrigin(myRow *row, unsigned long siteIndex);
    bool contains(Rect &rect, myPoint &point);
    bool isUnPlacedInst(Inst *inst);
    unsigned long getNumSite(Inst *inst, myRow *row);
    unsigned long getNumSite(unsigned long width, myRow *row);
    void printStatus(Inst *inst);
    inline unsigned long getSiteOffset(myRow *row, long offset);
protected:
    unsigned long getIndex(myRow *row);
    unsigned long getIndex(Inst *inst);
    unsigned long getIndex(myNet *net);
    unsigned long getInst(myRow *row, vector<Inst*> &res); 

public:
    bool place(vector<Inst*> &src, vector<Inst*> &failure);
    bool place(myRow *row, vector<Inst*> &throwInsts);
    void dplace();
    void setDefaultArg();
    void placeStds();
    void placeMacros();//hjy20130416
    void placeMacros(int MacroHeight);//hjy20130416
    void placeMacros(vector<Inst*> &src);//20130416
    static int getManhattanDist(int line1S, int line1T,
    		int line2S, int line2T);//======hjy20130416
    static int getManhattanDist(const Rect &box1, const Rect &box2);//hjy20130416
    void removeMacrosOverlap(int macroHeight);//hjy20130417
    bool removeOverlap(Inst *inst);//hjy20130417
    //bool disXCenter(Inst *ref1,Inst *ref2);//hjy20130516
    static void setOrigin(Inst *inst, const myPoint &origin);
    unsigned long getInvalidStdInsts(vector<Inst*> &invalidStdInsts);
    unsigned long getInvalidStdInsts(vector<Inst*> &src, 
	    vector<Inst*> &invalidStdInsts);
    void segmentPlace(unsigned long rowIndex, unsigned long startIndex, unsigned long endIndex,
	    vector<Inst*> &segInsts);
    void refineOrient();
    void guiFile(char* fname);

public:
    //============hjy20130513=================
    inline static bool xCenter(Inst *ref1,Inst *ref2)
    {
    	return ref1->getCenterX() < ref2->getCenterX();
    }
    //========================================
    //============================hjy20130530===========================
    inline static bool CompareW(Inst *inst1,Inst *inst2)
    {
    	return inst1->getWidth() > inst2->getWidth();
    }
    inline static bool CompareNumTerms(Inst *inst1,Inst *inst2)
    {
    	return inst1->getNumInstTerms() > inst2->getNumInstTerms();
    }
        //====================================================================



    //============hjy20130516==================
    /*inline  bool fdplDetailPlace::disXCenter(Inst *ref1,Inst *ref2)
    {
    	    	unsigned long Cx=(rowArea.left()>>1)+(rowArea.right()>>1);
    	    	long disx1=ref1->getCenterX()-Cx;
    	    	long disx2=ref2->getCenterX()-Cx;
    	    	if(disx1<0)
    	    		disx1=-disx1;
    	    	if(disx2<0)
    	    		disx2=-disx2;
    	    	return disx1<disx2;
    }*/
    //==========================================
    inline static bool xCompare(Inst *ref1, Inst *ref2)
    {
	    return ref1->getCoordX() < ref2->getCoordX();
    }

    inline static bool yCompare(Inst *ref1, Inst *ref2)
    {
	    return ref1->getCoordY() < ref2->getCoordY();
    }

    inline unsigned long getHPWL(Inst *inst) 
    {
	    unsigned long res(0);

	    vector<InstTerm> instTerm = inst->getInstTerms();
	    for (unsigned long i = 0 ; i < instTerm.size() ; i++)
	    {
		    unsigned long id = instTerm[i].getIndexNet();
		    res += myPlacement::getHPWL(nets[id]);
	    }
	    return res;
    }
	//=============hjy20130115add==========================
	inline void setVisible_LG(bool v)
	{
			visible_LG=v;
	}
	//====================================================

public:
    double getRateOfArea(unsigned long &totalInstArea, unsigned long &useSiteArea);

protected:
    Rect rowArea; //the boundary box of all the rows within block
    double density; //the density of rowArea
    vector<Inst*> insts;
    vector<myRow*> rows;
    vector<myNet*> nets;
    vector<Inst*> pins;
    vector<Inst*> stdCells;
    vector<Inst*> Macros;//hjy20130416
    map<unsigned long, vector<Inst*>*> macrosByHeight;//hjy20130417
    map<InstTerm*, myPoint> termPosition;//hjy20130417
    vector< vector<fdplSiteContent> * > instOfRows; 
    vector< vector< Inst * > * > instOutRows;
    vector<int> maxInsts;
    set<myNet*> excludedNets;
     
protected:
    unsigned long rowGap;
    long blockX;
    long blockY;
    long blockW;
    long blockH;

private:
    map<myRow*, unsigned long> rowIndexMap;

private:
    unsigned long defMaxNumMoveInsts;
    double offsetWeight;
    double overlapWeight;
    double maxSegLenCof;
	//================hjy20130115add================
private:
	bool visible_LG;
//=====================================================

};


//==========================hjy20130416=======================
inline int fdplDetailPlace::getManhattanDist(int line1S, int line1T,int line2S, int line2T)
{
    return max<int>(line1S, line2S) - min<int>(line1T, line2T);
}

inline int fdplDetailPlace::getManhattanDist(const Rect &box1,const Rect &box2)
{
	int x = getManhattanDist(box1.left(), box1.right(),
	                        box2.left(), box2.right());
	int y = getManhattanDist(box1.bottom(), box1.top(),
	                       box2.bottom(), box2.top());
   if (x < 0 && y < 0) {
       return x + y;//1;
   } else {
       return MAX(x, 0) + MAX(y, 0);
   }
}
//=============================================================

//========================hjy20130416=======================================
template<class ElemType>
class fdplCluster
{
public:
    fdplCluster();
    virtual ~fdplCluster();
    template<class RanIt>
    void init(RanIt begin, RanIt end,
	    bool (*isCluster)(const ElemType &e1, const ElemType &e2));
    template<class RanIt, class ObjType>
    void init(RanIt begin, RanIt end, const ObjType &obj,
	    bool (ObjType::*isCluster)(const ElemType &e1, const ElemType &e2)
	    const);
    template<class RanIt, class ObjType>
    void init(RanIt begin, RanIt end, ObjType &obj,
	    bool (ObjType::*isCluster)(const ElemType &e1, const ElemType &e2));
    size_t getNumCluster() const;
    const std::vector<std::set<ElemType>*>& getCluster() const;
protected:
    void destroy();
protected:
    std::vector<std::set<ElemType>*> cluster;
};

template<class ElemType>
inline fdplCluster<ElemType>::fdplCluster()
{
}

template<class ElemType>
inline fdplCluster<ElemType>::~fdplCluster()
{
    destroy();
}

template<class ElemType>
inline void fdplCluster<ElemType>::destroy()
{
    for (typename std::vector<std::set<ElemType>*>::iterator it =
	    cluster.begin(); it != cluster.end(); it++) {
	delete *it;
    }
    cluster.clear();
}

template<class ElemType>
inline size_t fdplCluster<ElemType>::getNumCluster() const
{
    return cluster.size();
}

template<class ElemType>
inline const std::vector<std::set<ElemType>*>&
fdplCluster<ElemType>::getCluster() const
{
    return cluster;
}

template<class ElemType>
template<class RanIt>
void fdplCluster<ElemType>::
init(RanIt begin, RanIt end,
	    bool (isCluster)(const ElemType &e1, const ElemType &e2))
{
    std::map<ElemType, std::set<ElemType>*> sets;
    std::set<ElemType>* pSet;
    std::set<ElemType>* e1;
    std::set<ElemType>* e2;
    destroy();
    for (RanIt it = begin; it != end; it++) {
	sets[*it] = NULL;
    }
    for (RanIt it = begin; it != end; it++) {
	for (RanIt j = it + 1; j != end; j++) {
	    e1 = sets[*it];
	    e2 = sets[*j];
	    if (e1 == e2 && e1) {
		continue;
	    }
	    if (isCluster(*it, *j)) {
		if (e1 == NULL && e2 == NULL) {
		    pSet = new std::set<ElemType>;
		    pSet->insert(*it);
		    pSet->insert(*j);
		    sets[*it] = sets[*j] = pSet;
		} else if (e1 != e2) {
		    if (e1 && e2) {
			pSet = e2;
			e1->insert(e2->begin(), e2->end());
			for (typename std::set<ElemType>::iterator k =
				pSet->begin(); k != pSet->end(); k++) {
			    sets[*k] = e1;
			}
			delete pSet;
		    } else if (e1) {
			e1->insert(*j);
			sets[*j] = e1;
		    } else {
			e2->insert(*it);
			sets[*it] = e2;
		    }
		}
	    }
	}
	if (!sets[*it]) {
	    pSet = new std::set<ElemType>;
	    pSet->insert(*it);
	    sets[*it] = pSet;
	}
    }
    for (RanIt it = begin; it != end; it++) {
	if (e1 = sets[*it]) {
	    cluster.push_back(e1);
	    for (typename std::set<ElemType>::iterator j = e1->begin();
		    j != e1->end(); j++) {
		sets[*j] = NULL;
	    }
	}
    }
}

template<class ElemType>
template<class RanIt, class ObjType>
void fdplCluster<ElemType>::
init(RanIt begin, RanIt end, const ObjType &obj,
	bool (ObjType::*isCluster)(const ElemType &e1, const ElemType &e2)
	const)
{
    std::map<ElemType, std::set<ElemType>*> sets;
    std::set<ElemType>* pSet;
    std::set<ElemType>* e1;
    std::set<ElemType>* e2;
    destroy();
    for (RanIt it = begin; it != end; it++) {
	sets[*it] = NULL;
    }
    for (RanIt it = begin; it != end; it++) {
	for (RanIt j = it + 1; j != end; j++) {
	    e1 = sets[*it];
	    e2 = sets[*j];
	    if (e1 == e2 && e1) {
		continue;
	    }
	    if ((obj.*isCluster)(*it, *j)) {
		if (e1 == NULL && e2 == NULL) {
		    pSet = new std::set<ElemType>;
		    pSet->insert(*it);
		    pSet->insert(*j);
		    sets[*it] = sets[*j] = pSet;
		} else if (e1 != e2) {
		    if (e1 && e2) {
			pSet = e2;
			e1->insert(e2->begin(), e2->end());
			for (typename std::set<ElemType>::iterator k =
				pSet->begin(); k != pSet->end(); k++) {
			    sets[*k] = e1;
			}
			delete pSet;
		    } else if (e1) {
			e1->insert(*j);
			sets[*j] = e1;
		    } else {
			e2->insert(*it);
			sets[*it] = e2;
		    }
		}
	    }
	}
	if (!sets[*it]) {
	    pSet = new std::set<ElemType>;
	    pSet->insert(*it);
	    sets[*it] = pSet;
	}
    }
    for (RanIt it = begin; it != end; it++) {
	if (e1 = sets[*it]) {
	    cluster.push_back(e1);
	    for (typename std::set<ElemType>::iterator j = e1->begin();
		    j != e1->end(); j++) {
		sets[*j] = NULL;
	    }
	}
    }
}

template<class ElemType>
template<class RanIt, class ObjType>
void fdplCluster<ElemType>::
init(RanIt begin, RanIt end, ObjType &obj,
	bool (ObjType::*isCluster)(const ElemType &e1, const ElemType &e2))
{
    std::map<ElemType, std::set<ElemType>*> sets;
    std::set<ElemType>* pSet;
    std::set<ElemType>* e1;
    std::set<ElemType>* e2;
    destroy();
    for (RanIt it = begin; it != end; it++) {
	sets[*it] = NULL;
    }
    for (RanIt it = begin; it != end; it++) {
	for (RanIt j = it + 1; j != end; j++) {
	    e1 = sets[*it];
	    e2 = sets[*j];
	    if (e1 == e2 && e1) {
		continue;
	    }
	    if ((obj.*isCluster)(*it, *j)) {
		if (e1 == NULL && e2 == NULL) {
		    pSet = new std::set<ElemType>;
		    pSet->insert(*it);
		    pSet->insert(*j);
		    sets[*it] = sets[*j] = pSet;
		} else if (e1 != e2) {
		    if (e1 && e2) {
			pSet = e2;
			e1->insert(e2->begin(), e2->end());
			for (typename std::set<ElemType>::iterator k =
				pSet->begin(); k != pSet->end(); k++) {
			    sets[*k] = e1;
			}
			delete pSet;
		    } else if (e1) {
			e1->insert(*j);
			sets[*j] = e1;
		    } else {
			e2->insert(*it);
			sets[*it] = e2;
		    }
		}
	    }
	}
	if (!sets[*it]) {
	    pSet = new std::set<ElemType>;
	    pSet->insert(*it);
	    sets[*it] = pSet;
	}
    }
    for (RanIt it = begin; it != end; it++) {
	if (e1 = sets[*it]) {
	    cluster.push_back(e1);
	    for (typename std::set<ElemType>::iterator j = e1->begin();
		    j != e1->end(); j++) {
		sets[*j] = NULL;
	    }
	}
    }
}
//=========================================================================
//==========================hjy20130416====================================
class fdplInstCluster : public fdplCluster<Inst*>
{
public:
    typedef Inst *ElemType;
    fdplInstCluster();
    void setMergeDist(unsigned long dist);
    bool compare(const ElemType &inst1, const ElemType &inst2) const;
private:
    unsigned long  mergeDist;
};

inline fdplInstCluster::fdplInstCluster():
    mergeDist(0)
{
}


inline void fdplInstCluster::setMergeDist(unsigned long  dist)
{
    mergeDist = dist;
}


inline bool fdplInstCluster::compare(const ElemType &inst1,	const ElemType &inst2)const
{
	Rect box1,box2;
	return (fdplDetailPlace::getManhattanDist(LdpUtility::getBBox(inst1),LdpUtility::getBBox(inst2)) < mergeDist);
}

inline void setOrigin(Inst *inst, const myPoint &origin)
{
	myPoint lowerLeft(LdpUtility::getBBox(inst).lowerLeft());
	myPoint point;
	point=inst->getOrigin();
	//point += origin - lowerLeft;

	point.setCoordX(point.coordX()+origin.coordX()-lowerLeft.coordX());
	point.setCoordY(point.coordY()+origin.coordY()-lowerLeft.coordY());
    inst->setOrigin(point);
}
//========================================================================================
#endif

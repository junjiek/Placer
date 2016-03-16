//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <place.h>
//
// myPlacement main entry classes
//
// Author: Wang Sifei
// History: 2012/02/15 created by Sifei
//*****************************************************************************
//*****************************************************************************
#if !defined _PLACE_H_
#define _PLACE_H_

#include "block.h"
#include "inst.h"
#include "net.h"
#include "instTerm.h"
#include "rect.h"
#include "point.h"

// One cannot directly initilize myPlacement object except for derived classes
class myPlacement
{
public:
    //initial placement for Block
    virtual void iplace(){};

    // global placement for Block
    virtual void gplace(){};

    // detail placement for Block
    virtual void dplace(){};

    // the main run entry
    // Global placement object need not to override it;
    // only detial placement need to override it
    virtual void run(){};
public:
    //static members
    // get origin of instance or instanterm/oaterm
    static inline void getOrigin(const InstTerm *term, myPoint &point)
    {
        InstTerm::getOrigin(term, point);
    };
    static inline myPoint getOrigin(const InstTerm *term)
    {
        myPoint point;
        InstTerm::getOrigin(term, point);
        return point;
    };
    static inline void getOrigin(const Inst *inst, myPoint &origin)
    {
        Inst::getOrigin(inst, origin);
    };
    static inline myPoint getOrigin(const Inst *inst)
    {
        myPoint point;
        Inst::getOrigin(inst, point);
        return point;
    };
    static inline void getOrigin(const myRow *row, myPoint &origin)
    {
        myRow::getOrigin(row, origin);
    }
    // set the lower-left origin
    static inline void setOrigin(Inst *inst, const myPoint &point)
    {
        Inst::setOrigin(inst, point);
    }
    // get orient of inst
    static inline Orient getOrient(const Inst *inst)
    {
        return Inst::getOrient(inst);
    }
    // set the orientation
    static inline void setOrient(Inst *inst, const Orient &orient)
    {
        Inst::setOrient(inst, orient);
    }
    static inline void getBBox(const Inst *inst, Rect &bbox)
    {
        Inst::getBBox(inst, bbox);
    }
    // HPWL total wirelength calculator
    // no array instance/nets considered, all bit nets assumed
    unsigned long getHPWL(myNet *net);
    double getHPWL(const Block *block);
    unsigned long getHPWL(const vector<myNet*>& netlist);
    static void getHPWL(const Block *block, unsigned long &total);
    static unsigned long getRowHeight(Block *block);
    void getInstsOfNet(myNet* net, vector<Inst*>& insts);
public:
	inline Rect& getRegion() {
		return region;
	}
	void setRegion();
	Inst* getInst(InstTerm& it);
	Inst* getInst(InstTerm *it);
	myNet* getNet(InstTerm& it);
	myNet* getNet(InstTerm *it);
	myPoint getOrigin(InstTerm& it);
	myPoint getOrigin(InstTerm *it);
	unsigned long getInstsArea();
protected:
    // cannot use this object directly, only derived objects feasible
    myPlacement(Block *topBlock);

    virtual ~myPlacement() {};

protected:
    // is valid placement object
    bool isValidCell(Inst *inst);
    bool isConnected(Inst *inst);
    // put to row for detailed placement
    inline void putInst2Row(Inst *inst, myRow *row)
    {
    };
protected:
    // for testing
    void printAllInsts();

protected:
    // the top block of the design
    Block *plcTopBlock;
    Rect region;

};

#endif

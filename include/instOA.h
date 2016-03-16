//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <inst.h>
//
// The inst related manipulations
// Please NOTE that, the setOrigin/Orient etc here are different from those
// of OA. 
// a) setOrient here are always keeping the lower-left origin not changed
// after any orientation changed; set Orient always set the abslute orient
// and is not the orient relative to the current
// b) setOrigin here are always changing the lower-left origin of the inst
// The same as those in Term class
//
// Author: Lu Yongqiang
// History: 2009/9/14 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_INST_H_) 
#define _SC_INST_H_

#include <iostream>
#include "oa/oaDesignDB.h"
#include "term.h"
#include "array.h"
#include "occTraverser.h"
using namespace std;
using namespace oa;

class InstOA
{
public:
    inline InstOA() { };
    inline ~InstOA() { };

public:
    static void reportInst(const oaInst *inst);

public:
    static void getAllOccInsts(oaOccurrence *top, Array<oaOccInst*> &insts)
    {
        OccInsts oalInst(top, insts);
    };
    // get boundary of the instance
    static void getMasterBoundary(const oaInst *inst, oaBox &bbox);
    // get area
    static inline double getArea(const oaInst *inst)
    {
        oaBox bbox;
        getMasterBoundary(inst, bbox);
        return ((double) bbox.getHeight()) * ((double)bbox.getWidth());
    }
    // get the master bounding box, equal to master bondary
    static inline void getMBox(const oaInst *inst, oaBox &bbox)
    {
        getMasterBoundary(inst, bbox);
    }
    // get the bounding box of the inst, with the real cooridinates on chip
    static void getBBox(const oaInst *inst, oaBox &bbox);
    // get lower-left origin
    static void getOrigin(const oaInst *inst, oaPoint &origin);
    // set lower-left origin
    static void setOrigin(oaInst *inst, const oaPoint &origin);
    // get orient
    static inline oaOrient getOrient(const oaInst *inst)
    {
        return inst->getOrient();
    };
    // set orientation, whenever this invoked, the inst will transformed 
    // according to the orientation
    static void setOrient(oaInst *inst, const oaOrient &orient);

public:
    // by default, the new inst will be located at (0, 0)
    static inline oaInst* create(oaBlock* block, oaDesign *master)
    {
        oaTransform trans(0, 0);
        return createInstInLayout(block, master, trans);
    };
    // this will create a inst located at (locX, locY) with default oreientation
    static inline oaInst *create(oaBlock *block, oaDesign *master,
            int locX, int locY)
    {
        oaTransform trans(locX, locY);
        return createInstInLayout(block, master, trans);
    };
    // this will need a inst location/orientation information in SoaTransform
    static inline oaInst* create(oaBlock* block, oaDesign *master,
            oaTransform &trans)
    {
        return createInstInLayout(block, master, trans);
    };

public:
    // inst utilities
    // get the unique output term
    static oaInstTerm* getUniqueOut(const oaInst *inst);

    // get unique sink (for repeaters)
    static oaInstTerm* getUniqueIn(const oaInst *inst);

    // get power pin
    static inline oaInstTerm* getPowerTerm(const oaInst *inst)
    {
        return getPGTerm(inst, oacPowerSigType);
    };
    // get power pin
    static inline oaInstTerm* getGroundTerm(const oaInst *inst)
    {
        return getPGTerm(inst, oacGroundSigType);
    };

    // get power or ground pin specified by pgSig
    static oaInstTerm* getPGTerm(const oaInst *inst, const oaSigTypeEnum &pgSig);

protected:
    static oaInst* createInstInLayout(oaBlock *block, oaDesign *master,
            oaTransform &trans);
};

#endif

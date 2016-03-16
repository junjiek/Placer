//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <occTraverser.h>
//
// Occ objects traverse utility
//
// Author: Lu Yongqiang
// History: 20010/1/19 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_OCC_TRAVERSE_H_) 
#define _SC_OCC_TRAVERSE_H_

#include <iostream>
#include "oa/oaDesignDB.h"
#include "array.h"
using namespace std;
using namespace oa;

// Base class for all kind of OccTraverser (inst, net, term...)
class OccTraverser : public oaOccTraverser
{
protected:
    inline OccTraverser(oaOccurrence *top)
        : oaOccTraverser(top),
          mTopOcc(top)
    {
        // add flags control here, by default is non_design_descend & post_order
    };

    inline ~OccTraverser() { };

protected:
    inline oaBoolean startInst(oaOccInst *inst)
    {
        return true;
    };
protected:
    oaOccurrence *mTopOcc;
};

// Util for uniquify design traverser
// Once this traverse is invoked, the design is also uniquified at the same time

// uniquify a design inst, that is flatten this design to the top design.
// Then every design inst of this design will be flattened
// NOTE: if a design is instanced by more than one times, any modification or
// analysis like sta/extr/ps must ensure this flatten procedure is invoked at
// first, otherwise the common master Design of those instances will be used,
// which will cause analysis error or unexpected modification propagation

// uniquify a module inst, that is make sure every module has only one single
// occurence, which can make sure any modifications in block domain (e.g. 
// bufferinig etc.) can be reflected properly into module domain.
class OccUniqTraverser : public OccTraverser
{
public:
    inline OccUniqTraverser(oaOccurrence *top)
        : OccTraverser(top)
    {
        traverse();
    }
    inline ~OccUniqTraverser() { };


    oaBoolean startInst(oaOccInst *inst);

};

// Util for traversing all occ objects

class OccInsts : public OccTraverser
{
public:
    inline OccInsts(oaOccurrence *top, Array<oaOccInst*> &insts)
        : OccTraverser(top),
          mAllInsts(insts)  
    {
        traverse(); 
    };
    inline ~OccInsts() { };

public:
    inline void processInst(oaOccInst *inst)
    {
        mAllInsts.add(inst);
    };

private:
    Array<oaOccInst*> &mAllInsts;
};

// Util for traversing all occ nets
class OccNets : public OccTraverser
{
public:
    inline OccNets(oaOccurrence *top, Array<oaOccNet*> &nets)
        : OccTraverser(top),
          mAllNets(nets)
    {
        traverse(); 
    };
    inline ~OccNets() { };

public:
    inline void processNet(oaOccNet *net)
    {
        mAllNets.add(net);
    };

private:
    Array<oaOccNet*> &mAllNets;
};

// Util for traversing all occ nets
class OccInstTerms : public OccTraverser
{
public:
    inline OccInstTerms(oaOccurrence *top, Array<oaOccInstTerm*> &terms)
        : OccTraverser(top),
          mAllTerms(terms)
    {
        traverse(); 
    };
    inline ~OccInstTerms() { };

public:
    inline void processInstTerm(oaOccInstTerm *term)
    {
        mAllTerms.add(term);
    };

private:
    Array<oaOccInstTerm*> &mAllTerms;
};

class OccTerms : public OccTraverser
{
public:
    inline OccTerms(oaOccurrence *top, Array<oaOccTerm*> &terms)
        : OccTraverser(top),
          mAllTerms(terms)
    {
        traverse(); 
    };
    inline ~OccTerms() { };

public:
    inline void processTerm(oaOccTerm *term)
    {
        mAllTerms.add(term);
    };

private:
    Array<oaOccTerm*> &mAllTerms;
};
#endif

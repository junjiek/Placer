//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <term.h>
//
// The term related manipulations
//
// Author: Lu Yongqiang
// History: 2009/8/28 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_TERM_H_) 
#define _SC_TERM_H_

#include <iostream>
#include "oa/oaDesignDB.h"
#include "diptr.h"
#include "array.h"
#include "occTraverser.h"

using namespace std;
using namespace oa;

//typedef DiPtr<oaInstTerm, oaTerm> NetTerm;
template <class InstTermT, class TermT>
class DiNetTerm : public DiPtr<InstTermT, TermT>
{
public:
    inline DiNetTerm() 
        : DiPtr<InstTermT, TermT>() {};
    inline DiNetTerm(const DiNetTerm &ptr) 
        : DiPtr<InstTermT, TermT>(ptr) { };
    inline DiNetTerm(InstTermT *ptr)
        : DiPtr<InstTermT, TermT>(ptr) { };
    inline DiNetTerm(TermT *ptr)
        : DiPtr<InstTermT, TermT>(ptr) { };
    inline ~DiNetTerm() {};

    // alias for get1() and get2()
    // get the oaInstTerm if it contains
    inline InstTermT* getInstTerm() const
    {
        return DiPtr<InstTermT, TermT>::get1();
    };
    // get the oaTerm if it caontains
    inline TermT* getTerm() const
    {
        return DiPtr<InstTermT, TermT>::get2();
    };


public:

    DiNetTerm& operator=(InstTermT *ptr)
    {
        this->main = ptr;
        this->type = DiPtr<InstTermT, TermT>::PTR_MAIN;
        return *this;
    };
    DiNetTerm& operator=(TermT *ptr)
    {
        this->subst = ptr;
        this->type = DiPtr<InstTermT, TermT>::PTR_SUBST;
        return *this;
    };
    bool operator==(const DiNetTerm &term)
    {
        return this->main == term.main;
    };

};

typedef DiNetTerm<oaInstTerm, oaTerm> NetTerm;
typedef DiNetTerm<oaOccInstTerm, oaOccTerm> OccNetTerm;

template <class NetT, class InstTermT, class TermT>
class DiNetTermIter
{
public:
    inline DiNetTermIter(const NetT *net)
        : ntitIt(net->getInstTerms()),
          nttIt(net->getTerms()),
          ntTravInst(true) { };
    inline ~DiNetTermIter(){ };


public:
    bool getNext(DiNetTerm<InstTermT, TermT > &nterm)
    {
        // traverse Inst Term
        if (ntTravInst) { 
            if (InstTermT *term = ntitIt.getNext()) {
                nterm = term;
                return true;
            }
            else {
                ntTravInst = false;
            }
        }

        // traverse oaTerm
        if (TermT *term = nttIt.getNext()) {
            nterm = term;
            return true;
        }

        return false;
    };
private:
    oaIter<InstTermT> ntitIt;
    oaIter<TermT> nttIt;
    // is traversing InstTerm
    bool ntTravInst;
};

typedef DiNetTermIter<oaNet, oaInstTerm, oaTerm> NetTermIter;
typedef DiNetTermIter<oaOccNet, oaOccInstTerm, oaOccTerm> OccNetTermIter;

class Term
{
public:
    enum TERM_TYPE {
        // all the same as oaTermType except TYPE_ALL
        oacInputTermType,
        oacOutputTermType,
        oacInputOutputTermType, 
        oacSwitchTermType,
        oacJumperTermType,
        oacUnusedTermType,
        oacTristateTermType,
        TYPE_ALL
    };

public:
    inline Term() {};
    inline ~Term() {};

public:
    // get all occ Terms in the design
    static inline void getAllOccTerms(oaOccurrence *top,
            Array<oaOccTerm*> &terms)
    {
        OccTerms allTerms(top, terms);
    };
    static inline void getAllOccInstTerms(oaOccurrence *top,
            Array<oaOccInstTerm*> &terms)
    {
        OccInstTerms allTerms(top, terms);
    };
    // get terminals with specified term type
    static void getTerms(const oaInst *inst, Array<oaInstTerm*> &instTerms,
            TERM_TYPE type = TYPE_ALL);

    // get the terminals with specified signal type
    static void getTerms(const oaInst *inst, Array<oaInstTerm*> &instTerms,
            const oaSigTypeEnum &sigType);

    static void getTerms(const oaNet *net, Array<NetTerm> &netTerms,
            TERM_TYPE type = TYPE_ALL);

    // only get inst terms
    static void getTerms(const oaNet *net, Array<oaInstTerm*> &netTerms,
            TERM_TYPE type = TYPE_ALL);

    // get the occ terminals with specified signal type
    static void getTerms(const oaOccNet *net, Array<OccNetTerm> &netTerms,
            TERM_TYPE type = TYPE_ALL);
    // only get occ inst terms
    static void getTerms(const oaOccNet *net, Array<oaOccInstTerm*> &netTerms,
            TERM_TYPE type = TYPE_ALL);

    static void getTerms(const oaDesign *master, Array<oaModTerm*> &terms,
            TERM_TYPE type = TYPE_ALL);

    static void getTerms(const oaDesign *master, Array<oaTerm*> &terms, 
            TERM_TYPE type); 

    // get one term only
    static oaTerm* getOneTerm(const oaDesign *master, 
            const oaSigTypeEnum &sigType);
public:
    inline static void getLowerLeft(const oaInstTerm *term, oaPoint &origin)
    {
        getOrigin(term, origin);
    }
    // This is always the master's term postion, with only N oreientation
    static void getOrigin(const oaTerm *term, oaPoint &origin);
    // get the absolute origin of the instTerm with respect to the chip
    static void getOrigin(const oaInstTerm *term, oaPoint &origin);
    // get the bbox of the master's term, only with respect to the master
    static void getBBox(const oaTerm *term, oaBox &bbox);
    // get the bbox of the instterm, absolute location, with respect to chip
    static void getBBox(const oaInstTerm *term, oaBox &bbox);
    static inline void getOrigin(const NetTerm &term, oaPoint &origin)
    {
        if (term.getInstTerm()) {
            getOrigin(term.getInstTerm(), origin);
        }
        else {
            getOrigin(term.getTerm(), origin);
        }
    };
};

#endif

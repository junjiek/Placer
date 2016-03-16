//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <term.cpp>
//
// The term related manipulations implementation
//
// Author: Lu Yongqiang
// History: 2009/8/28 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#include "term.h"
#include "base.h"
#include "baseMsg.h"
#include "inst.h"

void 
Term::getTerms(const oaInst *inst, Array<oaInstTerm*> &instTerms,
        TERM_TYPE type)
{
    instTerms.clear();

    if (type == TYPE_ALL) {
        oaIter<oaInstTerm> it(inst->getInstTerms());
        while (oaInstTerm *term = it.getNext()) {
            instTerms.add(term);
        }
    }
    else {
        oaIter<oaInstTerm> it(inst->getInstTerms());
        while (oaInstTerm *term = it.getNext()) {
            if (term->getTerm()->getTermType() == type) {
                instTerms.add(term);
            }
        }
    }
}

void 
Term::getTerms(const oaInst *inst, Array<oaInstTerm*> &instTerms,
            const oaSigTypeEnum &sigType)
{
    instTerms.clear();
    oaIter<oaInstTerm> it(inst->getMaster()->getTopBlock()->getTerms());
    while (oaInstTerm *iterm = it.getNext()) {
        oaTerm *term = iterm->getTerm();
        if (term && term->getNet() && term->getNet()->getSigType() == sigType) {
            instTerms.add(iterm);
        }
    }
}

void
Term::getTerms(const oaNet *net, Array<NetTerm> &netTerms,
            TERM_TYPE type)
{
    netTerms.clear();

    if (type == TYPE_ALL) {
        NetTermIter nit(net);
        NetTerm nterm;
        while (nit.getNext(nterm)) {
            netTerms.add(nterm);
        }
    }
    else {
        NetTermIter nit(net);
        NetTerm nterm;
        while (nit.getNext(nterm)) {
            if (nterm.getInstTerm()) {
                if (nterm.getInstTerm()->getTerm()->getTermType() == type) {
                    netTerms.add(nterm);
                }
            }
            else if (nterm.getTerm()) {
                if (nterm.getTerm()->getTermType() == type) {
                    netTerms.add(nterm);
                }
            }
        }
    }
}

void
Term::getTerms(const oaOccNet *net, Array<OccNetTerm> &netTerms,
            TERM_TYPE type)
{
    netTerms.clear();

    if (type == TYPE_ALL) {
        OccNetTermIter nit(net);
        OccNetTerm nterm;
        while (nit.getNext(nterm)) {
            netTerms.add(nterm);
        }
    }
    else {
        OccNetTermIter nit(net);
        OccNetTerm nterm;
        while (nit.getNext(nterm)) {
            if (nterm.getInstTerm()) {
                if (nterm.getInstTerm()->getTerm()->getTermType() == type) {
                    netTerms.add(nterm);
                }
            }
            else if (nterm.getTerm()) {
                if (nterm.getTerm()->getTermType() == type) {
                    netTerms.add(nterm);
                }
            }
        }
    }
}

void
Term::getTerms(const oaNet *net, Array<oaInstTerm*> &netTerms,
            TERM_TYPE type)
{
    netTerms.clear();

    if (type == TYPE_ALL) {
        oaIter<oaInstTerm> nit(net->getInstTerms());
        while (oaInstTerm *nterm = nit.getNext()) {
            netTerms.add(nterm);
        }
    }
    else {
        oaIter<oaInstTerm> nit(net->getInstTerms());
        while (oaInstTerm *nterm = nit.getNext()) {
            if (nterm->getTerm()->getTermType() == type) {
                netTerms.add(nterm);
            }
        }
    }
}

void
Term::getTerms(const oaOccNet *net, Array<oaOccInstTerm*> &netTerms,
            TERM_TYPE type)
{
    netTerms.clear();

    if (type == TYPE_ALL) {
        oaIter<oaOccInstTerm> nit(net->getInstTerms());
        while (oaOccInstTerm *nterm = nit.getNext()) {
            netTerms.add(nterm);
        }
    }
    else {
        oaIter<oaOccInstTerm> nit(net->getInstTerms());
        while (oaOccInstTerm *nterm = nit.getNext()) {
            if (nterm->getTerm()->getTermType() == type) {
                netTerms.add(nterm);
            }
        }
    }
}

void 
Term::getTerms(const oaDesign *master, Array<oaModTerm*> &terms,
        TERM_TYPE type)
{
    terms.clear();
    oaModule *top = master->getTopModule();
    if (!top) {
        cout<<"Error: no top module found on master design"<<endl;
        return;
    }

    if (type == TYPE_ALL) {
        oaIter<oaModTerm> it(top->getTerms());
        while (oaModTerm *term = it.getNext()) {
            terms.add(term);
        }
    }
    else {
        oaIter<oaModTerm> it(top->getTerms());
        while (oaModTerm *term = it.getNext()) {
            if (term->getTermType() == type) {
                terms.add(term);
            }
        }
    }
}

void 
Term::getTerms(const oaDesign *master, Array<oaTerm*> &terms,
        TERM_TYPE type)
{
    terms.clear();
    oaBlock *top = master->getTopBlock();
    if (!top) {
        return;
    }
    if (type == TYPE_ALL) {
        oaIter<oaTerm> it(top->getTerms());
        while (oaTerm *term = it.getNext()) {
            terms.add(term);
        }
    }
    else {
        oaIter<oaTerm> it(top->getTerms());
        while (oaTerm *term = it.getNext()) {
            if (term->getTermType() == type) {
                terms.add(term);
            }
        }
    }
}

oaTerm*
Term::getOneTerm(const oaDesign *master, const oaSigTypeEnum &sigType)
{
    oaBlock *top = master->getTopBlock();
    if (!top) {
        return NULL;
    }
    oaIter<oaTerm> it(top->getTerms());
    while (oaTerm *term = it.getNext()) {
        if (term->getNet() && term->getNet()->getSigType() == sigType) {
            return term;
        }
    }
    return NULL;
}

// This is always the master's term postion, with only N oreientation
void 
Term::getOrigin(const oaTerm *term, oaPoint &origin)
{
    origin.set(0, 0);
    if (!term) {
        return;
    }
    oaIter<oaPin> pinIter(term->getPins());
    oaPin *pin = pinIter.getNext();
    if (!pin)
    {
        // try to get oaPadCell
        if (term->getNet()) {
            oaIter<oaInstTerm> it(term->getNet()->getInstTerms());
            while (oaInstTerm *ct = it.getNext()) {
                if (ct->getInst()->getMaster()->getCellType() == 
                        oacPadCellType) {
                    getOrigin(ct, origin);
                    return;
                }
            }
        }
        static HashPtrSet<oaTerm*> *issuedPins = new HashPtrSet<oaTerm*>;
        if (issuedPins->add(const_cast<oaTerm*>(term))) {
            oaString name;
            term->getName(DB::getNS(), name);
            print(&db5, String::getBuffer(name));
            origin.x() = origin.y() = 0;
        }
        return;
    }
    oaBox bbox;
    oaIter<oaPinFig> figIt(pin->getFigs());
    oaPinFig *fig = figIt.getNext();
    if (fig) {
        fig->getBBox(bbox);
        origin = oaPoint(bbox.left(), bbox.bottom());
    }
}
void
Term::getOrigin(const oaInstTerm *term, oaPoint &origin)
{
    // first get the inst origin
    term->getInst()->getOrigin(origin);
    // then get the term relative orgin
    oaPoint tmOrigin;
    getOrigin(term->getTerm(), tmOrigin);
    oaTransform xtrans(0, 0);
    xtrans.orient() = term->getInst()->getOrient();
    tmOrigin.transform(xtrans);
    origin += tmOrigin;
}
// This is always the master's term BBox (all pins merge), 
// with only N oreientation
void 
Term::getBBox(const oaTerm *term, oaBox &bbox)
{
    bbox.makeZero();
    if (!term) {
        return;
    }
    oaIter<oaPin> pinIter(term->getPins());
    oaBox box;
    while (oaPin *pin = pinIter.getNext()) {
        oaIter<oaPinFig> figIt(pin->getFigs());
        while (oaPinFig *fig = figIt.getNext()) {
            fig->getBBox(box);
            if (bbox.hasNoArea()) {
                bbox = box;
            }
            else {
                bbox.merge(box);
            }
        }
    }

    // cannot find pins
    if (bbox.hasNoArea()) {
        // try to get oaPadCell
        if (term->getNet()) {
            oaIter<oaInstTerm> it(term->getNet()->getInstTerms());
            while (oaInstTerm *ct = it.getNext()) {
                if (ct->getInst()->getMaster()->getCellType() == 
                        oacPadCellType) {
                    getBBox(ct, bbox);
                    return;
                }
            }
        }
        static HashPtrSet<oaTerm*> *issuedPins = new HashPtrSet<oaTerm*>;
        if (issuedPins->add(const_cast<oaTerm*>(term))) {
            oaString name;
            term->getName(DB::getNS(), name);
            print(&db5, String::getBuffer(name));
        }
        return;
    }
}
void
Term::getBBox(const oaInstTerm *term, oaBox &bbox)
{
    // get the inst origin
    oaPoint origin;
    term->getInst()->getOrigin(origin);
    oaTransform xtrans(origin);
    // then get the term relative bbox
    getBBox(term->getTerm(), bbox);
    xtrans.orient() = term->getInst()->getOrient();
    bbox.transform(xtrans);
}

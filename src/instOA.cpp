//***********************************************************************void destroyInst(oaInst *inst);
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <inst.cpp>
//
// The inst related manipulations, implementation
//
// Author: Lu Yongqiang
// History: 2009/9/14 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "instOA.h"
#include "term.h"
#include "base.h"

oaInst* 
InstOA::createInstInLayout(oaBlock *block, oaDesign *master,
        oaTransform &trans)
{
    // at first process pg/wirelength and other placement constraints

    // then insert it to layout
    oaScalarInst *inst = oaScalarInst::create(block, master, trans);
    oaIter<oaTerm> it(inst->getMaster()->getTopBlock()->getTerms());
    while (oaTerm *term = it.getNext()) {
        oaInstTerm::create(NULL, inst, term);
    }
    return inst;
}

oaInstTerm*
InstOA::getUniqueOut(const oaInst *inst)
{
    Array<oaInstTerm*> terms;
    Term::getTerms(inst, terms, Term::oacOutputTermType);
    return (terms.getNumElements() == 1)? terms[0] : NULL;
}

oaInstTerm*
InstOA::getUniqueIn(const oaInst *inst)
{
    Array<oaInstTerm*> terms;
    Term::getTerms(inst, terms, Term::oacInputTermType);
    return (terms.getNumElements() == 1)? terms[0] : NULL;
}

oaInstTerm*
InstOA::getPGTerm(const oaInst *inst, const oaSigTypeEnum &pgSig)
{
    oaIter<oaInstTerm> it(inst->getMaster()->getTopBlock()->getTerms());
    while (oaInstTerm *iterm = it.getNext()) {
        oaTerm *term = iterm->getTerm();
        if (term && term->getNet() && term->getNet()->getSigType() == pgSig) {
            return iterm;
        }
    }
    return NULL;
}

void
InstOA::getMasterBoundary(const oaInst *inst, oaBox &bbox)
{
    bbox.makeZero();
    oaDesign *master = inst->getMaster();
    if (master) {
        oaBlock *blk = master->getTopBlock();

        if (blk) {
            oaSnapBoundary  *snapBoundary = oaSnapBoundary::find(blk);

            if (snapBoundary) {
                snapBoundary->getBBox(bbox);
            }
            else {
                oaPRBoundary    *prBoundary = oaPRBoundary::find(blk);
                if (prBoundary) {
                    prBoundary->getBBox(bbox);
                }
                else {
                    blk->getBBox(bbox);
                }
            }
        }
    }
}
// get lower-left origin
// oa::getOrigin is to get the coordinate of the origin point of the 
// master design cooridnate-system of the inst, which might not be the lower-left origin
void 
InstOA::getOrigin(const oaInst *inst, oaPoint &origin)
{
    oaBox       bBox;
    oaTransform tr(0, 0);

    inst->getOrigin(origin);
    getMasterBoundary(inst, bBox);

    switch (inst->getOrient()) {
    case oacR0:
        tr = oaTransform(bBox.left(), bBox.bottom());
        break;
    case oacR90:
        tr = oaTransform(-bBox.bottom() - bBox.getHeight(),
                         bBox.left());
        break;
    case oacR180:
        tr = oaTransform(-bBox.left() - bBox.getWidth(),
                         -bBox.bottom() - bBox.getHeight());
        break;
    case oacR270:
        tr = oaTransform(bBox.bottom(),
                         -bBox.left() - bBox.getWidth());
        break;
    case oacMY:
        tr = oaTransform(-bBox.left() - bBox.getWidth(),
                         bBox.bottom());
        break;
    case oacMYR90:
        tr = oaTransform(-bBox.bottom() - bBox.getHeight(),
                         -bBox.left() - bBox.getWidth());
        break;
    case oacMX:
        tr = oaTransform(bBox.left(),
                         -bBox.bottom() - bBox.getHeight());
        break;
    case oacMXR90:
        tr = oaTransform(bBox.bottom(),
                         bBox.left());
    }

    origin.transform(tr);
}

// this function always set the lower-left coordinate of the instance after
// the orientation commited
// So, after this set, you use Inst::getOrigin will always get the same lower-
// left origin coordinate as you set
void 
InstOA::setOrigin(oaInst *inst, const oaPoint &origin)
{
    // get the current lower-left origin
    oaPoint point;
    InstOA::getOrigin(inst, point);

    // transform the inst by the displace, forg records the displacement
    oaPoint forg = origin;
    forg -= point;

    // get the oa origin displacement and set it
    inst->getOrigin(point);
    point += forg;
    inst->setOrigin(point);
}
// set the absolute orient with the lower-left origin not changed
void 
InstOA::setOrient(oaInst *inst, const oaOrient &orient)
{
    if (inst->getOrient() == orient) {
        // no need to change
        return;
    }

    // first get the current LL origin
    oaPoint origin;
    InstOA::getOrigin(inst, origin);

    // then change orient
    inst->setOrient(orient);

    // map the inst back to the former LL origin
    InstOA::setOrigin(inst, origin);
}
void
InstOA::getBBox(const oaInst *inst, oaBox &bbox)
{
    // get the master
    getMasterBoundary(inst, bbox);
    // get origin
    oaPoint origin;
    inst->getOrigin(origin);
    oaTransform trans(origin, inst->getOrient());
    bbox.transform(trans);
}

void
InstOA::reportInst(const oaInst *inst)
{
    oaString instName;
    inst->getName(DB::getNS(), instName);
    cout<<"Report Inst: "<<instName<<endl;
    cout<<"1) Terms"<<endl;
    oaIter<oaInstTerm> it(inst->getInstTerms());
    while (oaInstTerm *term = it.getNext()) {
        oaString iname;
        term->getTermName(DB::getNS(), iname);
        cout<<"    InstTerm: "<<iname;
        if (term->getNet()) {
            term->getNet()->getName(DB::getNS(), iname);
            cout<<", linked Net: "<<iname;
        }
        cout<<endl;
    }

}

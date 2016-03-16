//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <net.cpp>
//
// The net related manipulations implementation
//
// Author: Lu Yongqiang
// History: 2009/9/14 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "netOA.h"
#include "baseMsg.h"
#include "base.h"

NetOA::NetOA()
{
}

NetOA::~NetOA()
{
}

NetTerm
NetOA::getDriver(const oaNet *net)
{
    Array<NetTerm> terms;
    Term::getTerms(net, terms, Term::oacOutputTermType);
    if (terms.getNumElements() > 1) {
        oaString name;
        net->getName(DB::getNS(), name);
        print(&db1, String::getBuffer(name));
    }
    else if (terms.getNumElements() == 0) {
        oaString name;
        net->getName(DB::getNS(), name);
        print(&db2, String::getBuffer(name));
        return NetTerm();
    }
    return terms[0];
}

oaInstTerm*
NetOA::getDriverInstTerm(const oaNet *net)
{
    Array<oaInstTerm*> terms;
    Term::getTerms(net, terms, Term::oacOutputTermType);
    if (terms.getNumElements() > 1) {
        oaString name;
        net->getName(DB::getNS(), name);
        print(&db1, String::getBuffer(name));
    }
    else if (terms.getNumElements() == 0) {
        return NULL;
    }
    return terms[0];
}

OccNetTerm
NetOA::getDriver(const oaOccNet *net)
{
    Array<OccNetTerm> terms;
    Term::getTerms(net, terms, Term::oacOutputTermType);
    if (terms.getNumElements() > 1) {
        oaString name;
        net->getNet()->getName(DB::getNS(), name);
        print(&db1, String::getBuffer(name));
    }
    else if (terms.getNumElements() == 0) {
        oaString name;
        net->getNet()->getName(DB::getNS(), name);
        print(&db2, String::getBuffer(name));
        return OccNetTerm();
    }
    return terms[0];
}

oaOccInstTerm*
NetOA::getDriverInstTerm(const oaOccNet *net)
{
    Array<oaOccInstTerm*> terms;
    Term::getTerms(net, terms, Term::oacOutputTermType);
    if (terms.getNumElements() > 1) {
        oaString name;
        net->getNet()->getName(DB::getNS(), name);
        print(&db1, String::getBuffer(name));
    }
    else if (terms.getNumElements() == 0) {
        return NULL;
    }
    return terms[0];
}

oaNet*
NetOA::getPower(const oaBlock *block)
{
    oaNet *net = oaNet::find(block, oaName(DB::getNS(), "VDD"));
    if (!net) {
        oaTerm *pterm = Term::getOneTerm(block->getDesign(), oacPowerSigType);
        net = pterm->getNet();
    }
    return net;
}

// get the bbox of net (if it is a bus net, the bbox of all bits will return)
void 
NetOA::getBBox(const oaNet *net, oaBox &bbox)
{
    oaPointArray terms;
    Array<oaBitNet*> bits;

    // collect all bits of the net (if it is bus/bundle net), otherwise only
    // includes itself
    getAllBits(net, bits);

    oaPoint pnt;
    for (UInt4 i = 0; i < bits.getNumElements(); i++) {
        // inst terms
        oaIter<oaInstTerm> iti(bits[i]->getInstTerms());
        while (oaInstTerm *term = iti.getNext()) {
            Term::getOrigin(term, pnt);
            terms.append(pnt);
        }

        // oaterms
        oaIter<oaTerm> itt(bits[i]->getTerms());
        while (oaTerm *term = itt.getNext()) {
            Term::getOrigin(term, pnt);
            terms.append(pnt);
        }
    }

    terms.getBBox(bbox);
}

oaNet*
NetOA::getGround(const oaBlock *block)
{
    oaNet *net = oaNet::find(block, oaName(DB::getNS(), "VSS"));
    if (!net) {
        oaTerm *gterm = Term::getOneTerm(block->getDesign(), oacGroundSigType);
        net = gterm->getNet();
    }
    return net;
}

void
NetOA::reportNet(const oaNet *net)
{
    const oaDefNS &ns(DB::getDefNS());
    cout<<""<<endl;
    try {
        // show multi-bit nets
        if (net->getNumBits() > 1) {
            cout<<"Checking net, multi-bit net checked, net bit number "<<net->getNumBits();
            cout<<"; the net bits are: "<<endl;
            Array<oaBitNet*> bits;
            getAllBits(net, bits);
            for (UInt4 i = 0; i < bits.getNumElements(); i++) {
                cout<<""<<endl;
                cout<<"Bit ["<<i<<"]:"<<endl;
                reportNet(bits[i]);
            }
            return;
        }
        oaIter<oaInstTerm> iit(net->getInstTerms(oacInstTermIterAll | 
                    oacInstTermIterEquivNets));
        oaString name, name2;
        net->getName(ns, name);
        cout<<"Checking net "<<name<<endl;
        while (oaInstTerm *term = iit.getNext()) {
            term->getTermName(ns, name);
            term->getInst()->getName(ns, name2);
            name2 += ":";
            name2 += name;
            cout<<"Inst term "<<name2<<endl;
        }
        oaIter<oaTerm> tit(net->getTerms(oacTermIterAll | 
                    oacTermIterEquivNets));
        while (oaTerm *term = tit.getNext()) {
            term->getName(ns, name);
            cout<<"Term "<<name<<endl;
        }
    }
    catch (oaException &exp) {
        cout<<"Error: "<<exp.getMsg()<<endl;
    }
}

void
NetOA::reportNet(const oaOccNet *net)
{
    const oaDefNS &ns(DB::getDefNS());
    cout<<""<<endl;
    try {
        // show multi-bit nets
        if (net->getNumBits() > 1) {
            cout<<"Checking Occ net, multi-bit occ net checked, net bit number "<<net->getNumBits();
            cout<<"; the net bits are: "<<endl;
            Array<oaOccBitNet*> bits;
            getAllBits(net, bits);
            for (UInt4 i = 0; i < bits.getNumElements(); i++) {
                cout<<""<<endl;
                cout<<"Bit ["<<i<<"]:"<<endl;
                reportNet(bits[i]);
            }
            return;
        }
        oaIter<oaOccInstTerm> iit(net->getInstTerms(oacInstTermIterAll | 
                    oacInstTermIterEquivNets));
        oaString name, name2;
        net->getPathName(ns, name);
        cout<<"Checking Occ net "<<name<<endl;
        net->getNet()->getName(ns, name);
        cout<<"Its parent oaNet "<<name<<endl;
        while (oaOccInstTerm *term = iit.getNext()) {
            term->getTermName(ns, name);
            term->getInst()->getPathName(ns, name2);
            name2 += ":";
            name2 += name;
            cout<<"Occ Inst term "<<name2<<endl;
        }
        // NotImplicit implies those terms not just connecting two endpoints
        // such as a bust bit term connecting only two pins, one outside and one inside hierarchy
        // Normally just use AllNotHidden
        oaIter<oaOccTerm> tit(net->getTerms(oacTermIterAll | 
                    oacTermIterEquivNets));
        while (oaOccTerm *term = tit.getNext()) {
            term->getName(ns, name);
            cout<<"Occ Term "<<name<<endl;
        }
    }
    catch (oaException &exp) {
        cout<<"Error: "<<exp.getMsg()<<endl;
    }
}
void 
NetOA::reportNet(oaOccurrence *top, const oaString &netName)
{
    oaName name(DB::getDefNS(), netName);
    try {
        oaOccNet *net = oaOccNet::find(top, name);
        if (net) {
            reportNet(net);
        }
        else {
            print(&db10, "No such net found");
        }
    }
    catch (oaException &exp) {
        print(&db50, String::getBuffer(exp.getMsg()));
    }
}

void 
NetOA::reportNet(oaBlock *top, const oaString &netName)
{
    oaName name(DB::getDefNS(), netName);
    try {
        oaNet *net = oaNet::find(top, name);
        if (net) {
            reportNet(net);
        }
        else {
            print(&db10, "No such net found");
        }
    }
    catch (oaException &exp) {
        print(&db50, String::getBuffer(exp.getMsg()));
    }
}

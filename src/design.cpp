//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <design.h>
//
// The design/flow/database manapulation class
//
// Author: Lu Yongqiang
// History: 2009/5/27 created by Yongqiang
//          2010/6/12 mainly revised by Yongqiang, Design::Open/save/close...
//*****************************************************************************
//*****************************************************************************
#include "design.h"
#include "sessionManager.h"
#include "staticData.h"
#include "baseMsg.h"
#include "net.h"
#include "inst.h"
#include "term.h"

oaDesign* Design::desCurrent = NULL;
DBExtension<oaBlock>* Design::dsvDataSheet = NULL;
oaIntAppDef<oaInst>* Design::dsvInstIdDef = NULL;
oaIntAppDef<oaRow>* Design::dsvRowPGDef = NULL;
const char* Design::DESIGN_DATA_TABLE = "DESIGN_DATA_TABLE";
const char* Design::DESIGN_INST_ID = "DESIGN_INST_ID";
const char* Design::DESIGN_ROW_PG = "DESIGN_CELLROW_PG";
const char* Design::DESIGN_SDC_FILE = "DESIGN_SDC_FILE";


// defined in main.cpp for how tool flow management
extern SessionManager *scgSessionMan;

static Design::DesignStatics sdsStaticData;

//==================================hjy20131026================================

void
Design::saveAs(const string &libNameIn,
                 const string &cellNameIn,
                 const string &viewNameIn, const string &newNameIn)
{
	oaNativeNS &oaNS(DB::getNS());
	oaDesign *tempDesign=NULL;

	oaString libStr(libNameIn.c_str());
	oaString cellStr(cellNameIn.c_str());
	oaString viewStr(viewNameIn.c_str());
	//oaString newStr("random");
	oaString newStr(newNameIn.c_str());


	oaScalarName libName(oaNS,libStr);
	oaScalarName cellName(oaNS,cellStr);
	oaScalarName viewName(oaNS,viewStr);
	oaScalarName newName(oaNS, newStr);

	tempDesign=oaDesign::find(libName,cellName,viewName);
	if(tempDesign)
	{
		tempDesign->saveAs(libName ,cellName,newName);
	}
}
//=============================================================================

oaDesign* 
Design::open(const string &library, const string &cell,
          const string &view, const char mode)
{
    oaNativeNS &oaNS(DB::getNS());
    oaDesign *design = NULL;
    bool needReset = true;
    try {
        oaString        libstr(library.c_str());
        oaString        cellstr(cell.c_str());
        oaString        viewstr(view.c_str());

        oaScalarName    libName(oaNS, libstr);
        oaScalarName    cellName(oaNS, cellstr);
        oaScalarName    viewName(oaNS, viewstr);

        design = oaDesign::find(libName, cellName, viewName);
        if (design) { 
            // has opened
            if (design->getMode() == 'r' && mode == 'a') {
                // turn to write
                design->reopen('a');
            }
            if (design == desCurrent) {
                // just the current design, need not to reset session data
                needReset = false;
            }
        }
        else {
            // not opened yet
            // first close the opened design
            design = oaDesign::open(libName, cellName, viewName, oaChar(mode));
        }
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }

    if (!design) {
        cerr<<"Error: Unable open lib: "<<library<<" cell: "<<cell<<" view: "<<view<<" with mode: "<<mode<<endl;
        return NULL;
    }
    // set up design current object
    desCurrent = design;

    // everytime opening a design, it implies all static data or session data
    //  relying on design need to be reset
    if (needReset) {
        scgSessionMan->reset();
    }
    return design;
}

// open a non-top design
oaDesign* 
Design::openNormal( const oaScalarName &libName,
        const oaScalarName &cellName,
        const oaScalarName &viewName,
        const char mode)
{
    oaDesign *design = oaDesign::find(libName, cellName, viewName);
    if (design) { 
        // has opened
        if (design->getMode() == 'r' && mode == 'a') {
            // turn to write
            design->reopen('a');
        }
    }
    else {
        design = oaDesign::open(libName, cellName, viewName, oaChar(mode));
    }
    return design;
}


// set SDC file path by user
void 
Design::setSDCFile(oaDesign *design, const char *filePath)
{
    // first check if the sdc file dm object exists
    oaString name(DESIGN_SDC_FILE);
    try {
        if (!design->getLib()->getAccess(oacWriteLibAccess)) {
            print(&db51, "on writing design library");
            return;
        }
        oaDMFile *dmFile = oaDMFile::find(design->getLib(), name);
        if (dmFile) {
            dmFile->destroy();
        }
        // now create a new one
        dmFile = oaDMFile::create(design->getLib(), name, filePath);
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }
    design->getLib()->releaseAccess();
}

// get SDC file name
void
Design::getSDCFile(oaDesign *design, String &filePath)
{
    filePath = "";
    oaString name(DESIGN_SDC_FILE);
    try {
        if (!design->getLib()->getAccess(oacReadLibAccess)) {
            print(&db51, "on writing design library");
            return;
        }
        oaDMFile *dmFile = oaDMFile::find(design->getLib(), name);
        if (dmFile) {
            dmFile->getPath(name);
            filePath = String::getBuffer(name);
        }
        else {
            filePath = DB::NULL_FILE;
            oaString name;
            design->getCellName(DB::getNS(), name);
            print(&db4, String::getBuffer(name));
        }
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }
    design->getLib()->releaseAccess();
}

Design::DesignStatus 
Design::getDesignStatus(oaBlock *block)
{
    int status;
    if (dsvDataSheet) {
        dsvDataSheet->getValue(DESIGN_STATUS, block, status);
    }
    return (DesignStatus) status;
}

void
Design::setDesignStatus(oaBlock *block, DesignStatus status)
{
    if (dsvDataSheet) {
        dsvDataSheet->setValue(DESIGN_STATUS, block, (int) status);
    }
    else {
        cout<<"Error: no design opened yet"<<endl;
    }
}


//static member setup, to new the data sheet
void 
Design::DesignStatics::setup() 
{
    Array<DBExtAttrType> attrHdrs(NUM_OF_DATA);
    // design status attr 
    attrHdrs[DESIGN_STATUS] = DB::EXT_ATTR_INT; 

    dsvDataSheet = new DBExtension<oaBlock>(Design::getCurrent(), 
            DESIGN_DATA_TABLE, attrHdrs);

    dsvInstIdDef = oaIntAppDef<oaInst>::get(DESIGN_INST_ID);
    if (!dsvInstIdDef) {
        cerr<<"Error: Design instance ID Def "<<DESIGN_INST_ID<<" not found"<<endl;
    }

    dsvRowPGDef = oaIntAppDef<oaRow>::get(DESIGN_ROW_PG);
    if (!dsvRowPGDef) {
        cerr<<"Error: Design Row PG Def "<<DESIGN_ROW_PG<<" not found"<<endl;
    }
}


//static member clean
void 
Design::DesignStatics::clear() 
{
    // clear data sheet object (but not clear the database extension table)
    if (dsvDataSheet) {
        delete dsvDataSheet;
        dsvDataSheet = NULL;
    }

    dsvInstIdDef = NULL;
    dsvRowPGDef = NULL;
}

void
Design::assembleInstID(Array<oaInst*> instArray, UInt4 shift)
{
    assert(dsvInstIdDef);

    for (UInt4 i = 0; i < instArray.getNumElements(); i++) {
        dsvInstIdDef->set(instArray[i], (int) (i + shift));
    }
}

oaCellView* 
Design::getCellView(oaCell *cell)
{
    oaIter<oaCellView> it(cell->getCellViews());
    if (oaCellView *view = it.getNext()) {
        return view;
    }
    return NULL;
}

oaCellView* 
Design::getCellView(oaCell *cell, 
            oaReservedViewTypeEnum type)
{
    oaIter<oaCellView> it(cell->getCellViews());
    while (oaCellView *view = it.getNext()) {
        if (view->getView()->getViewType() == oaViewType::get(type)) { 
            return view; 
        }
    }
    return NULL;
}
oaScalarName 
Design::getCellViewName(oaCellView *view)
{
    if (!view) {
        return oaScalarName();
    }
    oaScalarName name;
    view->getView()->getName(name);
    return name;
}

// For report data and testing
// report design data
void
Design::reportDesignData(oaDesign *design,
        const bool checkNet,
        const bool completeCheck,
        const bool checkInst)
{
    oaString name;
    design->getCellName(DB::getNS(), name);
    String rptStr("Checking design %s", String::getBuffer(name));
    print(&db10, rptStr.c_str());

    // report block instance bound
    oaBlock *topBlk = design->getTopBlock();
    oaIter<oaInst> iit(topBlk->getInsts());
    UInt4 totalNum = 0, bindedNum = 0;
    oaString iname;
    while (oaInst *inst = iit.getNext()) {
        totalNum++;
        inst->getName(DB::getNS(), iname);
        bool instBound = true;

        if (checkInst) {
            InstOA::reportInst(inst);
        }

        if (inst->getMaster()) {
            // get instTerms and their terms
            oaIter<oaInstTerm> it2(inst->getInstTerms());
            while (oaInstTerm *term = it2.getNext()) {
                if (!term->getTerm()) {
                    //Net::reportNet(term->getNet());
                    if (!term->getBit(0)->getTerm()) {
                        term->getTermName(DB::getNS(), name);
                        String unbstr("Unbound cell instTerm %s found on inst %s", String::getBuffer(name), String::getBuffer(iname));
                        print(&db10, unbstr.c_str());
                        instBound = false;
                    }
                }
            }
        }
        else {
            instBound = false;
        }
        if (instBound) {
            bindedNum++;
        }
        else {
            String unbstr("Unbound cell %s found", String::getBuffer(iname));
            print(&db10, unbstr.c_str());
        }
    }


    rptStr.format("There are %d cells and %d of them are bound to masters",
            totalNum, bindedNum);
    print(&db10, rptStr.c_str());

    if (totalNum != bindedNum) {
        print(&db52);
    }

    if (completeCheck) {
        // report module instance bound
        oaModule *topMod = design->getTopModule();
        oaIter<oaModInst> mit(topMod->getInsts());
        totalNum = 0;
        bindedNum = 0;
        while (oaModInst *inst = mit.getNext()) {
            totalNum++;
            inst->getName(DB::getNS(), iname);
            bool instBound = true;
            if (inst->getMasterModule()) {
                // get instTerms and their terms
                oaIter<oaModInstTerm> it2(inst->getInstTerms());
                while (oaModInstTerm *term = it2.getNext()) {
                    if (!term->getTerm()) {
                        //Net::reportNet(term->getNet());
                        if (!term->getBit(0)->getTerm()) {
                            term->getTermName(DB::getNS(), name);
                            String unbstr("Unbound Module instTerm %s found on inst %s", String::getBuffer(name), String::getBuffer(iname));
                            print(&db10, unbstr.c_str());
                            instBound = false;
                        }
                    }
                }
            }
            else {
                instBound = false;
            }
            if (instBound) {
                bindedNum++;
            }
            else {
                String unbstr("Unbound cell module %s found", String::getBuffer(iname));
                print(&db10, unbstr.c_str());
            }
        }

        rptStr.format("There are %d cell modules and %d of them are bound to masters",
                totalNum, bindedNum);
        print(&db10, rptStr.c_str());

        if (totalNum != bindedNum) {
            print(&db52);
        }

        // report occ instance bound
        Array<oaOccInst*> allOccInsts;
        InstOA::getAllOccInsts(design->getTopOccurrence(), allOccInsts);
        totalNum = allOccInsts.getNumElements();
        bindedNum = 0;
        for (UInt4 i = 0; i < totalNum; i++) {
            oaOccInst *inst = allOccInsts[i];
            inst->getName(DB::getNS(), iname);
            bool instBound = true;
            oaModInst *modInst = inst->getModInst();
            if (modInst->getMasterModule()) {
                // get instTerms and their terms
                oaIter<oaOccInstTerm> it2(inst->getInstTerms());
                while (oaOccInstTerm *term = it2.getNext()) {
                    if (!term->getBit(0)->getModInstTerm() || !term->getBit(0)->getModInstTerm()->getTerm()) {
                        term->getTermName(DB::getNS(), name);
                        String unbstr("Unbound Module instTerm %s found on inst %s", String::getBuffer(name), String::getBuffer(iname));
                        print(&db10, unbstr.c_str());
                        instBound = false;
                    }
                }
            }
            else {
                instBound = false;
            }
            if (instBound) {
                bindedNum++;
            }
            else {
                String unbstr("Unbound cell Occ-module %s found", String::getBuffer(iname));
                print(&db10, unbstr.c_str());
            }
        }

        rptStr.format("There are %d occ-cell modules and %d of them are bound to masters",
                totalNum, bindedNum);
        print(&db10, rptStr.c_str());

        if (totalNum != bindedNum) {
            print(&db52);
        }
    }

    // iterate nets
    if (checkNet) {
        HashPtrSet<oaNet*> visited;
        oaIter<oaNet> nit(topBlk->getNets());
        while (oaNet *net = nit.getNext()) {
            if (net->getNumBits() > 1) {
                cout<<"Checking net, multi-bit net checked, net bit number "<<net->getNumBits();
                cout<<"; the net bits are: "<<endl;
                Array<oaBitNet*> bits;
                NetOA::getAllBits(net, bits);
                for (UInt4 i = 0; i < bits.getNumElements(); i++) {
                    cout<<""<<endl;
                    cout<<"Bit ["<<i<<"]:"<<endl;
                    if (!visited.add(bits[i])) {
                        cout<<"Repeat ";
                    }
                    NetOA::reportNet(bits[i]);
                }
            }
            else {
                if (!visited.add(net)) {
                    cout<<"Repeat ";
                }
                NetOA::reportNet(net);
            }
        }
        cout<<"Net num "<<visited.getNumElements()<<endl;
    }


    uniquify(design);

    if (checkNet && completeCheck) {
        cout<<"Check Occ Nets"<<endl;
        Array<oaOccNet*> allNets;
        NetOA::getAllOccNets(design->getTopOccurrence(), allNets);
        cout<<"OCC Net num "<<allNets.getNumElements()<<endl;
        if (checkNet) {
            for (UInt4 i = 0; i < allNets.getNumElements(); i++) {
                NetOA::reportNet(allNets[i]);
            }
        }

        // check oaOccTerms
        Array<oaOccTerm*> allTerms;
        Term::getAllOccTerms(design->getTopOccurrence(), allTerms);
        cout<<"OCC Term num "<<allTerms.getNumElements()<<endl;
    }
}

void
Design::reportTerms(oaDesign *design)
{
    oaString dname;
    design->getCellName(DB::getNS(), dname);
    cout<<"Checking terms of design "<<dname<<endl;
    oaIter<oaTerm> tit(design->getTopBlock()->getTerms());
    while (oaTerm *term = tit.getNext()) {
        oaString name;
        term->getName(DB::getNS(), name);
        cout<<"    Term name: "<<name<<endl;
    }
}

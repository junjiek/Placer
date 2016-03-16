//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tech.cpp>
//
// The design/flow/database manapulation class
//
// Author: Lu Yongqiang
// History: 2009/12/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#include "tech.h"
#include "logfile.h"
#include "baseMsg.h"
#include "design.h"

static Array<oaScalarName> oldCellName;
static Array<oaScalarName> newCellName;
const char* Tech::tecgLIBFileNameFormat = "TechLIBFile_%d_%d";

oaTech* 
Tech::open(const string &library, const char mode)
{
    oaTech *tech = NULL;
    try {
        oaString        libstr(library.c_str());
        oaScalarName    libName(DB::getNS(), libstr);

        tech = oaTech::find(libName);
        if (tech) {
            if (tech->getMode() == 'r' && mode == 'a') {
                // turn to write
                tech->reopen('a');
            }
        }
        else { 
            // not opened yet
            tech = oaTech::open(libName, mode);
        }
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }

    return tech;
}
void
Tech::clearLibertyFiles(oaTech *tech, CellLibTMode mode)
{
    UInt4 id = 0;
    try {
        while (1) {
            String fileName;
            fileName.format(tecgLIBFileNameFormat, mode, id);
            oaString name(fileName.c_str());
            oaDMFile *dmFile = oaDMFile::find(tech->getLib(), name);
            if (dmFile) {
                dmFile->destroy();
            }
            else {
                break;
            }
            id++;
        }
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }
}

void
Tech::setLibertyFiles(oaTech *tech, 
            CellLibTMode mode, Array<String> &filePaths)
{
    // get library access
    try {
        if (!tech->getLib()->getAccess(oacWriteLibAccess)) {
            print(&db51, "on writing tech library");
            return;
        }

        // First clear all the existance
        clearLibertyFiles(tech, mode);

        // set new ones
        for (UInt4 i = 0; i < filePaths.getNumElements(); i++) {
            // judge source file exist
            if (DiskFile::exist(filePaths[i].c_str())) {
                String fileName;
                fileName.format(tecgLIBFileNameFormat, mode, i);
                oaString name(fileName.c_str());
                // now create a new one
                oaDMFile::create(tech->getLib(), name, filePaths[i].c_str());
            }
        }
        tech->getLib()->releaseAccess();
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }
}

// get .lib file names
void
Tech::getLibertyFiles(oaTech *tech, CellLibTMode mode, Array<String> &filePaths)
{
    // get library access
    try {
        if (!tech->getLib()->getAccess(oacReadLibAccess)) {
            print(&db51, "on reading tech library");
            return;
        }

        // get all LIB files
        UInt4 id = 0;
        while (1) {
            String fileName;
            fileName.format(tecgLIBFileNameFormat, mode, id);
            oaString name(fileName.c_str());
            oaDMFile *dmFile = oaDMFile::find(tech->getLib(), name);
            if (dmFile) {
                dmFile->getPath(name);
                filePaths.add(String(name));
            }
            else {
                break;
            }
            id++;
        }
        tech->getLib()->releaseAccess();
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }
}

// get all lib files
Array<String> 
Tech::getAllRefLibertyFiles(oaTech *mainTech, CellLibTMode mode)
{
    Array<String> files;

    // get parent's at first
    getLibertyFiles(mainTech, mode, files);

    // get all tech object
    oaTechHeaderArray techs;
    mainTech->getTechHeaders(techs);

    // collect all liberty file names corresponding to techs
    for (UInt4 i = 0; i < techs.getNumElements(); i++) {
        getLibertyFiles(techs[i]->getRefTech(), mode, files);
    }
    return files;
}

// only judge the equal masters by port number and port types
static bool checkEqualMasterByPorts(oaDesign *master1,
        oaDesign *master2,
        HashPtr<oaTerm*, oaTerm*> *portMap = NULL)
{
    // check term type and number
    Array<oaTerm*> ports1[oavNumTermTypeEnums];
    Array<oaTerm*> ports2[oavNumTermTypeEnums];

    memset(ports1, 0, sizeof(int) * oavNumTermTypeEnums);
    memset(ports2, 0, sizeof(int) * oavNumTermTypeEnums);

    oaIter<oaTerm> it1(master1->getTopBlock()->getTerms());
    while (oaTerm *term = it1.getNext()) {
        ports1[term->getTermType()].add(term);;
    }

    oaIter<oaTerm> it2(master2->getTopBlock()->getTerms());
    while (oaTerm *term = it2.getNext()) {
        ports2[term->getTermType()].add(term);
    }

    for (UInt4 i = 0; i < oavNumTermTypeEnums; i++) {
        if (ports1[i].getNumElements() != ports2[i].getNumElements()) {
            return false;
        }
    }

    // check net use type
    int use1[oavNumSigTypes];
    int use2[oavNumSigTypes];

    for (UInt4 i = 0; i < oavNumTermTypeEnums; i++) {
        memset(use1, 0, sizeof(int) * oavNumSigTypes);
        memset(use2, 0, sizeof(int) * oavNumSigTypes);
        for (UInt4 j = 0; j < ports1[i].getNumElements(); j++) {
            use1[ports1[i][j]->getNet()->getSigType()]++;
            use2[ports2[i][j]->getNet()->getSigType()]++;
        }
        for (UInt4 j = 0; j < oavNumSigTypes; j++) {
            if (use1[j] != use2[j]) {
                return false;
            }
        }
    }


    if (portMap) {
        // create port map
        for (UInt4 i = 0; i < oavNumTermTypeEnums; i++) {
            for (UInt4 j = 0; j < ports1[i].getNumElements(); j++) {
                for (UInt4 k = 0; k < ports2[i].getNumElements(); k++) {
                    if (ports2[i][k] && 
                            ports1[i][j]->getNet()->getSigType() == 
                            ports2[i][k]->getNet()->getSigType()) {
                        portMap->add(ports1[i][j], ports2[i][k]);
                        ports2[i][k] = NULL;
                        break;
                    }
                }
            }
        }
        // check
        it2.reset();
        while (oaTerm *term = it2.getNext()) {
            oaTerm *mapped;
            portMap->get(term, mapped);
            if (!mapped) {
                oaString name;
                term->getName(DB::getNS(), name);
                cout<<"Error: Ref No mapped equal port found on "<<name<<endl;
            }
        }
    }

    return true;
}

// change the insts of header by the master which is revised by refMaster's port
static
void changeMasterPortsAsRef(oaInstHeader *header, 
        oaDesign *master, oaDesign *refMaster ,bool reCreateTerm)
{
    oaBlock *topBlock = header->getDesign()->getTopBlock();
    //1. First, change the down master's ports as ref master

    // key is the ref master port, value is toRevise port
    HashPtr<oaTerm*, oaTerm*> portMap;
    // find equal port maps, refMaster is the key
    checkEqualMasterByPorts(refMaster, master, &portMap);

    HashPtr<oaTerm*, oaNet*> portNetMap;
    HashPtr<oaTerm*, oaName> portNameMap;
    oaIter<oaTerm> it(refMaster->getTopBlock()->getTerms());
    while (oaTerm *term = it.getNext()) {
        oaTerm *mapped = NULL;
        portMap.get(term, mapped);
        assert(mapped);
        portNetMap.add(term, mapped->getNet());
        oaName name;
        mapped->getName(name);
        portNameMap.add(term, name);
    }

    if(reCreateTerm) {
        // destroy current ports
        oaIter<oaTerm> it1(master->getTopBlock()->getTerms());
        while (oaTerm *term = it1.getNext()) {
            term->destroy();
        }

        // create new ports as ref
        it.reset();
        while (oaTerm *term = it.getNext()) {
            oaNet *net = NULL;
            portNetMap.get(term, net);
            assert(net);
            oaName name;
            term->getName(name);
            oaTerm::create(net, name, term->getTermType());
        }
    }

    // 2. Then, rebind the instance's instTerms as down master new ports

    oaIter<oaInst> hit(header->getInsts());
    while (oaInst *inst = hit.getNext()) {
        //cout<<"Before remaster"<<endl;
        //Inst::reportInst(inst);
        HashPtr<oaTerm*, oaNet*> portInNetMap;
        // setup the inst term net and the ref master port map
        HashPtrIter<oaTerm*, oaName> hnit(portNameMap);
        oaTerm *port;
        oaName name;
        while (hnit.getNext(port, name)) {
            oaInstTerm *inTerm = oaInstTerm::find(inst, name);
            if (inTerm) {
                portInNetMap.add(port, inTerm->getNet());
            }
            else {
                portInNetMap.add(port, NULL);
            }
        }

        oaInst *newInst = InstOA::create(topBlock, master);


        // create new inst terms as to the new names
        HashPtrIter<oaTerm*, oaNet*> tnit(portInNetMap);
        oaNet *net = NULL;
        while (tnit.getNext(port, net)) {
            port->getName(name);
            oaString tmp1;
            name.get(DB::getNS(), tmp1);
            oaInstTerm *inTerm = oaInstTerm::find(newInst, name);
            assert(inTerm);
            if (net) {
                inTerm->addToNet(net);
            }
        }

        inst->destroy();

        //cout<<"After remaster"<<endl;
        //Inst::reportInst(newInst);
    }

}

// find equal entities from refTech for given master
static void 
findEqualMasters(oaDesign *master, oaTech *refTech,
        Array<oaDesign*> &equals,
        const UInt4 maxFound)
{
    oaScalarName libNameFrom, cellNameFrom, viewNameFrom;
    UInt4 found = 0;

    // get access
    oaLib *libFrom = refTech->getLib();
    libFrom->getName(libNameFrom);

    if (!libFrom->getAccess(oacReadLibAccess)) {
        cout<<"Error: Connot open the techFrom library"<<endl;
        return;
    }
    // check the master to refTech's cells
    oaIter<oaCell> iterCellsFrom(libFrom->getCells());
    while (oaCell *cellFrom = iterCellsFrom.getNext()) {
        cellFrom->getName(cellNameFrom);
        oaDesign *masterRef = Design::openNormal(libNameFrom,
                cellNameFrom,
                Design::getCellViewName(Design::getCellView(cellFrom)), 
                'r');
        if (checkEqualMasterByPorts(master, masterRef)) {
            equals.add(masterRef);
            found++;
            if (found >= maxFound) {
                break;
            }
        }
    }

    libFrom->releaseAccess();
}

int Tech::checkEqualMasterByOldMaster(oaInstHeader *header,oaDesign *oldMaster)
{
    return 0;
}

void
Tech::convertDesignTech(oaDesign *design, oaTech *refTech, const bool onlyRef)
{
    oaScalarName libNameFrom, cellNameFrom, viewNameFrom;
    oaScalarName libNameTo, cellNameTo, viewNameTo;

    try {
        // get access
        oaLib *libTo = design->getTech()->getLib();

        if (!libTo->getAccess(oacWriteLibAccess)) {
            cout<<"Error: Connot open the techFrom library"<<endl;
            return;
        }

        oaIter<oaInstHeader> inhIt(design->getTopBlock()->getInstHeaders());
        while (oaInstHeader *header = inhIt.getNext()) {
            assert(header);

            // get the header of old master
            header->getLibName(libNameTo);
            header->getCellName(cellNameTo);
            header->getViewName(viewNameTo);
            oaDesign *master = Design::openNormal(
                    libNameTo, cellNameTo, viewNameTo);

            Array<oaDesign*> masterRef;
           
            //the master has converted,so directed setMaster

            bool isConverted = false;
            oaDesign *newMaster = NULL, *refMaster = NULL;

            int num = oldCellName.getNumElements();
            for(int i=0;i<num;i++) {
                if(oldCellName[i] == cellNameTo) {
                    isConverted = true;
                    newMaster = Design::openNormal(
                            libNameTo,newCellName[i],viewNameTo);
                    assert(newMaster);
                    oaLib *libFrom = refTech->getLib();
                    libFrom->getName(libNameFrom);

                    if (!libFrom->getAccess(oacReadLibAccess)) {
                        cout<<"Error: Connot open the techFrom library"<<endl;
                        return;
                    }

                    oaCell *cellFrom = oaCell::find(libFrom,newCellName[i]);
                    assert(cellFrom);

                    refMaster = Design::openNormal(libNameFrom,
                            newCellName[i],
                            Design::getCellViewName(Design::getCellView(cellFrom)), 
                            'r');
                    assert(refMaster);

                    libFrom->releaseAccess();
                    cout<<"Converted found"<<endl;
                    break;
                }
            }

            if (!isConverted) {
                // find the ref master equal to the old master
                findEqualMasters(master, refTech, masterRef, 10);

                // select one proper master template for new master
                for (UInt4 i = 0; i < masterRef.getNumElements(); i++) {
                    masterRef[i]->getCellName(cellNameFrom);
                    oaString name;
                    cellNameFrom.get(DB::getNS(), name);
                    cout<<"Find cell "<<name<<endl;
                    if (oaCell::find(libTo, cellNameFrom)) {
                        // target ref master has the same name with the current
                        continue;
                    }
                    // now can rename 
                    master->saveAs(libNameTo, cellNameFrom, viewNameTo); 
                    newMaster = Design::openNormal(
                            libNameTo, cellNameFrom, viewNameTo);
                    refMaster = masterRef[i];
                    oldCellName.append(cellNameTo);
                    newCellName.append(cellNameFrom);
                    break;
                }
            }

            assert(refMaster && newMaster);

            cout<<"Ref master "<<endl;
            Design::reportTerms(refMaster);
            // change the ports as to the ref cell
            changeMasterPortsAsRef(header, newMaster, refMaster, !isConverted);

            cout<<"changed master "<<endl;
            Design::reportTerms(newMaster);

            newMaster->save();

            master->close();
        }

        libTo->releaseAccess();
        design->save();
        refTech->close();
    }
    catch (oaException &exp) {
        print(&db50, String::getBuffer(exp.getMsg()));
    }
}

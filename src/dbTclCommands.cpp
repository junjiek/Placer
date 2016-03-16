//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <dbTclCommands.cpp>
//
// database tcl commands definitions
//
// Author: Lu Yongqiang
// History: 2009/12/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "base.h"
#include "design.h"
#include "tech.h"
#include "tacCommand.h"
#include "baseMsg.h"

class TacSetDesignSDC : public TACAutoCommand 
{
public: 
    TacSetDesignSDC(const char* p_name, const char* p_format) 
        : TACAutoCommand(p_name, p_format) { };
    int start(TACArgData *pArgData)
    {
        string lib, cell, view;
        oaDesign *design = NULL;
        if (pArgData->argPresent("lib")) {
            // get design from arg data strings
            pArgData->getStrValue("lib", lib);
            pArgData->getStrValue("cell", cell);
            pArgData->getStrValue("view", view);
            design = Design::open(lib.c_str(), cell.c_str(), view.c_str());
        }
        else {
            // get current design open
            design = Design::getCurrent();
        }
        String sdcFile;
        if (!pArgData->argPresent("file")) {
            cout<<"Please specify the SDC file path"<<endl;
            return TCL_ERROR;
        }
        pArgData->getStrValue("file", sdcFile);
        Design::setSDCFile(design, sdcFile.c_str());
        String sdcset;
        Design::getSDCFile(design, sdcset);
        cout<<"Get SDC file: "<<sdcset<<endl;
        return TCL_OK;
    };
};

static TacSetDesignSDC tacvSetDesignSDCFile("setDesignSDC", "-lib" "-cell" "-view" "-file");

class TacTechSetLIB: public TACAutoCommand 
{
public: 
    TacTechSetLIB(const char* p_name, const char* p_format) 
        : TACAutoCommand(p_name, p_format) { };
    int start(TACArgData *pArgData)
    {
        string lib;
        oaTech *tech = NULL;
        if (pArgData->argPresent("lib")) {
            // get design from arg data strings
            pArgData->getStrValue("lib", lib);
            tech = Tech::open(lib.c_str(), 'a');
        }
        else {
            cout<<"Please specify the tech library to set LIB file on"<<endl;
            return TCL_ERROR;
        }
        if (!pArgData->argPresent("file")) {
            cout<<"Please specify the LIB file path"<<endl;
            return TCL_ERROR;
        }
        String libFile;
        pArgData->getStrValue("file", libFile);
        string mode;
        if (pArgData->argPresent("mode")) {
            pArgData->getStrValue("mode", mode);
        }
        else {
            mode = "normal";
        }
        Tech::CellLibTMode tmode;
        if (mode == "slow") {
            tmode = Tech::LIB_SLOW;
        }
        else if (mode == "normal") {
            tmode = Tech::LIB_NORMAL;
        }
        else if (mode == "fast") {
            tmode = Tech::LIB_FAST;
        }
        Array<String> subStrs;
        libFile.getSubStrings(String::TCL_ARG_DIVIDER, subStrs);
        Tech::setLibertyFiles(tech, tmode, subStrs);

        Array<String> libFiles = Tech::getAllRefLibertyFiles(tech, tmode);
        for (UInt4 i = 0; i < libFiles.getNumElements(); i++) {
            cout<<"Get lib file: "<<libFiles[i]<<endl;
        }
        return TCL_OK;
    };
};

// -mode "fast" "normal" "slow"
static TacTechSetLIB tacvSetTechLIBFile("setTechLIB", "-lib" "-mode" "-file");

class TacReportDesignData: public TACAutoCommand 
{
public: 
    TacReportDesignData(const char* p_name, const char* p_format) 
        : TACAutoCommand(p_name, p_format) { };
    int start(TACArgData *pArgData)
    {
        string lib, cell, view;
        oaDesign *design = NULL;
        try {
            if (pArgData->argPresent("lib")) {
                // get design from arg data strings
                pArgData->getStrValue("lib", lib);
                pArgData->getStrValue("cell", cell);
                pArgData->getStrValue("view", view);
                design = Design::open(lib.c_str(), cell.c_str(), view.c_str());
            }
            else {
                // get current design open
                design = Design::getCurrent();
            }

            const bool isCheckNet = pArgData->argPresent("checkNet");
            const bool checkDomains = pArgData->argPresent("checkDomain");
            const bool checkInst = pArgData->argPresent("checkInst");
            Design::reportDesignData(design, isCheckNet, checkDomains, checkInst);
        }
        catch (oaException &exp) {
            print(&db50, String::getBuffer(exp.getMsg()));
        }
        return TCL_OK;
    };
};

static TacReportDesignData tacvReportDesignData("reportDesignData", "-lib" "-cell" "-view" "-checkNet" "-checkDomain" "-checkInst");

class TacReportNet: public TACAutoCommand 
{
public: 
    TacReportNet(const char* p_name, const char* p_format) 
        : TACAutoCommand(p_name, p_format) { };
    int start(TACArgData *pArgData)
    {
        string lib, cell, view, netName;
        oaDesign *design = NULL;
        try {
            if (pArgData->argPresent("lib")) {
                // get design from arg data strings
                pArgData->getStrValue("lib", lib);
                pArgData->getStrValue("cell", cell);
                pArgData->getStrValue("view", view);
                pArgData->getStrValue("net", netName);
                design = Design::open(lib.c_str(), cell.c_str(), view.c_str());
            }
            else {
                // get current design open
                design = Design::getCurrent();
            }
            if (pArgData->argPresent("occ")) {
                // to check an occ net
                NetOA::reportNet(design->getTopOccurrence(), oaString(netName.c_str()));
            }
            else {
                // check a oa Net
                NetOA::reportNet(design->getTopBlock(), oaString(netName.c_str()));
            }
        }
        catch (oaException &exp) {
            print(&db50, String::getBuffer(exp.getMsg()));
        }
        return TCL_OK;
    };
};

static TacReportNet tacvReportNet("reportNet", "-lib" "-cell" "-view" "-net" "-occ");

class TacTechConvert: public TACAutoCommand 
{
public: 
    TacTechConvert(const char* p_name, const char* p_format) 
        : TACAutoCommand(p_name, p_format) { };
    int start(TACArgData *pArgData)
    {
        try {
            string lib, cell, view, tech;
            oaDesign *design = NULL;


            if (pArgData->argPresent("lib")) {
                // get design from arg data strings
                pArgData->getStrValue("lib", lib);
                pArgData->getStrValue("cell", cell);
                pArgData->getStrValue("view", view);
                design = Design::open(lib.c_str(), cell.c_str(), view.c_str());
            }
            else {
                // get current design open
                design = Design::getCurrent();
            }
            pArgData->getStrValue("refTech", tech);
            oaTech *refTech = Tech::open(tech.c_str(), 'r');
            assert(refTech);
            const bool onlyRef = pArgData->argPresent("onlyRef");
            Tech::convertDesignTech(design, refTech, onlyRef);
        }
        catch (oaException &exp) { 
            print(&db50, String::getBuffer(exp.getMsg()));
        }
        return TCL_OK;
    }
};

static TacTechConvert tacvTechConvert("techConvert", "-lib" "-cell" "-view" "-refTech" "-onlyRef");

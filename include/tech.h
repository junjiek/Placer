//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <tech.h>
//
// The design/flow/database manapulation class
//
// Author: Lu Yongqiang
// History: 2009/12/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_TECH_H_) 
#define _SC_TECH_H_

#include "base.h"

class Tech 
{
public:
    enum CellLibTMode {
        LIB_SLOW = 0, 
        LIB_NORMAL = 1,
        LIB_FAST = 2
    };
public:
    // open a tech
    static oaTech* open(const string &library, const char mode = 'r');
    // get all lib files
    static Array<String> getAllRefLibertyFiles(oaTech *mainTech,
            CellLibTMode mode);
    // get .lib file name
    static void getLibertyFiles(oaTech *tech, CellLibTMode mode,
            Array<String> &filePaths);
    // set .lib file
    static void setLibertyFiles(oaTech *tech, 
            CellLibTMode mode, Array<String> &filePaths);
    // convert the tech of design to refTech
    // if onlyRef specified, only change the orignal master pointers to refTech,
    // otherwise change the original master's name and ports to refTech's
    // The formmer will use the refTech's all information including row/tracks,
    // but the latter will only use its master's names
    static void convertDesignTech(oaDesign *design, oaTech *refTech, const bool onlyRef);

    static int checkEqualMasterByOldMaster(oaInstHeader *,oaDesign *);
protected:
    static void clearLibertyFiles(oaTech *tech, CellLibTMode mode);


protected:
    static const char *tecgLIBFileNameFormat;
};

#endif

//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <occTraverser.cpp>
//
// Occ objects traverse utility
//
// Author: Lu Yongqiang
// History: 20010/1/19 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "occTraverser.h"
#include "baseMsg.h"
#include "base.h"

// first judge flags() (e.g. design_descend flag) and then judge startInst,
// if both pass, descend into the sub hierarchy,
// and then processInst() (if post-order)
oaBoolean 
OccUniqTraverser::startInst(oaOccInst *inst)
{
    // don't need to descend
    if (!OccTraverser::startInst(inst)) {
        return false;
    }

    // need to descend, check the uniquification
    try {
        if (inst->isOccDesignInst()) {
            // design inst
            oaModule *masOccMod = inst->getMasterOccurrence()->getModule();
            masOccMod->embed(mTopOcc->getDesign(), masOccMod->getDesign());
            inst->getMasterOccurrence()->uniquify();
        }
        else {
            // module inst
            inst->getMasterOccurrence()->uniquify();
        }
    }
    catch (oaException &excp) {
        print(&db50, String::getBuffer(excp.getMsg()));
    }
    return true;
}

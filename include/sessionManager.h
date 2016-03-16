//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <sessionManager.h>
//
// session manager to supervise the design sessions
//
// Author: Lu Yongqiang
// History: 2009/5/25 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_SESSION_MANAGER_H_)
#define _SC_SESSION_MANAGER_H_

#include "staticData.h"
#include "design.h"

// *****************************************************************************
// class SessionManager 
//
// A manager for design sessions. Normally used for a observer to make the
// static members or other global variables/session objects consistent with
// user's session/design operation.
// Only invoked in main.cpp, and only one instance in memory
// *****************************************************************************
class SessionManager 
{
public:
    inline SessionManager()
    {
    }
    inline ~SessionManager()
    {
        // release static data
        StaticData::releaseAll();

        // close all openeed libs
        oaIter<oaDesign> it(oaDesign::getOpenDesigns());
        while (oaDesign *design = it.getNext()) {
            design->close();
        }
        oaIter<oaTech> it2(oaTech::getOpenTechs());
        while (oaTech *tech = it2.getNext()) {
            tech->close();
        }
    };

    void reset()
    {
        StaticData::reset();
    };

};

#endif

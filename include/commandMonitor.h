//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <commandMonitor.h>
//
// monitor the command, report runtime and other run-time statistics
//
// Author: Lu Yongqiang
// History: 2009/1/20 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_COMMAND_MONITOR_H_)
#define _SC_COMMAND_MONITOR_H_

#include "runtime.h"

class CommandMonitor
{
public:
    inline CommandMonitor(const string &commandName) 
        : mCmdName(commandName), 
          mRunTime(RunTime())
    {
        print(&sc8, mCmdName.c_str());
    };
    inline ~CommandMonitor()
    {
        String runtimeStr;
        mRunTime.getRunTimeReport(runtimeStr);
        print(&sc9, mCmdName.c_str(), runtimeStr.c_str());
    };

protected:
    const string mCmdName;
    RunTime mRunTime;
};

#endif

//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <runtime.h>
//
// includes runtime related calculation class
//
// Author: Lu Yongqiang
// History: 2009/8/20 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#if !defined(_SC_RUNTIME_H_)
#define _SC_RUNTIME_H_
#include <time.h>
#include "baseMsg.h"

// User can just use a variable to handle the runtime outputs
// E.g. RunTime time;
// the destructor will output the final runtimes automatically
// User can also pass a string to be a prefix for the final run time message
// output
class RunTime
{
public:
    inline RunTime()
        : cpuTime(clock()),
          wallTime(time(0)) 
    { 
    };
    inline ~RunTime()
    {
    };
    // return cpu run time in seconds
    inline unsigned long getCpuTime()
    {
        clock_t cpuEndTime(clock());
        return (cpuEndTime - cpuTime) / CLOCKS_PER_SEC;
    };
    inline unsigned long getWallTime()
    {
        time_t wallEndTime(time(0));
        return (unsigned long) difftime(wallEndTime, wallTime);
    };

    inline void getRunTimeReport(String &report)
    {
        unsigned long cputime = getCpuTime();
        unsigned long walltime = getWallTime();

        // total cpu time
        report.format("Total CPU time: %d minutes %d seconds; Wall time: %d minutes %d seconds.", cputime / 60, cputime % 60, walltime / 60,  walltime % 60);
    };

private:
    clock_t cpuTime;
    time_t wallTime;
};
#endif

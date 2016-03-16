//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <message.h>
//
// includes some basic mathematic type definitions or utilities
//
// Author: GENG Dongjiu
// History: 2009/9/25 created by GENG
//*****************************************************************************
//*****************************************************************************
#include "message.h"

void print(Message *pMsg, ...)
{
    va_list list;
    va_start(list, pMsg);
    pMsg->printMsg(pMsg->getFormat(), list);
    va_end(list);
}

Message::Message(const LEVEL lvl, const char* mtag, const char* mformat,
        const DESTINATION dest)
        : msglvl(lvl),
          msgTag(mtag),
          msgFormat(mformat),
          destination(dest),
          contents(""),
          pcount(0),
          maxprint(0)

{
    contents = mtag;
    if (contents != "") { 
        switch (lvl) {
            case LDEBUG:
                contents = contents + " DEBUG" + ": ";
                break;
            case LVERBOSE:
                contents = contents + " VERBOSE" + ": ";
                break;
            case LNOTE:
                contents = contents + " NOTE" + ": ";
                break;
            case LWARNING:
                contents = contents + " WARNING" + ": ";
                break;
            case LERROR:
                contents = contents + " ERROR" + ": ";
                break;
            case LFATAL:
                contents = contents + " FATAL" + ": ";
                break;
            default:
                break;
        }
    }
    contents += msgFormat;
}

Message::Message(const LEVEL lvl, const char* mtag, const char* mformat,
        const int  maxpcount, const DESTINATION dest) 
        : msglvl(lvl),
          msgTag(mtag),
          msgFormat(mformat),
          destination(dest),
          contents(""),
          pcount(0),
          maxprint(maxpcount)
{
    Message(lvl, mtag, mformat, dest);
}

void Message::print(const char* msformat, ... ) 
{ 
    va_list apm;
    va_start(apm, msformat);
    String msString(msformat, apm);
    printm(msString);
    va_end(apm);
}

void Message::printm(String &substance) 
{  
    if (maxprint == 0 || pcount < maxprint) {
        if (destination == TOLOGFILE) {      
            printFile(substance);
        }
        else if (destination == TOSHELL) {     
            printShell(substance);
        }
        else if (destination == TODEFAULT) { 
            printFile(substance);
            printShell(substance);
        }
    }
}

void Message::printFile(String &content) 
{  
    //char *msChar = const_cast<char *>(content.c_str());
    //LogFile::logfile->fnWriteLog(msChar);
}



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
// Warning should use IDs below 50
// Error should use IDs above 50 less than 100
// Verbose should use IDs at least above 100
// Debug should use IDs at least above 200
//
// Author: GENG Dongjiu
// History: 2009/9/25 created by GENG
//*****************************************************************************
//*****************************************************************************

#if !defined (_SC_MESSAGE_H_)
#define _SC_MESSAGE_H_

#include <fstream>
#include <stdio.h>
#include <string>
#include <stdarg.h>  
#include <iostream>
#include <sys/stat.h>
#include "stdlib.h"
#include <cstring> 
#include <fcntl.h>
#include <stdexcept>
#include "logfile.h"
#include "String.h"
#include "datatype.h"

#define MSG_MAXLEN 1024

using namespace std;
class LogFile;
class Message
{
public:
    enum LEVEL { 
        LNONE,
        LDEBUG,
        LVERBOSE,
        LNOTE, 
        LWARNING, 
        LERROR,  
        LFATAL
    };		 
    enum DESTINATION {
        TONOWHERE = 0,            // don't send anywhere
	TOLOGFILE = 1,		// bit 0: send message to logfile
	TOSHELL = 2,		// bit 1: send message to command shell
	TODEFAULT = 3		// TODEFAULT: send to both
    };		

public:
    Message(const LEVEL, const char*, const char*, const DESTINATION );
    Message(const LEVEL, const char*, const char*, const int, const DESTINATION );
    inline ~Message() {}

public:
    void print(const char*,...)__attribute__((format(printf,2,3)));

    inline void setLevel(LEVEL &level)
    {
        msglvl = level;
    };

    inline LEVEL getLevel() const 
    { 
        return msglvl; 
    };

    inline const char* getFormat()
    {
        return contents.c_str();
    };


    inline void printMsg(const char* msformat, va_list &argp) 
    {   
        String msString(msformat, argp);
        printm(msString);
    };
    inline int getPrintTimes() const 
    { 
        return pcount;
    };
    inline void setMaxPrintTimes(int coutbreak) 
    { 
        maxprint = coutbreak;
    };      
    inline int getMaxPrintTimes() const
    {
       return maxprint;
    };

protected:
    void printm(String &substance);
    void printFile(String  &content);

    inline void printShell(String &ssbstance ) 
    {
        cout<<ssbstance<<endl;  
    };

    inline void setDestination(const DESTINATION dest)
    {
        destination = dest;
    };

    inline DESTINATION getdestination() const 
    { 
        return destination;
    };

protected:
    LEVEL  msglvl;	
    const char *msgTag;	
    const char *msgFormat; 
    DESTINATION destination;	
    string contents;
    UInt4 pcount;
    UInt4 maxprint; 
     
};

class MsgDebug : public Message
{
public:
    inline MsgDebug(const char* mtag, const char* mformat)
        : Message(Message::LDEBUG, mtag, mformat, TODEFAULT) { }; 

    inline MsgDebug(const char* mtag, const char* mformat,
            const int maxCount)
        : Message(Message::LDEBUG, mtag, mformat, maxCount, TODEFAULT) { };
};

class MsgVerbose : public Message
{
public:
    inline MsgVerbose(const char* mtag, const char* mformat)
        : Message(Message::LNOTE, mtag, mformat, TODEFAULT) { } ;
    inline MsgVerbose(const char* mtag, const char* mformat, const int maxCount)
        : Message(Message::LNOTE, mtag, mformat, maxCount, TODEFAULT) {};
};

class MsgNote : public Message
{
public:
    inline MsgNote(const char* mtag, const char* mformat)
        : Message(Message::LNOTE, mtag, mformat, TODEFAULT) { } ;
    inline MsgNote(const char* mtag, const char* mformat, const int maxCount)
        : Message(Message::LNOTE, mtag, mformat, maxCount, TODEFAULT) {};
};

class MsgWarning : public Message
{
public:
    inline MsgWarning(const char* mtag, const char* mformat) 
        : Message(Message::LWARNING, mtag, mformat, TODEFAULT) { } ;

    inline MsgWarning(const char* mtag, const char* mformat,
            const int maxCount)
        : Message(Message::LWARNING, mtag, mformat, maxCount, TODEFAULT) {} ;
};

class MsgError : public Message
{
public:
    inline MsgError(const char* mtag, const char* mformat) 
        : Message(Message::LERROR, mtag, mformat, TODEFAULT) { } ;

    inline MsgError(const char* mtag, const char* mformat,
            const int maxCount) 
        : Message(Message::LERROR, mtag, mformat, maxCount, TODEFAULT) {} ;
};

class MsgFatal : public Message
{
public:
    inline MsgFatal(const char* mtag, const char* mformat) 
        : Message(Message::LFATAL, mtag, mformat, TODEFAULT) { } ;

    inline MsgFatal(const char* mtag, const char* mformat,
            const int maxCount)
        : Message(Message::LFATAL, mtag, mformat, maxCount, TODEFAULT) { };

};
extern "C" 
void print(Message *pMsg, ...);

#endif

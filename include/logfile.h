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
#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sys/mman.h>
#include <string.h>

#if !defined (_SC_LOGFILE_H_)
#define _SC_LOGFILE_H_

#define NAMElEN 512
#define PREFIX 64
#define POSTFIX 64
using namespace std;
#define FILE_MODE (S_IRUSR   |   S_IWUSR   |   S_IRGRP   |   S_IROTH)
class Message;

class DiskFile
{
public:
    enum AccessMode {
        MODE_READ = 'r',
        MODE_WRITE = 'w'
    };

public:
    static bool exist(const char *filePath, AccessMode mode = MODE_READ);

public:
    inline DiskFile() { };
    inline ~DiskFile() { };

};

class LogFile : public DiskFile
{
public:
    LogFile(const char*, const char *, const char *);
    inline ~LogFile() 
    {
        fclose(logfp);
    };

public:
    int  fnSetLogDir();
    int  fnWriteLog(char *strLogMsg);
    bool copyFile(const string&, const string&);
    void backup(char * Fdir, const string &start2 ="sc", 
            const string &ending1 = ".log");

private:
    static  LogFile *logfile;
    FILE * logfp;                        // the name of file 
    char   strFileNew[NAMElEN];           // the backup of file
    char   strFile[NAMElEN];              //the full name
    char   logDir[NAMElEN] ;              //the dir of file
    char   strPrefix[PREFIX];             //the prefix of name
    char   strpostfix[POSTFIX];           // the postfix of name

    friend class Message;
};

#endif

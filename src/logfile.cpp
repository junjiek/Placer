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
#include "logfile.h"
LogFile *LogFile::logfile = NULL;


bool
DiskFile::exist(const char *filePath, AccessMode mode)
{
    string fmode(1, (char) mode);
    FILE *fp = fopen(filePath, fmode.c_str());
    if (fp == NULL) {
        return false;
    }
    fclose(fp);
    return true;
}

LogFile::LogFile(const char* dirlog, const char * prefix, const char * postfix)
{
    memset(strFile,0,sizeof(strFile));
    memset(logDir,0,sizeof(logDir));
    strcpy(logDir,dirlog);
    strcpy(strPrefix,prefix);
    strcpy(strpostfix,postfix);  
    if (logDir[strlen(logDir)-1] != '/') {
        strcat(logDir,"/");
    }
    snprintf(strFile,sizeof(strFile),"%s%s%s",logDir,prefix,postfix);
    int  iRet=fnSetLogDir();
    if (iRet < 0) { 
        printf("make directory error\n");
        exit(1);
    }
    else { 
        backup(logDir,prefix,postfix);
    }
    if ((logfp = fopen(strFile,"w+")) == NULL) { 
        printf("file can't open! \n");
        exit(1);
    }

    // set up the global static logfile pointer
    if (logfile) {
        cout<<"Warning: there has been one logfile opened"<<endl;
    }
    logfile = this;
}

bool LogFile::copyFile(const string &srcfile, const string &destfile) 
{       
    int    fdin, fdout;
    void   *src, *dst;
    struct stat statbuf;
    if ((fdin = open(srcfile.c_str(), O_RDONLY)) < 0) {
        return false;
    }
    if ((fdout = open(destfile.c_str(), O_RDWR | O_CREAT | O_TRUNC,FILE_MODE)) < 0) {
        return false;
    }
    if (fstat(fdin, &statbuf) < 0) {  
        return false;
    }
    if (lseek(fdout, statbuf.st_size - 1 , SEEK_SET) == -1) {
        return false;
    }
    if (write(fdout, "", 1) != 1) {
        return false;
    }
    if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED,fdin, 0)) 
            == MAP_FAILED) {
        return false;
    }
    if ((dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE,MAP_SHARED,
                    fdout, 0)) == MAP_FAILED) {
        return false;
    }
    memcpy(dst, src, statbuf.st_size); 
    return true;
}

void  LogFile::backup(char * Fdir, const string &start2, const string &ending1)
{
    vector<long> contain;
    char strFileNew[128],strFile[128];
    DIR* pDir = NULL;
    struct dirent* ent = NULL;
    pDir = opendir(Fdir);
    if (NULL == pDir) {
        printf("Source folder not exists!");   
        return ;
    }    
    while (NULL != (ent=readdir(pDir)))  
    {    
        if (strcmp(ent->d_name,".") == 0||strcmp(ent->d_name,"..") == 0) continue;   
        string sstart(ent->d_name,start2.size());
        string send(ent->d_name+strlen(ent->d_name)-ending1.size(),ending1.size());
        if ((sstart!=start2)||(send!=ending1))   continue;   
        int sz=strlen(ent->d_name)-start2.size()-ending1.size();
        string   Fname(ent->d_name+start2.size(), sz);
        long suffix;
        if (Fname == "") {
            suffix = 0;
        }
        else { 
            suffix = atol(Fname.c_str());
        }                      
        contain.push_back(suffix);
        sort(contain.begin(),contain.end());
    }
    for (vector<long>::reverse_iterator iter = contain.rbegin();
            iter != contain.rend(); iter++) {    
        long strlong=static_cast<long>(*iter);
        if (iter == contain.rbegin()) {
            snprintf(strFileNew,sizeof(strFileNew),"%s%s%ld%s",Fdir,start2.c_str(),strlong+1,ending1.c_str());
        }
        else {
            long strlongn = static_cast<long>(*(iter-1));
            snprintf(strFileNew,sizeof(strFileNew),"%s%s%ld%s",Fdir,start2.c_str(),strlongn,ending1.c_str());
        }
        if (strlong == 0) {
            snprintf(strFile,sizeof(strFile),"%s%s%s",Fdir,start2.c_str(),ending1.c_str());
        }
        else { 
            snprintf(strFile,sizeof(strFile),"%s%s%ld%s",Fdir,start2.c_str(),strlong,ending1.c_str());
        }
        copyFile(string(strFile), string(strFileNew)); 
    }
    closedir(pDir);
    pDir = NULL;
}


int LogFile::fnWriteLog( char * strLogMsg )
{   

    if (strlen(logDir) == 0) {
        strcpy(logDir,"./log/");
        if (access( logDir, W_OK ) < 0) {
            if (mkdir( logDir , 0744 ) < 0) {
                return -1;
            }
        }
    }
    if (fprintf(logfp, "%s\n", strLogMsg) == -1) {
        printf("write error \n");  
        exit(1);   
    }
    fflush(logfp ); 
    return 0;
}
int LogFile::fnSetLogDir( )
{         
    if (access( logDir, W_OK ) < 0 ) {
        if (mkdir( logDir, 0744 ) < 0) {        
            return -1;
        }
    }
    return 0;
}

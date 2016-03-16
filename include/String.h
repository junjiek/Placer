//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <string.h>
//
// a std string based String
//
// Author: Yongqiang Lu
// History: 2009/4/30 created by Yongqiang
// 	    2009/9/11 revised by GENG Dongjiu
//*****************************************************************************
#if !defined(_SC_STRING_H_)
#define _SC_STRING_H_
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include "oa/oaDesignDB.h"
#include "datatype.h"
#include "array.h"

using namespace std;
using namespace oa;

class String : public std::string
{
public:
    inline String()
        : std::string() {};

    inline String(const string &str)
        : string(str) { };

    inline String(const char *buf) 
        : std::string(buf) {};

    inline String(const oaString &oastr)
        : std::string(getBuffer(oastr)) { };

    inline String(const char *pFormat, ...)
    {
        va_list list;
        va_start(list, pFormat);
        format(pFormat, list);
        va_end(list);
    };
    inline String(const char* pFormat, va_list &ap)
    {
        format(pFormat, ap);
    };

    inline String& operator = (const char *buf)
    {
        static_cast<string*>(this)->operator=(buf);
        return *this;
    };

public:
    static inline const char* getBuffer(const oaString &oaStr)
    {
        return (const oaChar *)oaStr;
    };
    static inline string getString(const oaString &oaStr)
    {
        return string(getBuffer(oaStr));
    };
    static inline void toLowercase(string &str)
    {
        for (UInt4 i = 0; i < str.length(); i++) {
            str[i] = tolower(str[i]);
        }
    };

public:
    inline void format(const char *pFormat, ...)
    {
        va_list list;
        va_start(list, pFormat);
        format(pFormat, list);
        va_end(list);
    };
    void getSubStrings(const char divider, Array<String> &subs);

protected:
    void format(const char* pFormat, va_list &ap);

public:
    static const char TCL_ARG_DIVIDER = ' '; 

};
#endif

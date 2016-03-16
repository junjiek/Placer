//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <String.cpp>
//
// a std string based String
//
// Author: Yongqiang Lu
// History: 2010/1/5 created by Yongqiang
//*****************************************************************************
#include "String.h"

void 
String::format(const char* pFormat, va_list &ap)
{
    unsigned int num; 
    resize(1024);
    num = vsnprintf((const_cast<char *>(c_str())),size(), pFormat, ap);    
    if (num >= size()) {
        resize(size() + 1);
        while (1) {
            num = vsnprintf((const_cast<char *>(c_str())),size(),
                    pFormat, ap);    
            if (num >= size()) {
                resize(size() + 1);
            }
            else {
                break;
            }
        }
    }
}

void
String::getSubStrings(const char divider, Array<String> &subs)
{
    UInt4 pos1 = 0;
    UInt4 pos2 = 0;
    while (pos2 != string::npos) {
        pos1 = find_first_not_of(divider, pos1);
        if (pos1 == string::npos) {
            break;
        }
        pos2 = find(divider, pos1);
        subs.add(substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
    }
}

//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <base.h>
//
// Only include main utilities, headers, and wrapping classes of OA
//
// Author: Lu Yongqiang
// History: 2009/4/2 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#if !defined(_BASE_H_)
#define _BASE_H_

#include <limits.h>
#include <strings.h>
#include "hash.h"
#include "list.h"
#include "array.h"
#include <iostream>
#include "oa/oaDesignDB.h"
#include "collectionAndIter.h"
#include "datatype.h"
#include "String.h"

using namespace std;
using namespace oa;

// OA wrapper class for utilities and other interfaces
class DB
{
public:
    typedef enum _DBExtAttrType_ {
        EXT_ATTR_INT = 0,
        EXT_ATTR_BOOL = 1,
        EXT_ATTR_DOUBLE = 2,
        EXT_ATTR_STRING = 3,
        EXT_ATTR_POINTER = 4
    } DBExtAttrType;

public:
   // please refer to String::getBuffer/getString
    static inline const char* getBuffer(oaString &oaStr)
    {
        return (const oaChar *)oaStr;
    };
    static inline string getString(oaString &oaStr)
    {
        return string(getBuffer(oaStr));
    };
    static inline oaNativeNS& getNS()
    {
        static oaNativeNS *oaNS = new oaNativeNS;
        return *oaNS;
    };
    static inline oaDefNS& getDefNS()
    {
        static oaDefNS *oadefns = new oaDefNS;
        return *oadefns;
    };
    static inline oaVerilogNS& getVNS()
    {
        static oaVerilogNS *oavns = new oaVerilogNS;
        return *oavns;
    };
public:
    static const char* NULL_FILE;
};

typedef DB::DBExtAttrType DBExtAttrType;

#endif

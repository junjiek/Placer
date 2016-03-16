//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <row.h>
//
// The standard cell row wrap-up
//
// Author: Lu Yongqiang
// History: 2009/9/14 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_ROW_H_) 
#define _SC_ROW_H_

#include <iostream>
#include "oa/oaDesignDB.h"

using namespace std;
using namespace oa;
// a standard cell row wrap-up
class RowOA
{
public:
    inline RowOA() { };
    inline ~RowOA() { };

    // get lower-left origin
    static void getOrigin(const oaRow *row, oaPoint &origin);
    // set lower-left origin
    static void setOrigin(oaRow *row, const oaPoint &origin);
    // get orient
    static inline oaOrient getOrient(const oaRow *row)
    {
        return row->getOrient();
    };
    // set orientation, whenever this invoked, the row will transformed 
    // according to the orientation
    static void setOrient(oaRow *row, const oaOrient &orient);
};

#endif

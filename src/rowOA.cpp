//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <row.cpp>
//
// The standard cell row wrap-up
//
// Author: Lu Yongqiang
// History: 2009/9/14 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "rowOA.h"
// get lower-left origin
// oa::getOrigin is to get the coordinate of the origin point of the 
// master design cooridnate-system of the row, which might not be the lower-left origin
void 
RowOA::getOrigin(const oaRow *row, oaPoint &origin)
{
    oaBox       bBox;
    oaTransform tr(0, 0);

    row->getOrigin(origin);
    row->getBBox(bBox);

    switch (row->getOrient()) {
    case oacR0:
        break;
    case oacR90:
        tr = oaTransform(-bBox.getHeight(), 0);
        break;
    case oacR180:
        tr = oaTransform(-bBox.getWidth(), -bBox.getHeight());
        break;
    case oacR270:
        tr = oaTransform(0, -bBox.getWidth());
        break;
    case oacMY:
        tr = oaTransform(-bBox.getWidth(), 0);
        break;
    case oacMYR90:
        tr = oaTransform(-bBox.getHeight(), -bBox.getWidth());
        break;
    case oacMX:
        tr = oaTransform(0, -bBox.getHeight());
        break;
    case oacMXR90:
        break;
    }

    origin.transform(tr);
}

// this function always set the lower-left coordinate of the row after
// the orientation commited
// So, after this set, you use Row::getOrigin will always get the same lower-
// left origin coordinate as you set
void 
RowOA::setOrigin(oaRow *row, const oaPoint &origin)
{
    // get the current lower-left origin
    oaPoint point;
    RowOA::getOrigin(row, point);

    // transform the row by the displace, forg records the displacement
    oaPoint forg = origin;
    forg -= point;

    // get the oa origin displacement and set it
    row->getOrigin(point);
    point += forg;
    row->setOrigin(point);
}
// set the absolute orient with the lower-left origin not changed
void 
RowOA::setOrient(oaRow *row, const oaOrient &orient)
{
    if (row->getOrient() == orient) {
        // no need to change
        return;
    }

    // first get the current LL origin
    oaPoint origin;
    RowOA::getOrigin(row, origin);

    // then change orient
    row->setOrient(orient);

    // map the row back to the former LL origin
    RowOA::setOrigin(row, origin);
}

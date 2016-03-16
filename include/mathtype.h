//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <mathtype.h>
//
// includes some basic mathematic type definitions or utilities
//
// Author: Lu Yongqiang
// History: 2009/5/8 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#if !defined(_SC_MATH_TYPE_H_)
#define _SC_MATH_TYPE_H_

// The 1D range, is [begin, end).
template <typename T>
struct OneDRange{
    T begin;
    T end;
};

#endif

//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <define.h>
//
// Purpose : define common defines
//
// Author: Wang Sifei
// History: 2012/02/20 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _DEFINE_H_WSF_
#define _DEFINE_H_WSF_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ALLOC(type, num) ((type *)malloc(sizeof(type) * (num)))
#define FREE(obj)        ((obj != NULL) ? (free(obj), (obj) = NULL) : NULL)

#define ABS(a)			 ((a) < 0 ? -(a) : (a))
#define MAX(a, b)		 ((a) > (b) ? (a) : (b))
#define MIN(a, b)		 ((a) < (b) ? (a) : (b))

#define ZERO(ptr, type, num) (memset(ptr, 0, sizeof(type) * (num)))
#define COPY(dest, src, type, num) (memcpy(dest, src, sizeof(type) * (num)))

#endif

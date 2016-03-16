//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <point.h>
//
// Define class myPoint
//
// Author: Wang Sifei
// History: 2012/02/20 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _POINT_H_
#define _POINT_H_

#include <stdio.h>
#include <string>
#include "enum.h"
#include "util.h"

using namespace std;

class myPoint {
 public:
  inline myPoint() : x(0), y(0) {}; // by default construct an invalid rect
  inline myPoint(const double &xl, const double &yb)
      : x(xl), y(yb) {};
  inline myPoint(const myPoint&p) {
	setCoordXY(p.coordX(), p.coordY());
  }
  inline ~myPoint(){};

  const double coordX() const { return x; }
  const double coordY() const { return y; }

  void setCoordX(const double left) {
    x = left; 
  }
  void setCoordY(const double bottom) {
    y = bottom; 
  }
  void setCoordXY(const double xll,
                const double ytt) {
    setCoordX(xll); 
    setCoordY(ytt); 
  }

  void operator=(const myPoint& r) {
    setCoordXY(r.coordX(), r.coordY());
  }

 private:
  double x;
  double y;
};

#endif  


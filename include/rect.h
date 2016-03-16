//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <rect.h>
//
// Define class Rect
//
// Author: Wang Sifei
// History: 2012/02/15 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _RECT_H_
#define _RECT_H_

#include <stdio.h>
#include "enum.h"
#include "util.h"
#include "point.h"

class Rect {
 public:
  inline Rect() : xl(0), yb(0), xr(-1), yt(0) {}; // by default construct an invalid rect
  inline Rect(const double &xll, const double &ybb, const double &xrr, const double &ytt)
      : xl(xll), yb(ybb), xr(xrr), yt(ytt) {};
  inline Rect(const Rect& r) {
	setCoord(r.left(), r.bottom(), r.right(), r.top());
  }
  inline ~Rect(){};

  std::string name() const {
    std::string compose_name;
    std::string xl(Itostr(this->left()));
    std::string xr(Itostr(this->right()));
    std::string yb(Itostr(this->bottom()));
    std::string yt(Itostr(this->top()));
    compose_name = "{" + xl + " " + yb + " " + xr + " " + yt + "}";
    return compose_name;
  }

  const long left() const { return xl; }
  const long bottom() const { return yb; }
  const long right() const { return xr; }
  const long top() const { return yt; }
  void setLeft(const long left) {
    xl = left; 
  }
  void setBottom(const long bottom) {
    yb = bottom; 
  }
  void setRight(const long right) {
    xr = right; 
  }
  void setTop(const long top) {
    yt = top; 
  }
  void setCoord(const long xll,
                const long ybb,
                const long xrr,
                const long ytt) {
    setLeft(xll); 
    setBottom(ybb); 
    setRight(xrr); 
    setTop(ytt); 
  }

  void setCenter(const double cx, const double cy) {
	  double ori_cx = (left() + right()) / 2;
	  double ori_cy = (bottom() + top()) / 2;
	  setLeft(left() - ori_cx + cx);
	  setBottom(bottom() - ori_cy + cy);
	  setRight(right() - ori_cx + cx);
	  setTop(top() - ori_cy + cy);
  }

  void operator=(const Rect& r) {
    setCoord(r.left(), r.bottom(), r.right(), r.top());
  }

  const double width() const { return right() - left(); }
  const double height() const { return top() - bottom(); }
  const double width(const Dimension dimension) const {
    return (dimension == kX) ? width() : height();
  }
  double minWidth() const { return width() < height() ? width() : height(); }
  double maxWidth() const { return width() > height() ? width() : height(); }
  double area() const {
    return width() * height();
  }

  bool isValid() const {
    return (left() <= right()) && (bottom() <= top());
  }
  bool isPolong() const {
    return (left() == right()) && (bottom() == top());
  }
  bool isLine() const {
    return (left() == right()) ^ (bottom() == top());
  }

  bool touches(const Rect& r) const {
    return left() < r.right() && right() > r.left() &&
           bottom() < r.top() && top() > r.bottom();
  }
  bool overlaps(const Rect& r) const {
    return left() < r.right() && right() > r.left() &&
           bottom() < r.top() && top() > r.bottom();
  }
  bool covers(const Rect& r) const {
    return left() <= r.left() && right() >= r.right() &&
           bottom() <= r.bottom() && top() >= r.top();
  }
  void bloat(const long delta_x, const long delta_y) {
    	setLeft(left() - delta_x); 
	setRight(right() + delta_x);
    	setBottom(bottom() - delta_y); 
	setTop(top() + delta_y);
  }

  void stretch(const Rect& r) {
  	if (left() > r.left()) 
		setLeft(r.left());
  	if (bottom() > r.bottom()) 
		setBottom(r.bottom());
  	if (right() < r.right()) 
		setRight(r.right());
  	if (top() < r.top()) 
		setTop(r.top());
  }
  
  void confine_to(const Rect& r) {
  	if (left() < r.left()) setLeft(r.left());
  	if (bottom() < r.bottom()) setBottom(r.bottom());
  	if (right() > r.right()) setRight(r.right());
  	if (top() > r.top()) setTop(r.top());
  	if (right() < left()) setRight(left());
  	if (top() < bottom()) setTop(bottom());
  }


  double coordLow(Dimension dim) const {
    return (dim == kX) ? left() : bottom();
  }

  double coordHigh(Dimension dim) const {
    return (dim == kX) ? right() : top();
  }

  double coordCenter(Dimension dim) const {
    return (dim == kX) ? (left()+right()) / 2 : (bottom()+top()) / 2;
  }

  myPoint &lowerLeft()
  {
  		myPoint point;
  		point.setCoordX(xl);
  		point.setCoordY(yb);
  		return point;
  }
 private:
  long xl;
  long yb;
  long xr;
  long yt;
  friend class Block;
};

#endif  


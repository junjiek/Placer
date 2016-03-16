#ifndef LDPUTILITY_H_
#define LDPUTILITY_H_

#include<vector>
#include "rect.h"
#include "point.h"

class LdpUtility {
public:
	// used
	static bool containInBox(Inst *inst, Rect bbox) {
		Rect instBox;
		getBBox(inst, instBox);
		myPoint center(instBox.coordCenter(kX),instBox.coordCenter(kY));
		if (center.coordX() > bbox.left() && center.coordX() < bbox.right() && center.coordY()
				> bbox.bottom() && center.coordY() < bbox.top()) {
			return true;
		}
		return false;
	}
//used
	static inline long getInstWidth(Inst * inst) {
		Rect bbox;
		getBBox(inst, bbox);
		return bbox.width();
	}
//used
/*	static inline long getXCoord(Inst *inst) {
		myPoint origin;
		Inst::getOrigin(inst, origin);
		return origin.coordX();
	}
	*/
//used
	static inline void getBBox(Inst* inst, Rect& bbox) {
		bbox.setLeft(inst->getCoordX());
		bbox.setRight(inst->getCoordX() + inst->getWidth());
		bbox.setBottom(inst->getCoordY());
		bbox.setTop(inst->getCoordY() + inst->getHeight());
	}
	static inline Rect getBBox(Inst* inst)//hjy20130416=====
	{
		Rect box;
		box.setLeft(inst->getCoordX());
		box.setRight(inst->getCoordX() + inst->getWidth());
		box.setBottom(inst->getCoordY());
		box.setTop(inst->getCoordY() + inst->getHeight());

		return box;
	}


//used

	/*
	static inline long getInstWeight(Inst * inst) {
		//return (inst->getInstTerms()).getCount();
		return 1;
	}

	static inline long getInstHeight(Inst * inst) {
		Rect bbox;
		Inst::getBBox(inst, bbox);
		return bbox.getHeight();
	}


	static inline long getYCoord(Inst *inst) {
		myPoint origin;
		Inst::getOrigin(inst, origin);
		return origin.y();
	}

	static inline long getXCoord(myRow *row) {
		myPoint origin;
		row->getOrigin(origin);
		return origin.x();
	}

	static inline long getYCoord(myRow *row) {
		myPoint origin;
		row->getOrigin(origin);
		return origin.y();
	}
	


	static double getHPWLChange(Inst *inst, myPoint newOrigin){
		return 0.0;
	}

	static void print(myPoint &p) {
		cout << "(" << p.x() << ", " << p.y() << ")" << endl;
	}

	static void print(Inst *inst) {
		oaString name;
		inst->getName(DB::getNS(), name);
		cout << "Inst "<<name<<": (" << getXCoord(inst) << ", " << getYCoord(inst)
				<< "), width = " << getInstWidth(inst) << endl;
	}

	static bool checkOverlap(Inst *a, Inst *b) {
		Rect bboxa, bboxb;
		Inst::getBBox(a, bboxa);
		Inst::getBBox(b, bboxb);

		if (bboxa.left() <= bboxb.left()) {
			if (bboxa.left() + (Int4) bboxa.getWidth() <= bboxb.left()) {
				return true;
			}
		} else {
			if (bboxb.left() + (Int4) bboxb.getWidth() <= bboxa.left()) {
				return true;
			}
		}

		if (bboxa.bottom() <= bboxb.bottom()) {
			if (bboxa.bottom() + (Int4) bboxa.getHeight() <= bboxb.bottom()) {
				return true;
			}
		} else {
			if (bboxb.bottom() + (Int4) bboxb.getHeight() <= bboxa.bottom()) {
				return true;
			}
		}
		return false;
	}

	static inline long getSiteXOrigin(long x, long left, long siteWidth){
		long sx = (long)(x - left) / siteWidth;
		double sxd = (x - -left) / siteWidth;
		if( sxd - sx <0.5){
			return left + siteWidth * sx;
		} else {
			return left + siteWidth * (sx + 1);
		}
	}

	static inline long getSiteIndex(long x, long left, long siteWidth){
		return (long)(x - left) / siteWidth;
	}
	*/
};

#endif /* LDPUTILITY_H_ */

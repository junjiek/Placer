/*
 * Layer.h
 *
 *  Created on: Feb 25, 2014
 *      Author: thueda
 */

#ifndef LAYER_H_
#define LAYER_H_

#include <string>
#include <enum.h>
#include <rect.h>

class Layer{
private:
	char* name;
	long index;
	double spacing;
	Dimension direction;
	LayerType type;
	double pitch;
	double offset;
	double width;
	double maxWidth;
	vector<Rect> blocks;

public:
	inline Layer(){name = new char[128];};
	inline ~Layer(){delete[] name;};


	inline long getIndex(){
		return index;
	}
	inline string getName(){
		return string(name);
	}
	inline double getSpacing(){
		return spacing;
	}
	inline Dimension getDirection(){
		return direction;
	}
	inline LayerType getType(){
		return type;
	}
	inline double getWidth(){
		return width;
	}
	inline double getMaxWidth(){
		return maxWidth;
	}
	inline double getPitch(){
		return pitch;
	}
	inline vector<Rect> getBlocks(){
		return blocks;
	}
private:
	friend class Block;
};


#endif /* LAYER_H_ */

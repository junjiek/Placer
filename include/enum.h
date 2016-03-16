//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// Define various enum type  
//
// Author: Wang Sifei
// History: 2012/02/12 created by Sifei Wang
//*****************************************************************************
//*****************************************************************************

#ifndef _ENUM_H_
#define _ENUM_H_

enum PinType
{
	Input = 0,
	Output = 1,
	INOUT = 2,
  Other = 3,
	numPinTypes = 4
};
enum NodeType
{
	Moved = 0,
	Fixed = 1,
	FixNI = 2,
	LeftFixed = 3,
	RightFixed = 4,
	numNodeTypes = 5
};

enum LayerType
{
	Routing = 0,
	Cut = 1,
	OtherLayerType = 2,
};
  enum Dimension {
    kX = 0,
    kY = 1,
    kZ = 2,
    kUndefinedDimension = 3 
  };  

  enum Direction {
    kXL = 0,
    kXR = 1,
    kYB = 2,
    kYT = 3,
    kZD = 4,
    kZU = 5,
    kUndefinedDirection = 6 
  };  

 enum Rotation {
    kNorth = 0,
    kSouth = 1,
    kWest  = 2,
    kEast  = 3,
    kFlipNorth = 4,
    kFlipSouth = 5,
    kFlipWest  = 6,
    kFlipEast  = 7
  };

  enum Orient{
    kR0       = 0,
    kR90      = 1,
    kR180     = 2,
    kR270     = 3,           
    kMY       = 4,
    kMYR90    = 5,
    kMX       = 6,
    kMXR90    = 7
  };

#endif

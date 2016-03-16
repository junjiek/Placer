//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <staticData.h>
//
// Static Data manipulation class
//
// Author: Lu Yongqiang
// History: 2009/5/25 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_STATIC_DATA_H_)
#define _SC_STATIC_DATA_H_

#include "datatype.h"

// *****************************************************************************
// class StaticData
//
// A pool for user to register all static data members of application, which 
// will automatically be destroyed before application quits or reset.
// The only thing user need to do is to derive a class off this class which
// overloads the virtual function clear()/setup(), in which user defines the 
// customized clearing/setup operations for your own static data. 
// After derivation, you just need to declare such a variable of this 
// derived class that the static data allocated in memory.
// E.g. MyStatic : public StaticData {
//      public: clear() {...}; setup() {...};};
// static MyStatic staticWatch; // use this static member to manage all your 
//                                static data
// There is a example in base/design/floorplan.h, named FlpStatics.
// *****************************************************************************
class SessionManager;

class StaticData 
{
public:
    StaticData()
    {
        // register this/derived object
        getAllStaticsPool().add(this);
        //cout<<"register static "<<endl;
    }
    ~StaticData()
    {
    };

private:
    virtual void clear()
    {
        // here to clear all your static data, such as newed objects, in your
        // derived class' clear() function
    };
    virtual void setup()
    {
        // here to setup all your static data, such as new pointers
        // But remeber to release them in your clean() function
    };
    static Array<StaticData*>& getAllStaticsPool()
    {
        static Array<StaticData*> *stdAllStatics = new Array<StaticData*>;
        return *stdAllStatics;
    };

    // static data reset, only feasible for SessionManager
    static void releaseAll()
    {
        // clear each static members registered in managers,
        // and free the static object managers
        Array<StaticData*> &sdAllStatics(getAllStaticsPool());
        for (UInt4 i = 0; i < sdAllStatics.getNumElements(); i++) {
        }
        for (UInt4 i = 0; i < sdAllStatics.getNumElements(); i++) {
            sdAllStatics[i]->clear();
        }
        sdAllStatics.clear();
    }

    static void reset()
    {
        Array<StaticData*> &sdAllStatics(getAllStaticsPool());
        // at first clear all old
        for (UInt4 i = 0; i < sdAllStatics.getNumElements(); i++) {
            sdAllStatics[i]->clear();
        }
        // then setup the new
        for (UInt4 i = 0; i < sdAllStatics.getNumElements(); i++) {
            sdAllStatics[i]->setup();
        }
    }

private:
    friend class SessionManager;
};


#endif

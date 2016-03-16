//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <diptr.h>
//
// Two-slot auto ptr
//
// Author: Lu Yongqiang
// History: 2009/5/5 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined (_SC_BIPTR_H_)
#define _SC_BIPTR_H_
// utility for different type of pointers conversion
// NOTE: MainT pointer will be deleted automatically
// while the SubsT pointer will not touched when the object destructs
template <class MainT, class SubsT>
class DiPtr
{
public:
    union {
        // main pointer
        MainT *main;
        // substitute backup pointer
        SubsT *subst;
    };
    enum DIPTR_TYPE{
        // main pointer current
        PTR_MAIN = 0,
        // subsitute pointer valid in current
        PTR_SUBST = 1
    } type;

    inline DiPtr()
    {
        main = NULL;
        type = PTR_MAIN;
    };

    // copy constructor, union safe
    inline DiPtr(const DiPtr *ptr) 
        : main(ptr->main),
          type(ptr->type) { };

    inline DiPtr(const DiPtr &ptr) 
        : main(ptr.main),
          type(ptr.type) { };

    inline DiPtr(MainT *ptr) 
        : main(ptr),
          type(PTR_MAIN) { };

    inline DiPtr(SubsT *ptr) 
        : subst(ptr),
          type(PTR_SUBST) { };

    inline ~DiPtr() { };

    inline DIPTR_TYPE getType() const
    {
        return type;
    }
    bool operator==(const void *ptr)
    {
        return ((void*)main == ptr);
    }

    DiPtr& operator=(MainT *ptr)
    {
        this->main = ptr;
        type = PTR_MAIN;
        return *this;
    };
    DiPtr& operator=(SubsT *ptr)
    {
        this->subst = ptr;
        type = PTR_SUBST;
        return *this;
    };

    DiPtr& operator=(const DiPtr *ptr)
    {
        main = ptr->main;
        type = ptr->type;
        return *this;
    };
    DiPtr& operator=(const DiPtr &ptr)
    {
        main = ptr.main;
        type = ptr.type;
        return *this;
    };
    void operator delete(void *ptr)
    {
        DiPtr biptr = (DiPtr) ptr;
        if (biptr.type == PTR_MAIN) {
            ::delete biptr.main;
            biptr.main = NULL;
        }
        else {
            ::delete biptr.subst;
            biptr.main = NULL;
        }
    };

    // overload the pointer dereference operator
    MainT* operator->() const
    {
        return main;
    };

    //MainT& operator*() const
    //{
    //    return *main;
    //}

    inline MainT* get1() const
    {
        return ((type == PTR_MAIN)? main : NULL);
    }
    inline SubsT* get2() const
    {
        return ((type == PTR_SUBST)? subst : NULL);
    }
};

#endif

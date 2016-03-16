//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <list.h>
//
// a STL list based double-linked list
//
// Author: Lu Yongqiang
// History: 2009/4/30 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_LIST_H_)
#define _SC_LIST_H_

#include <list>
using namespace std;

// list also has other methods as STL standar methods:
// clear(), size(), remove(T value) etc.
//
template <class T>
class List : public list<T>
{
public:
    typedef typename list<T>::iterator iterator;

    inline void insertLast(const T &data)
    {
        list<T>::push_back(data);
    };
    inline void insertBegin(const T &data)
    {
        list<T>::push_front(data);
    };

    // default add is for adding to the last
    inline void add(const T &data)
    {
        insertLast(data);
    };

    inline void insert(iterator &pos, T &val)
    {
        list<T>::insert(pos, sizeof(T), val);
    };

    inline T getLast()
    {
        return list<T>::back();
    };
    inline T getBegin()
    {
        return list<T>::front();
    };

    // delete and get
    inline T popLast()
    {
        T val = getLast();
        list<T>::pop_back();
        return val;
    };
    inline T popBegin()
    {
        T val = getBegin();
        list<T>::pop_front();
        return val;
    };

    inline void removeLast()
    {
        list<T>::pop_back();
    };
    inline void removeBegin()
    {
        list<T>::pop_front();
    };

    inline bool isEmpty()
    {
        return list<T>::empty();
    }

    inline unsigned int getNumElements()
    {
        return list<T>::size();
    }
    // Other methods of standar STL
    // clear() for clear all items
    // size() for the list size, O(n) time
    // remove(val) to remove the item with val in the list, O(n) time
/* commented on 04/05/09 16:21:44 
    List<T>& operator=(const List<T> &list)
    {
        *((list<T> *)this) = *((list<T> *) &list);
        return (*this);
    };
 */
};

// iterator for List
// can delete items in item traversing with iterators, without any
// performance issues.

template <class T>
class ListIter
{
public:
    inline ListIter(List<T> &listIn) : list(&listIn)
    {
        sIter = list->begin();
        isFirst = true;
    };

    inline ~ListIter()
    {
    };

    inline bool getNext(T &data)
    {
        if (isFirst) {
            isFirst = false;
        }
        else {
            sIter++;
        }

        if (sIter != list->end()) {
            data = *sIter;
            return true;
        }
        return false;
    };
    // delete the current iter's data
    inline void removeCurrent()
    {
        // siter locates the next data after removal
        sIter = list->remove(sIter);
        // only not to trigger ++ in getNext
        isFirst = true;
    }
    inline void reset()
    {
        sIter = list->begin();
        isFirst = true;
    }
    ListIter<T>& operator=(const ListIter<T> &it)
    {
        this->isFirst = it.isFirst;
        this->sIter = it.sIter;
        this->list = it.list;
        return *this;
    };

protected:
    typename list<T>::iterator sIter;
    bool isFirst;
    List<T> *list;
};

#endif

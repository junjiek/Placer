//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <hash.h>
//
// Defines two kinds of mappers, STL hash table based MapHash and STL 
// (Red-black tree) MapTree. The former has big memory and constant search time;
// the latter has no redundant memory and log(n) search time.
// For small and medium data volum, say 10k, please use MapTree; otherwise use
// Hash map.
// 
//
// NOTE: Users can define other tables/maps similarly. You can just add them 
//       into this file and log them in this header
//
// NOTE: all iterator should be remove-safe, insert-safe (not 
//       well-tested yet). For best safety, please try not to insert items
//       in iterator, but record them in iterator and insert them afterwards.
//
// Author: Lu Yongqiang
// History: 2009/4/16 created by Yongqiang
// 
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_HASH_H_)
#define _SC_HASH_H_


#include <iostream>
#include <map>
#include "./scHashTable.h"

using namespace std;

// *****************************************************************************
// class MapHash
//
// Use this MapHash to get any pair-data based hash map
// Here, we also pre-defined some ptr based HashPtr, HashPtrSet, HashStr etc.
// *****************************************************************************

// HashTable<K, D, H, E> is a STL hash map, now wrap it
template <class K, class D, class H = SCHashFunction<K>, class E = SCIsEqualTo<K> >
class MapHash : public SCHashTable<K, D, H, E>
{
public:
    typedef typename SCHashTable<K, D, H, E>::iterator iterator;

    enum {
        HASH_FAIL = 0,
        HASH_SUCC = 1,
        HASH_SUCC_UPDATE = 2
    };

public:
    // when updateRepeat is true, the data attached to the key will be updated
    // when a former key (object) found already in the table in adding new ones
    inline MapHash(bool updateRepeatIn = false, unsigned int sizeIn = 32)  
                : SCHashTable<K, D, H, E>(sizeIn),
                  updateRepeat(updateRepeatIn) { };
    inline ~MapHash() { };
public:
    // return 
    // <=0: not successful, repeated found when no update allowed on repeat
    //   1: successful and no repeat with the same key found
    // > 1: successfull and update the former data attached to the key if upate
    //      repeate is allowed (updateRepeat is on)
    int add(const K &key, const D &dataIn);
    inline bool get(const K &key, D &val)
    {
        return SCHashTable<K, D, H, E>::find(key, val);
    };

    inline bool exists(const K &key) const
    {
        D val;
        return SCHashTable<K, D, H, E>::find(key, val);
    };
    inline int insert(const K &key, const D &dataIn)
    {
        return add(key, dataIn);
    }
    inline unsigned int getNumElements() const 
    { 
        return SCHashTable<K, D, H, E>::size();
    };

    // clear function is just from base class
    //void clear()

protected:
    bool updateRepeat;

};


// *****************************************************************************
// HashIter,  iter for MapHash 
// This iter template can use directly by HashIter<K, D> iter and loop.
// It can be used for any type of Hash table defined here
// e.g. HashIter<YourType*, Another_Type*> pIter 
// to traverse a pointer hash table with <YourType*, Another_Type*> pairs
// *****************************************************************************
template <class K, class D, class H = SCHashFunction<K>, class E = SCIsEqualTo<K> >
class HashIter : public SCHashTableIter<K, D, H, E>
{
public:
    inline HashIter(MapHash<K, D, H, E> &mapIn) 
        : SCHashTableIter<K, D, H, E>(&mapIn) { };
};

// *****************************************************************************
// HashPtr, a pre-defined Map only for storing ptr-key based values.
// D is value (key is pointer)
// *****************************************************************************

template <class K, class D>
class HashPtr : public MapHash<K, D, SCHashFunction<void*>, SCIsEqualTo<void*> >
{
public:
    inline HashPtr(bool updateRepeatIn = false, unsigned int sizeIn = 32)
                : MapHash<K, D, SCHashFunction<void*>, SCIsEqualTo<void*> >(updateRepeatIn, sizeIn) 
    { };
    inline ~HashPtr() { };
};


template <class K, class D>
class HashPtrIter : public HashIter<K, D, SCHashFunction<void*>, SCIsEqualTo<void*> >
{
public:
    inline HashPtrIter(HashPtr<K, D> &mapIn) 
        : HashIter<K, D, SCHashFunction<void*>, SCIsEqualTo<void*> >(mapIn) { };
};

// *****************************************************************************
// HashPtrSet, a utility only for ptrs to avoid ptr repeat.
// By using it, we can simply avoid repeat operating pointers
// e..g
// HashPtrSet visited;
// if (visited.add(ptr)) { // if can add, it is not visited ever
//      processNotVisitedItems(ptr);
// }
// *****************************************************************************
template <class K>
class HashPtrSet : public HashPtr<K, int>
{
public:
    inline HashPtrSet() 
        : HashPtr<K, int>(false) { };
    inline ~HashPtrSet() { };
    inline int add(const K &key)
    {
        // if repeat appears, cannot add the key to table
        return HashPtr<K, int>::add(key, 1);
    };
};

template <class K>
class HashPtrSetIter : public HashPtrIter<K, int>
{
public: 
    inline HashPtrSetIter(HashPtrSet<K> &mapIn) 
        : HashPtrIter<K, int>(mapIn) { };

    K getNext()
    {
        K key = (K) 0;
        int val = 0;
        HashPtrIter<K, int>::getNext(key, val);
        return key;
    };
};

// *****************************************************************************
// HashStr, a pre-defined Map only for storing string-key based values.
// D is value (key is char* string)
// *****************************************************************************

template <class D>
class HashStr: public MapHash<string, D>
{
public:

    inline HashStr(bool updateRepeatIn = false, unsigned int sizeIn = 32)
                : MapHash<string, D>(updateRepeatIn, sizeIn) { };
    inline ~HashStr() { };
};

template <class D>
class HashStrIter : public HashIter<string, D>
{
public:
    inline HashStrIter(HashStr<D> &mapIn) 
        : HashIter<string, D>(mapIn) { };
};

// *****************************************************************************
// MapTree, a red-black tree based data structure, 
// based on STL map. It's a binary-tree map
// pair <K, D> is stored to table
// K is the key, D is value
// *****************************************************************************

// *****************************************************************************
// class MapCompare
//
// defines the compare function used in MapTree.sort()
// If your key is pointer, and you want to sort the objects by some meaning of
// the object, you must derive a class off MapCompare or write a class like 
// and use MapTree template to create your tree map. 
// e.g.
// class MapObjCompare : MapCompare { inline bool operator() ...};
// typedef MapTree<YourObjType*, YourValueType, MapObjCompare> YourMapType;
// YourMapType var;
// use var to operate the map, like add, remove the YourObjType objects
// NOTE: if you use this class directly for compare, please note the comparison
// with integral types are defined here, otherwise the behavior not defined, 
// e.g. you use this compare function to sort a string map.
// *****************************************************************************

// to derive from this class, please note the K type. It is the same as
// those of key1 and key2, which might affect the function definition form.
// To override this operator, you must keep the function declaration consistent
// with this template, please refer to the following pre-defined MapCompareStr
template <class K>
class MapCompare
{
public:
    inline bool operator() (const K &key1, const K &key2) const

    {
        // must override when key is not integral types, say structs
        return (key1 < key2);
    };
};
// Here defines several integral based compar functions by typdef template
typedef MapCompare<void*> MapCompareVPtr;
typedef MapCompare<int> MapCompareInt;
typedef MapCompare<unsigned int> MapCompareUInt;

// *****************************************************************************
// MapTree, the map base class
// NOTE: this template can be used directly
// K is the key, D is value, Compare is the comapre function
// *****************************************************************************
// map table base:

template <class K, class D, class Compare>
class MapTree : public map<K, D, Compare>
{
public:
    typedef typename map<K, D, Compare>::iterator iterator;
    typedef typename map<K, D, Compare>::const_iterator const_iterator;

public:
    // when updateRepeat is true, the data attached to the key will be updated
    // when a former key (object) found already in the table in adding new ones
    inline MapTree(bool updateRepeatIn = false) 
                : updateRepeat(updateRepeatIn) { };
    inline ~MapTree() { };

    // return 
    // <=0: not successful, repeated found when no update allowed on repeat
    //   1: successful and no repeat with the same key found
    // > 1: successfull and update the former data attached to the key if upate
    //      repeate is allowed (updateRepeat is on)
    int add(const K &key, const D &dataIn);
    inline int insert(const K &key, const D &dataIn)
    {
        return add(key, dataIn);
    }
    inline void remove(const K &key)
    {
        map<K, D, Compare>::erase(key);
    };
    inline void remove(iterator &iter)
    {
        map<K, D, Compare>::erase(iter);
    };
    inline void clear()
    {
        map<K, D, Compare>::clear();
    };
    bool find(const K &key, D &dataOut);
    inline bool get(const K &key)
    {
        D val;
        return find(key, val);
    };
    // find the key's iterator' index
    inline iterator findIndex(const K &key)
    {
        return map<K, D, Compare>::find(key);
    }
    inline unsigned int getNumElements() const 
    { 
        return map<K, D, Compare>::size();
    };
    inline bool isEmpty() const
    {
        return (getNumElements() == 0);
    };

    // size() to get the size of the element number of the mapper.

public:
    enum {
        MAP_NULL_INDEX = -1,
        MAP_FAIL = 0,
        MAP_SUCC = 1,
        MAP_SUCC_UPDATE = 2
    };

protected:
    bool updateRepeat;
};


// *****************************************************************************
// TreeIter, the map base iter class
// NOTE: this template can be used directly
// You can use this iter and getNext() to traverse the map.
//
// You can also directly use STL::iteraotr,
// e.g.
// TreePtr<int> visited;
// TreePtr<int>::iterator  iter;
// for(iter = visited.begin(); iter != visited.end(); iter++) {
//     //first is the key, second is the value (in pair)
//     cout<<"key is "<<iter.first<<" Value is "<<iter.second<<endl;
// }
// *****************************************************************************

// do not encourage removing in getNext cycles for performance issues
// getNext loop add/remove safe
template <class K, class D, class Compare>
class TreeIter
{
public:
    inline TreeIter(map<K, D, Compare> &mapIn) 
        : sMapIn(&mapIn),
          isBegin(true)
    {
        sIter = sMapIn->begin();
    };
    inline TreeIter(const TreeIter &iter) 
        : sIter(iter.sIter),
          sMapIn(iter.mapIn),
          isBegin(iter.isBegin) { };

    inline bool getNext(K &key, D &dataOut)
    {
        if (isBegin) {
            isBegin = false;
        }
        else {
            sIter++;
        }
        if (sIter != sMapIn->end()) {
            key = sIter->first;
            dataOut = sIter->second;
            return true;
        }
        return false;
    };
    inline void reset()
    {
        sIter = sMapIn->begin();
        isBegin = true;
    }
    inline TreeIter& operator=(const TreeIter &iter)
    {
        isBegin = iter.isBegin;
        sIter = iter.sIter;
        sMapIn = iter.sMapIn;
        return *this;
    }

protected:
    typename MapTree<K, D, Compare>::iterator sIter;
    map<K, D, Compare> *sMapIn;
    bool isBegin;
};

// *****************************************************************************
// TreePtr, a pre-defined Map only for storing ptr-key based values.
// D is value (key is pointer)
// *****************************************************************************

template <class K, class D>
class TreePtr : public MapTree<K, D, MapCompareVPtr>
{
public:
    inline TreePtr(bool updateRepeatIn = false) 
                : MapTree<K, D, MapCompareVPtr>(updateRepeatIn) { };
    inline ~TreePtr() { };
};


template <class K, class D>
class TreePtrIter : public TreeIter<K, D, MapCompareVPtr>
{
public:
    inline TreePtrIter(TreePtr<K, D> &mapIn) 
        : TreeIter<K, D, MapCompareVPtr>(mapIn) { };
};

// *****************************************************************************
// TreePtrSet, a utility only for ptrs to avoid ptr repeat.
// By using it, we can simply avoid repeat operating pointers
// e..g
// TreePtrSet visited;
// if (visited.add(ptr)) { // if can add, it is not visited ever
//      processNotVisitedItems(ptr);
// }
// *****************************************************************************
template <class K>
class TreePtrSet : public TreePtr<K, int>
{
public:
    inline TreePtrSet() 
                : TreePtr<K, int>(false) { };
    inline ~TreePtrSet() { };
    inline int add(const K &key)
    {
        // if repeat appears, cannot add the key to table
        return TreePtr<K, int>::add(key, 1);
    };
    inline bool exists(const K &key)
    {
        int val;
        return TreePtr<K, int>::find(key, val);
    };
};

template <class K>
class TreePtrSetIter : public TreePtrIter<K, int>
{
public: 
    inline TreePtrSetIter(TreePtrSet<K> &mapIn) : 
        TreePtrIter<K, int>(mapIn) { };

    K getNext()
    {
        K key = (K) 0;
        int val = 0;
        TreePtrIter<K, int>::getNext(key, val);
        return key;
    };
};
// *****************************************************************************
// TreeStr, a pre-defined Map only for storing string-key based values.
// D is value (key is char* string)
// *****************************************************************************
class MapCompareStr : public MapCompare<string>
{
public:
    inline bool operator() (const string &key1, const string &key2) const

    {
        // must override when key is not integral types, say structs
        return (strcmp(key1.c_str(), key2.c_str()));
    };
};

template <class D>
class TreeStr: public MapTree<string, D, MapCompareStr>
{
public:

    inline TreeStr(bool updateRepeatIn = false) 
                : MapTree<string, D, MapCompareStr>(updateRepeatIn) { };
    inline ~TreeStr() { };
};

template <class D>
class TreeStrIter : public TreeIter<string, D, MapCompareStr>
{
public:
    inline TreeStrIter(TreeStr<D> &mapIn) 
        : TreeIter<string, D, MapCompareStr>(mapIn) { };
};

// *****************************************************************************
// Hash/Map templates implementation
// *****************************************************************************

template <class K, class D, class H, class E>
int
MapHash<K, D, H, E>::add(const K &key, const D &dataIn)
{
    // add a new one
    pair<iterator, bool> inPair = 
        SCHashTable<K, D, H, E>::insert(pair<K, D>(key, dataIn));

    if (!inPair.second) {
        // repeate found, not succeed
        if (updateRepeat && inPair.first->first == key) {
            // need update repeate
            inPair.first->second = dataIn;
            return HASH_SUCC_UPDATE;
        }
        else {
            return HASH_FAIL;
        }
    }
    return HASH_SUCC;
}

// *****************************************************************************
// STL RedBlackTree map derived implementaion
// *****************************************************************************

template <class K, class D, class Compare>
int
MapTree<K, D, Compare>::add(const K &key, const D &dataIn)
{
    // add a new one
    pair<iterator, bool> inPair = 
        map<K, D, Compare>::insert(pair<K, D>(key, dataIn));

    if (!inPair.second) {
        // repeate found, not succeed
        if (updateRepeat && inPair.first->first == key) {
            // need update repeate
            inPair.first->second = dataIn;
            return MAP_SUCC_UPDATE;
        }
        else {
            return MAP_FAIL;
        }
    }
    return MAP_SUCC;
}

template <class K, class D, class Compare>
bool
MapTree<K, D, Compare>::find(const K &key, D &dataOut)
{
    iterator iter;
    iter = findIndex(key);
    if (iter != map<K, D, Compare>::end()) {
        dataOut = iter->second;
        return true;
    }
    else {
        return false;
    }
}

class DBTEST
{
public:
    static int runTest();
};


#endif

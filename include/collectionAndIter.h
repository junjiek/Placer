//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <collectionAndIter.h>
//
// defines a universal collection and iter interfaces
// default is HashPtrSet based collection and its iter
// others need to be specialized
//
// Usage is similar with that of OA
//
// Author: Lu Yongqiang
// History: 2009/5/4 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_COLLECTION_ITER_H_)
#define _SC_COLLECTION_ITER_H_

#include "oa/oaDesignDB.h"
#include "list.h"
#include "array.h"
#include "hash.h"

using namespace std;
using namespace oa;

// forward declaration
template <class MT> class Iter;

// MT: member class, CT container class
class BaseCollection
{
public:
    typedef enum {
        NON_COLLECTION = 0,
        HASH_COLLECTION = 1,
        LIST_COLLECTION = 2,
        OA_COLLECTION = 3
    } CollectionType;

public:
    inline BaseCollection() 
        : type(NON_COLLECTION),
          createdData(false),
          dataTable(NULL) { };

    inline BaseCollection(const BaseCollection &col) 
        : type(col.type), 
          createdData(col.createdData),
          dataTable(col.dataTable) 
    { 
        // make col.data obsolete
        if (col.createdData) {
            // col has a newed object, should only destruct once
            // make the col absolete
            BaseCollection *tmp = const_cast<BaseCollection*>(&col);
            tmp->dataTable = NULL;
            tmp->type = NON_COLLECTION;
        }
    };

    inline ~BaseCollection() 
    {
        // only delete dataTable when it is created newly
        if (createdData && dataTable) {
            if (type == HASH_COLLECTION) {
                delete ((HashPtrSet<void*>*) dataTable);
            }
            else if (type == LIST_COLLECTION) {
                delete ((List<void*>*) dataTable);
            }
            else if (type == OA_COLLECTION) {
                delete ((oaBaseCollection*) dataTable);
            }
        }
    };
public:
    CollectionType type;
    bool createdData;
    // data table, only accepts List and HashPtrSet table
    void *dataTable;

};

template <class MT>
class Collection : public BaseCollection
{
public:
    inline Collection() {};

    inline Collection(HashPtrSet<MT*> *table, bool isCreatedData = false)
    {
        dataTable = table;
        type = BaseCollection::HASH_COLLECTION;
        createdData = isCreatedData;
    };
    inline Collection(List<MT*> *list, bool isCreatedData = false)
    {
        dataTable = list;
        type = BaseCollection::LIST_COLLECTION;
        createdData = isCreatedData;
    };

    inline Collection(oaBaseCollection *oaColl, bool isCreatedData = false)
    {
        dataTable = oaColl;
        type = BaseCollection::OA_COLLECTION;
        createdData = isCreatedData;
    };

    inline Collection(const Collection &col)
        : BaseCollection(col)
    {
    };
    inline ~Collection()
    {
    };
    inline unsigned int getNumElements()
    {
        if (BaseCollection::type == BaseCollection::HASH_COLLECTION)
        { 
            HashPtrSet<MT*> *ptr = static_cast<HashPtrSet<MT*>*>(BaseCollection::dataTable);
            return ptr->getNumElements() ;
        }
        else if (BaseCollection::type == BaseCollection::LIST_COLLECTION){
            List<MT*> *ptr = static_cast<List<MT*>*>(BaseCollection::dataTable);
            return ptr->getNumElements() ;
        }
        else if (BaseCollection::type == BaseCollection::OA_COLLECTION){
            oaBaseCollection *ptr = 
                static_cast<oaBaseCollection*>(BaseCollection::dataTable);
            return ptr->getCount() ;
        }
        return 0;
    };

    Collection& operator=(Collection &col)
    {
        this->dataTable = col.dataTable;
        this->type = col.type;
        this->createdData = col.createdData;
        if (col.createdData) {
            // col has a newed object, should only destruct once
            // make the col absolete
            col.dataTable = NULL;
            col.type = NON_COLLECTION;
        }
        return *this;
    };

    friend class Iter<MT>;

};

// PLEASE use Collection<MT, CT> instead of Collection<MT*, CT*>
// this is not similar with hash tables and other basic data structures
template <class MT>
class Collection<MT*>
{
private:
    inline Collection(){};
};


template <class MT>
class Iter
{
public:
    Iter(const BaseCollection &col)
        : colType(col.type),
          colNewData(NULL)
    {
        assert(col.dataTable);

        if (colType == BaseCollection::HASH_COLLECTION) {
            sIter = new HashPtrSetIter<MT*>(*static_cast<HashPtrSet<MT*>*>(col.dataTable));
        }
        else if (colType == BaseCollection::LIST_COLLECTION){
            lIter = new ListIter<MT*>(*static_cast<List<MT*>*>(col.dataTable));
        }
        else if (colType == BaseCollection::OA_COLLECTION) {
            oIter = new oaIter<MT>(*static_cast<oaBaseCollection*>(col.dataTable));
        }
        else {
            sIter = NULL;
            return;
        }

        if (col.createdData) {
            // col has a newed object, should only destruct once
            // make the col absolete
            colNewData = col.dataTable;
            BaseCollection *tmp = const_cast<BaseCollection*>(&col);
            tmp->dataTable = NULL;
        }
    };
    ~Iter()
    {
        if (colType == BaseCollection::HASH_COLLECTION && sIter) {
            delete sIter;
            if (colNewData) {
                delete static_cast<HashPtrSet<MT*>*>(colNewData);
            }
        }
        else if (colType == BaseCollection::LIST_COLLECTION && lIter){
            delete lIter;
            if (colNewData) {
                delete static_cast<List<MT*>*>(colNewData);
            }
        }
        else if (colType == BaseCollection::OA_COLLECTION && oIter) {
            delete oIter;
            if (colNewData) {
                delete static_cast<oaBaseCollection*>(colNewData);
            }
        }
    };
    MT* getNext()
    {
        if (colType == BaseCollection::HASH_COLLECTION) {
            return sIter->getNext();
        }
        else if (colType == BaseCollection::LIST_COLLECTION){
            MT *val = NULL;
            if (lIter->getNext(val)) {
                return val;
            }
            else {
                return NULL;
            }
        }
        else if (colType == BaseCollection::OA_COLLECTION) {
            return oIter->getNext();
        }
        else {
            return NULL;
        }
    };
    void reset()
    {
        if (colType == BaseCollection::HASH_COLLECTION) {
            return sIter->reset();
        }
        else if (colType == BaseCollection::LIST_COLLECTION){
            return lIter->reset();
        }
        else if (colType == BaseCollection::OA_COLLECTION) {
            return oIter->reset();
        }
        else {
            ;
        }
    };

protected:
    const BaseCollection::CollectionType colType;
    void *colNewData;
    union {
        HashPtrSetIter<MT*> *sIter;
        ListIter<MT*> *lIter;
        oaIter<MT> *oIter;
    };

};

// no need pointer type to specify, default is pointer type
template <class MT>
class Iter<MT*>
{
private:
    inline Iter() {};
};

#endif

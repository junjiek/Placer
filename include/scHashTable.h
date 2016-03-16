#if !defined(_SC_HASH_TABLE_H_)
#define _SC_HASH_TABLE_H_

#include "./scHashFunc.h"
using namespace std;

// *****************************************************************************
// Forward class declarations.
// *****************************************************************************
template<class K, class D, class H, class E>
class SCHashTableIter;


// *****************************************************************************
// SCHashTableRec
// *****************************************************************************
template<class K, class D>
class SCHashTableRec 
{

public:
    typedef enum {
        Empty,
        Deleted,
        Filled
    } State;

public:
    SCHashTableRec();

    K& key();
    D& data();
    inline pair<K, D>& value()
    {
        return m_dpair;
    };
    State& state();

    bool           isEmpty() const;
    bool           isDeleted() const;
    bool           isFilled() const;

private:
    pair<K, D>              m_dpair;
    State                   m_state;
};



// *****************************************************************************
// SCHashTableArray
// *****************************************************************************
template<class K, class D>
class SCHashTableArray {
public:
                                SCHashTableArray(unsigned int initSize = 2048);
                                SCHashTableArray(const SCHashTableArray<K, D> &init);
                                ~SCHashTableArray();

    SCHashTableRec<K, D>          &operator[](unsigned int index);
    SCHashTableRec<K, D>          operator[](unsigned int index) const;
    SCHashTableArray<K, D>        &operator=(const SCHashTableArray<K, D> &rhs);
    unsigned int                 getSize() const;
    void                        setSize(unsigned int newSize);


private:
    typedef SCHashTableRec<K, D>  Element;
    typedef Element             **PageContainer;

    static const unsigned int    pageShift;
    static const unsigned int    pageSize;
    static const unsigned int    pageMask;
    unsigned int                 nPages;
    unsigned int                 arraySize;

    PageContainer               page;
};



// *****************************************************************************
// SCHashTable
// *****************************************************************************
template<class K, class D, class H = SCHashFunction<K>, class E = SCIsEqualTo<K> >
class SCHashTable 
{
public:
    typedef SCHashTableIter<K, D, H, E> iterator;
    typedef unsigned int  size_type;

public:
    SCHashTable(unsigned int initSize = 512, unsigned int minSize = 512);
    SCHashTable(const SCHashTable<K, D, H, E>   &in);
    ~SCHashTable();

public:
    bool                    find(const K &key, D &data) const;
    bool                    remove(const K &key);
    bool                    remove(iterator &where);
    void                    clear();

    unsigned int            numEntries() const;
    unsigned int            maxEntries() const;

    unsigned long           calcVMSize() const;

    SCHashTable<K, D, H, E>   &operator=(const SCHashTable<K, D, H, E>  &rhs);
    iterator                begin();
    iterator                end();
    bool                    isEmpty() const;
    iterator                find(const K &key);

public:
    inline bool erase(const K &key)
    {
        return remove(key);
    };
    inline bool erase(iterator &where)
    {
        return remove(where);
    };
    inline size_type max_size() const
    {
        return maxEntries();
    };
    inline size_type size() const
    {
        return numEntries();
    };


protected:
    pair<iterator, bool>    insert(const pair<K, D>  &data);
    bool                    remove(SCHashTableRec<K, D> *record);
    unsigned int            findIndex(const K &key, 
                                    SCHashTableRec<K, D> **rec = NULL) const;
    void                    incIterCount();
    void                    decIterCount();
    static unsigned int     powerUp(unsigned int value);

private:
    unsigned int            wrap(unsigned int value) const;
    void                    resizeTable(unsigned int newSize);

    unsigned int            m_numEntries;
    unsigned int            m_numDeleted;
    unsigned int            m_minSize;
    unsigned int            m_tableSize;
    SCHashTableArray<K, D>    *m_table;
    unsigned int            m_iterCount;

    static const unsigned int    m_stride;

    friend class SCHashTableIter<K, D, H, E>;
};



// *****************************************************************************
// SCHashTableIter
// *****************************************************************************
template<class K, class D, class H = SCHashFunction<K>, class E = SCIsEqualTo<K> >
class SCHashTableIter 
{
public:

    SCHashTableIter();
    SCHashTableIter(SCHashTable<K, D, H, E>  *table, const unsigned int index);
    SCHashTableIter(SCHashTable<K, D, H, E> *table,
            const bool   end = false);
    SCHashTableIter(SCHashTable<K, D, H, E> *table,
            const K               &key,
            const bool   keyOnly = true);
    SCHashTableIter(const SCHashTableIter<K, D, H, E> &iter);
    ~SCHashTableIter();

    SCHashTableIter<K, D, H, E>   &operator=(const SCHashTableIter<K, D, H, E> &iter);
    void init(SCHashTable<K, D, H, E> *table);
    void finish();

    // get each element, after it returns, the iterator is still at the current.
    // User and thus modify this iterator simultaneously.
    // This makes removing/modifying even adding element in getNext loop safe.
    bool getNext(K &key, D &data);

    bool operator==(const SCHashTableIter<K, D, H, E> &rhs) const;

    bool operator!=(const SCHashTableIter<K, D, H, E> &rhs) const;

    pair<K, D>&         operator*();

    SCHashTableIter<K, D, H, E>& operator++(int);

    // reset the iter to the begin, the iter can iterator the table again
    inline void reset()
    {
        m_index = 0;
        findNextIndex(false);
        // begin again
        m_iterBegin = true;
    };
    inline pair<K, D>* operator->()
    {
        return &(operator*());
    };

protected:
    bool  getCurr(K &key, D &data) const;

    void  findNextIndex(const bool skipCurrent);

private:
    SCHashTable<K, D, H, E> *m_table;
    unsigned int   m_index;
    bool  m_keyOnly;
    bool  m_iterBegin;
    pair<K, D>   m_value;
    friend class SCHashTable<K, D, H, E>;
};


// *****************************************************************************
// Initialize static data members.
// *****************************************************************************
template<class K, class D, class H, class E>
const unsigned int SCHashTable<K, D, H, E>::m_stride = 31;

template<class K, class D>
const unsigned int SCHashTableArray<K, D>::pageShift = 11;

template<class K, class D>
const unsigned int SCHashTableArray<K, D>::pageSize = 2048;

template<class K, class D>
const unsigned int SCHashTableArray<K, D>::pageMask = 2047;



// *****************************************************************************
// SCHashTableArray::SCHashTableArray()
//
// This is the hash table array constructor.
// *****************************************************************************
template<class K, class D>
SCHashTableArray<K, D>::SCHashTableArray(unsigned int initSize)
: nPages((initSize + pageSize - 1) >> pageShift),
  arraySize(initSize)
{
    page = (PageContainer) malloc(sizeof(Element*) * nPages);

    unsigned int allocSize = nPages == 1 ? arraySize : pageSize;

    for (unsigned int i = 0; i < nPages; i++) {
        page[i] = new Element[allocSize];
    }
}


// *****************************************************************************
// SCHashTableArray::SCHashTableArray()
//
// This is the copy constructor for the hash table array.
// *****************************************************************************
template<class K, class D>
SCHashTableArray<K, D>::SCHashTableArray(const SCHashTableArray<K, D> &init)
: nPages(init.nPages),
  arraySize(init.arraySize)
{
    page = (PageContainer) malloc(sizeof(Element*) * nPages);

    unsigned int allocSize = nPages == 1 ? arraySize : pageSize;

    for (unsigned int i = 0; i < nPages; i++) {
        Element *newPage = new Element[allocSize];

        for (unsigned int j = 0; j < allocSize; j++) {
            newPage[j] = (init.page[i])[j];
        }

        page[i] = newPage;
    }
}



// *****************************************************************************
// SCHashTableArray::~SCHashTableArray()
//
// This is the hash table array destructor.
// *****************************************************************************
template<class K, class D>
SCHashTableArray<K, D>::~SCHashTableArray()
{
    for (unsigned int i = 0; i < nPages; i++) {
        delete [] page[i];
    }

    free(page);
}



// *****************************************************************************
// SCHashTableArray::operator[]()
//
// These are the index operators for the array.
// *****************************************************************************
template<class K, class D>
inline SCHashTableRec<K, D> &
SCHashTableArray<K, D>::operator[](unsigned int index)
{
    return ((Element *) (page[index >> pageShift]))[index & pageMask];
}

template<class K, class D>
inline SCHashTableRec<K, D>
SCHashTableArray<K, D>::operator[](unsigned int index) const
{
    Element *array = page[index >> pageShift];

    return array[index & pageMask];
}


// *****************************************************************************
// SCHashTableArray::operator=()
//
// This is the assignment operator for the hash table array.
// *****************************************************************************
template<class K, class D>
SCHashTableArray<K, D> &
SCHashTableArray<K, D>::operator=(const SCHashTableArray<K, D> &rhs)
{
    for (unsigned int i = 0; i < nPages; i++) {
        delete [] page[i];
    }

    page = (PageContainer) realloc(page, sizeof(Element*) * rhs.nPages);
    nPages = rhs.nPages;

    unsigned int allocSize = nPages == 1 ? rhs.arraySize : pageSize;

    for (unsigned int i = 0; i < nPages; i++) {
        Element *newPage = new Element[allocSize];

        for (unsigned int j = 0; j < allocSize; j++) {
            newPage[j] = (rhs.page[i])[j];
        }

        page[i] = newPage;
    }

    arraySize = rhs.arraySize;

    return *this;
}



// *****************************************************************************
// SCHashTableArray::getSize()
//
// This function returns the size of the array.
// *****************************************************************************
template<class K, class D>
unsigned int
SCHashTableArray<K, D>::getSize() const
{
    return arraySize;
}



// *****************************************************************************
// SCHashTableArray::setSize()
//
// This function resizes the array up or down.
// *****************************************************************************
template<class K, class D>
void
SCHashTableArray<K, D>::setSize(unsigned int newSize)
{
    unsigned int reqPages = (newSize + pageSize - 1) >> pageShift;

    if (newSize > arraySize) {
        page = (PageContainer) realloc(page, sizeof(Element*) * reqPages);

        unsigned int allocSize = reqPages == 1 ? newSize : pageSize;

        if (reqPages == 1) {
            Element *newPage = new Element[allocSize];

            for (unsigned int j = 0; j < arraySize; j++) {
                newPage[j] = (page[0])[j];
            }

            delete page[0];
            page[0] = newPage;
        } else {
            while (nPages < reqPages) {
                page[nPages++] = new Element[allocSize];
            }
        }
    } else {
        while (nPages > reqPages) {
            delete [] page[--nPages];
        }

        page = (PageContainer) realloc(page, sizeof(Element*) * reqPages);
    }

    arraySize = newSize;
}






// *****************************************************************************
// SCHashTableRec<K, D>::SCHashTableRec()
//
// This is the constructor for the hash table record.
// *****************************************************************************
template<class K, class D>
inline
SCHashTableRec<K, D>::SCHashTableRec()
: m_state(Empty)
{
}



// *****************************************************************************
// SCHashTableRec<K, D>::key()
// SCHashTableRec<K, D>::data()
//
// These functions return a reference to the record's key or data.
// *****************************************************************************
template<class K, class D>
inline K &
SCHashTableRec<K, D>::key()
{
    return m_dpair.first;
}

template<class K, class D>
inline D &
SCHashTableRec<K, D>::data()
{
    return m_dpair.second;
}



// *****************************************************************************
// SCHashTableRec<K, D>::state()
//
// This function returns a reference to the state of the record.
// *****************************************************************************
template<class K, class D>
inline typename SCHashTableRec<K, D>::State &
SCHashTableRec<K, D>::state()
{
    return m_state;
}



// *****************************************************************************
// SCHashTableRec<K, D>::isEmpty()
// SCHashTableRec<K, D>::isDeleted()
// SCHashTableRec<K, D>::isFilled()
//
// These functions test the state of the record.
// *****************************************************************************
template<class K, class D>
inline bool
SCHashTableRec<K, D>::isEmpty() const
{
    return m_state == Empty;
}

template<class K, class D>
inline bool
SCHashTableRec<K, D>::isDeleted() const
{
    return m_state == Deleted;
}

template<class K, class D>
inline bool
SCHashTableRec<K, D>::isFilled() const
{
    return m_state == Filled;
}



// *****************************************************************************
// SCHashTable::SCHashTable()
//
// These are the hash table constructors. The first creates a new, empty hash
// table. The second form creates a new hash table by copying and existing hash
// table.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTable<K, D, H, E>::SCHashTable(unsigned int    initSize,
                                 unsigned int    minSize)
: m_numEntries(0),
  m_numDeleted(0),
  m_minSize(minSize),
  m_tableSize(powerUp(initSize)),
  m_table(new SCHashTableArray<K, D>(m_tableSize)),
  m_iterCount(0)
{
}

template<class K, class D, class H, class E>
SCHashTable<K, D, H, E>::SCHashTable(const SCHashTable<K, D, H, E>    &in)
: m_numEntries(in.m_numEntries),
  m_numDeleted(in.m_numDeleted),
  m_minSize(in.m_minSize),
  m_tableSize(in.m_tableSize),
  m_table(new SCHashTableArray<K, D>(m_tableSize)),
  m_iterCount(in.m_iterCount)
{
    *m_table = *in.m_table;
}



// *****************************************************************************
// SCHashTable::~SCHashTable()
//
// This is the hash table destructor.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTable<K, D, H, E>::~SCHashTable()
{
    delete m_table;
}



// *****************************************************************************
// SCHashTable::operator=()
//
// This is the assignment operator for the hash table.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTable<K, D, H, E>&
SCHashTable<K, D, H, E>::operator=(const SCHashTable<K, D, H, E>    &rhs) 
{
    m_numEntries = rhs.m_numEntries;
    m_numDeleted = rhs.m_numDeleted;
    m_minSize = rhs.m_minSize;
    m_tableSize = rhs.m_tableSize;
    
    delete m_table;

    m_table = new SCHashTableArray<K, D>(m_tableSize);
    *m_table = *rhs.m_table;
    m_iterCount = rhs.m_iterCount;

    return *this;
}



// *****************************************************************************
// SCHashTable::find()
//
// This function looks up a key in the hash table.  If an entry with that key is
// found, then the function returns true and passes back the matching data; if
// there is no entry with that key, then the function returns false.
// *****************************************************************************
template<class K, class D, class H, class E>
bool
SCHashTable<K, D, H, E>::find(const K &key,
                            D       &data) const
{
    SCHashTableRec<K, D> *record;

    findIndex(key, &record);

    if (record && record->isFilled()) {
        data = record->data();
        return true;
    }

    return false;
}



// *****************************************************************************
// SCHashTable::findIndex()
//
// This function looks up a key, and returns the index of the entry containing
// it, or m_tableSize if it is not found.  This is done by hashing the key, and
// then walking the table from there in steps of length m_stride until it either
// finds an empty entry or finds the target key.  If a rec pointer has been 
// supplied and a record with the matching key is found, then the rec pointer
// will be set to the record that was found.  If a rec pointer has been 
// supplied, but no record with a matching key is found, then the rec pointer 
// will be set to NULL.
// *****************************************************************************
template<class K, class D, class H, class E>
unsigned int
SCHashTable<K, D, H, E>::findIndex(const K            &key,
                                 SCHashTableRec<K, D> **rec) const
{
    unsigned int         index = wrap(H()(key));
    unsigned int         firstIndex = index;
    SCHashTableRec<K, D>  *record = &(*m_table)[index];

    while (record && !record->isEmpty()) {
        if (record->isFilled() && E()(record->key(), key)) {
            if (rec) {
                *rec = record;
            }

            return index;
        }

        index = wrap(index + m_stride);

        if (index == firstIndex) {
            break;
        }

        record = &(*m_table)[index];
    }

    if (rec) {
        *rec = NULL;
    }

    return m_tableSize;
}



// *****************************************************************************
// SCHashTable::powerUp()
//
// This is a utility function to compute the power of two that is greater than
// or equal to a given value. This function is used to initialize the size of
// the hash tables. The hash table must be at least twice as large as the
// number of items in the table and the size of the table must be a power of 
// two. This function will not return anything smaller than 2 or larger than 
// 2^31.
// *****************************************************************************
template<class K, class D, class H, class E>
unsigned int
SCHashTable<K, D, H, E>::powerUp(unsigned int value)
{
    unsigned int power = 31;
    unsigned int vmask = 0x1 << power;

    while (power > 1 && (vmask & value) == 0) {
        --power;
        vmask = vmask >> 1;
    }

    if (power != 31 && (value & (vmask - 1)) != 0) {
        vmask = vmask << 1;
    }

    return vmask;
}



// *****************************************************************************
// SCHashTable::insert()
//
// This function inserts a key/data pair.  This is done by hashing the key, and
// then walking the table from there in steps of length m_stride until it either
// finds an empty entry or finds the target key.  In the latter case, it will 
// return the place and false
// *****************************************************************************
template<class K, class D, class H, class E>
pair<SCHashTableIter<K, D, H, E>, bool>
SCHashTable<K, D, H, E>::insert(const pair<K, D> &data)
{
    // Enlarge the table if it is about to become over half full.
    if (m_numEntries > m_tableSize / 2) {
        resizeTable(m_tableSize * 2);
    }

    // Walk the entries in steps of size m_stride, starting from the key's hash
    // value, until the key or an available (empty or deleted) entry is found.
    unsigned int         index = wrap(H()(data.first));
    SCHashTableRec<K, D>  *record = &(*m_table)[index];

    while (record->isFilled() && !E()(data.first, record->key())) {
        index = wrap(index + m_stride);
        record = &(*m_table)[index];
    }

    // If the entry found is filled, then its key must equal the insertion key.
    if (record->isFilled()) {
        // do not overwrite the current existance, leave it for user's decision
        return pair<SCHashTableIter<K, D, H, E>, bool>(
                SCHashTableIter<K, D, H, E>(this, index), false);
    }

    // Fill the entry.
    record->key() = data.first;
    record->data() = data.second;
    record->state() = SCHashTableRec<K, D>::Filled;
    m_numEntries++;
    return pair<SCHashTableIter<K, D, H, E>, bool>(
            SCHashTableIter<K, D, H, E>(this, index), true);
}



// *****************************************************************************
// SCHashTable::remove()
//
// This function removes a key/data pair from the hash table by marking it
// Deleted.  The function returns true if the element was found and removed, or
// false if it was not in the table.  If the table becomes less than 1/8 filled,
// it is resized smaller.  If it becomes too heavy with deleted entries, it is
// rehashed with the same size.
// *****************************************************************************
template<class K, class D, class H, class E>
bool
SCHashTable<K, D, H, E>::remove(SCHashTableRec<K, D> *record)
{
    if (record) {
        record->state() = SCHashTableRec<K, D>::Deleted;

        m_numEntries--;
        m_numDeleted++;
        if (m_iterCount == 0) {
            if (m_tableSize > m_minSize && m_numEntries < m_tableSize / 8) {
                resizeTable(m_tableSize / 2);
            } else if (m_numDeleted > 10 && m_numDeleted > m_numEntries / 4) {
                resizeTable(m_tableSize);
            }
        }

        return true;
    }

    return false;
}
template<class K, class D, class H, class E>
bool
SCHashTable<K, D, H, E>::remove(const K &key)
{
    SCHashTableRec<K, D> *record;

    findIndex(key, &record);
    return remove(record);
}
template<class K, class D, class H, class E>
bool
SCHashTable<K, D, H, E>::remove(iterator &where)
{
    if (where.m_index < m_tableSize) {
        SCHashTableRec<K, D> &rec = (*m_table)[where.m_index];
        return remove(&rec);
    }
    else {
        return false;
    }
}



// *****************************************************************************
// SCHashTable::clear()
//
// This function clears the table, i.e. empties every entry.
// *****************************************************************************
template<class K, class D, class H, class E>
void
SCHashTable<K, D, H, E>::clear()
{
    for (unsigned int i = 0; i < m_tableSize; i++) {
        (*m_table)[i].state() = SCHashTableRec<K, D>::Empty;
    }

    m_numEntries = 0;
    m_numDeleted = 0;
}



// *****************************************************************************
// SCHashTable<K, D, H, E>::numEntries()
// SCHashTable<K, D, H, E>::maxEntries()
//
// These functions return the current and allocated number of entries in the
// hash table.
// *****************************************************************************
template<class K, class D, class H, class E>
inline unsigned int
SCHashTable<K, D, H, E>::numEntries() const
{
    return m_numEntries;
}

template<class K, class D, class H, class E>
inline unsigned int
SCHashTable<K, D, H, E>::maxEntries() const
{
    return m_tableSize;
}



// *****************************************************************************
// SCHashTable::calcVMSize()
//
// This function calculates and returns the number of bytes this hash table is
// currently using in virtual memory including all memory it allocates.
// *****************************************************************************
template<class K, class D, class H, class E>
inline unsigned long
SCHashTable<K, D, H, E>::calcVMSize() const
{
    return sizeof(*this) + m_table->calcVMSize();
}



// *****************************************************************************
// SCHashTable::wrap()
//
// This method converts the given value to a table index by mod'ing by the table
// size.  It is assumed that the table size is a power of two.
// *****************************************************************************
template<class K, class D, class H, class E>
inline unsigned int
SCHashTable<K, D, H, E>::wrap(unsigned int value) const
{
    return value & (m_tableSize - 1);
}



// *****************************************************************************
// SCHashTable::resizeTable()
//
// This function resizes the table to the given size.  No checking is done to be
// sure the new table is large enough to hold all of the filled entries in the
// old table.
// *****************************************************************************
template<class K, class D, class H, class E>
void
SCHashTable<K, D, H, E>::resizeTable(const unsigned int newSize)
{
    const unsigned int       oldTableSize = m_tableSize;
    SCHashTableArray<K, D>    *oldTable = m_table;

    m_tableSize = newSize;
    m_table = new SCHashTableArray<K, D>(m_tableSize);

    clear();

    for (unsigned int i = 0; i < oldTableSize; i++) {
        SCHashTableRec<K, D> &entry = (*oldTable)[i];
	
        if (entry.isFilled()) {
            insert(pair<K, D>(entry.key(), entry.data()));
        }
    }

    delete oldTable;
}



// *****************************************************************************
// SCHashTable<K, D, H, E>::incIterCount()
// SCHashTable<K, D, H, E>::decIterCount()
//
// These functions increment or decrement the iterator reference count.
// *****************************************************************************
template<class K, class D, class H, class E>
inline void
SCHashTable<K, D, H, E>::incIterCount()
{
    m_iterCount++;
}

template<class K, class D, class H, class E>
inline void
SCHashTable<K, D, H, E>::decIterCount()
{
    m_iterCount--;
}


// *****************************************************************************
// SCHashTable::begin()
//
// This method returns an iterator pointing to the first element in 
// this SCHashTable.
// *****************************************************************************

template<class K, class D, class H, class E>
typename SCHashTable<K, D, H, E>::iterator
SCHashTable<K, D, H, E>::begin()
{
    return iterator(this);
}

// *****************************************************************************
// SCHashTable::isEmpty()
//
// This method returns a boolean indicating whether or not this hashTable is
// empty.
// *****************************************************************************
template<class K, class D, class H, class E>
inline bool
SCHashTable<K, D, H, E>::isEmpty() const
{
    return (numEntries() == 0);
}


// *****************************************************************************
// SCHashTable::end()
//
// This method returns an iterator pointing to the first
// element beyond the end of the hash table.
// *****************************************************************************
template<class K, class D, class H, class E>
typename SCHashTable<K, D, H, E>::iterator
SCHashTable<K, D, H, E>::end()
{
    return iterator(this, true);
}

// *****************************************************************************
// SCHashTable::find()
//
// This method returns an iterator pointing to the entry
// with the given key value, if any.  It returns end() if there is no such key
// value in the table.
// *****************************************************************************
template<class K, class D, class H, class E>
typename SCHashTable<K, D, H, E>::iterator
SCHashTable<K, D, H, E>::find(const K &key)
{
    return iterator(this, key);
}

// *****************************************************************************
// SCHashTableIter::SCHashTableIter()
//
// This function creates a new, empty hash table iterator.  It must be
// initialized using the init() method before it can be used; until then, the
// getNext() method will always return false.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E>::SCHashTableIter()
: m_table(NULL),
  m_iterBegin(true)
{
}

// specially for having found the index for data in table
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E>::SCHashTableIter(SCHashTable<K, D, H, E>  *table,
                                         const unsigned int index)
: m_table(table),
  m_index(index),
  m_iterBegin(true)
{
    m_table->incIterCount();
}

// *****************************************************************************
// SCHashTableIter::SCHashTableIter()
//
// This function creates a new hash table iterator.  If 'end' is true, then the
// iterator is set to the state it would be in if it were finished iterating.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E>::SCHashTableIter(SCHashTable<K, D, H, E>  *table,
                                         const bool    end)
: m_table(table),
  m_index(end ? table->m_tableSize : 0),
  m_keyOnly(end),
  m_iterBegin(true)
{
    m_table->incIterCount();

    if (!end) {
        findNextIndex(false);
    }
}



// *****************************************************************************
// SCHashTableIter::SCHashTableIter()
//
// This function creates a new hash table iterator that will start from the
// entry with the given key value.  The parameter 'keyOnly' indicates whether
// the iterator should continue producing entries after the keyed one, or only
// produce the single keyed entry.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E>::SCHashTableIter(SCHashTable<K, D, H, E>  *table,
                                         const K                &key,
                                         const bool    keyOnly)
: m_table(table),
  m_index(table->findIndex(key)),
  m_keyOnly(keyOnly),
  m_iterBegin(true)
{
    m_table->incIterCount();
}



// *****************************************************************************
// SCHashTableIter::SCHashTableIter()
//
// This copy constructor creates a new hash table iterator that is a copy of an
// existing one.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E>::SCHashTableIter(const SCHashTableIter<K, D, H, E> &iter)
: m_table(iter.m_table),
  m_index(iter.m_index),
  m_keyOnly(iter.m_keyOnly),
  m_iterBegin(iter.m_iterBegin)
{
    if (m_table) {
        m_table->incIterCount();
    }
}

// *****************************************************************************
// SCHashTableIter::operator=(const SCHashTableIter&)
//
// This is the assignment operator for SCHashTableIter
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E> &
SCHashTableIter<K, D, H, E>::operator=(const SCHashTableIter<K, D, H, E> &iter)
{
    if (this != &iter) {
        m_table = iter.m_table;
        m_index = iter.m_index;
        m_keyOnly = iter.m_keyOnly;
        m_iterBegin = iter.m_iterBegin;
        m_value = iter.m_value;
    }

    return *this;
}


// *****************************************************************************
// SCHashTableIter::~SCHashTableIter()
//
// This function destroys a hash table iterator.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E>::~SCHashTableIter()
{
    if (m_table) {
        m_table->decIterCount();
    }
}



// *****************************************************************************
// SCHashTableIter::init()
//
// This method initializes the hash table iterator to generate all entries from
// the given table.
// *****************************************************************************
template<class K, class D, class H, class E>
void
SCHashTableIter<K, D, H, E>::init(SCHashTable<K, D, H, E> *table)
{
    if (m_table) {
        m_table->decIterCount();
    }
    m_table = table;
    m_index = 0;
    m_keyOnly = false;
    if (m_table) {
        m_table->incIterCount();
        findNextIndex(false);
    }
}



// *****************************************************************************
// SCHashTableIter<K, D, H, E>::finish()
//
// This function terminates the iterator.
// *****************************************************************************
template<class K, class D, class H, class E>
inline void
SCHashTableIter<K, D, H, E>::finish()
{
    init(NULL);
}



// *****************************************************************************
// SCHashTableIter::getNext()
//
// This function returns the key and data portions of the next filled entry in
// the hash table.  The first call returns the first nonempty entry in the
// table.  The function returns false when there are no more entries.
// *****************************************************************************
template<class K, class D, class H, class E>
bool
SCHashTableIter<K, D, H, E>::getNext(K    &key,
                                   D    &data)
{
    if (m_iterBegin) {
        m_iterBegin = false;
    }
    else {
        // begin dont move
        findNextIndex(true);
    }

    if (!m_table || m_index >= m_table->m_tableSize) {
        return false;
    }

    SCHashTableRec<K, D> &rec = (*(m_table->m_table))[m_index];
    key = rec.key();
    data = rec.data();

    return true;
}



// *****************************************************************************
// SCHashTableIter::findNextIndex()
//
// This function increments the index, if necessary, until it is the index of a
// nonempty entry in the hash table.  If 'skipCurrent' is true, then this method
// finds the next nonempty entry *after* the current one; if not, then it will
// return the current entry if it is nonempty, incrementing only if necessary.
// *****************************************************************************
template<class K, class D, class H, class E>
void
SCHashTableIter<K, D, H, E>::findNextIndex(const bool skipCurrent)
{
    if (m_keyOnly) {
        m_index = m_table->m_tableSize;
        return;
    }

    if (skipCurrent) {
        m_index++;
    }

    while (m_index < m_table->m_tableSize
           && !(*(m_table->m_table))[m_index].isFilled()) {
        m_index++;
    }
}



// *****************************************************************************
// SCHashTableIter::getCurr()
//
// This function returns the key and data portions of the current entry in the
// hash table and returns true, unless the iterator is finished, in which case
// it returns false.
// *****************************************************************************
template<class K, class D, class H, class E>
bool
SCHashTableIter<K, D, H, E>::getCurr(K    &key,
                                   D    &data) const
{
    if (m_index >= m_table->m_tableSize) {
        return false;
    }

    SCHashTableRec<K, D> &rec = (*(m_table->m_table))[m_index];
    key = rec.key();
    data = rec.data();

    return true;
}



// *****************************************************************************
// SCHashTableIter::operator==()
//
// This function tests this SCHashTableIter for equality with another one.  They
// are equal if they point to the same index in the same SCHashTable, where any
// indices beyond the end of the table are considered equal.
// *****************************************************************************
template<class K, class D, class H, class E>
bool
SCHashTableIter<K, D, H, E>::operator==(const SCHashTableIter<K, D, H, E> &rhs) const
{
    if (m_table != rhs.m_table) {
        return false;
    }

    const unsigned int size = m_table->m_tableSize;
    return m_index >= size && rhs.m_index >= size || m_index == rhs.m_index;
}



// *****************************************************************************
// SCHashTableIter::operator!=()
//
// This function tests this SCHashTableIter for inequality with another one.
// *****************************************************************************
template<class K, class D, class H, class E>
bool
SCHashTableIter<K, D, H, E>::operator!=(const SCHashTableIter<K, D, H, E> &rhs) const
{
    return !(*this == rhs);
}

// *****************************************************************************
// SCHashTableIter::operator*()
//
// This method returns the data value currently pointed to by the iterator.
// *****************************************************************************
template<class K, class D, class H, class E>
pair<K, D>&
SCHashTableIter<K, D, H, E>::operator*()
{
    assert(m_index < m_table->m_tableSize);
    SCHashTableRec<K, D> &rec = (*(m_table->m_table))[m_index];
    return rec.value();
}



// *****************************************************************************
// SCHashTableIter::operator++()
//
// This method 'increments' the iterator to point to the next piece of data in
// the hash table.
// *****************************************************************************
template<class K, class D, class H, class E>
SCHashTableIter<K, D, H, E> &
SCHashTableIter<K, D, H, E>::operator++(int)
{
    // HP compiler warns that you must call via 'this' pointer.
    this->findNextIndex(true);

    return *this;
}


#endif

//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <array.h>
//
// a STL vector based Array
//
// Author: Lu Yongqiang
// History: 2009/4/30 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_ARRAY_H_)
#define _SC_ARRAY_H_
#include <vector>
#include <algorithm>

using namespace std;
const unsigned int ARRAY_INIT_SIZE = 8;

template <class T>
class Array: public vector<T>
{
public:
    // Please refer to STL manual for more details.

    // create an array with sizeIn number of elements, if emptyArray is on
    // the size of the array will be set to 0, and this is
    // to create a pre-alloced space empty array (size == 0)
    inline Array(unsigned int sizeIn, const bool emptyArray = false)
        : vector<T>(sizeIn)
    {
        if (emptyArray) {
            setSize(0);
        }
    };
    // create an array with sizeIn number of copies of def
    inline Array(unsigned int sizeIn, const T &def)
        : vector<T>(sizeIn, def)
    {
    };

    // copy constructor with arrIn
    inline Array(const Array<T> &arrIn)
        : vector<T>(arrIn)
    {
    };
    inline Array()
    {
    };

    inline ~Array()
    {
    };
    inline unsigned int getNumElements() const
    {
        return vector<T>::size();
    };
    inline bool isEmpty()
    {
        return vector<T>::empty();
    };

    inline void append(const T &data)
    {
        vector<T>::push_back(data);
    };
    inline void append(const Array<T> &arrayIn)
    {
        vector<T>::insert(vector<T>::end(), arrayIn.begin(), arrayIn.end());
    };
    inline void add(const T &data)
    {
        vector<T>::push_back(data);
    };
    inline void insertLast(const T &data)
    {
        vector<T>::push_back(data);
    };
    inline void setSize(unsigned int size)
    {
        vector<T>::resize(size);
    };
    inline void setSize(unsigned int size, const T &def)
    {
        vector<T>::resize(size, def);
    };
    inline void reset()
    {
        vector<T>::clear();
    }

    // clear(), clear the array

    // operator =
    template <typename ARR> 
    Array<T>& operator=(const ARR &arr)
    {
        for (unsigned int i = 0; i < arr.getNumElements(); i++)
        {
            this->append(arr[i]);
        }
        return *this;
    }

    // compar function gives "less" judgement, if less, return true
    void sort(bool (*compar)(const T&, const T&))
    {
        // create solid array
        std::sort(vector<T>::begin(), vector<T>::end(), compar);
    };
};
#endif

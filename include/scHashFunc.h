//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <hashfunc.h>
//
// Defines hash functions
//
// Author: Lu Yongqiang
// History: 2009/4/16 created by Yongqiang
// 
//*****************************************************************************
//*****************************************************************************

#if !defined(_SC_HASH_FUNCTION_H_)
#define _SC_HASH_FUNCTION_H_

#include "oa/oaDesignDB.h"
#include <iostream>
using namespace std;
// *****************************************************************************
// SCHashFunction
//
// This class defines the hash function, which is a unary functor mapping the
// templatized type to unsigned int
// *****************************************************************************
template<class K>
class SCHashFunction : public std::unary_function<const K, unsigned int> {
public:
    unsigned int  operator()(const K &key);
};



// *****************************************************************************
// SCIsEqualTo
//
// This class defines the SCIsEqualTo function, which is a binary functor mapping
// two values of the templatized type to bool (true indicates equal, false not
// equal).
// *****************************************************************************
template<class K>
class SCIsEqualTo : public std::binary_function<const K, const K, bool> {
public:
    bool  operator()(const K  &key1, const K  &key2);
};

// *****************************************************************************
// SCHashFunction<>::operator()
//
// These methods specialize the hash function for a number of commonly used
// types.  The hash functions provided are reasonably good, but not necessarily
// optimal and certainly not crypto-quality.
// *****************************************************************************
template<>
inline unsigned int
SCHashFunction<unsigned int>::operator()(const unsigned int &key)
{
    return key;
}

template<>
inline unsigned int
SCHashFunction<const char*>::operator()(const char*const &key)
{
    unsigned int ret = 0;
    const char  *str = key;
    for (oa::oaInt4 i = ((oa::oaInt4) strlen(key)) - 1; i >= 0; i--) {
        ret = (ret + str[i] * 733) * 1973;
    }
    return ret;
}

template<>
inline unsigned int
SCHashFunction<void*>::operator()(void*const &key)
{
    const unsigned char *bytes = (const unsigned char*) &key;
    unsigned int         ret = 0;
    for (oa::oaInt4 i = sizeof(void*) - 1; i >= 0; i--) {
        ret = (ret + bytes[i] * 733) * 1973;
    }
    return ret;
}

template<>
inline unsigned int
SCHashFunction<std::string>::operator()(const std::string &key)
{
    SCHashFunction<const char*> hash;

    return hash(key.c_str());
}



// *****************************************************************************
// SCIsEqualTo<>::operator()
//
// This method is the default implementation of SCIsEqualTo.  It should work for
// any class for which '==' is defined.
// *****************************************************************************
template<class K>
bool
SCIsEqualTo<K>::operator()(const K    &key1,
                         const K    &key2)
{
    return key1 == key2;
}



template<>
inline bool
SCIsEqualTo<const char*>::operator()(const char *const    &key1,
                                   const char *const    &key2)
{
    return strcmp(key1, key2) == 0;
}

#endif

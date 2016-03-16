//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <message.h>
//
// includes some basic mathematic type definitions or utilities
//
// Author: GENG Dongjiu
// History: 2009/9/25 created by GENG
//*****************************************************************************
//*****************************************************************************
#if !defined(_SC_UNIT_TEST_H_)
#define _SC_UNIT_TEST_H_

#include<iostream> 
#include "array.h"
#include "String.h"
#include "message.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "base.h"
#include "design.h"
using namespace std;

#if !defined(UT_MODULE)
#define UT_MODULE "ut"
#endif

// TEST_CHECK only reports check results and doesn't make exe terminated 
// when it fails
#define TEST_CHECK(s)  UnitTest::Check(s, #s, UT_MODULE, __FILE__, __LINE__, this)
// Please note, TEST_REQUIRE will terminate the exe to run if it fails
#define TEST_REQUIRE(s) UnitTest::Require(s, #s, UT_MODULE, __FILE__, __LINE__, this)


class UnitTest
{ 
public:
    UnitTest();
    inline ~UnitTest() { };

    // the unit test judgement elementary tool
    static void Check(bool ucase, const char *utItem, const char *moduleName,
            const char *fileName, const long lineNo, UnitTest *test);
    static void Require(bool ucase, const char *utItem, const char *moduleName,
            const char *fileName, const long lineNo, UnitTest *test);

    static inline void Throw(bool checkVal) 
    {        
        if (!checkVal) {
            throw checkVal;
        }
    }
    static void runAllTests();
    static inline Array<UnitTest*>& getAllTestsPool()
    {
        static Array<UnitTest*> *utAllTests = new Array<UnitTest*>;
        return *utAllTests;
    };

    virtual void start()=0;

private:
    static void openUnitBenchs();

public:
    string utFileName;
    string utTestName; 
    bool utSuccess;
};
#endif

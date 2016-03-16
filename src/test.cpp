//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <hash.cpp>
//
// implement the Hash tables defined in hash.h
//
// Author: Lu Yongqiang
// History: 2009/4/16 created by Yongqiang
//*****************************************************************************
//*****************************************************************************
#include "base.h"
#include "regionQuery.h"

// *****************************************************************************
// Tesing
// *****************************************************************************
int
DBTEST::runTest()
{
    int *array = new int[100];

/* commented on 11/08/09 16:09:36 
    HashPtr<int*, int> hash;
    for (int i = 0; i < 100; i++)
    {
        array[i] = i + 1;
    }
    for (int i = 0; i < 100; i++)
    {
        hash.add(&array[i], array[i]);
        std::cout<<"add key "<<&array[i]<<" val "<<array[i]<<endl;
    }
    int *key = NULL;
    int val = 0;

    for (int i = 0; i < 100; i++)
    {
        key = &array[i];
        val = 0;
        hash.find(key, val);
        std::cout<<"array i "<<i<<" val "<<val<<endl;
    }

    if (!hash.add(&array[49], 100)) {
        cout<<"repeat found, not inserted"<<endl;
    }
    hash.find(&array[49], val);
    cout<<" hash at 49 now is "<<val<<endl;
    HashPtrIter<int*, int> iter(hash);
    while (iter.getNext(key, val)) {
        cout<<"hash iter "<<key<<" "<<val<<endl;
    }
    TreeStr<string> strTree;
    string test1("test1");
    string test2("test2");
    strTree.add(test1, test1);
    strTree.add(test2, test2);
    TreeStrIter<string> siter(strTree);
    string c1, c2;
    while(siter.getNext(c1, c2)) {
        cout<<c1<<": "<<c2<<endl;
    }

    HashPtrSet<int*> ptrset;
    for (int i = 0; i < 100; i++)
    {
        ptrset.add(&array[i]);
    }
    if (!ptrset.add(&array[58]))
    {
        cout<<"repeat found in set"<<endl;
    }
 */

    List<int*> *ptrList = new List<int*>;
    ptrList->add(&array[0]);
/* commented on 11/08/09 16:30:04 
    for (int i = 0; i < 100; i++)
    {
        ptrList.add(&array[i]);
    }
 */
    //List<int> anList = ptrList;
/* commented on 26/08/09 18:23:32 
    Collection<int> col2(ptrList, true);
    Collection<int> col(col2);
    Iter<int> it2(col);
    int *pval;
    while (pval = it2.getNext())
    {
        cout<<"list iter "<<pval<<endl;
    }
 */
    Array<int> oat(8);
    cout<<" oat size "<<oat.getNumElements()<<endl;
    for (UInt4 i = 0; i < 8; i++) {
        oat[i] = i;
    }
    cout<<" oat size "<<oat.getNumElements()<<endl;
    oat.append(1);
    cout<<" oat size after append "<<oat.getNumElements()<<endl;
    for (UInt4 i = 0; i < oat.getNumElements(); i++) {
        cout<<"oat i "<<oat[i]<<endl;
    }
    oat.setSize(12);
    cout<<" size "<<oat.getNumElements()<<endl;

    Array<int> ta(oat);
    cout<<" size "<<ta.size()<<endl;
    Array<int> ta2(8, true);
    cout<<" size "<<ta2.getNumElements()<<endl;
    //ta.append(2);
    vector<int>::iterator ita = ta.begin();
    cout<<" first arr "<<*ita<<endl;
    for (UInt4 i = 0; i < ta.getNumElements(); i++)
    {
        cout<<" arr "<< ta[i]<<endl;
    }


    delete []array;

    QueryPlcBlockage qpb;
    Array<oaBlockage*> results;
    qpb.queryBlockages(oaBox(0, 0, 100, 100), results);


/* commented on 30/04/09 17:53:40 
    oaArray<DBExtAttrType> initHd;
    initHd.append(0);
    //DBExtension<oaNet*> test(NULL, "aha", initHd);
    DBExtension<oaAppObject*> test(NULL, "aha", initHd);
 */
    //test.save();
    return 1;
}

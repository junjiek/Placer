//*****************************************************************************
//*****************************************************************************
// This file contains copyrighted material developed by EDA Lab. of 
// Tsinghua University and may not be reproduced in any form by any 
// other persons or organizations without prior written permissions 
// of EDA Lab. of Tsinghua University.
//
// <extension.h>
//
// This file defines the extesion object to OA, which can enable the auto
// database archive/read mechanism of user-defined values
// 
// Two classes: DBExtension and DBExtObj; The former is for oaAppDef access
// and oaAppObjectDef definition; the latter only for oaAppObject based access
// 
// The template class oaManagedClassT must be OA managed class
//
// If users want to archive their own data objects, please first create 
// a object to the DBExtension and manipulate this object. If you would like to
// indicate your own data in oa database, please derive your class off DBExtObj
// after you have an access to DBExtension<oaAppObject>. As long as you have
// a class off DBExtObj and new it, there is one db object created in oa 
// database for this object. You can then add other correlations for your data
// objects and this oa database object. See the example in floorplan.h
// 
// NOTE: You cannot directly store your customized object to database 
// unless you wrap it using oa attributes like what we do here
//
// Of course there is other methods to store your own data, that is, 
// 1. to store your file data, refer to OA manual
// 2. to store real your own objects like oa, just write the datbase mechanism
//    as oa does.
//
// NOTE: If you would like to add other attributes support to current extension,
//       please modify two processAttributes() functions together,
//       one of which is specially for oaAppObj object
//
// Author: Lu Yongqiang
// History: 2009/4/28 created by Yongqiang
//*****************************************************************************
//*****************************************************************************

#if !defined(_DB_EXTENSION_H_)
#define _DB_EXTENSION_H_

#include "base.h"

// *****************************************************************************
// class template DBExtension
//
// A DB extension of the oa managed class oaManagedClassT
// Don't need to use * for pointer types here,
// e.g. DBExtension<oaNet> is for stroing oaNet *
// if you happen to use *, it is the same as without it too because we
// specialize pointer types here
// *****************************************************************************

// support utility for DBExtension classes
// type traits for different extension type operations
template <class oaManagedClassT, typename VT> 
struct DBExtValueTraits 
{ 
};

// specilizations for different types
// for int
template <class oaManagedClassT>
struct DBExtValueTraits<oaManagedClassT, int> {
    static inline int get(DBExtAttrType type, oaAppDef *def, oaManagedClassT *obj)
    {
        assert(type == DB::EXT_ATTR_INT);
        return ((oaIntAppDef<oaManagedClassT>*)def)->get(obj);
    }
    static inline void set(DBExtAttrType type, oaAppDef *def, 
            oaManagedClassT *obj, int val)
    {
        assert(type == DB::EXT_ATTR_INT);
        ((oaIntAppDef<oaManagedClassT>*) def)->set(obj, val);
    }
};


// for double
template <class oaManagedClassT>
struct DBExtValueTraits<oaManagedClassT, double> {
    static inline double get(DBExtAttrType type, oaAppDef *def, oaManagedClassT *obj)
    {
        assert(type == DB::EXT_ATTR_DOUBLE); 
        return ((oaDoubleAppDef<oaManagedClassT>*) def)->get(obj);
    };
    static inline void set(DBExtAttrType type, oaAppDef *def, 
            oaManagedClassT *obj, double val)
    {
        assert(type == DB::EXT_ATTR_DOUBLE); 
        ((oaDoubleAppDef<oaManagedClassT>*) def)->set(obj, val);
    };
};

// for pointer, partial specialization
template <class oaManagedClassT, typename VT>
struct DBExtValueTraits<oaManagedClassT, VT*> {
    static inline VT* get(DBExtAttrType type, oaAppDef *def, oaManagedClassT *obj)
    {
        assert(type == DB::EXT_ATTR_POINTER);
        return (VT*) ((oaInterPointerAppDef<oaManagedClassT>*) def)->get(obj);
    }
    static inline void set(DBExtAttrType type, oaAppDef *def, 
            oaManagedClassT *obj, VT *val)
    {
        assert(type == DB::EXT_ATTR_POINTER);
        ((oaInterPointerAppDef<oaManagedClassT>*) def)->set(obj, val);
    }
};

// for double
template <class oaManagedClassT>
struct DBExtValueTraits<oaManagedClassT, bool> {
    static inline bool get(DBExtAttrType type, oaAppDef *def, oaManagedClassT *obj)
    {
        assert(type == DB::EXT_ATTR_BOOL); 
        return ((oaBooleanAppDef<oaManagedClassT>*) def)->get(obj);
    };
    static inline void set(DBExtAttrType type, oaAppDef *def, 
            oaManagedClassT *obj, bool val)
    {
        assert(type == DB::EXT_ATTR_BOOL); 
        ((oaBooleanAppDef<oaManagedClassT>*) def)->set(obj, val);
    };
};

template <class oaManagedClassT>
struct DBExtValueTraits<oaManagedClassT, oaString> {
    static inline oaString get(DBExtAttrType type, oaAppDef *def, oaManagedClassT *obj)
    {
        assert(type == DB::EXT_ATTR_STRING); 
        oaString reVal;
        ((oaStringAppDef<oaManagedClassT>*) def)->get(obj, reVal);
        return reVal;
    };
    static inline void set(DBExtAttrType type, oaAppDef *def, 
            oaManagedClassT *obj, oaString &val)
    {
        assert(type == DB::EXT_ATTR_STRING); 
        ((oaStringAppDef<oaManagedClassT>*) def)->set(obj, val);
    };
};

// The base Extension class
template <class oaManagedClassT>
class DBExtBase: public DB
{
public:
    typedef union _DefaultValues_ {
        int iVal;
        double dVal;
        bool bVal;
        char *sVal;
        void *pVal;
    } DefaultValue;

public:
    // specify the db extension name and its attribute list (table header)
    // The defVals stores the default values in an array with each item
    // corresponding to that of attrList respectively.
    // when needDBObj is on, there is one oaAppObject will be created 
    // in the meanwhile, and now, please just use oaAppObject to initialize
    // this template (i.e. oaManagedClassT = oaAppObject)
    DBExtBase(oaDesign *design, const Array<DBExtAttrType> &attrList);
    virtual ~DBExtBase();

    inline oaDesign* getDesign()
    {
        return extParentDesign;
    };



    // here should specify pointer type if VT needs to be a pointer
    template <typename VT >
        bool getValue(UInt4 headerIndex, oaManagedClassT *obj, VT &val);
    template <typename VT >
        bool setValue(UInt4 headerIndex, oaManagedClassT *obj, VT val);

    inline void eraseAll(oaManagedClassT *obj)
    {
        // default value will be assigned again
        for (unsigned int i = 0; i < extAttrDefList.getNumElements(); i++) {
            extAttrDefList[i]->destroy(obj);
        }
    };
    inline void erase(unsigned int headerIndex, oaManagedClassT *obj)
    {
        // default value will be assigned again
        if (headerIndex < extAttrDefList.getNumElements()) {
            extAttrDefList[headerIndex]->destroy(obj);
        }
    };

protected:
    // the attribute table header types
    Array<DBExtAttrType> extAttrTypeList;
    // the attribute table object list
    Array<oaAppDef*> extAttrDefList;

    oaDesign *extParentDesign;

};

// template specializations
template <class oaManagedClassT>
class DBExtension: public DBExtBase<oaManagedClassT>
{
public:
    typedef typename DBExtBase<oaManagedClassT>::DefaultValue DefaultValue;

public:
    DBExtension(oaDesign *design, const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals = NULL)
        : DBExtBase<oaManagedClassT>(design, attrList) 
    {
        // only take attributes for managed objects, like oaNet, oaPin etc.
        if (!setupExtension(design, extName, attrList, defVals)) {
            return;
        }
    };

protected:
    bool processAttributes( const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals = NULL);
    inline bool setupExtension(oaDesign *design,
            const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals = NULL)
    {
        return processAttributes(extName, attrList, defVals);
    };
};

// template specialization for oaAppObject
template <>
class DBExtension<oaAppObject>: public DBExtBase<oaAppObject>
{
public:
    typedef DBExtBase<oaAppObject>::DefaultValue DefaultValue;

public:
    DBExtension(oaDesign *design, const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals = NULL)
        : DBExtBase<oaAppObject>(design, attrList),
          extAppObjDef(NULL)
    {
        // only take attributes for managed objects, like oaNet, oaPin etc.
        if (!setupExtension(design, extName, attrList, defVals)) {
            return;
        }
    };

    inline oaAppObjectDef* getAppObjectDef() 
    {
        return extAppObjDef;
    };


protected:
    // using inlines here to hack multi-definitions of gcc compiler in
    // class template specialization
    inline bool processAttributes( const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals = NULL);
    inline bool setupExtension(oaDesign *design,
            const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals = NULL);
protected:
    // the following are only for oaAppObj type template
    oaAppObjectDef *extAppObjDef;
};

class DBExtObj 
{
public:
    DBExtObj(DBExtension<oaAppObject> *dbExt, oaAppObject *dbObj = NULL)
        : extDBext(dbExt)
    {
        if (!dbObj) {
            isSaved = false;
            // create the object in db under this def
            extAppObj = oaAppObject::create(dbExt->getDesign(), 
                    dbExt->getAppObjectDef());
            if (!extAppObj) {
                cerr<<"Error: Unable to create database object"<<endl;
            }
        }
        else {
            isSaved = true;
            extAppObj = dbObj;
        }
    };

    inline virtual ~DBExtObj()
    {
        if (!isSaved && extAppObj) {
            // erase the db obj's attribute table entries
            extDBext->eraseAll(extAppObj);
            // delete this object
            extAppObj->destroy();
            // only leave the objDef in memory in case of any other usage
        }
    };
protected:
    DBExtension<oaAppObject> *extDBext;
    oaAppObject *extAppObj;
    bool isSaved;

};

template <class oaManagedClassT>
DBExtBase<oaManagedClassT>::DBExtBase(
                   oaDesign *design, 
                   const Array<DBExtAttrType> &attrList)
        : extAttrTypeList(attrList),
          extParentDesign(design)
{
};

template <class oaManagedClassT>
DBExtBase<oaManagedClassT>::~DBExtBase()
{
};


template <class oaManagedClassT>
template <typename VT>
bool
DBExtBase<oaManagedClassT>::getValue(UInt4 headerIndex, 
        oaManagedClassT *obj, VT &val)
{
        //assert(extAttrTypeList[headerIndex] == EXT_ATTR_INT);
    if (headerIndex >= extAttrDefList.getNumElements()) {
        return false;
    }
    val = DBExtValueTraits<oaManagedClassT, VT>::get(
            extAttrTypeList[headerIndex], extAttrDefList[headerIndex], obj);
    return true;
};

template <class oaManagedClassT>
template <typename VT >
bool
DBExtBase<oaManagedClassT>::setValue(UInt4 headerIndex, 
        oaManagedClassT *obj, VT val)
{
    if (headerIndex >= extAttrDefList.getNumElements()) {
        return false;
    }
    DBExtValueTraits<oaManagedClassT, VT>::set(
            extAttrTypeList[headerIndex], extAttrDefList[headerIndex], 
            obj, val);
    return true;
};


// *****************************************************************************
// specializaton for different oaManagedClassT types
// *****************************************************************************

// *****************************************************************************
// DBExtension<oaAppObject> specialization
// *****************************************************************************
template <class oaManagedClassT>
bool
DBExtension<oaManagedClassT>::processAttributes(
                   const char *extName, // should not collide with others
                   const Array<DBExtAttrType> &attrList,
                   const Array<DefaultValue> *defVals)
{
    if (defVals) {
        if (defVals->getNumElements() != attrList.getNumElements()) {
            cerr<<"Error, ambitious default value number for attributes"<<endl;
            return false;
        }
    }
    try {
        // get/create attributes of this db object def
        string attrName;
        char id[32];
        for (unsigned int i = 0; i < attrList.getNumElements(); i++) {
            sprintf(id, "%d", i);
            switch (attrList[i]) {
                case DB::EXT_ATTR_INT:
                {
                    attrName = string(extName) + "_ATTR_INT_" + id;
                    oaIntAppDef<oaManagedClassT> *intDef = 
                        oaIntAppDef<oaManagedClassT>::get( 
                                attrName.c_str(), 
                                (defVals? (*defVals)[i].iVal : 0));
                    DBExtBase<oaManagedClassT>::extAttrDefList.append(intDef);
                    break;
                }
                case DB::EXT_ATTR_DOUBLE:
                {
                    attrName = string(extName) + "_ATTR_DOUBLE_" + id;
                    oaDoubleAppDef<oaManagedClassT> *doubleDef = 
                        oaDoubleAppDef<oaManagedClassT>::get(
                                attrName.c_str(),
                                (defVals? (*defVals)[i].dVal : .0));
                    DBExtBase<oaManagedClassT>::extAttrDefList.append(doubleDef);
                    break;
                }
                case DB::EXT_ATTR_BOOL:
                {
                    attrName = string(extName) + "_ATTR_BOOL_" + id;
                    oaBooleanAppDef<oaManagedClassT> *boolDef = 
                        oaBooleanAppDef<oaManagedClassT>::get(
                                attrName.c_str(),
                                (defVals? (*defVals)[i].bVal : false));
                    DBExtBase<oaManagedClassT>::extAttrDefList.append(boolDef);
                    break;
                }
                case DB::EXT_ATTR_STRING:
                {
                    // oa string
                    attrName = string(extName) + "_ATTR_STRING_" + id;
                    oaStringAppDef<oaManagedClassT> *stringDef = 
                        oaStringAppDef<oaManagedClassT>::get(
                                attrName.c_str(),
                                (defVals? (*defVals)[i].sVal : ""));
                    DBExtBase<oaManagedClassT>::extAttrDefList.append(stringDef);
                }
                    break;
                case DB::EXT_ATTR_POINTER:
                {
                    attrName = string(extName) + "_ATTR_POINTER_" + id;
                    oaInterPointerAppDef<oaManagedClassT> *ptrDef = 
                        oaInterPointerAppDef<oaManagedClassT>::get(
                                attrName.c_str());
                    DBExtBase<oaManagedClassT>::extAttrDefList.append(ptrDef);
                    break;
                }
                default:
                    cout<<"ERROR: not defined attribute type"<<endl;
                    break;
            }
        }
    }
    catch (oaException &excp) {
        cerr<<"Error: "<<excp.getMsg()<<endl;;
        return false;
    }

    if (DBExtBase<oaManagedClassT>::extAttrDefList.getNumElements() != DBExtBase<oaManagedClassT>::extAttrTypeList.getNumElements()) {
        cerr<<"Error, ambitious attributes definition"<<endl;
        DBExtBase<oaManagedClassT>::extAttrDefList.setSize(0);
        DBExtBase<oaManagedClassT>::extAttrTypeList.setSize(0);
        return false;
    }
    return true;
};

bool
DBExtension<oaAppObject>::processAttributes(
                   const char *extName, // should not collide with others
                   const Array<DBExtAttrType> &attrList,
                   const Array<DefaultValue> *defVals)
{
    if (defVals) {
        if (defVals->getNumElements() != attrList.getNumElements()) {
            cerr<<"Error, ambitious default value number for attributes"<<endl;
            return false;
        }
    }
    if (!extAppObjDef) {
        cerr<<"Error, no db object def found for "<<extName<<endl;
        return false;
    }
    try {
        // get/create attributes of this db object def
        string attrName;
        char id[32];
        for (unsigned int i = 0; i < attrList.getNumElements(); i++) {
            sprintf(id, "%d", i);
            switch (attrList[i]) {
                case DB::EXT_ATTR_INT:
                {
                    attrName = string(extName) + "_ATTR_INT_" + id;
                    oaIntAppDef<oaAppObject> *intDef = 
                        oaIntAppDef<oaAppObject>::get( 
                                attrName.c_str(), extAppObjDef);
                    extAttrDefList.append(intDef);
                    break;
                }
                case DB::EXT_ATTR_DOUBLE:
                {
                    attrName = string(extName) + "_ATTR_DOUBLE_" + id;
                    oaDoubleAppDef<oaAppObject> *doubleDef = 
                        oaDoubleAppDef<oaAppObject>::get( 
                                attrName.c_str(), extAppObjDef);
                    extAttrDefList.append(doubleDef);
                    break;
                }
                case DB::EXT_ATTR_BOOL:
                {
                    attrName = string(extName) + "_ATTR_BOOL_" + id;
                    oaBooleanAppDef<oaAppObject> *boolDef = 
                        oaBooleanAppDef<oaAppObject>::get( 
                                attrName.c_str(), extAppObjDef);
                    extAttrDefList.append(boolDef);
                    break;
                }
                case DB::EXT_ATTR_STRING:
                {
                    // oa string
                    attrName = string(extName) + "_ATTR_STRING_" + id;
                    oaStringAppDef<oaAppObject> *stringDef = 
                        oaStringAppDef<oaAppObject>::get( 
                                attrName.c_str(), extAppObjDef);
                    extAttrDefList.append(stringDef);
                }
                    break;
                case DB::EXT_ATTR_POINTER:
                {
                    attrName = string(extName) + "_ATTR_POINTER_" + id;
                    oaInterPointerAppDef<oaAppObject> *ptrDef = 
                        oaInterPointerAppDef<oaAppObject>::get( 
                                attrName.c_str(), extAppObjDef);
                    extAttrDefList.append(ptrDef);
                    break;
                }
                default:
                    cout<<"ERROR: not defined attribute type"<<endl;
                    break;
            }
        }
    }
    catch (oaException &excp) {
        cerr<<"Error: "<<excp.getMsg()<<endl;;
        return false;
    }

    if (extAttrDefList.getNumElements() != extAttrTypeList.getNumElements()) {
        cerr<<"Error, ambitious attributes definition"<<endl;
        extAttrDefList.setSize(0);
        extAttrTypeList.setSize(0);
        return false;
    }
    return true;
}


bool
DBExtension<oaAppObject>::setupExtension(oaDesign *design,
            const char *extName, 
            const Array<DBExtAttrType> &attrList,
            const Array<DefaultValue> *defVals)
{
    if (!design) {
        cerr<<"Error: no design is open"<<endl;
        return false;
    }
    // get a objDef in db
    extAppObjDef = oaAppObjectDef::get(extName);
    if (!extAppObjDef) {
        cerr<<"Error: Unable to create database definition"<<endl;
        return false;
    }
    // set up attributes of oaObject
    if (!processAttributes(extName, attrList, defVals)) {
        return false;
    }
    return true;
}

// *****************************************************************************
// DBExtension<type *> specialization
//
// for pointer types, actually we by default only accept object pointers,
// but we don't need to specify the pointer * in tempalte. This is the 
// same as oa's templates, but different with hash tables we defined.
// *****************************************************************************
template <class oaManagedClassT>
class DBExtension<oaManagedClassT*>
{
private:
    inline DBExtension() 
    {
        cerr<<"Please use DBExtension<C> instead of DBExtension<C*>"<<endl;
    };
};
#endif

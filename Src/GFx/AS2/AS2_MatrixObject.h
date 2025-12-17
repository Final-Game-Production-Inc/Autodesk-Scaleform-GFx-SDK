/**************************************************************************

Filename    :   AS2_MatrixObject.h
Content     :   Matrix class implementation
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_MATRIX_H
#define INC_SF_GFX_MATRIX_H

#include "Kernel/SF_RefCount.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// Some magic number used for scaling coefficients in CreateGradientBox.
// This number was deduced empirically; in reality it may be more complex
// and depend on something else
const float GASGradientBoxMagicNumber = 1.0f/1638.4f;


// SF_NO_FXPLAYER_AS_MATRIX disables Matrix support
#ifdef GFX_AS2_ENABLE_MATRIX

// ***** Declared Classes
class MatrixObject;
class MatrixProto;

// ***** External Classes
class Value;

// ActionScript Matrix object

class MatrixObject : public Object
{
    friend class MatrixProto;
protected:
    MatrixObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    // a - [0]
    // b - [1]
    // c - [2]
    // d - [3]
    // tx - [4]
    // ty - [5]
    typedef Value GASMatrixArray[6];

    MatrixObject(Environment* penv);

    virtual ObjectType          GetObjectType() const   { return Object_Matrix; }

    // marr should be at least Value[6]
    GASMatrixArray* GetMatrixAsValuesArray(ASStringContext *psc, GASMatrixArray* marr);

    Render::Matrix2F GetMatrix(Environment *env);
    void SetMatrix(Environment *env, const Render::Matrix2F& m);

    void SetMatrixTwips(ASStringContext *psc, const Render::Matrix2F& m);
};

class MatrixProto : public Prototype<MatrixObject>
{
public:
    MatrixProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    static void ToString           (const FnCall& fn);
    static void Clone              (const FnCall& fn);
    static void Concat             (const FnCall& fn);
    static void CreateBox          (const FnCall& fn);
    static void CreateGradientBox  (const FnCall& fn);
    static void DeltaTransformPoint(const FnCall& fn);
    static void Identity           (const FnCall& fn);
    static void Invert             (const FnCall& fn);
    static void Rotate             (const FnCall& fn);
    static void Scale              (const FnCall& fn);
    static void TransformPoint     (const FnCall& fn);
    static void Translate          (const FnCall& fn);
};

class GASMatrixCtorFunction : public CFunctionObject
{
public:
    GASMatrixCtorFunction(ASStringContext *psc) 
        : CFunctionObject(psc, GlobalCtor) {}

    virtual Object* CreateNewObject(Environment* penv) const;    

    static void GlobalCtor(const FnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};

#endif  // SF_NO_FXPLAYER_AS_MATRIX

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_MATRIX_H


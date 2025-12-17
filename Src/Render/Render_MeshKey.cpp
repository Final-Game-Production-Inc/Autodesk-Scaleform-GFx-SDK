/**************************************************************************

Filename    :   Render_MeshKey.cpp
Created     :   2005-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_MeshKey.h"
//#include "Render_Math2D.h"
//#include "Render_BoundBox.h"
#include "Render_TessGen.h"
#include "Kernel/SF_HeapNew.h"
//#include "Renderer2D_Linear.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
// ***** MeshKey

unsigned MeshKey::GetKeySize(unsigned flags)
{
    unsigned size = 3;
    if (flags & KF_Scale9Grid)
    {
        size = Scale9GridData::MeshKeySize;
    }
    size++; // Morph ratio
 
    // TO DO: possibly revise, consider XY-offset in the MeshKey
    //if ((flags & KF_KeyTypeMask) == KF_StrokeHinted)
    //{
    //    size += 2;
    //}

    SF_ASSERT(size <= MaxKeySize);
    return size;
}


//------------------------------------------------------------------------
bool MeshKey::Match(unsigned layer, unsigned flags,
                    const float* keyData, const ToleranceParams& cfg) const
{
    if (flags != Flags || layer != pMesh->GetLayer() ) 
        return false;

    if (flags & KF_Degenerate)
        return true;

    unsigned i;

    if (flags & KF_Scale9Grid)
    {
        for(i = 0; i < 8; ++i)
        {
            if (keyData[i] != Data[i])
            {
                return false;
            }
        }

        for(i = 0; i < 3; ++i)
        {
            if (keyData[i+8] < Data[i+8] * cfg.Scale9LowerScale || 
                keyData[i+8] > Data[i+8] * cfg.Scale9UpperScale)
            {
                return false;
            }
        }

        for(i = 0; i < 2; ++i)
        {
            if (keyData[i+11] != Data[i+11])
            {
                return false;
            }
        }


    }
    else
    {
        unsigned s = 3;
        float lowerScale = cfg.FillLowerScale;
        float upperScale = cfg.FillUpperScale;
        if ((flags & KF_EdgeAA) == 0 || (flags & KF_Mask) != 0)
        {
            lowerScale = cfg.FillAliasedLowerScale;
            upperScale = cfg.FillAliasedUpperScale;
        }

        switch(flags & KF_KeyTypeMask)
        {
        case KF_Fill:
            for(i = 0; i < s; ++i)
            {
                if (keyData[i] < Data[i] * lowerScale || 
                    keyData[i] > Data[i] * upperScale)
                {
                    return false;
                }
            }
            break;

        case KF_Stroke:
            lowerScale = cfg.StrokeLowerScale;
            upperScale = cfg.StrokeUpperScale;
            if (flags & KF_NormalStroke)
            {
                // Optimization for uniform scaling. In case stroke has normal scale we can 
                // afford to reuse meshes with bigger tolerances, namely the same as fill. 
                // Uniform scaling means ScaleX1/ScaleX2 == ScaleY1/ScaleY2, or, better
                // ScaleX1 * ScaleY2 == ScaleX2 * ScaleY1. Besides, skewing must not change
                //--------------------------
                float sx = keyData[0] * *(Data+1);        // sx = ScaleX1 * ScaleY2
                float sy = keyData[1] * Data[0];        // sy = ScaleX2 * ScaleY1
                if (keyData[2] >= *(Data+2) * 0.999f &&
                    keyData[2] <= *(Data+2) * 1.001f &&
                    sx >= sy * 0.999f && sx <= sy * 1.001f)
                {
                    lowerScale = cfg.FillLowerScale;
                    upperScale = cfg.FillUpperScale;
                }
            }
            for(i = 0; i < s; ++i)
            {
                if (keyData[i] < Data[i] * lowerScale || 
                    keyData[i] > Data[i] * upperScale)
                {
                    return false;
                }
            }
            break;

        case KF_StrokeHinted:
            // TO DO: possibly revise, consider offset in the MeshKey
            for(i = 0; i < s; ++i)
            {
                if (keyData[i] < Data[i] * cfg.HintedStrokeLowerScale || 
                    keyData[i] > Data[i] * cfg.HintedStrokeUpperScale)
                {
                    return false;
                }
            }
            break;
        }
    }

    // Morph ratio
    if (keyData[Size-1] < Data[Size-1] * (1-cfg.MorphTolerance) ||
        keyData[Size-1] > Data[Size-1] * (1+cfg.MorphTolerance))
    {
        return false;
    }
    return true;
}

#ifdef SF_ENABLE_SIMD
// Note: This function assumes an aligned address for 'key'. In current usage, the key array is a local variable
// (which is aligned), and the matrix key is always the first entry.
bool MeshKey::CalcMatrixKey(const Matrix2F& m, float* key, Matrix2F* m2)
{
    using namespace Scaleform::SIMD;

	// Don't handle the case which modifies the matrix.
    if ( m2 )
        return CalcMatrixKey_NonOpt(m, key, m2);

    Vector4f vx       = IS::LoadAligned(m.M[0]); // Sx() == x2, Shx() == x3.
    Vector4f t0       = IS::Multiply( vx, vx );
    Vector4f vy       = IS::LoadAligned(m.M[1]); // Sy() == y2, Shy() == y3.
    Vector4f x2x2y2y2 = IS::Shuffle<0,0,0,0>(vx, vy);
    Vector4f x3x3y3y3 = IS::Shuffle<1,1,1,1>(vx, vy);
    Vector4f y2y2x2x2 = IS::Shuffle<0,0,0,0>(vy, vx);

    Vector4f dxdxdydy = IS::Subtract( x3x3y3y3, x2x2y2y2 );
    Vector4f pxpxpypy = IS::Multiply( x3x3y3y3, x2x2y2y2 );
    Vector4f d        = IS::MultiplyAdd( vy, vy, t0 );
    Vector4f pypypxpx = IS::Shuffle<2,2,0,0>(pxpxpypy, pxpxpypy);

    Vector4f dxy2_dxy2_dyx2_dyx2 = IS::Multiply( dxdxdydy, y2y2x2x2 );
    Vector4f rsqrt               = IS::ReciprocalSqrt( d );
    Vector4f dyx2_dyx2_dxy2_dxy2 = IS::Shuffle<2,2,0,0>(dxy2_dxy2_dyx2_dyx2, dxy2_dxy2_dyx2_dyx2);
    Vector4f t1                  = IS::Abs( IS::Subtract( dyx2_dyx2_dxy2_dxy2, dxy2_dxy2_dyx2_dyx2 ) );
    Vector4f t2                  = IS::Add( pxpxpypy, pypypxpx );

    Vector4f h_w_h_w  = IS::Multiply( IS::UnpackLo( t1, t2 ), IS::Shuffle<0,0,0,0>(rsqrt,rsqrt) );
    bool degenerate   = IS::CompareEQ_Any( IS::Shuffle<0,1,0,0>( d, h_w_h_w ), IS::Constant<0,0,0,0>() );
    Vector4f w_w_h_w  = IS::Shuffle<3,3,0,3>(h_w_h_w, h_w_h_w);

    Vector4f dhw_w_h_w       = IS::Subtract( h_w_h_w, IS::And( w_w_h_w, IS::Constant<0xFFFFFFFF, 0, 0, 0>() ) );
    Vector4f dhw_h_d3_d2     = IS::Shuffle<0, 2, 0, 1>(dhw_w_h_w, rsqrt);
    Vector4f rsp_dhw_h_d3_d2 = IS::Reciprocal( dhw_h_d3_d2 );

    static const Vector4f c0100 = { 0, 1.0f, 0, 0 };
    Vector4f key_2a_2b_1_0 = IS::MultiplyAdd( h_w_h_w, rsp_dhw_h_d3_d2, c0100 );
    Vector4f key_2b_2a_1_0 = IS::Shuffle<1,0,2,3>(key_2a_2b_1_0, key_2a_2b_1_0);
    Vector4f key_0         = IS::Select( key_2a_2b_1_0, key_2b_2a_1_0, IS::CompareGE(w_w_h_w, IS::Constant<0,0,0,0>()));
    Vector4f keydata       = IS::Shuffle<2,3,0,0>(rsp_dhw_h_d3_d2, key_0 );

    // Write it. NOTE: This actually stores 4 floats, when the original only stores 3. There should
    // be enough space in the key data to store this extra float, and it should be either unused, or
    // overwritten by subsequent key operations.
    IS::StoreAligned(key, keydata);
    return !degenerate;
}
#else
bool MeshKey::CalcMatrixKey(const Matrix2F& m, float* key, Matrix2F* m2)
{
    return CalcMatrixKey_NonOpt(m, key, m2);
}
#endif

//------------------------------------------------------------------------
bool MeshKey::CalcMatrixKey_NonOpt(const Matrix2F& m, float* key, Matrix2F* m2)
{
    // DBG: GFx3.0 scaling key
    //key[0] = m.GetMaxScale();
    //key[1] = 1;
    //key[2] = 1;
    //if (m2) *m2 = GMatrix2D::Scaling(key[0]);
    //return;

    // Advanced, 3-component scaling key. It takes into account
    // ScaleX, ScaleY and Skewing
    //----------------------
    float x2 = m.Sx();
    float y2 = m.Shy();
    float x3 = m.Shx();
    float y3 = m.Sy();

    // The legend:
    //----------------------------------------------------
    //               x3,y3
    //                 +-----------------+
    //                /|                /|
    //               / |               / |
    //              /  |              /  |
    //             /   |             /   |
    //          sy/    |h           /    |
    //           /     |           /     |
    //          /      |          /      |
    //   (0,0) +---w---+---------+ x2,y2 |
    //         |--------sx-------|       |
    //         |-----------sx+w----------|
    //----------------------------------------------------
    float d2 = x2*x2 + y2*y2;
    float d3 = x3*x3 + y3*y3;
    if (d2 == 0 || d3 == 0) 
        return false;                       // Degenerate matrix (zero scale)
    float sx = sqrtf(d2);
    float h  = fabsf((y3-y2)*x2 - (x3-x2)*y2) / sx;
    if (h < 1e-8f)
        return false;                       // Degenerate matrix (infinite skew)
    float w  = sx * (x3*x2 + y3*y2) / d2;

    key[0] = sx;
    key[1] = sqrtf(d3);                     // sy
    key[2] = (w >= 0) ? 1+w/h : h/(h-w);    // sh (skew)

    if (m2)
    {
        // Normalize the matrix to remove the rotation. We just take
        // the parallelogram based on the OX axis and construct
        // the matrix to transform rectangle (0,0)(1,1) to (0,0)(sx,0)(sx+w,h).
        //---------------------
        float p[8] = { 0, 0, sx, 0, sx+w, h }; // p[8] is used later
        m2->SetRectToParl(0, 0, 1, 1, p);

        // Optimization. When we have a skewed matrix it's better to select
        // the longest base and put it vertically, so that, the tessellator
        // could potentially produce better triangles. The following code
        // can be commented out or removed:
        //-------------------- from here
        p[0] =  1;              // 0 degree
        p[1] =  0;
        p[2] =  0.707106781f;   // 45 degree
        p[3] =  0.707106781f;
        p[4] =  0;       
        p[5] =  1.05f;          // 90 degree, make it have higher priority
        p[6] = -0.707106781f;
        p[7] =  0.707106781f;   // 135 degree
        float    maxLen = 0;
        unsigned maxIdx = 4;
        for (unsigned i = 0; i < 8; i += 2)
        {
            m2->Transform2x2(&p[i+0], &p[i+1]);
            d2 = p[i+0]*p[i+0] + p[i+1]*p[i+1];
            if (d2 > maxLen)
            {
                maxLen = d2;
                maxIdx = i;
            }
        }
        m2->AppendRotation(3.141592653589793f/2 - atan2f(p[maxIdx+1], p[maxIdx+0]));
        //------------------- to here
    }
    return true;
}

void MeshKey::Release()
{
    if (--UseCount == 0)
    {
        // Remove us if Mesh is also evicted.
        // In case of Scale9Grid or gradient morphing we force removing the Mesh and gradient images
        // in order to reduce mesh thrashing for the price of regenerating both, meshes, and gradients
        if (!pMesh || pMesh->IsEvicted() || (Flags & (KF_Scale9Grid | KF_GradientMorph)))
            pKeySet->DestroyKey(this);
    }
}


//------------------------------------------------------------------------
void MeshKeySetHandle::releaseCache()
{
    MeshKeyManager* manager = pManager.Exchange_NoSync(0);
    if (manager)
    {
        manager->providerLost(*this);
        manager->Release();
    }
}
void MeshKeySetHandle::releaseCache_NoNotify()
{
    MeshKeyManager* manager = pManager.Exchange_NoSync(0);
    if (manager)
        manager->Release();
}


//------------------------------------------------------------------------
// ***** MeshKeySet

MeshKey* MeshKeySet::CreateKey(const float* keyData, unsigned flags)
{
    unsigned size = MeshKey::GetKeySize(flags);
    void*    buf = SF_HEAP_ALLOC(pManager->pRenderHeap,
                                 sizeof(MeshKey) + sizeof(float) * (size - 1),
                                 StatRender_Mem);
    if (!buf) return 0;

    MeshKey* key = Construct<MeshKey>(buf);
    key->pKeySet = this;
    key->Size  = (UInt16)size;
    key->Flags = (UInt16)flags;

    SF_ASSERT((flags&0xFFFF) == flags);
    memcpy(key->Data, keyData, sizeof(float) * size);

    Meshes.PushBack(key);
    return key;
}

void MeshKeySet::DestroyKey(MeshKey* key)
{
    SF_ASSERT(key->pKeySet == this);

    // Key should no longer be in use when this is called.
    // If it is, it means MeshKey references in TreeCache weren't flushed,
    // which they should be before MeshKeyManager cleanup.
    SF_ASSERT(key->NotInUse());

    key->RemoveNode();
    if (key->pMesh)
    {
        key->pMesh->ClearProvider();
        key->pMesh.Clear();
    }    
    SF_FREE(key);

    // Free this keySet if it became empty.
    if (Meshes.IsEmpty())
        pManager->destroyKeySet(this);
}

void MeshKeySet::DestroyAllKeys()
{    
    MeshKey *key = Meshes.GetFirst();
    MeshKey *temp;

    // Clear out mesh handlers first, so that they don't call us recursively,
    // trying to remove following items.
    while(!Meshes.IsNull(key))
    {
        // Key should no longer be in use (see comment in DestroyKey).
        SF_ASSERT(key->NotInUse());
        if (key->pMesh)
            key->pMesh->ClearProvider();
        key = key->pNext;
    }

    key = Meshes.GetFirst();

    while(!Meshes.IsNull(key))
    {
        temp = key->pNext;
        if (key->pMesh)
            key->pMesh.Clear();
        SF_FREE(key);
        key = temp;
    }
    Meshes.Clear();
}


void MeshKeySet::OnEvict(MeshBase *pmesh)
{
    MeshKey* key = Meshes.GetFirst();
    while (!Meshes.IsNull(key))
    {
        if (pmesh == key->pMesh.GetPtr())
        {
            if (key->NotInUse())
            {
                DestroyKey(key);
            }
            break;
        }
        key = key->pNext;
    }
}

MeshKey* MeshKeySet::findMatchingKey(unsigned layer, unsigned flags,
                                     const float* keyData, const ToleranceParams& cfg)
{
//int i=0;
    MeshKey* key = Meshes.GetFirst();
    while (!Meshes.IsNull(key))
    {
//++i;
        // Prefetch the our mesh and the next key.
        SIMD::IS::PrefetchObj(key->pMesh.GetPtr());
        SIMD::IS::PrefetchObj(key->pNext);

        if (key->Match(layer, flags, keyData, cfg))
        {
//printf("++ %u ", i);
            return key;
        }
        key = key->pNext;
    }
//printf("-- %u ", i);
    return 0;
}

void MeshKeySet::releaseDelegate_RenderThread()
{
    // Must be called on RenderThread only during Lock!
    if (pDelegate)
    {
        MeshKeySetHandle& handle = pDelegate->hKeySet;

        // pDelegate MeshProvider is guaranteed alive until releaseCache_NoNotify
        // executes, since it stores atomically exchanged pointer. Even if ~MeshProvider
        // executes then, it will be blocked on our Lock.
        SF_ASSERT(handle.pKeySet == this);
        handle.pKeySet = 0;
        handle.releaseCache_NoNotify();
        pDelegate = 0;
    }
}


//------------------------------------------------------------------------
// ***** MeshKeyManager

void MeshKeyManager::DestroyAllKeys()
{
    Lock::Locker scopeLock(getLock());
    destroyKeySetList_NTS(KeySet_KillList);
    destroyKeySetList_NTS(KeySet_LiveList);
}

void MeshKeyManager::ProcessKillList()
{
    Lock::Locker scopeLock(getLock());
    destroyKeySetList_NTS(KeySet_KillList);
}


MeshKey* MeshKeyManager::CreateMatchingKey(MeshProvider_KeySupport* provider,
                                           unsigned layer, unsigned flags,
                                           const float* keyData,
                                           const ToleranceParams& cfg)
{
    MeshKeySetHandle& handle = provider->hKeySet;
    SF_ASSERT(!handle.pManager || (handle.pManager == this));

    // When we are called here, we are assumed to own MeshKeySetHandle container
    // reference, such that it can't possibly be destroyed in the other thread,
    // This means that our access is safe.
    if (!handle.pManager)
    {
        AddRef();
        handle.pManager = this;
    }
    if (!handle.pKeySet)
    {
        Lock::Locker scope(getLock());

        handle.pKeySet = SF_HEAP_NEW(pRenderHeap) MeshKeySet(this, provider);
        if (!handle.pKeySet)
            return 0;
        KeySets[KeySet_LiveList].PushBack(handle.pKeySet);
    }

    return CreateMatchingKey(handle.pKeySet, layer, flags, keyData, cfg);
}

MeshKey* MeshKeyManager::CreateMatchingKey(MeshKeySet *keySet,
                                           unsigned layer, unsigned flags,
                                           const float* keyData,
                                           const ToleranceParams& cfg)
{
    SF_ASSERT(keySet != 0);

    MeshKey* key = keySet->findMatchingKey(layer, flags, keyData, cfg);
    if (key)
    {
        key->AddRef();
        return key;
    }
    return keySet->CreateKey(keyData, flags);
}


void MeshKeyManager::providerLost(MeshKeySetHandle& handle)
{
    Lock::Locker scopeLock(getLock());    

    // Since handle.pKeySet is always modified in lock, we can
    // check it directly to see if MeshKeySet was destroyed on us.
    // This is possible if RenderThread swapped out all related meshes
    // or called DestroyAllKeys().
    // Key set was already destroyed, nothing to do.
    MeshKeySet* keySet = handle.pKeySet;
    if (!keySet)
        return;

    // If key set wasn't destroyed yet, place it into KillList.
    // That list is freed on render thread.
    SF_ASSERT(keySet->pDelegate != 0);

    // Move to kill/evict list. 
    keySet->RemoveNode();
    KeySets[KeySet_KillList].PushBack(keySet);
    keySet->pDelegate = 0;
    handle.pKeySet = 0;
}


void MeshKeyManager::destroyKeySet(MeshKeySet* keySet)
{
    Lock::Locker scopeLock(getLock());

    keySet->releaseDelegate_RenderThread();
    keySet->RemoveNode();
    delete keySet;

    // Doesn't hurt to check KillList occasionally.
    if (!KeySets[KeySet_KillList].IsEmpty())
        destroyKeySetList_NTS(KeySet_KillList);
}

void MeshKeyManager::destroyKeySetList_NTS(KeySetListType type)
{    
    if (KeySets[type].IsEmpty())
        return;

    // Steal items from source list so that we short-circuit if (KillList.IsEmpty())
    // call in case we are called recursively. Recursive calls come in due to mesh
    // handlers in 'delete keySet'; to address this we remove one item at a time.
    List<MeshKeySet> list;
    list.PushListToFront(KeySets[type]);

    do {
        MeshKeySet* keySet = list.GetFirst();

        if (type != KeySet_KillList)
        {   // Update mesh providers if destroying main lust.
            keySet->releaseDelegate_RenderThread();
        }
        else
        {   // For kill list, pDelegate provider should be clear.
            SF_ASSERT(keySet->pDelegate == 0);
        }

        keySet->RemoveNode();
        delete keySet;
    } while(!list.IsEmpty());
}


}} // Scaleform::Render

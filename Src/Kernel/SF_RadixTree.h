/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_RadixTree.h
Content     :   Template implementation for a simple radix tree.
Created     :   2009
Authors     :   MaximShemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_RadixTree_H
#define INC_SF_Kernel_RadixTree_H

#include "SF_Types.h"

namespace Scaleform {

// Intrusive radix tree adopted from Doug Lea malloc (public domain license). 
// The tree works only with an integer key of UPInt type. The tree 
// typically grows quickly to the number of bits in UPInt (32 or 64), 
// but the tree height will never exceed this number. That is, it 
// guarantees operation time of at most 32 or 64 operations respectively. 
// In some sense it can be said that the access time is O(1), although 
// the constant coefficient is rather hight.
//
// The tree requires the following accessor (as an example):
//
//  struct TreeNodeAccessor
//  {
//      static       UPInt      GetKey     (const TreeNode* n)          { return  n->Key;      }
//      static       TreeNode*  GetChild   (      TreeNode* n, UPInt i) { return  n->Child[i]; }
//      static const TreeNode*  GetChild   (const TreeNode* n, UPInt i) { return  n->Child[i]; }
//      static       TreeNode** GetChildPtr(      TreeNode* n, UPInt i) { return &n->Child[i]; }
//
//      static       TreeNode*  GetParent  (      TreeNode* n)          { return n->Parent;    }
//      static const TreeNode*  GetParent  (const TreeNode* n)          { return n->Parent;    }
//
//      static void SetParent (TreeNode* n, TreeNode* p)                { n->Parent   = p; }
//      static void SetChild  (TreeNode* n, UPInt i, TreeNode* c)       { n->Child[i] = c; }
//      static void ClearLinks(TreeNode* n) { n->Parent = n->Child[0] = n->Child[1] = 0; }
//};
//
// Operations on the tree:
//
// T*       Insert(T* node); // Returns NULL on success, or head node
// void     Remove(T* node);
// const T* FindEqual(UPInt key) const;
// const T* FindGrEq (UPInt key) const;
// const T* FindLeEq (UPInt key) const;
//
//------------------------------------------------------------------------
template<class T, class Accessor> class RadixTree
{
    enum { KeyBits  = 8*sizeof(UPInt) };


public:
    RadixTree() : Root(0) {}

    void Clear() { Root = 0; }

    // Insert node. Returns NULL on success, or the pointer to the 
    // node if the node with the same key already exists in the tree.
    //--------------------------------------------------------------------
    T* Insert(T* node)
    {
        T** root = &Root;

        Accessor::ClearLinks(node);

        if (Root == 0) 
        {
            *root = node;
            Accessor::SetParent(node, (T*)root);
        }
        else 
        {
            UPInt key  = Accessor::GetKey(node);
            UPInt bits = key;
            T*    head = *root;
            for (;;) 
            {
                if (Accessor::GetKey(head) == key)
                {
                    return head;
                }
                T** link = Accessor::GetChildPtr(head, (bits >> (KeyBits-1)) & 1);
                bits <<= 1;

                if (*link != 0)
                {
                    head = *link;
                }
                else
                {
                    *link = node;
                    Accessor::SetParent(node, head);
                    break;
                }
            }
        }
        return 0;
    }


    //--------------------------------------------------------------------
    void Remove(T* node, T* rotor)
    {
        T* parent = Accessor::GetParent(node);
        if (parent != 0) 
        {
            T** root = &Root;

            if (node == *root) 
            {
                *root = rotor;
            }    
            else
            {
                UPInt ic = Accessor::GetChild(parent, 0) != node;
                Accessor::SetChild(parent, ic, rotor);
            }

            if (rotor != 0)
            {
                T* child;

                Accessor::SetParent(rotor, parent);

                if ((child = Accessor::GetChild(node, 0)) != 0)
                {
                    Accessor::SetChild(rotor, 0, child);
                    Accessor::SetParent(child, rotor);
                }

                if ((child = Accessor::GetChild(node, 1)) != 0)
                {
                    Accessor::SetChild(rotor, 1, child);
                    Accessor::SetParent(child, rotor);
                }
            }
        }
        Accessor::ClearLinks(node);
    }


    //--------------------------------------------------------------------
    void Remove(T* node)
    {
        T* rotor;
        T** rp;
        if (((rotor = *(rp = Accessor::GetChildPtr(node, 1))) != 0) || 
            ((rotor = *(rp = Accessor::GetChildPtr(node, 0))) != 0)) 
        {
            T** cp;
            while ((*(cp = Accessor::GetChildPtr(rotor, 1)) != 0) || 
                   (*(cp = Accessor::GetChildPtr(rotor, 0)) != 0)) 
            {
                rotor = *(rp = cp);
            }
            *rp = 0;
        }
        Remove(node, rotor);
    }


    //--------------------------------------------------------------------
    const T* FindEqual(UPInt key) const
    {
        const T* node = Root;
        UPInt bits = key;
        while (node && Accessor::GetKey(node) != key)
        {
            node = Accessor::GetChild(node, (bits >> (KeyBits-1)) & 1);
            bits <<= 1;
        }
        return node;
    }

    //--------------------------------------------------------------------
    static const T* GetLeftmost(const T* node)
    {
        return Accessor::GetChild(node, Accessor::GetChild(node, 0) == 0);
    }

    //--------------------------------------------------------------------
    static const T* GetRightmost(const T* node)
    {
        return Accessor::GetChild(node, Accessor::GetChild(node, 1) != 0);
    }


    //--------------------------------------------------------------------
    const T* FindGrEq(UPInt key) const
    {
        const T* best = 0;

        UPInt rkey = ~UPInt(0);
        UPInt nkey;
        UPInt diff;

        const T* node = Root;
        if (node != 0)
        {
            // Traverse tree looking for node with node->Key == key
            //--------------------------
            const T* rst = 0;  // The deepest untaken right subtree
            UPInt bits = key;
            for (;;)
            {
                const T* rt;

                nkey = Accessor::GetKey(node);
                diff = nkey - key;
                if (nkey >= key && diff < rkey)
                {
                    best = node;
                    if ((rkey = diff) == 0)
                    {
                        return best;
                    }
                }
                rt   = Accessor::GetChild(node, 1);
                node = Accessor::GetChild(node, (bits >> (KeyBits-1)) & 1);
                if (rt != 0 && rt != node)
                {
                    rst = rt;
                }
                if (node == 0)
                {
                    node = rst; // set node to least subtree holding Keys > key
                    break;
                }
                bits <<= 1;
            }
        }

        while (node)
        { 
            // Find smallest of subtree.
            //------------------------
            nkey = Accessor::GetKey(node);
            diff = nkey - key;
            if (nkey >= key && diff < rkey)
            {
                rkey = diff;
                best  = node;
            }
            node = GetLeftmost(node);
        }

        return best;
    }


    //--------------------------------------------------------------------
    const T* FindLeEq(UPInt key) const
    {
        const T* best = 0;

        UPInt rkey = ~UPInt(0);
        UPInt nkey;
        UPInt diff;

        const T* node = Root;
        if (node != 0)
        {
            // Traverse tree looking for node with node->Key == key
            //--------------------------
            const T* lst = 0;  // The deepest untaken left subtree
            UPInt bits = key;
            for (;;)
            {
                const T* lt;
                nkey = Accessor::GetKey(node);
                diff = key - nkey;
                if (nkey <= key && diff < rkey)
                {
                    best = node;
                    if ((rkey = diff) == 0)
                    {
                        return best;
                    }
                }
                lt   = Accessor::GetChild(node, 0);
                node = Accessor::GetChild(node, (bits >> (KeyBits-1)) & 1);
                if (lt != 0 && lt != node)
                {
                    lst = lt;
                }
                if (node == 0)
                {
                    node = lst; // set node to most subtree holding Keys < key
                    break;
                }
                bits <<= 1;
            }
        }

        while (node)
        { 
            // Find biggest of subtree.
            //------------------------
            nkey = Accessor::GetKey(node);
            diff = key - nkey;
            if (nkey <= key && diff < rkey)
            {
                rkey = diff;
                best = node;
            }
            node = GetRightmost(node);
        }
        return best;
    }

    T*  Root;
};


//------------------------------------------------------------------------
template<class T, class Accessor> class RadixTreeMulti
{
public:
    //--------------------------------------------------------------------
    void Insert(T* node)
    {
        node->pPrev = node;
        node->pNext = node;
        T* head = Tree.Insert(node);
        if (head)
        {
            // Insert node next to the head one:
            // Before: head <----------------> next1 <----> next2...
            // After:  head <----> node <----> next1 <----> next2...
            //------------------------
            node->pNext = head->pNext;
            node->pPrev = head;
            head->pNext = node;
            node->pNext->pPrev = node;
        }
    }

    //--------------------------------------------------------------------
    void Remove(T* node)
    {
        if (node->pPrev != node) 
        {
            // The list isn't empty.
            //------------------------
            T* rotor;
            T* f  = node->pNext;
            rotor = node->pPrev;

            f->pPrev = rotor;
            rotor->pNext = f;
            Tree.Remove(node, rotor);
        }
        else
        {
            Tree.Remove(node);
        }
    }

    //--------------------------------------------------------------------
    const T* FindBestGrEq(UPInt key)
    {
        return Tree.FindGrEq(key);
    }

    //--------------------------------------------------------------------
    const T* FindBestLeEq(UPInt key)
    {
        return Tree.FindLeEq(key);
    }

    //--------------------------------------------------------------------
    T* PullBestGrEq(UPInt key)
    {
        T* node = (T*)Tree.FindGrEq(key);
        if (node)
        {
            // Take the next node to reduce tree thrashing in Remove().
            //------------------------
            node = (T*)node->pNext;
            Remove(node);
        }
        return node;
    }

    //--------------------------------------------------------------------
    T* PullBestLeEq(UPInt key)
    {
        T* node = (T*)Tree.FindLeEq(key);
        if (node)
        {
            // Take the next node to reduce tree thrashing in Remove().
            //------------------------
            node = (T*)node->pNext;
            Remove(node);
        }
        return node;
    }

    RadixTree<T, Accessor> Tree;
};

} // Scaleform 

#endif

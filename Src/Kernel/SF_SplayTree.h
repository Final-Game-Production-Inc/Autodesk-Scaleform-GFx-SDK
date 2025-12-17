/**************************************************************************

Filename    :   SF_SplayTree.h
Content     :   Template implementation for a simple splay tree.
Created     :   2009
Authors     :   MaximShemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_SplayTree_H
#define INC_SF_Kernel_SplayTree_H

#include "SF_Types.h"

namespace Scaleform {

// Intrusive splay tree adopted from heap manager written by Pradeep Chulliyan 
// chulliyan@hotmail.com (public domain license).
//
// Good performance for a splay tree depends on the fact that it is 
// self-balancing, and indeed self optimizing, in that frequently 
// accessed nodes will move nearer to the root where they can be 
// accessed more quickly. This is an advantage for nearly all practical 
// applications, and is particularly useful for implementing caches and 
// garbage collection algorithms; however it is important to note that 
// for uniform access, a splay tree's performance will be considerably 
// (although not asymptotically) worse than a somewhat balanced simple 
// binary search tree.
//
// The tree requires the following accessor (as an example):
//
//  struct TreeNodeAccessor
//  {
//      static bool  Less(const TreeNode& a, const TreeNode& b)  { return a.Key < b.Key; }
//      static bool  Equal(const TreeNode& a, const TreeNode& b) { return a.Key == b.Key; }
//
//      static       TreeNode*  GetChild   (      TreeNode* n, UPInt i) { return  n->Child[i]; }
//      static const TreeNode*  GetChild   (const TreeNode* n, UPInt i) { return  n->Child[i]; }
//      static       TreeNode** GetChildPtr(      TreeNode* n, UPInt i) { return &n->Child[i]; }
//
//      static void SetChild  (TreeNode* n, UPInt i, TreeNode* c)       { n->Child[i] = c; }
//
//      static void ClearLinks(TreeNode* n) { n->Child[0] = n->Child[1] = 0; }
//  };
//
// Operations on the tree:
//
// static T* Splay(T** root, const T& key);
// void      Insert(T* n, T* t);
// T*        Insert(T* n); 
// void      Remove(T* t, bool splayed);
// const  T* FindEqual(const T& key) const;
// const  T* FindGrEq (const T& key) const;
// const  T* FindLeEq (const T& key) const;
//
//------------------------------------------------------------------------
template<class T, class Accessor> class SplayTree
{

public:
    SplayTree() : Root(0) {}

    void Clear() { Root = 0; }

    // Adjust the tree (splay the heap) to get the best matching node
    // at the top of the heap.
    //--------------------------------------------------------------------
    static T* Splay(T** root, const T& key)
    {
        T* t = *root;
        if (t == 0)
            return 0;

        T n;
        Accessor::ClearLinks(&n);
        T* l = &n;
        T* r = l;
        T* y = 0;

        for (;;)
        {
            if (Accessor::Less(key, *t))
            {
                if (Accessor::GetChild(t, 0) == 0)
                    break;

                // Rotate right
                //------------------
                if (Accessor::Less(key, *Accessor::GetChild(t, 0)))
                {
                    y = Accessor::GetChild(t, 0);
                        Accessor::SetChild(t, 0, Accessor::GetChild(y, 1));
                        Accessor::SetChild(y, 1, t);
                    t = y;

                    if (Accessor::GetChild(t, 0) == 0)
                        break;
                }

                // Link right
                //------------------
                    Accessor::SetChild(r, 0, t);
                r = t;
                t = Accessor::GetChild(t, 0);
            }
            else if (Accessor::Less(*t, key))
            {
                if (Accessor::GetChild(t, 1) == 0)
                    break;

                // Rotate left
                //------------------
                if (Accessor::Less(*Accessor::GetChild(t, 1), key))
                {
                    y = Accessor::GetChild(t, 1);
                        Accessor::SetChild(t, 1, Accessor::GetChild(y, 0));
                        Accessor::SetChild(y, 0, t);
                    t = y;

                    if (Accessor::GetChild(t, 1) == 0)
                        break;
                }

                // Link left
                //------------------
                    Accessor::SetChild(l, 1, t);
                l = t;
                t = Accessor::GetChild(t, 1);
            }
            else
                break;
        }

        // Re-create the node
        //------------------
        Accessor::SetChild(l, 1, 0);
        Accessor::SetChild(r, 0, 0);
        Accessor::SetChild(l, 1, Accessor::GetChild(t, 0));
        Accessor::SetChild(r, 0, Accessor::GetChild(t, 1));
        Accessor::SetChild(t, 0, Accessor::GetChild(&n, 1));
        Accessor::SetChild(t, 1, Accessor::GetChild(&n, 0));

        return *root = t;
    }


    // Link node 'n' to node 't' and move 'n' to the root.
    //--------------------------------------------------------------------
    void Insert(T* n, T* t)
    {
        if (t == 0)
        {
            Accessor::ClearLinks(n);
        }
        else if (Accessor::Less(*n, *t))
        {
            Accessor::SetChild(n, 0, Accessor::GetChild(t, 0));
            Accessor::SetChild(n, 1, t);
            Accessor::SetChild(t, 0, 0);
        }
        else
        {
            Accessor::SetChild(n, 1, Accessor::GetChild(t, 1));
            Accessor::SetChild(n, 0, t);
            Accessor::SetChild(t, 1, 0);
        }
        Root = n;
    }


    // Insert node into the tree starting from the Root node.
    // Returns NULL on success, or the pointer to the node if the node 
    // with the same key already exists in the tree.
    //--------------------------------------------------------------------
    T* Insert(T* n)
    {
        //Insert(n, Splay(&Root, *n));
        T* t = Splay(&Root, *n);
        if (t && Accessor::Equal(*t, *n))
        {
            return t;
        }
        Insert(n, t);
        return 0;
    }


    // Remove a node from the tree. If 'splayed' is true, then the tree
    // is assumed to be splayed with the correct key.
    //--------------------------------------------------------------------
    void Remove(T* t, bool splayed = false)
    {
        T* x = Root;

        if (splayed)
            x = t;
        else
            x = Splay(&x, *t);

        if (Accessor::Equal(*x, *t))
        {
            if (Accessor::GetChild(t, 0) == 0)
            {
                x = Accessor::GetChild(t, 1);
            }
            else
            {
                x = Splay(Accessor::GetChildPtr(t, 0), *t);
                Accessor::SetChild(x, 1, Accessor::GetChild(t, 1));
            }
        }
        Root = x;
        Accessor::ClearLinks(t);
    }


    //--------------------------------------------------------------------
    const T* FindEqual(const T& key) const
    {
        const T* node = Root;
        while(node && !Accessor::Equal(*node, key))
        {
            node = Accessor::GetChild(node, Accessor::Less(*node, key));
        }
        return node;
    }


    //--------------------------------------------------------------------
    const T* FindGrEq(const T& key) const
    {
        const T* node = Root;
        const T* best = 0;
        while (node)
        {
            if (Accessor::Less(*node, key))
            {
                node = Accessor::GetChild(node, 1);
            }
            else
            {
                best = node;
                node = Accessor::GetChild(node, 0);
            }
        }
        return best;
    }


    //--------------------------------------------------------------------
    const T* FindLeEq(const T& key) const
    {
        const T* node = Root;
        const T* best = 0;
        while (node)
        {
            if (Accessor::Less(key, *node))
            {
                node = Accessor::GetChild(node, 0);
            }
            else
            {
                best = node;
                node = Accessor::GetChild(node, 1);
            }
        }
        return best;
    }


    T*  Root;
};



//------------------------------------------------------------------------
template<class T, class Accessor> class SplayTreeMulti
{
public:

    //--------------------------------------------------------------------
    void Insert(T* n)
    {
        T* t = Tree.Splay(&Tree.Root, *n);

        if (t == 0 || !Accessor::Equal(*t, *n))
        {
            // Insert into the tree
            //--------------------
            n->pPrev = n->pNext = 0;
            Tree.Insert(n, t);
        }
        else
        {
            // Link to the existing tree node
            //--------------------
            n->pNext = t->pNext;
            n->pPrev = t;
            t->pNext = n;
            if (n->pNext)
                n->pNext->pPrev = n;
        }
    }

    //--------------------------------------------------------------------
    void Remove(T* n, bool splayed = false)
    {
        // If the node 'n' is not the first node in the linked list,
        // simply de-link 'n' from the linked list.
        //-------------------
        if (n->pPrev)
        {
            n->pPrev->pNext = n->pNext;
            if (n->pNext)
                n->pNext->pPrev = n->pPrev;
        }
        else
        {
            // The node 'n' is the first node of the linked list.
            // Remove it from the tree and add the next node from the linked
            // list to the tree.
            //--------------------
            if (n->pNext)
            {
                T* t = (T*)n->pNext;
                T* x = Tree.Root;
                
                if (splayed)
                    x = n;
                else
                    x = Tree.Splay(&x, *n);

                if (x && Accessor::Equal(*x, *n))
                {
                    Accessor::SetChild(t, 0, Accessor::GetChild(x, 0));
                    Accessor::SetChild(t, 1, Accessor::GetChild(x, 1));
                    t->pPrev  = 0;
                    Tree.Root = (T*)n->pNext;
                }
            }
            else
            {
                Tree.Remove(n, false);
            }
        }
    }

    //--------------------------------------------------------------------
    T* PullBestGrEq(const T& key)
    {
        T* node = Tree.Splay(&Tree.Root, key);
        bool splayed = true;

        if (node && Accessor::Less(*node, key))
        {
            splayed = false;
            if ((node = Accessor::GetChild(node, 1)) != 0)
            {
                while (Accessor::GetChild(node, 0))
                    node = (T*)Accessor::GetChild(node, 0);
            }
        }

        if (node)
        {
            // Take the next node to reduce splaying 
            // the tree in RemoveMulti().
            //------------------------
            if (node->pNext)
                node = (T*)node->pNext;

            Remove(node, splayed);
        }
        return node;

    }

    //--------------------------------------------------------------------
    T* PullBestLeEq(UPInt key)
    {
        // TO DO
        return 0;
    }

    SplayTree<T, Accessor> Tree;
};


} // Scaleform

#endif

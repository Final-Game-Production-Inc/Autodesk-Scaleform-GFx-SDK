/**************************************************************************

Filename    :   GameUIMessageBuffer.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameUIMessageBuffer_H
#define INC_GameUIMessageBuffer_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_HeapNew.h"
using namespace Scaleform;


// Messages of arbitrary lengths can be appended to the buffer
// and they are tracked appropriately. When the buffer overflows,
// the oldest message is popped off until there is enough space
// for the new message.


//////////////////////////////////////////////////////////////////////////
// A list of message lengths that track the items in the message 
// buffer
//
class GameMsgLenList
{
    struct MsgLengthListNode : 
        public NewOverrideBase<Stat_Default_Mem>, 
        public ListNode<MsgLengthListNode>
    {
        unsigned   Data;
        MsgLengthListNode(const unsigned& val) { Data = val; }
    };

public:  
    GameMsgLenList() : Size(0) {};
    ~GameMsgLenList() 
    {
        MsgLengthListNode* node = List.GetFirst();
        while (!List.IsNull(node))
        {        
            MsgLengthListNode* temp = node;
            node = List.GetNext(node);
            delete temp;
        }        
    }

    SF_INLINE unsigned    Front() 
    { 
        SF_ASSERT(List.GetFirst()); 
        return List.GetFirst()->Data;
    }  
    void    PopFront()
    {
        MsgLengthListNode* node = List.GetFirst();
        List.Remove(List.GetFirst());
        delete node;
        Size--;
    }
    void    PushBack(const unsigned& val)
    {
        MsgLengthListNode* node = SF_HEAP_NEW(Memory::GetGlobalHeap()) MsgLengthListNode(val);
        List.PushBack(node);
        Size++;
    }
    SF_INLINE unsigned    GetSize() { return Size; }

private:
    List<MsgLengthListNode>     List;
    unsigned                    Size;
};


//////////////////////////////////////////////////////////////////////////
// Cyclic message buffer with configurable storage size.
//
template <int sz>
class GameMessageBuffer : public RefCountBase<GameMessageBuffer<sz>, Stat_Default_Mem>
{
public:
    GameMessageBuffer()
    {
        SF_ASSERT(sz > 0);
        Size = sz;
        pData = (char*)SF_HEAP_ALLOC(Memory::GetGlobalHeap(), Size+1, Stat_Default_Mem);
        memset(pData, NULL, Size+1);

        Sentinel = 0;

        UncompiledLength = 0;
        OverflowLength = 0;

        FirstIndex = 0;
    }
    virtual ~GameMessageBuffer()
    {
        SF_FREE(pData);
    }

    // The algorithm that provides the cyclic buffering ability
    // causes the raw buffer to be split into two logical parts.
    // Appending the two parts provides the logical representation
    // of the messages in the raw buffer.

    // Return the first part of the buffer
    SF_INLINE char* GetPartA()
    {
        if (FirstIndex)
            return (pData + FirstIndex);
        return (pData + Sentinel);
    }

    // Return the second part of the buffer
    SF_INLINE char* GetPartB() { return (pData); }

    // Return the size of the buffer
    SF_INLINE unsigned GetSize() { return Size; }

    // Return the number of messages in the buffer
    SF_INLINE unsigned GetNumMessages() { return MessageLengths.GetSize(); }

    // Return the raw buffer
    SF_INLINE char* GetBuffer() { return pData; }

    // Append part of the message (Not added to the buffer until
    // Compile is called)
    GameMessageBuffer& Append(const String& str) 
    {
        return Append(str.ToCStr(), (unsigned)str.GetSize());
    }
    GameMessageBuffer& Append(const char* str)
    {
        return Append(str, SFstrlen(str));
    }
    GameMessageBuffer& Append(const char* str, unsigned len)
    {
        // step through each character
        unsigned count = 0;
        while (count < len)
        {
            // set the character in our buffer
            pData[Sentinel] = *(str + count);

            // if the sentinel buffer overruns, reset to 0
            if (++Sentinel > (Size-1))
            {
                Sentinel = 0;
            }

            // if the sentinel moves to first index's position
            // then we have an overflow
            if (Sentinel == FirstIndex)
            {
                // move the first index in front of the sentinel
                FirstIndex = Sentinel + 1;

                // if the first index buffer overruns, reset to 0
                if (FirstIndex > (Size-1))
                {
                    FirstIndex = 0;
                }

                // increment overflow count
                OverflowLength++;
            }
            count++;
        }

        // set the sentinel to \0
        pData[Sentinel] = NULL;

        // compute the uncompiled length and clamp it between 0 and buffer size
        UncompiledLength += len;
        UncompiledLength = Alg::Clamp<unsigned>(UncompiledLength, 0, Size-1);

        // clamp overflow length between 0 and buffer size
        OverflowLength = Alg::Clamp<unsigned>(OverflowLength, 0, Size-1);

        return *this;
    }

    // Compile the parts and submit to the buffer as a complete 
    // message
    void Compile()
    {
        // calculate how far first index needs to travel
        int remainder = OverflowLength;
        while ( (remainder > 0) && MessageLengths.GetSize() )
        {
            unsigned l = MessageLengths.Front();
            //cout << "l: " << l << endl;
            MessageLengths.PopFront();
            remainder -= l;
        }
        // if remainder is ever greater than 0, clamp it to 0
        if (remainder > 0) 
            remainder = 0;

        // handle case when the first index is at 0 and we're moving it
        if (OverflowLength && !FirstIndex)
            pData[0] = NULL;

        // move the first index appropriately
        FirstIndex -= remainder;
        if (FirstIndex > (Size-1))
        {
            FirstIndex -= Size;
            if (FirstIndex)
                pData[0] = NULL;
        }

        // remember the message
        MessageLengths.PushBack(UncompiledLength);

        // clear the tracking variables
        UncompiledLength = 0;
        OverflowLength = 0;
    }

protected:
    GameMsgLenList     MessageLengths;

    // Tracking variables used when new messages are added to the buffer
    unsigned        UncompiledLength;
    unsigned        OverflowLength; 

    unsigned        Size;         // Size of the buffer
    char*           pData;        // The string data
    unsigned        Sentinel;     // Floating sentinel
    unsigned        FirstIndex;   // Index of the first entry
};

#endif  // INC_GameUIMessageBuffer_H

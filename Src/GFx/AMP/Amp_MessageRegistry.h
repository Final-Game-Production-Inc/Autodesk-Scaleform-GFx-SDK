/*
* Copyright 2011 Autodesk, Inc.  All rights reserved.
* Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download, 
* or which otherwise accompanies this software in either electronic or hard copy form.
*/

#ifndef INCLUDE_GFX_AMP_MESSAGEREGISTRY_H
#define INCLUDE_GFX_AMP_MESSAGEREGISTRY_H

#include "GFxConfig.h"
#if defined(SF_AMP_SERVER) || defined(SF_AMP_CLIENT)

#include "Amp_Interfaces.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_StringHash.h"
#include "Kernel/SF_Atomic.h"

namespace Scaleform
{
namespace GFx
{
namespace AMP
{

class ThreadMgr;
class Message;

// Only a base class, it is used directly by the registry.
class BaseMessageTypeDescriptor : public RefCountBase<BaseMessageTypeDescriptor, Stat_Default_Mem>
{
public:
    virtual ~BaseMessageTypeDescriptor() { }
    
    const String& GetMessageTypeName() const  { return MessageTypeName; }
    bool CanHandle() const { return MessageHandler != NULL; }
    bool ShouldHandleImmediately() const { return HandleImmediately; }
    void Handle(Message* msg) const  { MessageHandler->Handle(msg); }
    
    virtual Message* CreateMessage() const = 0;

protected:
    BaseMessageTypeDescriptor(const char* messageTypeName, Ptr<IMessageHandler> messageHandler)
        : MessageHandler(messageHandler)
        , MessageTypeName(messageTypeName)
        , HandleImmediately(false)
    {
    }

    void SetHandleImmediately(bool handleImmediately) { HandleImmediately = handleImmediately; }

    friend class MessageTypeRegistry;

    Ptr<IMessageHandler>    MessageHandler;
    StringLH                MessageTypeName;
    bool                    HandleImmediately;
};

// MessageTypeDescriptor is used directly by the registry.
// TMessage is expected to inherit from Message class
template <class TMessage>
class MessageTypeDescriptor : public BaseMessageTypeDescriptor
{
private:
    friend class MessageTypeRegistry;

    MessageTypeDescriptor(const char* messageTypeName, Ptr<IMessageHandler> messageHandler)
        : BaseMessageTypeDescriptor(messageTypeName, messageHandler)
    {
    }

    virtual TMessage* CreateMessage() const 
    { 
        return SF_HEAP_AUTO_NEW(this) TMessage();
    }
};

// This is class is used to register different kind of messages and is in charge to assign them an id/index
// It can also be used to translate/convert message type
class MessageTypeRegistry : public RefCountBase<MessageTypeRegistry, StatAmp_Message>
{
public:
    // If handler is NULL it means that the message is not treated
    // TMessage is expected to inherit from Message class
    template<class TMessage>
    void AddMessageType(Ptr<IMessageHandler> handler, bool bypassQueue = false)
    {
        BaseMessageTypeDescriptor* myDescriptor = GetMessageTypeDescriptor(TMessage::GetStaticTypeName());
        if (myDescriptor == NULL)
        {
            String descName(TMessage::GetStaticTypeName());
            MessageTypeDescriptor<TMessage>* desc = SF_HEAP_AUTO_NEW(this) MessageTypeDescriptor<TMessage>(descName, handler);
            if (bypassQueue)
            {
                desc->SetHandleImmediately(true);
            }
            DescriptorMap.Add(descName, *desc);
        }
        else
        {
            SF_DEBUG_WARNING1(myDescriptor->MessageHandler != NULL, "Handler of descriptor %s is overridden", myDescriptor->GetMessageTypeName().ToCStr());
            myDescriptor->MessageHandler = handler;
        }            
    }

    void AddMessageTypeRegistry(const MessageTypeRegistry& other);

    BaseMessageTypeDescriptor* GetMessageTypeDescriptor(const String& messageTypeName) const
    {
        TypeNameDescriptorMap::ConstIterator it = DescriptorMap.Find(messageTypeName);
        if (it != DescriptorMap.End())
        {
            return it->Second;
        }

        return NULL;
    }

    // TMessage is expected to inherit from Message class
    template<class TMessage>
    TMessage* CreateMessage() const
    {
        return static_cast<TMessage*>(CreateMessage(TMessage::GetStaticTypeName()));
    }

    Message* CreateMessage(const String& messageTypeName) const;

protected:

    typedef StringHashLH< Ptr<BaseMessageTypeDescriptor> > TypeNameDescriptorMap;
    TypeNameDescriptorMap DescriptorMap;

};


}
}
}
#endif

#endif

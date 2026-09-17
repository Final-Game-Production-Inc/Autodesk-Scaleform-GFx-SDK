/*
* Copyright 2011 Autodesk, Inc.  All rights reserved.  
* This computer source code and related instructions and comments are the 
* unpublished confidential and proprietary information of Autodesk, Inc. and 
* are protected under applicable copyright and trade secret law. They may 
* not be disclosed to, copied or used by any third party without 
* the prior written consent of Autodesk, Inc.
*/


#include "Amp_MessageRegistry.h"

#ifdef SF_AMP_SERVER
#include "Amp_Message.h"

namespace Scaleform {
namespace GFx {
namespace AMP {


Message* MessageTypeRegistry::CreateMessage(const String& messageTypeName) const
{
    Message* message = NULL;
    BaseMessageTypeDescriptor* descriptor = GetMessageTypeDescriptor(messageTypeName);
    if (descriptor != NULL)
    {
        message = descriptor->CreateMessage();
    }
    return message;
}


void MessageTypeRegistry::AddMessageTypeRegistry(const MessageTypeRegistry& other)
{
    for (TypeNameDescriptorMap::ConstIterator it = other.DescriptorMap.Begin(); it != other.DescriptorMap.End(); ++it)
    {
        BaseMessageTypeDescriptor* descriptor = GetMessageTypeDescriptor(it->First);
        if (descriptor == NULL)
        {
            DescriptorMap.Add(it->First, it->Second);
        }
        else
        {
            SF_DEBUG_WARNING1(descriptor->MessageHandler != NULL, "Handler of %s is overridden", descriptor->GetMessageTypeName().ToCStr());
            descriptor->MessageHandler = it->Second->MessageHandler;
        }

    }
}

}
}
}

#endif

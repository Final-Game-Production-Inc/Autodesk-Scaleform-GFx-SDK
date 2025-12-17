/**************************************************************************

Filename    :   FxSharedObjectManager.cpp
Content     :   Shared object manager for FxPlayer 
Created     :   January 22, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxSharedObjectManager.h"

#ifdef GFX_AS2_ENABLE_SHAREDOBJECT
#if !defined(SF_NO_FXPLAYER_AS_SHAREDOBJECT) && !defined(SF_NO_XML_SUPPORT)

#include "XML/XML_Expat.h"
#include "GFx/XML/XML_DOM.h"
#include "Kernel/SF_SysFile.h"

//#define SF_DEBUG_FXSHAREDOBJECTWRITER


FxSharedObjectWriter::FxSharedObjectWriter(File* pfile)
{
    pFile = pfile;
    LineIndentation = 0;
}


FxSharedObjectWriter::~FxSharedObjectWriter()
{
    pFile->Close();
    pFile = NULL;
}


void FxSharedObjectWriter::WriteOut()
{
    // First pad by tab level
    for (UPInt i = 0; i < LineIndentation; i++)
        pFile->Write((UByte*)"  ", 2);
    pFile->Write((UByte*)LineBuf.ToCStr(), (int)LineBuf.GetSize());
    LineBuf.Clear();
}


void FxSharedObjectWriter::Begin()
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE(1, "FxSharedObjectWriter::Begin");
#endif // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write XML header
    LineBuf += "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n";
    LineBuf += "<data>\n";
    WriteOut();
    LineIndentation++;
}


void FxSharedObjectWriter::PushObject( const String& name )
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE1(1, "FxSharedObjectWriter::PushObject('%s')", name.ToCStr());
#endif  // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write object header
    LineBuf += "<object name=\"";
    LineBuf += name;
    LineBuf += "\">\n";
    WriteOut();
    LineIndentation++;
}


void FxSharedObjectWriter::PushArray( const String& name )
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE1(1, "FxSharedObjectWriter::PushArray('%s')", name.ToCStr());
#endif  // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write array header
    LineBuf += "<array name=\"";
    LineBuf += name;
    LineBuf += "\">\n";
    WriteOut();
    LineIndentation++;
}

void FxSharedObjectWriter::AddProperty( const String& name, 
                                       const String& value, 
                                       GFx::Value::ValueType type )
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE2(1, "FxSharedObjectWriter::PushProperty('%s', '%s')", 
        name.ToCStr(), value.ToCStr());
#endif  // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write property
    switch (type)
    {
    case GFx::Value::VT_Int:
        {
            LineBuf += "<int";
            break;
        }
    case GFx::Value::VT_UInt:
        {
            LineBuf += "<uint";
            break;
        }
    case GFx::Value::VT_Number:
        {
            LineBuf += "<number";
            break;
        }
    case GFx::Value::VT_String:
        {
            LineBuf += "<string";
            break;
        }
    case GFx::Value::VT_Boolean:
        {
            LineBuf += "<boolean";
            break;
        }
    case GFx::Value::VT_Null:
        {
            LineBuf += "<null";
            break;
        }
    case GFx::Value::VT_Undefined:
        {
            LineBuf += "<undefined";
            break;
        }
    default:
        {
            SF_ASSERT(0); // Unknown
        }
    }
    LineBuf += " name=\"";
    LineBuf += name;
    if (value.GetSize())
    {
        LineBuf += "\" value=\"";
        LineBuf += value;
    }
    LineBuf += "\" />\n";
    WriteOut();
}


void FxSharedObjectWriter::PopObject()
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE(1, "FxSharedObjectWriter::PopObject");
#endif  // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write object footer
    LineIndentation--;
    LineBuf += "</object>\n";
    WriteOut();
}


void FxSharedObjectWriter::PopArray()
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE(1, "FxSharedObjectWriter::PopArray");
#endif  // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write array footer
    LineIndentation--;
    LineBuf += "</array>\n";
    WriteOut();
}


void FxSharedObjectWriter::End()
{
#ifdef SF_DEBUG_FXSHAREDOBJECTWRITER
    SF_DEBUG_MESSAGE(1, "FxSharedObjectWriter::End");
#endif  // SF_DEBUG_FXSHAREDOBJECTWRITER

    // Write XML footer
    LineIndentation--;
    LineBuf += "</data>\n";
    WriteOut();
}


///////////////////////////////////////////////
//

FxSharedObjectManager::FxSharedObjectManager(const String& soCachePath)
{
    SOCachePath = soCachePath;
}


FxSharedObjectManager::~FxSharedObjectManager()
{
}


#ifdef GFX_ENABLE_XML
//
// Find attributes matching 'name' and 'value'
//
static void    FindNameValueAttributes(const GFx::XML::ElementNode* pelem, 
									   String& name, 
									   String& value)
{
    // Step through all attributes to find the matching one
    for (GFx::XML::Attribute* pattr = pelem->FirstAttribute; pattr != NULL; 
        pattr = pattr->Next)
    {
        if ( !SFstrncmp(pattr->Name.ToCStr(), "name", 4) )
        {
            name = pattr->Value.ToCStr();
        }
        else if ( !SFstrncmp(pattr->Name.ToCStr(), "value", 5) )
        {
            value = pattr->Value.ToCStr();
        }
    }
}


static void		LoadFromXML(GFx::XML::Node* pnode, GFx::SharedObjectVisitor* reader)
{
    if (!pnode) return;
    String name, value;
    if (pnode->Type == GFx::XML::ElementNodeType)
    {
        GFx::XML::ElementNode* pelem = static_cast<GFx::XML::ElementNode*>(pnode);
        // Iterate over all children
        for (GFx::XML::Node* pchild = pelem->FirstChild ; pchild != NULL; 
            pchild = pchild->NextSibling.GetPtr() )
        {
            if (pchild->Type != GFx::XML::ElementNodeType) continue;
            GFx::XML::ElementNode* pchildelem = static_cast<GFx::XML::ElementNode*>(pchild);
            FindNameValueAttributes(pchildelem, name, value);
            if ( !SFstrncmp(pchildelem->Value.ToCStr(), "object", 6) )
            {
                reader->PushObject( name );
                LoadFromXML( pchildelem, reader );
                reader->PopObject();
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "array", 5) )
            {
                reader->PushArray( name );
                LoadFromXML( pchildelem, reader );
                reader->PopArray();
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "string", 6) )
            {                
                reader->AddProperty(name, value, GFx::Value::VT_String);
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "int", 3) )
            {
                reader->AddProperty(name, value, GFx::Value::VT_Int);
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "uint", 4) )
            {
                reader->AddProperty(name, value, GFx::Value::VT_UInt);
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "number", 6) )
            {
                reader->AddProperty(name, value, GFx::Value::VT_Number);
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "undefined", 9) )
            {
                reader->AddProperty(name, value, GFx::Value::VT_Undefined);
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "null", 4) )
            {
                reader->AddProperty(name, value, GFx::Value::VT_Null);
            }
            else if ( !SFstrncmp(pchildelem->Value.ToCStr(), "boolean", 7) )
            {
                reader->AddProperty(name, value, GFx::Value::VT_Boolean);
            }
            else
            {
                // Error
                SF_DEBUG_MESSAGE1(1, 
                    "Error: FxSharedObjectManager::LoadSharedObject - Unknown tag '%s'!", 
                    pchild->Value.ToCStr());
            }
        }
    }
    return;
}
#endif


bool FxSharedObjectManager::LoadSharedObject(const String& name, 
                                             const String& localPath, 
                                             GFx::SharedObjectVisitor* psobj, 
                                             GFx::FileOpenerBase* pfo)
{
    if (!psobj)
    {
        SF_DEBUG_MESSAGE(1, 
            "Error: FxSharedObjectManager::LoadSharedObject - SharedObjectVisitor is NULL!");
        return false;
    }
    if (!pfo)
    {
        SF_DEBUG_MESSAGE(1, 
            "Error: FxSharedObjectManager::LoadSharedObject - FileOpener is not set!");
        return false;
    }
#ifdef GFX_ENABLE_XML
    Ptr<GFx::XML::Parser> pparser = *SF_HEAP_NEW(Memory::GetGlobalHeap()) GFx::XML::ParserExpat;
    Ptr<GFx::XML::Support> pxmlsupport = *SF_HEAP_NEW(Memory::GetGlobalHeap()) GFx::XML::Support(pparser);
    GFx::XML::DOMBuilder domb(pxmlsupport, true);

    GenerateFilePath(name, localPath);

    // Check if file exists; bail if not found (not an error)
    FileStat fileStats;
    if ( !SysFile::GetFileStat(&fileStats, GeneratedFilePath.ToCStr()) )
        return true;

    // NOTE: It's best to consolidate the xml object manager that is created in the next statement
    // with the MovieRoot's obj manager if possible
    Ptr<GFx::XML::Document> pdoc = domb.ParseFile(GeneratedFilePath.ToCStr(), pfo);
    if (!pdoc)
    {
        SF_DEBUG_MESSAGE1(1, 
            "Error: FxSharedObjectManager::LoadSharedObject - File '%s' cannot be opened!", 
            GeneratedFilePath.ToCStr());
        return false;
    }
    psobj->Begin();
    LoadFromXML(pdoc->FirstChild, psobj);
    psobj->End();
    return true;
#else
    SF_UNUSED2(name, localPath);
    SF_DEBUG_MESSAGE(1, 
        "Error: FxSharedObjectManager::LoadSharedObject - XML support is disabled!");
    return false;
#endif
}


GFx::SharedObjectVisitor* FxSharedObjectManager::CreateWriter(const String& name, 
                                                            const String& localPath,
                                                            GFx::FileOpenerBase* pfileOpener) 
{        
    if (!pfileOpener)
    {
        SF_DEBUG_MESSAGE(1, "Error: FxSharedObjectManager::CreateWriter - FileOpener is NULL!");
        return NULL;
    }
    GenerateFilePath(name, localPath);
    
    Ptr<File> pfile = *pfileOpener->OpenFile(GeneratedFilePath, 
        FileConstants::Open_Create|FileConstants::Open_Truncate);
    if (!pfile)
    {
        SF_DEBUG_MESSAGE(1, "Error: FxSharedObjectManager::CreateWriter - OpenFile failed!");
        return NULL;    // System does not support writing files?
    }
    return SF_HEAP_NEW(Memory::GetGlobalHeap()) FxSharedObjectWriter(pfile);
}


void FxSharedObjectManager::GenerateFilePath(const String& name, const String& localPath)
{
    GeneratedFilePath = SOCachePath;
    if (localPath.GetSize() > 0)
    {
        GeneratedFilePath += localPath;
        GeneratedFilePath.AppendChar('_');
    }
    GeneratedFilePath.AppendString(name.ToCStr());
    GeneratedFilePath.AppendString(".sox");
}

#endif  // SF_NO_FXPLAYER_AS_SHAREDOBJECT && SF_NO_XML_SUPPORT
#endif //GFX_AS2_ENABLE_SHAREDOBJECT
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

#include "GFxCompat.h"
#include "FxSharedObjectManager.h"

#if !defined(GFC_NO_FXPLAYER_AS_SHAREDOBJECT) && !defined(GFC_NO_XML_SUPPORT)

#include <GFxXML.h>
#include <GFxXMLExpat.h>
#include <GHeapNew.h>
#include <GSysFile.h>

//#define GFC_DEBUG_FXSHAREDOBJECTWRITER


FxSharedObjectWriter::FxSharedObjectWriter(GFile* pfile)
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
    pFile->Write((UByte*)LineBuf.ToCStr(), (SInt)LineBuf.GetSize());
    LineBuf.Clear();
}


void FxSharedObjectWriter::Begin()
{
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE(1, "FxSharedObjectWriter::Begin");
#endif // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write XML header
    LineBuf += "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n";
    LineBuf += "<data>\n";
    WriteOut();
    LineIndentation++;
}


void FxSharedObjectWriter::PushObject( const GString& name )
{
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE1(1, "FxSharedObjectWriter::PushObject('%s')", name.ToCStr());
#endif  // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write object header
    LineBuf += "<object name=\"";
    LineBuf += name;
    LineBuf += "\">\n";
    WriteOut();
    LineIndentation++;
}


void FxSharedObjectWriter::PushArray( const GString& name )
{
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE1(1, "FxSharedObjectWriter::PushArray('%s')", name.ToCStr());
#endif  // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write array header
    LineBuf += "<array name=\"";
    LineBuf += name;
    LineBuf += "\">\n";
    WriteOut();
    LineIndentation++;
}

void FxSharedObjectWriter::AddProperty( const GString& name, 
                                       const GString& value, 
                                       GFxValue::ValueType type )
{
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE2(1, "FxSharedObjectWriter::PushProperty('%s', '%s')", 
        name.ToCStr(), value.ToCStr());
#endif  // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write property
    switch (type)
    {
    case GFxValue::VT_Number:
        {
            LineBuf += "<number";
            break;
        }
    case GFxValue::VT_String:
        {
            LineBuf += "<string";
            break;
        }
    case GFxValue::VT_Boolean:
        {
            LineBuf += "<boolean";
            break;
        }
    case GFxValue::VT_Null:
        {
            LineBuf += "<null";
            break;
        }
    case GFxValue::VT_Undefined:
        {
            LineBuf += "<undefined";
            break;
        }
    default:
        {
            GASSERT(0); // Unknown
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
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE(1, "FxSharedObjectWriter::PopObject");
#endif  // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write object footer
    LineIndentation--;
    LineBuf += "</object>\n";
    WriteOut();
}


void FxSharedObjectWriter::PopArray()
{
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE(1, "FxSharedObjectWriter::PopArray");
#endif  // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write array footer
    LineIndentation--;
    LineBuf += "</array>\n";
    WriteOut();
}


void FxSharedObjectWriter::End()
{
#ifdef GFC_DEBUG_FXSHAREDOBJECTWRITER
    GFC_DEBUG_MESSAGE(1, "FxSharedObjectWriter::End");
#endif  // GFC_DEBUG_FXSHAREDOBJECTWRITER

    // Write XML footer
    LineIndentation--;
    LineBuf += "</data>\n";
    WriteOut();
}


///////////////////////////////////////////////
//

FxSharedObjectManager::FxSharedObjectManager(const GString& soCachePath)
{
    SOCachePath = soCachePath;
}


FxSharedObjectManager::~FxSharedObjectManager()
{
}


//
// Find attributes matching 'name' and 'value'
//
static void    FindNameValueAttributes(const GFxXMLElementNode* pelem, 
									   GString& name, 
									   GString& value)
{
    // Step through all attributes to find the matching one
    for (GFxXMLAttribute* pattr = pelem->FirstAttribute; pattr != NULL; 
        pattr = pattr->Next)
    {
        if ( !G_strncmp(pattr->Name.ToCStr(), "name", 4) )
        {
            name = pattr->Value.ToCStr();
        }
        else if ( !G_strncmp(pattr->Name.ToCStr(), "value", 5) )
        {
            value = pattr->Value.ToCStr();
        }
    }
}


static void		LoadFromXML(GFxXMLNode* pnode, GFxSharedObjectVisitor* reader)
{
    if (!pnode) return;
    GString name, value;
    if (pnode->Type == GFxXMLElementNodeType)
    {
        GFxXMLElementNode* pelem = static_cast<GFxXMLElementNode*>(pnode);
        // Iterate over all children
        for (GFxXMLNode* pchild = pelem->FirstChild ; pchild != NULL; 
            pchild = pchild->NextSibling.GetPtr() )
        {
            if (pchild->Type != GFxXMLElementNodeType) continue;
            GFxXMLElementNode* pchildelem = static_cast<GFxXMLElementNode*>(pchild);
            FindNameValueAttributes(pchildelem, name, value);
            if ( !G_strncmp(pchildelem->Value.ToCStr(), "object", 6) )
            {
                reader->PushObject( name );
                LoadFromXML( pchildelem, reader );
                reader->PopObject();
            }
            else if ( !G_strncmp(pchildelem->Value.ToCStr(), "array", 5) )
            {
                reader->PushArray( name );
                LoadFromXML( pchildelem, reader );
                reader->PopArray();
            }
            else if ( !G_strncmp(pchildelem->Value.ToCStr(), "string", 6) )
            {                
                reader->AddProperty(name, value, GFxValue::VT_String);
            }
            else if ( !G_strncmp(pchildelem->Value.ToCStr(), "number", 6) )
            {
                reader->AddProperty(name, value, GFxValue::VT_Number);
            }
            else if ( !G_strncmp(pchildelem->Value.ToCStr(), "undefined", 9) )
            {
                reader->AddProperty(name, value, GFxValue::VT_Undefined);
            }
            else if ( !G_strncmp(pchildelem->Value.ToCStr(), "null", 4) )
            {
                reader->AddProperty(name, value, GFxValue::VT_Null);
            }
            else if ( !G_strncmp(pchildelem->Value.ToCStr(), "boolean", 7) )
            {
                reader->AddProperty(name, value, GFxValue::VT_Boolean);
            }
            else
            {
                // Error
                GFC_DEBUG_MESSAGE1(1, 
                    "Error: FxSharedObjectManager::LoadSharedObject - Unknown tag '%s'!", 
                    pchild->Value.ToCStr());
            }
        }
    }
    return;
}


bool FxSharedObjectManager::LoadSharedObject(const GString& name, 
                                             const GString& localPath, 
                                             GFxSharedObjectVisitor* psobj, 
                                             GFxFileOpenerBase* pfo)
{
    if (!psobj)
    {
        GFC_DEBUG_MESSAGE(1, 
            "Error: FxSharedObjectManager::LoadSharedObject - SharedObjectVisitor is NULL!");
        return false;
    }
    if (!pfo)
    {
        GFC_DEBUG_MESSAGE(1, 
            "Error: FxSharedObjectManager::LoadSharedObject - FileOpener is not set!");
        return false;
    }
    GPtr<GFxXMLParser> pparser = *GHEAP_NEW(GMemory::GetGlobalHeap()) GFxXMLParserExpat;
    GPtr<GFxXMLSupport> pxmlsupport = *GHEAP_NEW(GMemory::GetGlobalHeap()) GFxXMLSupport(pparser);
    GFxXMLDOMBuilder domb(pxmlsupport, true);

    GenerateFilePath(name, localPath);

    // Check if file exists; bail if not found (not an error)
    GFileStat fileStats;
    if ( !GSysFile::GetFileStat(&fileStats, GeneratedFilePath.ToCStr()) )
        return true;

    // NOTE: It's best to consolidate the xml object manager that is created in the next statement
    // with the MovieRoot's obj manager if possible
    GPtr<GFxXMLDocument> pdoc = domb.ParseFile(GeneratedFilePath.ToCStr(), pfo);
    if (!pdoc)
    {
        GFC_DEBUG_MESSAGE1(1, 
            "Error: FxSharedObjectManager::LoadSharedObject - File '%s' cannot be opened!", 
            GeneratedFilePath.ToCStr());
        return false;
    }
    psobj->Begin();
    LoadFromXML(pdoc->FirstChild, psobj);
    psobj->End();
    return true;
}


GFxSharedObjectVisitor* FxSharedObjectManager::CreateWriter(const GString& name, 
                                                            const GString& localPath,
                                                            GFxFileOpenerBase* pfileOpener) 
{        
    if (!pfileOpener)
    {
        GFC_DEBUG_MESSAGE(1, "Error: FxSharedObjectManager::CreateWriter - FileOpener is NULL!");
        return NULL;
    }
    GenerateFilePath(name, localPath);
    
    GPtr<GFile> pfile = *pfileOpener->OpenFile(GeneratedFilePath, 
        GFileConstants::Open_Create|GFileConstants::Open_Truncate);
    if (!pfile)
    {
        GFC_DEBUG_MESSAGE(1, "Error: FxSharedObjectManager::CreateWriter - OpenFile failed!");
        return NULL;    // System does not support writing files?
    }
    return GHEAP_NEW(GMemory::GetGlobalHeap()) FxSharedObjectWriter(pfile);
}


void FxSharedObjectManager::GenerateFilePath(const GString& name, const GString& localPath)
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

#endif  // GFC_NO_FXPLAYER_AS_SHAREDOBJECT && GFC_NO_XML_SUPPORT

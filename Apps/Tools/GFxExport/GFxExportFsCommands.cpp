/**************************************************************************

Filename    :   GFxExportFsCommands.cpp
Content     :   SWF to GFX resource extraction and conversion tool
Created     :   October, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#define GFX_EXPORT_MAJOR_VERSION    1
#define GFX_EXPORT_MINOR_VERSION    11
#define GFX_EXPORT_VERSION (((GFX_EXPORT_MAJOR_VERSION)<<8)|(GFX_EXPORT_MINOR_VERSION))

#include "GFxExport.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_Button.h"
#include "Kernel/SF_SysFile.h"
#include "Kernel/SF_MsgFormat.h"

#include "GFx/AS2/AS2_ActionTypes.h"

// Standard includes
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <zlib.h>

struct GFxFsCommand
{
    int                     EventId;  // for placeObject
    String               EventStr; // for placeObject
    int                     Conditions; // button condition mask
    Array<int>             CommandIndices;
    Array<int>             ParameterIndices;

    GFxFsCommand() : EventId(0), Conditions(0) {}
};

struct GFxFsCommandOrigin
{
    enum 
    {
        None,
        Action,
        InitAction,
        Button,
        MovieClip,
        PlaceObject
    }                           Type;
    int                         Id;
    String                   Name;
    Array<GFxFsCommand>        Commands;
    Array<GFxFsCommandOrigin>  NestedCommands;

    GFxFsCommandOrigin() : Type(None), Id(0) {}
};

static const UByte* memstr (const UByte * str1, UPInt strSize,
                            const char * str2)
{
    if ( !*str2 )
        return(str1);

    UPInt i;
    for(i = 0; i < strSize; i++)
    {
        UPInt j = i, k = 0;
        while ( j < strSize && *(str2 + k) && !(*(str1 + j)-*(str2 + k)) )
            j++, k++;

        if (!*(str2 + k))
            return(str1 + i);
    }

    return(NULL);

}

template <class T>
T* GetBufferPtr(unsigned reqSize, T** pdynBuf, unsigned* pdynBufSize, T* statBuf, unsigned statBufSize)
{
    if (statBuf && reqSize <= statBufSize)
        return statBuf;
    if (pdynBufSize && pdynBuf && *pdynBuf && reqSize <= *pdynBufSize)
        return *pdynBuf;
    T* pbuf = (T*) SF_ALLOC(reqSize, Stat_Default_Mem);
    if (pdynBuf)
        *pdynBuf = pbuf;
    if (pdynBufSize)
        *pdynBufSize = reqSize;
    return pbuf;
}

template <class T>
void FreeBuffer(T** pdynBuf, unsigned* pdynBufSize)
{
    if (pdynBuf && *pdynBuf)
    {
        SF_FREE(*pdynBuf);
        *pdynBuf = 0;
    }
    if (pdynBufSize)
        *pdynBufSize = 0;
}

static void WriteIndents(FILE* fp, unsigned indent)
{
    for (unsigned i = 0; i < indent; i++)
        fputs("   ", fp);
}

static int SearchForCmds(const UByte* pbuf, UPInt bufSize, GFxFsCommand& cmd, Array<String>& stringHolderArray, bool exportParams)
{
    if (!pbuf)
        return 0;
    const UByte* fsCmd = pbuf;
    UPInt remainingSize = bufSize;
    const UByte* newfsCmd;
    do 
    {
        newfsCmd = memstr(fsCmd, remainingSize, "FSCommand:");
        if (newfsCmd)
        {
            UPInt len = strlen((const char*)newfsCmd);
            stringHolderArray.PushBack(String((const char*)newfsCmd + sizeof("FSCommand:")-1));
            cmd.CommandIndices.PushBack((int)stringHolderArray.GetSize()-1);

            remainingSize -= (len + (newfsCmd - fsCmd));
            fsCmd = newfsCmd;
            fsCmd += len;

            if (exportParams)
            {
                const unsigned char* pend = (const unsigned char*)fsCmd;
                const unsigned char* pparam = pend + 1;
                for (unsigned i = 1; i < remainingSize; ++i)
                {
                    ++pend;
                    if (*pend == 0)
                        break;
                    if (*pend < 32 || *pend > 127)
                    {
                        pparam = NULL;
                        break;
                    }
                }
                if (pparam && *pend == 0)
                {
                    stringHolderArray.PushBack(String((const char*)pparam));
                    cmd.ParameterIndices.PushBack((int)stringHolderArray.GetSize()-1);
                }
                else
                    cmd.ParameterIndices.PushBack(-1);
            }
            else
                cmd.ParameterIndices.PushBack(-1);

        }
    } while(newfsCmd != 0 && remainingSize > 0);
    return (int)cmd.CommandIndices.GetSize();
}

static String KeyCode2Str(int kc)
{
    String keyStr = "";
    switch(kc)
    {
    case 1: keyStr = "<Left>"; break;
    case 2: keyStr = "<Right>"; break;
    case 3: keyStr = "<Home>"; break;
    case 4: keyStr = "<End>"; break;
    case 5: keyStr = "<Insert>"; break;
    case 6: keyStr = "<Delete>"; break;
    case 8: keyStr = "<Backspace>"; break;
    case 13: keyStr = "<Enter>"; break;
    case 14: keyStr = "<Up>"; break;
    case 15: keyStr = "<Down>"; break;
    case 16: keyStr = "<PageUp>"; break;
    case 17: keyStr = "<PageDown>"; break;
    case 18: keyStr = "<Tab>"; break;
    case 19: keyStr = "<Escape>"; break;
    case 32: keyStr = "<Space>"; break;
    default: 
        {
            if (kc >= 32 && kc <= 126)
            {
                char buf[2];
                buf[0] = char(kc);
                buf[1] = '\0';
                keyStr = buf;
            }
        }
    }
    return keyStr;
}

static void WriteFsCommand(FILE* fp, const GFxFsCommand& cmd, Array<String>& stringHolderArray, bool writeParams, int indent)
{
    for (UPInt j = 0, k = cmd.CommandIndices.GetSize(); j < k; j++)
    {
        WriteIndents(fp, indent);
        fprintf(fp, "%s", stringHolderArray[cmd.CommandIndices[j]].ToCStr());
        if (writeParams && cmd.ParameterIndices[j] >= 0 && stringHolderArray[cmd.ParameterIndices[j]].GetSize() > 0)
        {
            fprintf(fp, "(\"%s\")", stringHolderArray[cmd.ParameterIndices[j]].ToCStr());
        }
        fprintf(fp, "\n");
    }
}

void GFxDataExporter::LookForFsCommandsInTags(MovieDef* pmovieDef, 
                                              File* pin, unsigned finalOffset, 
                                              Array<GFxFsCommandOrigin>& fscommands, 
                                              Array<String>& stringHolderArray)
{
    String errorMsg;

    SF_UNUSED(pmovieDef);
    SF_UNUSED4(pin, finalOffset, fscommands, stringHolderArray);

    Stream sin(pin, Memory::GetGlobalHeap(), NULL, NULL);
    TagInfo tag;

    UByte readStatBuf[4096];
    UByte* preadDynBuf = 0;
    unsigned dynReadBufSize = 0;
    while (sin.Tell() < (int)finalOffset)
    {
        int tagType = sin.OpenTag(&tag);
        SF_ASSERT(tag.TagOffset + tag.TagLength <= (int)finalOffset);

        sin.SyncFileStream();

        // skip tag header
        pin->Seek(tag.TagDataOffset);

        switch(tagType)
        {
        case 12:
        case 59:
            {
                UByte* preadBuf = GetBufferPtr(tag.TagLength, &preadDynBuf, &dynReadBufSize, readStatBuf, sizeof(readStatBuf));
                if (pin->Read(preadBuf, int(tag.TagLength)) != int(tag.TagLength))
                {
                    Format(errorMsg, "Error: Can't read from '{0}'", pin->GetFilePath());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    break;
                }
                GFxFsCommand cmd;
                if (SearchForCmds(preadBuf, tag.TagLength, cmd, stringHolderArray, FsCommandsParams) > 0)
                {
                    GFxFsCommandOrigin cmdOrig;
                    if (tagType == 12)
                        cmdOrig.Type = GFxFsCommandOrigin::Action;
                    else
                        cmdOrig.Type = GFxFsCommandOrigin::InitAction;
                    cmdOrig.Commands.PushBack(cmd);
                    fscommands.PushBack(cmdOrig);
                }
            }
            break;
        case 7:  // DefineButton
        case 34: // DefineButton2
            {
                UInt16 id = pin->ReadUInt16(); // button id

                Scaleform::GFx::MovieDefImpl* pmovieImpl = static_cast<Scaleform::GFx::MovieDefImpl*>(pmovieDef);
                String exportName;
                const String* pname = pmovieImpl->GetNameOfExportedResource(ResourceId(id));
                if (pname)
                    exportName = *pname;

                GFxFsCommandOrigin cmdOrig;
                cmdOrig.Type = GFxFsCommandOrigin::Button;
                cmdOrig.Id = id;
                cmdOrig.Name = exportName;

                Array<GFxFsCommand>& fscmds = cmdOrig.Commands;

                if (tag.TagType == 34)
                {
                    pin->ReadUByte(); // skip octet
                    int curoff = pin->Tell();
                    UInt16 actionOff = pin->ReadUInt16(); // actions offset
                    curoff += actionOff;
                    pin->Seek(curoff); // move to condactions

                    // read cond actions
                    while(curoff < tag.TagDataOffset + tag.TagLength)
                    {
                        UInt16 nextActionOff = pin->ReadUInt16();
                        UInt16 conditions = pin->ReadUInt16();

                        unsigned actionSize;
                        if (nextActionOff == 0)
                            actionSize = (tag.TagDataOffset + tag.TagLength) - pin->Tell();
                        else
                            actionSize = nextActionOff;
                        UByte* preadBuf = GetBufferPtr(actionSize, &preadDynBuf, &dynReadBufSize, readStatBuf, sizeof(readStatBuf));
                        if (pin->Read(preadBuf, int(actionSize)) != int(actionSize))
                        {
                            Format(errorMsg, "Error: Can't read from '{0}'", pin->GetFilePath());
                            Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                            break;
                        }
                        GFxFsCommand cmd;
                        cmd.Conditions = conditions;

                        if (SearchForCmds(preadBuf, actionSize, cmd, stringHolderArray, FsCommandsParams))
                            fscmds.PushBack(cmd);

                        if (nextActionOff == 0)
                            break;
                        curoff += actionSize;
                        pin->Seek(curoff);
                    }
                }
                else
                {
                    unsigned actionSize = (tag.TagDataOffset + tag.TagLength) - pin->Tell();
                    UByte* preadBuf = GetBufferPtr(actionSize, &preadDynBuf, &dynReadBufSize, readStatBuf, sizeof(readStatBuf));
                    if (pin->Read(preadBuf, int(actionSize)) != int(actionSize))
                    {
                        Format(errorMsg, "Error: Can't read from '{0}'", pin->GetFilePath());
                        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                        break;
                    }
                    GFxFsCommand cmd;

                    if (SearchForCmds(preadBuf, actionSize, cmd, stringHolderArray, FsCommandsParams))
                        fscmds.PushBack(cmd);

                }

                if (fscmds.GetSize() > 0)
                {
                    fscommands.PushBack(cmdOrig);
                }
            }
            break;
        case 39: // Define Sprite
            {
                UInt16 id = pin->ReadUInt16(); // sprite id
                pin->ReadUInt16(); // skip numFrames
                Scaleform::GFx::MovieDefImpl* pmovieImpl = static_cast<Scaleform::GFx::MovieDefImpl*>(pmovieDef);
                String exportName;
                const String* pname = pmovieImpl->GetNameOfExportedResource(ResourceId(id));
                if (pname)
                    exportName = *pname;

                GFxFsCommandOrigin cmdOrig;
                cmdOrig.Type = GFxFsCommandOrigin::MovieClip;
                cmdOrig.Id = id;
                cmdOrig.Name = exportName;

                int curOff = pin->Tell();
                unsigned size = tag.TagLength - (curOff - tag.TagDataOffset);
                UByte* preadBuf = GetBufferPtr(size, &preadDynBuf, &dynReadBufSize, readStatBuf, sizeof(readStatBuf));
                if (pin->Read(preadBuf, int(size)) != int(size))
                {
                    Format(errorMsg, "Error: Can't read from '{0}'", pin->GetFilePath());
                    Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                    break;
                }
                pin->Seek(curOff);

                // do a preliminary search
                GFxFsCommand cmd;
                if (SearchForCmds(preadBuf, tag.TagLength, cmd, stringHolderArray, FsCommandsParams) > 0)
                {
                    LookForFsCommandsInTags(pmovieDef, pin, tag.TagDataOffset + tag.TagLength, cmdOrig.NestedCommands, stringHolderArray);
                    fscommands.PushBack(cmdOrig);
                }
            }
            break;
        case 26: // PlaceObj2
        case 70: // PlaceObj3
            {
                GFxPlaceObjectBase::UnpackedData data;
                UPInt dataSz;
                PlaceObject2Tag* ptag2 = NULL;
                PlaceObject3Tag* ptag3 = NULL;
                if (tagType == 26)
                {
                    dataSz = PlaceObject2Tag::ComputeDataSize(&sin, ExpMovieInfo.Version);
                    bool hasEventHandlers = PlaceObject2Tag::HasEventHandlers(&sin);
                    if (hasEventHandlers)
                        dataSz += sizeof(PlaceObject2Tag::EventArrayType*);
                    if (ExpMovieInfo.Version >= 6)
                        ptag2 = Construct<PlaceObject2Tag>(SF_ALLOC(dataSz + sizeof(PlaceObject2Tag), StatMD_Tags_Mem));
                    else
                        ptag2 = Construct<PlaceObject2Taga>(SF_ALLOC(dataSz + sizeof(PlaceObject2Taga), StatMD_Tags_Mem));
                    if (hasEventHandlers)
                    {
                        ExecuteTag::LoadData(&sin, ptag2->pData, dataSz-sizeof(PlaceObject2Tag::EventArrayType*), sizeof(GFxPlaceObjectBase::EventArrayType*));
                        PlaceObject2Tag::RestructureForEventHandlers(ptag2->pData);
                    }
                    else
                        ExecuteTag::LoadData(&sin, (UByte*)ptag2->pData, dataSz);
                    ptag2->Unpack(data);
                }
                else
                {
                    dataSz = PlaceObject3Tag::ComputeDataSize(&sin);
                    bool hasEventHandlers = PlaceObject2Tag::HasEventHandlers(&sin);
                    if (hasEventHandlers)
                        dataSz += sizeof(PlaceObject3Tag::EventArrayType*);
                    ptag3 = Construct<PlaceObject3Tag>(SF_ALLOC(dataSz + sizeof(PlaceObject3Tag), StatMD_Tags_Mem));
                    if (hasEventHandlers)
                    {
                        ExecuteTag::LoadData(&sin, ptag3->pData, dataSz-sizeof(PlaceObject3Tag::EventArrayType*), sizeof(PlaceObject3Tag::EventArrayType*));
                        PlaceObject2Tag::RestructureForEventHandlers(ptag3->pData);
                    }
                    else
                        ExecuteTag::LoadData(&sin, (UByte*)ptag3->pData, dataSz);
                    ptag3->Unpack(data);
                }
                if (data.pEventHandlers)
                {
                    GFxFsCommandOrigin cmdOrig;
                    cmdOrig.Type = GFxFsCommandOrigin::PlaceObject;
                    cmdOrig.Id = data.Pos.CharacterId.GetIdIndex();
                    cmdOrig.Name = data.Name;

                    // check event handlers for fscommands
                    for(UPInt i = 0, n = data.pEventHandlers->GetSize(); i < n; ++i)
                    {
                        SwfEvent* pev = (*data.pEventHandlers)[i];
                        SF_ASSERT(pev);
                        if (!pev->pActionOpData)
                            continue;
                        const UByte* pactions = pev->pActionOpData->GetBufferPtr();
                        unsigned actionSize = pev->pActionOpData->GetLength();

                        GFxFsCommand cmd;
                        if (SearchForCmds(pactions, actionSize, cmd, stringHolderArray, FsCommandsParams) > 0)
                        {
                            cmd.EventId = pev->Event.Id;
                            String eventStr;
                            switch(pev->Event.Id)
                            {
                            case EventId::Event_Press: eventStr = "on("; eventStr += "press"; break;
                            case EventId::Event_Release: eventStr = "on("; eventStr += "release"; break;
                            case EventId::Event_ReleaseOutside: eventStr = "on("; eventStr += "releaseOutside"; break;
                            case EventId::Event_RollOver: eventStr = "on("; eventStr += "rollOver"; break;
                            case EventId::Event_RollOut: eventStr = "on("; eventStr += "rollOut"; break;
                            case EventId::Event_DragOver: eventStr = "on("; eventStr += "dragOver"; break;
                            case EventId::Event_DragOut: eventStr = "on("; eventStr += "dragOut"; break;
                            case EventId::Event_KeyPress: 
                                {
                                    eventStr = "on("; eventStr += "keyPress \""; 
                                    eventStr += KeyCode2Str(pev->Event.KeyCode);
                                    eventStr += "\"";
                                    break;
                                }

                            case EventId::Event_Initialize: eventStr = "onClipEvent("; eventStr += "initialize"; break;
                            case EventId::Event_Load: eventStr = "on("; eventStr += "load"; break;
                            case EventId::Event_Unload: eventStr = "on("; eventStr += "unload"; break;
                            case EventId::Event_EnterFrame: eventStr = "onClipEvent("; eventStr += "enterFrame"; break;
                            case EventId::Event_MouseDown: eventStr = "onClipEvent("; eventStr += "mouseDown"; break;
                            case EventId::Event_MouseUp: eventStr = "onClipEvent("; eventStr += "mouseUp"; break;
                            case EventId::Event_MouseMove: eventStr = "onClipEvent("; eventStr += "mouseMove"; break;
                            case EventId::Event_KeyDown: eventStr = "onClipEvent("; eventStr += "keyDown"; break;
                            case EventId::Event_KeyUp: eventStr = "onClipEvent("; eventStr += "keyUp"; break;
                            case EventId::Event_Data: eventStr = "on("; eventStr += "data"; break;
                            case EventId::Event_Construct: eventStr = "onClipEvent("; eventStr += "construct"; break;
                            default: eventStr = "(unknown";
                            }
                            eventStr += ")";
                            cmd.EventStr = eventStr;
                            cmdOrig.Commands.PushBack(cmd);
                        }
                    }
                    if (cmdOrig.Commands.GetSize() > 0)
                        fscommands.PushBack(cmdOrig);
                }
                if (tagType == 26)
                {
                    if (ExpMovieInfo.Version >= 6)
                        ptag2->~PlaceObject2Tag();
                    else
                        ((PlaceObject2Taga*)ptag2)->~PlaceObject2Taga();
                    SF_FREE(ptag2);
                }
                else
                {
                    ptag3->~PlaceObject3Tag();
                    SF_FREE(ptag3);
                }
            }
            break;
        }
        sin.CloseTag();
        sin.SyncFileStream();
    }
    FreeBuffer(&preadDynBuf, &dynReadBufSize);
}

void GFxDataExporter::DumpFsCommandsAsTree(FILE* fout, 
                                           Array<GFxFsCommandOrigin>& fscommands, 
                                           Array<String>& stringHolderArray, int indent)
{
    for (UPInt i = 0, n = fscommands.GetSize(); i < n; ++i)
    {
        GFxFsCommandOrigin& cmdOrig = fscommands[i];
        switch(cmdOrig.Type)
        {
        case GFxFsCommandOrigin::None:
            break;

        case GFxFsCommandOrigin::Action:
        case GFxFsCommandOrigin::InitAction:
            WriteIndents(fout, indent);
            if (cmdOrig.Type == GFxFsCommandOrigin::Action)
                fprintf(fout, "Action:\n");
            else
                fprintf(fout, "InitAction:\n");
            WriteFsCommand(fout, cmdOrig.Commands[0], stringHolderArray, FsCommandsParams, indent + 1);
            fputs("\n", fout);
            break;
        case GFxFsCommandOrigin::Button:
            {
                WriteIndents(fout, indent);
                fprintf(fout, "Button, id = %d", cmdOrig.Id);
                if (cmdOrig.Name.GetSize() > 0)
                    fprintf(fout, ", export name = %s", cmdOrig.Name.ToCStr());
                fprintf(fout, ":\n");
                for(UPInt ui = 0, un = cmdOrig.Commands.GetSize(); ui < un; ui++)
                {
                    GFxFsCommand& cmd = cmdOrig.Commands[ui];
                    int ind = indent;
                    if (cmd.Conditions != 0)
                    {
                        ind++;
                        WriteIndents(fout, ind);
                        fprintf(fout, "on(");
                        unsigned nn = 0;
                        for(unsigned j = 0, mask = 1; j < 9; j++, mask <<= 1)
                        {
                            const char* event = NULL;
                            if (cmd.Conditions & mask)
                            {
                                if (mask == AS2::ButtonAction::IDLE_TO_OVER_UP)
                                    event = "rollOver";
                                else if (mask == AS2::ButtonAction::OVER_UP_TO_IDLE)
                                    event = "rollOut";
                                else if (mask == AS2::ButtonAction::OVER_UP_TO_OVER_DOWN)
                                    event = "press";
                                else if (mask == AS2::ButtonAction::OVER_DOWN_TO_OVER_UP)
                                    event = "release";
                                else if (mask == AS2::ButtonAction::OVER_DOWN_TO_OUT_DOWN)
                                    event = "dragOut";
                                else if (mask == AS2::ButtonAction::OUT_DOWN_TO_OVER_DOWN)
                                    event = "dragOver";
                                else if (mask == AS2::ButtonAction::OUT_DOWN_TO_IDLE)
                                    event = "releaseOutside";
                                else if (mask == AS2::ButtonAction::IDLE_TO_OVER_DOWN)
                                    event = "mouseDown";
                                else if (mask == AS2::ButtonAction::OVER_DOWN_TO_IDLE)
                                    event = "mouseUp";
                                if (event)
                                {
                                    if (nn != 0)
                                        fprintf(fout, ", ");
                                    fprintf(fout, "%s", event);
                                    nn++;
                                }
                            }
                        }
                        if (cmd.Conditions & 0xFE00)
                        {
                            // keyPress
                            int kc = (cmd.Conditions >> 9) & 0x7F;
                            String keyStr = KeyCode2Str(kc);
                            if (nn != 0)
                                fprintf(fout, ", ");
                            fprintf(fout, "keyPress \"%s\"", keyStr.ToCStr());
                        }
                        fprintf (fout, "):\n");
                        ind++;
                    }
                    WriteFsCommand(fout, cmd, stringHolderArray, FsCommandsParams, ind);
                    fputs("\n", fout);
                }
                fputs("\n", fout);
            }
            break;
        case GFxFsCommandOrigin::MovieClip:
            WriteIndents(fout, indent);
            fprintf(fout, "MovieClip, id = %d", cmdOrig.Id);
            if (cmdOrig.Name.GetSize() > 0)
                fprintf(fout, ", export name = %s", cmdOrig.Name.ToCStr());
            fprintf(fout, ":\n");
            if (cmdOrig.NestedCommands.GetSize() > 0)
                DumpFsCommandsAsTree(fout, cmdOrig.NestedCommands, stringHolderArray, indent + 1);
            fputs("\n", fout);
            break;
        case GFxFsCommandOrigin::PlaceObject:
            WriteIndents(fout, indent);
            fprintf(fout, "Object instance");
            if (cmdOrig.Id != 0)
                fprintf(fout, ", id = %d", cmdOrig.Id);
            if (cmdOrig.Name.GetSize() > 0)
                fprintf(fout, ", instance name = %s", cmdOrig.Name.ToCStr());
            fprintf(fout, ":\n");
            for(UPInt ui = 0, un = cmdOrig.Commands.GetSize(); ui < un; ui++)
            {
                GFxFsCommand& cmd = cmdOrig.Commands[ui];
                int ind = indent + 1;
                          
                WriteIndents(fout, ind++);
                fprintf(fout, "%s:\n", cmd.EventStr.ToCStr());
                WriteFsCommand(fout, cmd, stringHolderArray, FsCommandsParams, ind);
                fputs("\n", fout);
            }
            fputs("\n", fout);
            break;
        }
    }
}

static void AddFsCommandsToList(Array<String>& cmdList, 
                                Array<int>& cmdSortedIdx, 
                                const GFxFsCommand& cmd, 
                                Array<String>& stringHolderArray, 
                                bool addParams)
{
    for (UPInt j = 0, k = cmd.CommandIndices.GetSize(); j < k; j++)
    {
        String str = stringHolderArray[cmd.CommandIndices[j]].ToCStr();
        if (addParams && cmd.ParameterIndices[j] >= 0 && stringHolderArray[cmd.ParameterIndices[j]].GetSize() > 0)
        {
            str += "(\"";
            str += stringHolderArray[cmd.ParameterIndices[j]];
            str += "\")";
        }
        // find a location. Should be a sorted array. 
        UPInt i,n;
        for (i = 0, n = cmdSortedIdx.GetSize(); i < n; ++i)
        {
            if (str.CompareNoCase(cmdList[cmdSortedIdx[i]]) <= 0)
                break;
        }
        if (i < n)
        {
            if (str.CompareNoCase(cmdList[cmdSortedIdx[i]]) != 0)
            {
                cmdList.PushBack(str);
                cmdSortedIdx.InsertAt(i, (int)cmdList.GetSize()-1);
            }
        }
        else
        {
            cmdList.PushBack(str);
            cmdSortedIdx.PushBack((int)cmdList.GetSize()-1);
        }
    }

}

void GFxDataExporter::MakeFsCommandsAsList(Array<GFxFsCommandOrigin>& fscommands, 
                                           Array<String>& stringHolderArray, 
                                           Array<String>& cmdList, 
                                           Array<int>& cmdSortedIdx)
{
    for (UPInt i = 0, n = fscommands.GetSize(); i < n; ++i)
    {
        GFxFsCommandOrigin& cmdOrig = fscommands[i];
        switch(cmdOrig.Type)
        {
        case GFxFsCommandOrigin::None:
            break;

        case GFxFsCommandOrigin::Action:
        case GFxFsCommandOrigin::InitAction:
            AddFsCommandsToList(cmdList, cmdSortedIdx, cmdOrig.Commands[0], stringHolderArray, FsCommandsParams);
            break;
        case GFxFsCommandOrigin::Button:
            {
                for(UPInt ui = 0, un = cmdOrig.Commands.GetSize(); ui < un; ui++)
                {
                    GFxFsCommand& cmd = cmdOrig.Commands[ui];
                    AddFsCommandsToList(cmdList, cmdSortedIdx, cmd, stringHolderArray, FsCommandsParams);
                }
            }
        case GFxFsCommandOrigin::MovieClip:
            if (cmdOrig.NestedCommands.GetSize() > 0)
                MakeFsCommandsAsList(cmdOrig.NestedCommands, stringHolderArray, cmdList, cmdSortedIdx);
            break;
        case GFxFsCommandOrigin::PlaceObject:
            for(UPInt pi = 0, pn = cmdOrig.Commands.GetSize(); pi < pn; pi++)
            {
                GFxFsCommand& cmd = cmdOrig.Commands[pi];
                AddFsCommandsToList(cmdList, cmdSortedIdx, cmd, stringHolderArray, FsCommandsParams);
            }
            break;
        }
    }
}

void GFxDataExporter::WriteFsCommands(MovieDef* pmovieDef, const char* swfFileName, const String& path, const String& name, unsigned mask)
{
    String errorMsg;

    if (!Quiet) 
    {
        Format(errorMsg, "Looking for fscommands in '{0}'", swfFileName);
        Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
    }

    Ptr<File> pin = *new SysFile(swfFileName, File::Open_Read);
    if (!pin || !pin->IsValid())
    {
        Format(errorMsg, "Error: Can't open source file '{0}' to read from", swfFileName);
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return;
    }

    // load header
    UInt32 header          = pin->ReadUInt32();
    pin->ReadUInt32(); // fileLength
    bool   compressed      = (header & 255) == 'C';
    if (compressed)
    {
        pin = *new ZLibFile(pin);
    }

    if (!pin || !pin->IsValid())
    {
        Format(errorMsg, "Error: Can't read from source file '{0}'", pin->GetFilePath());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return;
    }

    String names;

    Array<GFxFsCommandOrigin> fscommands;
    Array<String> stringHolderArray;

    for (UPInt i = 0, n = TagsWithActions.GetSize(); i < n; ++i)
    {
        const TagInfo& tag = TagsWithActions[i];
        pin->Seek(tag.TagOffset);
        LookForFsCommandsInTags(pmovieDef, pin, tag.TagDataOffset + tag.TagLength, fscommands, stringHolderArray);
    }

    for (unsigned curMask = 1; curMask <= mask; curMask <<= 1)
    {
        if (curMask & mask)
        {
            String fname = path;
            fname += name;
            if (curMask == FSList)
            {
                fname += ".fsl";
                if (!Quiet) 
                {
                    Format(errorMsg, "Saving list of fscommands to '{0}'", fname.ToCStr());
                    Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
                }
            }
            else
            {
                fname += ".fst";
                if (!Quiet) 
                {
                    Format(errorMsg, "Saving tree of fscommands to '{0}'", fname.ToCStr());
                    Logger->OnMessage(GFxExportListener::MSG_Normal, errorMsg);
                }
            }

            FILE* fout;
#if defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
            fout = NULL;
            fopen_s(&fout, fname, "w");
#else
            fout = fopen(fname, "w");
#endif // defined(SF_CC_MSVC) && (SF_CC_MSVC >= 1400)
            if (!fout)
            {
                Format(errorMsg, "Error: Can't open destination file '{0}' to write to", fname.ToCStr());
                Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
                return;
            }
            
            if (curMask == FSTree)
            {
                DumpFsCommandsAsTree(fout, fscommands, stringHolderArray, 0);
            }
            if (curMask == FSList)
            {
                Array<String>  cmdList;
                Array<int>        cmdSortedIdx;
                MakeFsCommandsAsList(fscommands, stringHolderArray, cmdList, cmdSortedIdx);
                for (UPInt i = 0, n = cmdSortedIdx.GetSize(); i < n; ++i)
                {
                    fprintf(fout, "%s\n", cmdList[cmdSortedIdx[i]].ToCStr());
                }
            }

            fclose(fout);
//            if (!Quiet) printf("\n");
        }
    }
}


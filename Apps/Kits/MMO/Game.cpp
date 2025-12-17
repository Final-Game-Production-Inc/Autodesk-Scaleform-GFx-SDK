/**************************************************************************

Filename    :   Game.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Game.h"


class MMOGameConnection : public NewOverrideBase<Stat_Default_Mem>, public GameConnection
{
public:
    MMOGameConnection() {}
    virtual ~MMOGameConnection();

    virtual void                SendRequest(GameClientRequest* request);    
    virtual GameClientRequest*  GetNextRequest();
    virtual bool                HasRequests();

    virtual void                SendEvent(GameServerEvent* event);
    virtual GameServerEvent*    GetNextEvent();
    virtual bool                HasEvents();

protected:    
    struct RNode : public NewOverrideBase<Stat_Default_Mem>, public ListNode<RNode>
    {
        GameClientRequest*      Request;
        RNode(GameClientRequest* request) : Request(request) {}
    };
    List<RNode>     RequestQueue;
    Lock            RequestQueueLock;

    struct ENode : public NewOverrideBase<Stat_Default_Mem>, public ListNode<ENode>
    {
        GameServerEvent*        Event;
        ENode(GameServerEvent* event) : Event(event) {}
    };
    List<ENode>     EventQueue;
    Lock            EventQueueLock;

};

MMOGameConnection::~MMOGameConnection()
{
    RNode* rdata = RequestQueue.GetFirst();
    while (!RequestQueue.IsNull(rdata))
    {
       RNode* next = RequestQueue.GetNext(rdata);
       RequestQueue.Remove(rdata);
       delete rdata->Request;
       delete rdata;
       rdata = next;
    }

    ENode* edata = EventQueue.GetFirst();
    while (!EventQueue.IsNull(edata))
    {
        ENode* next = EventQueue.GetNext(edata);
        EventQueue.Remove(edata);
        delete edata->Event;
        delete edata;
        edata = next;
    }
}

void    MMOGameConnection::SendRequest(GameClientRequest* request)
{
    Lock::Locker    locker(&RequestQueueLock);
    RNode* node = SF_NEW RNode(request);
    RequestQueue.PushBack(node);
}

bool    MMOGameConnection::HasRequests()
{
    bool yes = false;
    {
        Lock::Locker    locker(&RequestQueueLock);
        yes = !RequestQueue.IsEmpty();
    }
    return yes;
}

GameClientRequest*  MMOGameConnection::GetNextRequest()
{
    Lock::Locker    locker(&RequestQueueLock);
    if (RequestQueue.IsEmpty()) return NULL;
    RNode* node = RequestQueue.GetFirst();
    RequestQueue.Remove(node);
    GameClientRequest* request = node->Request;
    delete node;
    return request;
}

void    MMOGameConnection::SendEvent(GameServerEvent* event)
{
    Lock::Locker    locker(&EventQueueLock);
    ENode* node = SF_NEW ENode(event);
    EventQueue.PushBack(node);
}

bool    MMOGameConnection::HasEvents()
{
    bool yes = false;
    {
        Lock::Locker    locker(&EventQueueLock);
        yes = !EventQueue.IsEmpty();
    }
    return yes;
}

GameServerEvent*  MMOGameConnection::GetNextEvent()
{
    Lock::Locker    locker(&EventQueueLock);
    if (EventQueue.IsEmpty()) return NULL;
    ENode* node = EventQueue.GetFirst();
    EventQueue.Remove(node);
    GameServerEvent* event = node->Event;
    delete node;
    return event;
}


//////////////////////////////////////////////////////////////////////////
//
//

MMOGame::MMOGame()
{
    Connection = SF_NEW MMOGameConnection;

    TheServer = *SF_NEW GameServer(Connection);
    TheServer->Start();
}

MMOGame::~MMOGame()
{
    delete Connection;
}

void    MMOGame::Initialize(GFx::Movie* pmovie, GameUIRuntimeAtlasManager* pimagecreator)
{   
    if (!pmovie) return;

    TheClient = *SF_NEW GameClient(pmovie, Connection);
    TheClient->Initialize(pimagecreator);

    TheClient->Connect("Terranin");
}

void    MMOGame::Shutdown()
{
    TheClient = NULL;
    TheServer->SetExitFlag(true);
    while (!TheServer->IsFinished())
    {
        Thread::MSleep(100);
    }
    TheServer = NULL;
}

void    MMOGame::Update()
{
    if (TheClient) TheClient->Update();
}

/**************************************************************************

Filename    :   GameAiBehavior.cpp
Content     :   
Created     :   2011
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// Includes
#include "GameServer.h"
#include "GamePawn.h"


const char* fakeChat[] = 
{
    "hi",
    "What's up?",
    "lol :)",
    "no :(",
    "Yay!",
    "that's good :P",
    "I'm done",
    " I'll be there tue night through fri",
    "as stuff in the spec changed incompatibly",
    "yes exactly",
    "when?",
    "July I believe",
    "depends on how many distractions etc",
    "we're now about 9 companie",
    "s",
    "I suppose I won't bore you with the rest.",
    "Funny you should ask about that...",
    "argh!",
    "HEEEEEEEEELP!!!",
    "Evening",
    "hey sh1m",
    "Hey Gregg.",
    "I hope you are happy now.",
    "hmm?",
    "hahahaha",
    "That's going to look great if the guys who I just asked to apply for a PhD goaogle.",
    "i'd really only be worried about the one of you licking adam's head",
    "haha",
    "That one is quality.",
    "yup",
    "It's the dancing ones that bother me",
    "I seem to be enjoying it a bit too much.",
    "The beardy guy is scary, and I don't even know who he was.",
    "well, you seemed to be enjoying it at the time",
    "I think he was hanging around with the South African Merceniary or something",
    "haha... now /that/ guy was creepy",
    "damn jiggy",
    "I just have the feeling he had like a 3' machete down the back of his pants",
    "machete",
    "hahaha",
    "I can't speel. ;) Well known farct.",
    "i was laughing more at the idea of that guy having something 3' long down his pants",
    "Well I wouldn't have been suprised.",
    "pook so is Adam back in Cali now?",
    "not quite yet. he was here on the east coast with me until wednesday. and now he's in phoenix visiting family.",
    "i think he's on his way back to cali some time next week.",
    "oh. you aren't in Cali. What have I forgotten?",
    "i moved to rhode island upon returning from galway",
    "how is that going then?",
    "everything is great except for not having a job :)",
    "keeping myself busy with the photo annotation, though",
    "hehe",
    "yeah you could sink lots of time into that",
    "could?? I'm way past that point!",
    "it's basically all I do anymore",
    "heh",
    "LMAO",
    "I think my talents are bugged. it still says my ticket is going to take 5 days",
    "i applaude that to be honest",
    "haha xD",
    "pretty clever lol",
    "Ok real quick!",
    "Yay!",
    "O_O I dunno",
    "I had fun chatting with you though!",
    "byebye",
    "you know how i can tell that you did not place the trap correctly for a trap pull?",
    "it's because i have eyes",
    "and they are blind",
    "yeah",
    "im back, sorry bout that...nature was calling",
    "What did it have to say",
    "how can you have the king slayer when you pull 2.9k fps lol",
    "beacuse im the tank",
    "i just reported you afk",
    "ok that was funny",
    "sup",
    "writing a new song",
    "i call it",
    "cool",
    "is that server time?",
    "Hm...",
    "sorry",
    "hey, can you help me out with a portal ? :)",
    "lfm bg 10, need dps",
    "head",
    "Are you here?",
    "That's mean..",
    "please answer",
    "thats cold",
    "I HATE dat place",
    "Dont remember what its called",
    "hellfire?",
    "Hmm I said black",
    "RIGHT??",
    "u do that kind of stuff?",
    "Giggity",
    "8",
    "its over nyan thousand!",
    "You gotta multiply first before you do adding",
    "then? or not?",
    "just use it..",
    "noobs",
    "ivs started ?",
    "not yet it seems",
    "How do magnets work?",
    "Ok ty",
    "tele hack",
    "speed thing",
    "why can't i mana burn you?",
    "oh right",
    "That made my day",
    "why are there 3 hunters here",
    "i'm leaven because there are already 2",
    "oh my",
    "eh, depends",
    "that's kinda disturbing",
    "ew",
    "and there it is",
    "gaah!!#",
    "same to you",
    "yeh",
    "maybe he wants to learn?",
    "maybe",
    "IM SO ANGRY",
    "NO",
    "SO I CAN GET MY HONOR",
    "screenshot",
    "greed it?",
    "well",
    "and you kiss your mom with that mouth?",
    "i'm standing on boat it leave and i am just standing in same place in mid air",
    "and i keep getting back on and same thing happens",
    "3rd time its happened",
    "does anyone ever read the terms and conditions ? :P",
    "Of course, everyone does...just...really fast as they scroll by :P",
    "thx bro",
    "btw",
    "well tell me why do they speak like that? :D",
    "hahahahah nice one",
    "If you need help with anything, just give me a shout",
    "Unemployed",
    "Ohhhh, lets see now. Will have to make up a new one then.",
    "Depends how hard you throw them :P",
    "The second if you find one with a mohawk haircut and use him as painbrush",
    "oe i'm excited",
    "TRUST ME I WILL BE YOUR DEATH",
    "YOUR CORPSE WILL BE MY GAME",
    "the badgers guide to the galaxy",
    "Alien vs Badger",
    "i was just stating a fact",
    "Yea, 5 of them, like the quest says",
    "wat",
    "do the mobs just drop the scraps",
    "wow",
    "the explanation is so thorough",
    "the part where theres a purpose to kill them?",
    "when it says kill x monster then use this item",
    "well here's a joke for you :-)",
    "HAHAHAHAHAHAH",
    "Ah well he's dead",
    "... Not amused",
    "Fail",
    "People are dying and you make fun of it",
    "and 1 raid",
    "curse you dragon!!!",
    "xD",
    "Yea, I guess so :)",
    "Always!",
    "for this boss",
    "If they are killed they dont spawn adds",
    "easy to eat",
    "yes it can be iterrupted",
    "burst portals and then boss",
    "DO YOU HAVE GOLD 4 SALE",
    "The answer is \"A mirror\"",
    "well done",
    "i'm having a bbq this wkdn",
};

// Nice trick from http://heifner.blogspot.com/2008/04/c-array-size-determination.html
template <typename T, size_t N>
inline
size_t array_size(const T (&lhs)[N])
{
    SF_UNUSED(lhs);
    return N;
}


//////////////////////////////////////////////////////////////////////////

GameAiBehaviorChatterBox::GameAiBehaviorChatterBox() : LastChatterTicks(0)
{
    Interval = (rand() % 8000) + 2000;

    Mode = 0;
}

void    GameAiBehaviorChatterBox::OnWhisper(GameChannelMessage msg)
{
    (void)msg;
    Mode = 2;
    Interval = 1500;
    LastChatterTicks = Timer::GetTicksMs();
}

void    GameAiBehaviorChatterBox::Process(GameServer* env, UInt32 ticks, GamePawn* pawn)
{
    // Broadcast a random message in the world channel
    if (ticks - LastChatterTicks > Interval)
    {
        GameChannelMessage  msg;
        msg.SourceId = pawn->GetInfo(Game::INFO_ID);
        msg.SourceName = pawn->GetInfo(Game::INFO_Name);

        SF_ASSERT(pawn->GetType() == GamePawn::PAWN_Player);
        GamePlayerPawn* player = (GamePlayerPawn*)pawn;

        switch (Mode)
        {
        case 0:     // Message to world channel
            {
                msg.Message = fakeChat[rand() % array_size(fakeChat)];
                env->GetChannel("world")->Broadcast(msg);
                break;
            }
        case 1:     // Message to guild
            {
                msg.Message = fakeChat[rand() % array_size(fakeChat)];
                if (player->GetGuild())
                    player->GetGuild()->Broadcast(msg);
                break;
            }
        case 2:     // Private message to specific pawn
            {
                msg.Message = fakeChat[rand() % array_size(fakeChat)];
                env->SendWhisperToPlayer("Terranin", msg);
                break;
            }
        }

        unsigned newMode = rand() % 100;
        if (newMode < 70)
            Mode = 0;
        else
            Mode = 1;

        Interval = (rand() % 8000) + 2000;
        LastChatterTicks = ticks;
    }
}


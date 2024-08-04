/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PetitionSignAction.h"

#include "ArenaTeam.h"
#include "Event.h"
#include "Playerbots.h"

bool PetitionSignAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid petitionGuid;
    ObjectGuid inviter;
    uint8 unk = 0;
    bool isArena = false;
    p >> petitionGuid >> inviter;

    QueryResult result =
        CharacterDatabase.Query("SELECT `type` FROM `petition` WHERE `petitionguid` = {}", petitionGuid.GetCounter());
    if (!result)
    {
        return false;
    }

    Field* fields = result->Fetch();
    uint32 type = fields[0].Get<uint32>();

    bool accept = true;

    if (type != 9)
    {
        isArena = true;
        uint8 slot = ArenaTeam::GetSlotByType(ArenaType(type));
        if (bot->GetArenaTeamId(slot))
        {
            // player is already in an arena team
            botAI->TellError("对不起，我已经加入这样的队伍了");
            accept = false;
        }
    }
    else
    {
        if (bot->GetGuildId())
        {
            botAI->TellError("对不起，我已经加入公会了");
            accept = false;
        }

        if (bot->GetGuildIdInvited())
        {
            botAI->TellError("对不起，我已经被邀请加入公会了");
            accept = false;
        }

        // check for same acc id
        /*if (QueryResult* result = CharacterDatabase.Query("SELECT playerguid FROM petition_sign WHERE player_account =
        {} AND petitionguid = {}'", bot->GetSession()->GetAccountId(), petitionGuid.GetCounter()))
        {
            botAI->TellError("Sorry, I already signed this pettition");
            accept = false;
        }
        */
    }

    Player* _inviter = ObjectAccessor::FindPlayer(inviter);
    if (!_inviter)
        return false;

    if (_inviter == bot)
        return false;

    if (!accept || !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, _inviter, true))
    {
        WorldPacket data(MSG_PETITION_DECLINE);
        data << petitionGuid;
        bot->GetSession()->HandlePetitionDeclineOpcode(data);
        LOG_INFO("playerbots", "机器人 {} <{}> 谢绝了 {} 邀请", bot->GetGUID().ToString().c_str(),
                 bot->GetName().c_str(), isArena ? "竞技场" : "公会");
        return false;
    }

    if (accept)
    {
        WorldPacket data(CMSG_PETITION_SIGN, 20);
        data << petitionGuid << unk;
        bot->GetSession()->HandlePetitionSignOpcode(data);
        bot->Say("谢谢邀请", LANG_UNIVERSAL);
        LOG_INFO("playerbots", "机器人 {} <{}> 接受了 {} 邀请", bot->GetGUID().ToString().c_str(),
                 bot->GetName().c_str(), isArena ? "竞技场" : "公会");
        return true;
    }

    return false;
}

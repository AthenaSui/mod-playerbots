/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotSecurity.h"
#include "LFGMgr.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

PlayerbotSecurity::PlayerbotSecurity(Player* const bot) : bot(bot)
{
    if (bot)
        account = sCharacterCache->GetCharacterAccountIdByGuid(bot->GetGUID());
}

PlayerbotSecurityLevel PlayerbotSecurity::LevelFor(Player* from, DenyReason* reason, bool ignoreGroup)
{
    if (from->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        return PLAYERBOT_SECURITY_ALLOW_ALL;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI) {
        return PLAYERBOT_SECURITY_DENY_ALL;
    }
    if (botAI->IsOpposing(from))
    {
        if (reason)
            *reason = PLAYERBOT_DENY_OPPOSING;

        return PLAYERBOT_SECURITY_DENY_ALL;
    }

    if (sPlayerbotAIConfig->IsInRandomAccountList(account))
    {
        if (botAI->IsOpposing(from))
        {
            if (reason)
                *reason = PLAYERBOT_DENY_OPPOSING;

            return PLAYERBOT_SECURITY_DENY_ALL;
        }

        // if (sLFGMgr->GetState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
        // {
        //     if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
        //     {
        //         if (reason)
        //             *reason = PLAYERBOT_DENY_LFG;

        //         return PLAYERBOT_SECURITY_TALK;
        //     }
        // }

        Group* group = from->GetGroup();
        if (group)
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (player == bot && !ignoreGroup)
                    return PLAYERBOT_SECURITY_ALLOW_ALL;
            }
        }

        if (sPlayerbotAIConfig->groupInvitationPermission <= 0) {
            if (reason)
                *reason = PLAYERBOT_DENY_NONE;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (sPlayerbotAIConfig->groupInvitationPermission <= 1 && (int32)bot->getLevel() - (int8)from->getLevel() > 5)
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_LOW_LEVEL;

                return PLAYERBOT_SECURITY_TALK;
            }
        }

        int32 botGS = (int32)botAI->GetEquipGearScore(bot, false, false);
        int32 fromGS = (int32)botAI->GetEquipGearScore(from, false, false);
        if (sPlayerbotAIConfig->gearscorecheck)
        {
            if (botGS && bot->getLevel() > 15 && botGS > fromGS &&
                static_cast<float>(100 * (botGS - fromGS) / botGS) >= static_cast<float>(12 * sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) / from->getLevel()))
            {
                if (reason) *reason = PLAYERBOT_DENY_GEARSCORE;
                return PLAYERBOT_SECURITY_TALK;
            }
        }

        if (bot->InBattlegroundQueue())
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_BG;

                return PLAYERBOT_SECURITY_TALK;
            }
        }

        /*if (bot->isDead())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_DEAD;

            return PLAYERBOT_SECURITY_TALK;
        }*/

        group = bot->GetGroup();
        if (!group)
        {
            /*if (bot->GetMapId() != from->GetMapId() || bot->GetDistance(from) > sPlayerbotAIConfig->whisperDistance)
            {
                if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
                {
                    if (reason)
                        *reason = PLAYERBOT_DENY_FAR;

                    return PLAYERBOT_SECURITY_TALK;
                }
            }*/

            if (reason)
                *reason = PLAYERBOT_DENY_INVITE;

            return PLAYERBOT_SECURITY_INVITE;
        }

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->GetSource();
            if (player == from)
                return PLAYERBOT_SECURITY_ALLOW_ALL;
        }

        if (group->IsFull())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_FULL_GROUP;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (group->GetLeaderGUID() != bot->GetGUID())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_NOT_LEADER;

            return PLAYERBOT_SECURITY_TALK;
        }
        else
        {
            if (reason)
                *reason = PLAYERBOT_DENY_IS_LEADER;

            return PLAYERBOT_SECURITY_INVITE;
        }

        if (reason)
            *reason = PLAYERBOT_DENY_INVITE;

        return PLAYERBOT_SECURITY_INVITE;
    }

    return PLAYERBOT_SECURITY_ALLOW_ALL;
}

bool PlayerbotSecurity::CheckLevelFor(PlayerbotSecurityLevel level, bool silent, Player* from, bool ignoreGroup)
{
    DenyReason reason = PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel realLevel = LevelFor(from, &reason, ignoreGroup);
    if (realLevel >= level)
        return true;

    PlayerbotAI* fromBotAI = GET_PLAYERBOT_AI(from);
    if (silent || (fromBotAI && !fromBotAI->IsRealPlayer()))
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Player* master = botAI->GetMaster();
    if (master && botAI && botAI->IsOpposing(master) && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        return false;

    std::ostringstream out;
    switch (realLevel)
    {
        case PLAYERBOT_SECURITY_DENY_ALL:
            out << "我现在有点忙";
            break;
        case PLAYERBOT_SECURITY_TALK:
            switch (reason)
            {
            case PLAYERBOT_DENY_NONE:
                out << "我稍后再做";
                break;
            case PLAYERBOT_DENY_LOW_LEVEL:
                out << "你等级太低了：|cffff0000" << (uint32)from->getLevel() << "|cffffffff/|cff00ff00" << (uint32)bot->getLevel();
                break;
            case PLAYERBOT_DENY_GEARSCORE:
                {
                    int botGS = (int)botAI->GetEquipGearScore(bot, false, false);
                    int fromGS = (int)botAI->GetEquipGearScore(from, false, false);
                    int diff = (100 * (botGS - fromGS) / botGS);
                    int req = 12 * sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) / from->getLevel();
                    out << "你的装备等级太低了：|cffff0000" << fromGS << "|cffffffff/|cff00ff00" << botGS << " |cffff0000" << diff << "%|cffffffff/|cff00ff00" << req << "%";
                }
                break;
            case PLAYERBOT_DENY_NOT_YOURS:
                out << "I have a master already";
                break;
            case PLAYERBOT_DENY_IS_BOT:
                out << "你是机器人";
                break;
            case PLAYERBOT_DENY_OPPOSING:
                out << "你是敌人";
                break;
            case PLAYERBOT_DENY_DEAD:
                out << "我死了。稍后再做";
                break;
            case PLAYERBOT_DENY_INVITE:
                out << "请先邀请我加入你的队伍";
                break;
            case PLAYERBOT_DENY_FAR:
                {
                    out << "你必须近一点以邀请我加入你的小组。我在 ";

                    if (AreaTableEntry const* entry = sAreaTableStore.LookupEntry(bot->GetAreaId()))
                    {
                        out << " |cffffffff(|cffff0000" << entry->area_name[4] << "|cffffffff)";
                    }
                }
                break;
            case PLAYERBOT_DENY_FULL_GROUP:
                out << "我在一个满员的队伍里。稍后再做";
                break;
            case PLAYERBOT_DENY_IS_LEADER:
                out << "我目前正在带领一个队伍。如果你愿意，我可以邀请你。";
                break;
            case PLAYERBOT_DENY_NOT_LEADER:
                if (botAI->GetGroupMaster())
                {
                    out << "我跟 " << botAI->GetGroupMaster()->GetName() << "在一个队伍中。你可以让他邀请你。";
                }
                else
                {
                    out << "我和其他人在一个队伍里。你可以让他邀请你。";
                }
                break;
            case PLAYERBOT_DENY_BG:
                out << "我在排战场。稍后再做";
                break;
            case PLAYERBOT_DENY_LFG:
                out << "我在排副本。稍后再做";
                break;
            default:
                out << "我不能那样做。";
                break;
            }
            break;
        case PLAYERBOT_SECURITY_INVITE:
            out << "请先邀请我加入您的队伍";
            break;
        default:
            out << "我不能那样做";
            break;
    }

    std::string const text = out.str();
    ObjectGuid guid = from->GetGUID();
    time_t lastSaid = whispers[guid][text];
    if (!lastSaid || (time(nullptr) - lastSaid) >= sPlayerbotAIConfig->repeatDelay / 1000)
    {
        whispers[guid][text] = time(nullptr);
        bot->Whisper(text, LANG_UNIVERSAL, from);
    }

    return false;
}

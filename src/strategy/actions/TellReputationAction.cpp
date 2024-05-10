/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellReputationAction.h"
#include "ReputationMgr.h"
#include "Event.h"
#include "Playerbots.h"

bool TellReputationAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid selection = master->GetTarget();
    if (selection.IsEmpty())
        return false;

    Unit* unit = ObjectAccessor::GetUnit(*master, selection);
    if (!unit)
        return false;

    FactionTemplateEntry const* factionTemplate = unit->GetFactionTemplateEntry();
    uint32 faction = factionTemplate->faction;
    FactionEntry const* entry = sFactionStore.LookupEntry(faction);
    int32 reputation = bot->GetReputationMgr().GetReputation(faction);

    std::ostringstream out;
    out << entry->name[4] << ": ";
    out << "|cff";

    ReputationRank rank = bot->GetReputationMgr().GetRank(entry);
    switch (rank)
    {
        case REP_HATED:
            out << "cc2222仇恨";
            break;
        case REP_HOSTILE:
            out << "ff0000敌对";
            break;
        case REP_UNFRIENDLY:
            out << "ee6622冷淡";
            break;
        case REP_NEUTRAL:
            out << "ffff00中立";
            break;
        case REP_FRIENDLY:
            out << "00ff00友善";
            break;
        case REP_HONORED:
            out << "00ff88尊敬";
            break;
        case REP_REVERED:
            out << "00ffcc崇敬";
            break;
        case REP_EXALTED:
            out << "00ffff崇拜";
            break;
        default:
            out << "808080未知";
            break;
    }

    out << "|cffffffff";

    int32 base = ReputationMgr::Reputation_Cap + 1;
    for (int32 i = MAX_REPUTATION_RANK - 1; i >= rank; --i)
        base -= ReputationMgr::PointsInRank[i];

    out << "（" << (reputation - base) << "/" << ReputationMgr::PointsInRank[rank] << "）";
    botAI->TellMaster(out);

    return true;
}

/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GuildBankAction.h"
#include "GuildMgr.h"
#include "Playerbots.h"

bool GuildBankAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text.empty())
        return false;

    if (!bot->GetGuildId() || (GetMaster() && GetMaster()->GetGuildId() != bot->GetGuildId()))
    {
        botAI->TellMaster("我不在你的公会中！");
        return false;
    }

    GuidVector gos = *botAI->GetAiObjectContext()->GetValue<GuidVector >("nearest game objects");
    for (GuidVector::iterator i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (!go || !bot->GetGameObjectIfCanInteractWith(go->GetGUID(), GAMEOBJECT_TYPE_GUILD_BANK))
            continue;

        return Execute(text, go);
    }

    botAI->TellMaster("找不到附近的公会银行");
    return false;
}

bool GuildBankAction::Execute(std::string const text, GameObject* bank)
{
    bool result = true;

    std::vector<Item*> found = parseItems(text);
    if (found.empty())
        return false;

    for (std::vector<Item*>::iterator i = found.begin(); i != found.end(); i++)
    {
        Item* item = *i;
        if (item)
            result &= MoveFromCharToBank(item, bank);
    }

    return result;
}

bool GuildBankAction::MoveFromCharToBank(Item* item, GameObject* bank)
{
    uint32 playerSlot = item->GetSlot();
    uint32 playerBag = item->GetBagSlot();

    std::ostringstream out;

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    //guild->SwapItems(bot, 0, playerSlot, 0, INVENTORY_SLOT_BAG_0, 0);

    // check source pos rights (item moved to bank)
    if (!guild->MemberHasTabRights(bot->GetGUID(), 0, GUILD_BANK_RIGHT_DEPOSIT_ITEM))
        out << "我不能把 " << chat->FormatItem(item->GetTemplate()) << " 放到公会银行。我没有权利将物品放入公会银行第一个标签";
    else
    {
        out << chat->FormatItem(item->GetTemplate()) << " 放入公会银行";
        guild->SwapItemsWithInventory(bot, false, 0, 255, playerBag, playerSlot, 0);
    }

    botAI->TellMaster(out);

    return true;
}

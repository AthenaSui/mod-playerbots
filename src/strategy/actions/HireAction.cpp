/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HireAction.h"
#include "Event.h"
#include "Playerbots.h"

bool HireAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    uint32 account = master->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.Query("SELECT COUNT(*) FROM characters WHERE account = {}", account);

    uint32 charCount = 10;
    if (results)
    {
        Field* fields = results->Fetch();
        charCount = uint32(fields[0].Get<uint64>());
    }

    if (charCount >= 10)
    {
        botAI->TellMaster("你已拥有最大角色数量");
        return false;
    }

    if (bot->getLevel() > master->getLevel())
    {
        botAI->TellMaster("你不能雇佣比你等级还高的角色");
        return false;
    }

    uint32 discount = sRandomPlayerbotMgr->GetTradeDiscount(bot, master);
    uint32 m = 1 + (bot->getLevel() / 10);
    uint32 moneyReq = m * 5000 * bot->getLevel();
    if (discount < moneyReq)
    {
        std::ostringstream out;
        out << "你不能雇用我，我几乎不了解你。确保你至少有 " << chat->formatMoney(moneyReq) << " 才行";
        botAI->TellMaster(out.str());
        return false;
    }

    botAI->TellMaster("我会在你下次登录时加入你");

    bot->SetMoney(moneyReq);
    sRandomPlayerbotMgr->Remove(bot);
    CharacterDatabase.Execute("UPDATE characters SET account = {} WHERE guid = {}", account, bot->GetGUID().GetCounter());

    return true;
}

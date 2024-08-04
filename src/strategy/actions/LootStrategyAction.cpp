/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LootStrategyAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "LootStrategyValue.h"
#include "Playerbots.h"

bool LootStrategyAction::Execute(Event event)
{
    std::string const strategy = event.getParam();

    LootObjectStack* lootItems = AI_VALUE(LootObjectStack*, "available loot");
    std::set<uint32>& alwaysLootItems = AI_VALUE(std::set<uint32>&, "always loot list");
    Value<LootStrategy*>* lootStrategy = context->GetValue<LootStrategy*>("loot strategy");

    if (strategy == "?")
    {
        {
            std::ostringstream out;
            out << "拾取策略：";
            out << lootStrategy->Get()->GetName();
            botAI->TellMaster(out);
        }

        {
            std::ostringstream out;
            out << "总是拾取物品：";

            for (uint32 itemId : alwaysLootItems)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (!proto)
                    continue;

                out << chat->FormatItem(proto);
            }

            botAI->TellMaster(out);
        }
    }
    else
    {
        ItemIds items = chat->parseItems(strategy);

        if (items.size() == 0)
        {
            lootStrategy->Set(LootStrategyValue::instance(strategy));

            std::ostringstream out;
            out << "拾取策略设置为 " << lootStrategy->Get()->GetName();
            botAI->TellMaster(out);
            return true;
        }

        bool remove = strategy.size() > 1 && strategy.substr(0, 1) == "-";
        bool query = strategy.size() > 1 && strategy.substr(0, 1) == "?";
        for (uint32 itemid : items)
        {
            if (query)
            {
                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid))
                {
                    std::ostringstream out;
                    out << (StoreLootAction::IsLootAllowed(itemid, botAI) ? "|cFF000000将拾取 "
                                                                          : "|c00FF0000不再拾取 ")
                        << ChatHelper::FormatItem(proto);
                    botAI->TellMaster(out.str());
                }
            }
            else if (remove)
            {
                std::set<uint32>::iterator j = alwaysLootItems.find(itemid);
                if (j != alwaysLootItems.end())
                    alwaysLootItems.erase(j);

                botAI->TellMaster("物品已从总是拾取战利品列表中删除");
            }
            else
            {
                alwaysLootItems.insert(itemid);
                botAI->TellMaster("物品已添加到总是拾取战利品列表");
            }
        }
    }

    return true;
}

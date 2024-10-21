/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellLosAction.h"
#include <istream>
#include <sstream>

#include "ChatHelper.h"
#include "DBCStores.h"
#include "Event.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "StatsWeightCalculator.h"
#include "World.h"

bool TellLosAction::Execute(Event event)
{
    std::string const param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits("--- 目标 ---", *context->GetValue<GuidVector>("possible targets"));
        ListUnits("--- 目标（全部）---", *context->GetValue<GuidVector>("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits("--- NPCs ---", *context->GetValue<GuidVector>("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits("--- 尸体 ---", *context->GetValue<GuidVector>("nearest corpses"));
    }

    if (param.empty() || param == "gos" || param == "game objects")
    {
        ListGameObjects("--- 游戏物品 ---", *context->GetValue<GuidVector>("nearest game objects"));
    }

    if (param.empty() || param == "players")
    {
        ListUnits("--- 友方玩家 ---", *context->GetValue<GuidVector>("nearest friendly players"));
    }

    if (param.empty() || param == "triggers")
    {
        ListUnits("--- 触发器 ---", *context->GetValue<GuidVector>("possible triggers"));
    }

    return true;
}

void TellLosAction::ListUnits(std::string const title, GuidVector units)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : units)
    {
        if (Unit* unit = botAI->GetUnit(guid))
        {
            botAI->TellMaster(unit->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()));
        }
    }
}
void TellLosAction::ListGameObjects(std::string const title, GuidVector gos)
{
    botAI->TellMaster(title);

    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            botAI->TellMaster(chat->FormatGameobject(go));
    }
}

bool TellAuraAction::Execute(Event event)
{
    botAI->TellMaster("--- 光环 ---");
    sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "--- 光环 ---");
    Unit::AuraApplicationMap& map = bot->GetAppliedAuras();
    for (Unit::AuraApplicationMap::iterator i = map.begin(); i != map.end(); ++i)
    {
        Aura* aura = i->second->GetBase();
        if (!aura)
            continue;
        const std::string auraName = aura->GetSpellInfo()->SpellName[0];
        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG, "光环信息 - 名字：" + auraName);
        AuraObjectType type = aura->GetType();
        WorldObject* owner = aura->GetOwner();
        std::string owner_name = owner ? owner->GetName() : "未知";
        float distance = bot->GetDistance2d(owner);
        Unit* caster = aura->GetCaster();
        std::string caster_name = caster ? caster->GetName() : "未知";
        bool is_area = aura->IsArea();
        int32 duration = aura->GetDuration();
        const SpellInfo* spellInfo = aura->GetSpellInfo();
        int32 spellId = aura->GetSpellInfo()->Id;
        bool isPositive = aura->GetSpellInfo()->IsPositive();
        sLog->outMessage("playerbot", LOG_LEVEL_DEBUG,
                         "光环信息 - 名字：" + auraName + " 施法者：" + caster_name + " 类型：" +
                             std::to_string(type) + " 拥有者：" + owner_name + " 距离：" + std::to_string(distance) +
                             " 区域法术：" + std::to_string(is_area) + " 持续时间：" + std::to_string(duration) +
                             " 法术Id：" + std::to_string(spellId) + " 增益：" + std::to_string(isPositive));

        botAI->TellMaster("光环信息 - 名字：" + auraName + " 施法者：" + caster_name + " 类型：" +
                          std::to_string(type) + " 拥有者：" + owner_name + " 距离：" + std::to_string(distance) +
                          " 区域法术：" + std::to_string(is_area) + " 持续时间：" + std::to_string(duration) +
                          " 法术Id：" + std::to_string(spellId) + " 增益：" + std::to_string(isPositive));

        if (type == DYNOBJ_AURA_TYPE)
        {
            DynamicObject* dyn_owner = aura->GetDynobjOwner();
            float radius = dyn_owner->GetRadius();
            int32 spellId = dyn_owner->GetSpellId();
            int32 duration = dyn_owner->GetDuration();
            sLog->outMessage("playerbot", LOG_LEVEL_DEBUG,
                             std::string("动态物品信息 -") + " 名字：" + dyn_owner->GetName() +
                                 " 半径：" + std::to_string(radius) + " 法术id：" + std::to_string(spellId) +
                                 " 持续时间：" + std::to_string(duration));

            botAI->TellMaster(std::string("动态物品信息 -") + " 名字：s" + dyn_owner->GetName() +
                              " 半径：" + std::to_string(radius) + " 法术id：" + std::to_string(spellId) +
                              " 持续时间：" + std::to_string(duration));
        }
    }
    return true;
}

bool TellEstimatedDpsAction::Execute(Event event)
{
    float dps = AI_VALUE(float, "estimated group dps");
    botAI->TellMaster("预期队伍DPS：" + std::to_string(dps));
    return true;
}

bool TellCalculateItemAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ItemIds ids = chat->parseItems(text);
    StatsWeightCalculator calculator(bot);
    for (const uint32 &id : ids)
    {
        const ItemTemplate* proto = sObjectMgr->GetItemTemplate(id);
        if (!proto)
            continue;
        float score = calculator.CalculateItem(id);
        std::ostringstream out;
        out << chat->FormatItem(proto) << "计算得分：" << score;
        botAI->TellMasterNoFacing(out.str());
    }
    return true;
}
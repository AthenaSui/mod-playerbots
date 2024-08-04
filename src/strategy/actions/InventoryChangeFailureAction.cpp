/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InventoryChangeFailureAction.h"

#include "Event.h"
#include "Playerbots.h"

std::map<InventoryResult, std::string> InventoryChangeFailureAction::messages;

bool InventoryChangeFailureAction::Execute(Event event)
{
    if (!botAI->GetMaster())
        return false;

    if (messages.empty())
    {
        messages[EQUIP_ERR_CANT_EQUIP_LEVEL_I] = "我等级太低了";
        messages[EQUIP_ERR_CANT_EQUIP_SKILL] = "我的技能等级太低了";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT] = "无效的插槽";
        messages[EQUIP_ERR_BAG_FULL] = "我的背包满了";
        messages[EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG] = "这个背包是空的";
        messages[EQUIP_ERR_CANT_TRADE_EQUIP_BAGS] = "无法交易已装备的背包";
        messages[EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE] = "无效的插槽（只需要弹药）";
        messages[EQUIP_ERR_NO_REQUIRED_PROFICIENCY] = "我没有必要的技能";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE] = "没有可用的装备插槽";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM] = "我将无法再使用这个";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2] = "我将无法再使用这个";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE2] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED] = "无法在装备双手武器装备";
        messages[EQUIP_ERR_CANT_DUAL_WIELD] = "我无法双武器";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG] = "这件装备无法放入这个背包";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2] = "这件装备无法放入这个背包";
        messages[EQUIP_ERR_CANT_CARRY_MORE_OF_THIS] = "我不能拿更多了";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE3] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_ITEM_CANT_STACK] = "物品无法堆叠";
        messages[EQUIP_ERR_ITEM_CANT_BE_EQUIPPED] = "物品无法装备";
        messages[EQUIP_ERR_ITEMS_CANT_BE_SWAPPED] = "无法交换这些物品";
        messages[EQUIP_ERR_SLOT_IS_EMPTY] = "没有装备";
        messages[EQUIP_ERR_ITEM_NOT_FOUND] = "无法找到装备";
        messages[EQUIP_ERR_CANT_DROP_SOULBOUND] = "无法丢弃灵魂绑定的物品";
        messages[EQUIP_ERR_OUT_OF_RANGE] = "我超出了范围";
        messages[EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT] = "无效的分裂数字";
        messages[EQUIP_ERR_COULDNT_SPLIT_ITEMS] = "Cannot split this";
        messages[EQUIP_ERR_MISSING_REAGENT] = "缺少试剂";
        messages[EQUIP_ERR_NOT_ENOUGH_MONEY] = "钱不够";
        messages[EQUIP_ERR_NOT_A_BAG] = "这不是一个背包";
        messages[EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS] = "背包是空的";
        messages[EQUIP_ERR_DONT_OWN_THAT_ITEM] = "这不是我的物品";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER] = "只能装备箭袋";
        messages[EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT] = "在此之前我必须购买位置";
        messages[EQUIP_ERR_TOO_FAR_AWAY_FROM_BANK] = "我离银行太远了";
        messages[EQUIP_ERR_ITEM_LOCKED] = "物品已锁定";
        messages[EQUIP_ERR_YOU_ARE_STUNNED] = "我惊呆了";
        messages[EQUIP_ERR_YOU_ARE_DEAD] = "我死了";
        messages[EQUIP_ERR_CANT_DO_RIGHT_NOW] = "我现在不能这么做";
        messages[EQUIP_ERR_INT_BAG_ERROR] = "内部错误";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_BOLT] = "只有机器人被允许";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH] = "允许偷猎弹药";
        messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED] = "物品无法被包装";
        messages[EQUIP_ERR_EQUIPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_WRAPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BOUND_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_UNIQUE_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BAGS_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_ALREADY_LOOTED] = "已拾取";
        messages[EQUIP_ERR_INVENTORY_FULL] = "我的库存已满";
        messages[EQUIP_ERR_BANK_FULL] = "我的银行满了";
        messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT] = "物品已售完";
        messages[EQUIP_ERR_BAG_FULL3] = messages[EQUIP_ERR_BANK_FULL];
        messages[EQUIP_ERR_ITEM_NOT_FOUND2] = messages[EQUIP_ERR_ITEM_NOT_FOUND];
        messages[EQUIP_ERR_ITEM_CANT_STACK2] = messages[EQUIP_ERR_ITEM_CANT_STACK];
        messages[EQUIP_ERR_BAG_FULL4] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_ITEM_SOLD_OUT] = messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT];
        messages[EQUIP_ERR_OBJECT_IS_BUSY] = "物品繁忙";
        messages[EQUIP_ERR_NOT_IN_COMBAT] = "我不在战斗中";
        messages[EQUIP_ERR_NOT_WHILE_DISARMED] = "被缴械时无法执行";
        messages[EQUIP_ERR_BAG_FULL6] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_CANT_EQUIP_RANK] = "等级不足";
        messages[EQUIP_ERR_CANT_EQUIP_REPUTATION] = "声望不足";
        messages[EQUIP_ERR_TOO_MANY_SPECIAL_BAGS] = "太多特殊背包了";
        messages[EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW] = "现在不能拾取这个";
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 err;
    p >> err;
    if (err == EQUIP_ERR_OK)
        return false;

    std::string const msg = messages[(InventoryResult)err];
    if (!msg.empty())
    {
        botAI->TellError(msg);
        return true;
    }

    return false;
}

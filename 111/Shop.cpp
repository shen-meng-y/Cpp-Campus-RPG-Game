#include "Shop.h"
#include <iostream>
#include <iomanip>

Shop::Shop() {
    goods.push_back(Item("面包", ItemType::Food, 20, 20, "恢复 20 点生命值"));
    goods.push_back(Item("牛奶", ItemType::Food, 30, 30, "恢复 30 点生命值"));
    goods.push_back(Item("急救药水", ItemType::Medicine, 60, 60, "恢复 60 点生命值"));
    goods.push_back(Item("木剑", ItemType::Equipment, 80, 8, "攻击力增加 8 点"));
    goods.push_back(Item("学习宝典", ItemType::Equipment, 120, 12, "攻击力增加 12 点"));
}

void Shop::showGoods() const {
    std::cout << "\n========== 商店商品 ==========\n";

    for (int i = 0; i < static_cast<int>(goods.size()); i++) {
        std::cout << std::setw(2) << i + 1 << ". "
                  << goods[i].getName()
                  << " | 类型：" << Item::typeToString(goods[i].getType())
                  << " | 价格：" << goods[i].getPrice()
                  << " | 效果值：" << goods[i].getEffectValue()
                  << " | 描述：" << goods[i].getDescription()
                  << std::endl;
    }
}

bool Shop::buyItem(int index, Character& player, std::vector<Item>& backpack) const {
    if (index < 0 || index >= static_cast<int>(goods.size())) {
        std::cout << "商品编号无效。\n";
        return false;
    }

    const Item& item = goods[index];

    if (!player.spendGold(item.getPrice())) {
        std::cout << "金币不足，购买失败。\n";
        return false;
    }

    backpack.push_back(item);

    std::cout << "购买成功：" << item.getName() << std::endl;
    return true;
}

bool Shop::sellItem(int index, Character& player, std::vector<Item>& backpack) const {
    if (backpack.empty()) {
        std::cout << "背包为空，无法出售物品。\n";
        return false;
    }

    if (index < 0 || index >= static_cast<int>(backpack.size())) {
        std::cout << "物品编号无效。\n";
        return false;
    }

    Item item = backpack[index];
    int sellPrice = item.getPrice() / 2;

    backpack.erase(backpack.begin() + index);
    player.addGold(sellPrice);

    std::cout << "出售成功：" << item.getName()
              << "，获得金币 " << sellPrice
              << "。\n";

    return true;
}
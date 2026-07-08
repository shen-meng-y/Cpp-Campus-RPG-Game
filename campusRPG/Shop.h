#ifndef SHOP_H
#define SHOP_H

#include <vector>
#include "Item.h"
#include "Character.h"

class Shop {
private:
    std::vector<Item> goods;

public:
    Shop();

    void showGoods() const;
    const std::vector<Item>& getGoods() const;

    bool buyItem(int index, Character& player, std::vector<Item>& backpack) const;
    bool sellItem(int index, Character& player, std::vector<Item>& backpack) const;
};

#endif
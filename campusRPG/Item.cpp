#include "Item.h"

Item::Item()
    : name("未知物品"),
    type(ItemType::Food),
    price(0),
    effectValue(0),
    description("无") {}

Item::Item(const std::string& name, ItemType type, int price, int effectValue, const std::string& description)
    : name(name),
    type(type),
    price(price),
    effectValue(effectValue),
    description(description) {}

std::string Item::getName() const {
    return name;
}

ItemType Item::getType() const {
    return type;
}

int Item::getPrice() const {
    return price;
}

int Item::getEffectValue() const {
    return effectValue;
}

std::string Item::getDescription() const {
    return description;
}

std::string Item::typeToString(ItemType type) {
    switch (type) {
    case ItemType::Food:
        return "食物";
    case ItemType::Medicine:
        return "药品";
    case ItemType::Equipment:
        return "装备";
    default:
        return "未知";
    }
}

ItemType Item::stringToType(const std::string& text) {
    if (text == "食物") {
        return ItemType::Food;
    }
    if (text == "药品") {
        return ItemType::Medicine;
    }
    if (text == "装备") {
        return ItemType::Equipment;
    }
    return ItemType::Food;
}
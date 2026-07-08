#ifndef ITEM_H
#define ITEM_H

#include <string>

enum class ItemType {
    Food,
    Medicine,
    Equipment
};

class Item {
private:
    std::string name;
    ItemType type;
    int price;
    int effectValue;
    std::string description;

public:
    Item();
    Item(const std::string& name, ItemType type, int price, int effectValue, const std::string& description);

    std::string getName() const;
    ItemType getType() const;
    int getPrice() const;
    int getEffectValue() const;
    std::string getDescription() const;

    static std::string typeToString(ItemType type);
    static ItemType stringToType(const std::string& text);
};

#endif
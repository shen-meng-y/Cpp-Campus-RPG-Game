#include "Character.h"
#include <iostream>
#include <algorithm>

Character::Character()
    : name("未命名"),
    level(1),
    hp(100),
    maxHp(100),
    exp(0),
    gold(100),
    baseAttack(15),
    equipmentAttackBonus(0),
    evolutionStage(EvolutionStage::Primary),
    personStyle(1),
    dressStyle(1),
    hairStyle(1),
    shoeStyle(1){}

Character::Character(const std::string& name)
    : name(name.empty() ? "未命名" : name),
    level(1),
    hp(100),
    maxHp(100),
    exp(0),
    gold(100),
    baseAttack(15),
    equipmentAttackBonus(0),
    evolutionStage(EvolutionStage::Primary),
    personStyle(1),
    dressStyle(1),
    hairStyle(1),
    shoeStyle(1){}

std::string Character::getName() const {
    return name;
}

int Character::getLevel() const {
    return level;
}

int Character::getHp() const {
    return hp;
}

int Character::getMaxHp() const {
    return maxHp;
}

int Character::getExp() const {
    return exp;
}

int Character::getGold() const {
    return gold;
}

int Character::getBaseAttack() const {
    return baseAttack;
}

int Character::getEquipmentAttackBonus() const {
    return equipmentAttackBonus;
}

int Character::getAttack() const {
    return baseAttack + equipmentAttackBonus;
}

int Character::getEvolutionStageValue() const {
    return static_cast<int>(evolutionStage);
}

std::string Character::getEvolutionStageName() const {
    switch (evolutionStage) {
    case EvolutionStage::Primary:
        return "初级形态";
    case EvolutionStage::Middle:
        return "中级形态";
    case EvolutionStage::Ultimate:
        return "终极形态";
    default:
        return "未知形态";
    }
}

bool Character::isAlive() const {
    return hp > 0;
}

bool Character::canEvolve() const {
    if (evolutionStage == EvolutionStage::Primary && level >= 3) {
        return true;
    }

    if (evolutionStage == EvolutionStage::Middle && level >= 5) {
        return true;
    }

    return false;
}

void Character::setName(const std::string& name) {
    this->name = name.empty() ? "未命名" : name;
}

void Character::showInfo() const {
    std::cout << "\n========== 角色信息 ==========\n";
    std::cout << "角色名称：" << name << std::endl;
    std::cout << "当前形态：" << getEvolutionStageName() << std::endl;
    std::cout << "等级：" << level << std::endl;
    std::cout << "生命值：" << hp << "/" << maxHp << std::endl;
    std::cout << "经验值：" << exp << "/" << level * 100 << std::endl;
    std::cout << "金币：" << gold << std::endl;
    std::cout << "攻击力：" << getAttack()
              << "（基础攻击 " << baseAttack
              << " + 装备加成 " << equipmentAttackBonus
              << "）" << std::endl;
}

void Character::showGrowthInfo() const {
    std::cout << "\n========== 成长信息 ==========\n";
    std::cout << "当前等级：" << level << std::endl;
    std::cout << "当前经验：" << exp << "/" << level * 100 << std::endl;
    std::cout << "当前形态：" << getEvolutionStageName() << std::endl;
    std::cout << "最大生命值：" << maxHp << std::endl;
    std::cout << "基础攻击力：" << baseAttack << std::endl;
    std::cout << "总攻击力：" << getAttack() << std::endl;

    if (evolutionStage == EvolutionStage::Primary) {
        std::cout << "下一形态：中级形态，需要等级达到 3 级。\n";
    } else if (evolutionStage == EvolutionStage::Middle) {
        std::cout << "下一形态：终极形态，需要等级达到 5 级。\n";
    } else {
        std::cout << "当前已经是最高形态。\n";
    }

    if (canEvolve()) {
        std::cout << "当前已经满足进化条件，可以选择进化。\n";
    } else {
        std::cout << "当前暂未满足进化条件。\n";
    }
}

void Character::takeDamage(int damage) {
    damage = std::max(0, damage);
    hp -= damage;

    if (hp < 0) {
        hp = 0;
    }
}

void Character::heal(int value) {
    value = std::max(0, value);
    hp += value;

    if (hp > maxHp) {
        hp = maxHp;
    }
}

void Character::addExp(int value) {
    value = std::max(0, value);
    exp += value;

    std::cout << "获得经验值：" << value << std::endl;

    checkLevelUp();
}

void Character::addGold(int value) {
    value = std::max(0, value);
    gold += value;
}

bool Character::spendGold(int value) {
    if (value < 0 || gold < value) {
        return false;
    }

    gold -= value;
    return true;
}

void Character::useItem(const Item& item) {
    switch (item.getType()) {
    case ItemType::Food:
        heal(item.getEffectValue());
        std::cout << "你食用了 " << item.getName()
                  << "，恢复生命值 " << item.getEffectValue()
                  << " 点。\n";
        break;

    case ItemType::Medicine:
        heal(item.getEffectValue());
        std::cout << "你使用了 " << item.getName()
                  << "，恢复生命值 " << item.getEffectValue()
                  << " 点。\n";
        break;

    case ItemType::Equipment:
        equipmentAttackBonus += item.getEffectValue();
        std::cout << "你装备了 " << item.getName()
                  << "，攻击力增加 " << item.getEffectValue()
                  << " 点。\n";
        break;
    }
}

void Character::revive() {
    if (hp <= 0) {
        hp = maxHp / 2;
        std::cout << "角色已恢复至 " << hp << " 点生命值。\n";
    }
}

void Character::train() {
    int gainedExp = 30 + level * 10;

    std::cout << "\n你进行了训练。\n";
    std::cout << "训练完成，获得经验值：" << gainedExp << std::endl;

    addExp(gainedExp);

    std::cout << "训练后的成长结果如下：\n";
    showGrowthInfo();
}

void Character::evolve() {
    if (evolutionStage == EvolutionStage::Ultimate) {
        std::cout << "你已经是终极形态，无法继续进化。\n";
        return;
    }

    if (!canEvolve()) {
        std::cout << "当前等级不足，暂时无法进化。\n";

        if (evolutionStage == EvolutionStage::Primary) {
            std::cout << "从初级形态进化到中级形态需要等级达到 3 级。\n";
        } else if (evolutionStage == EvolutionStage::Middle) {
            std::cout << "从中级形态进化到终极形态需要等级达到 5 级。\n";
        }

        return;
    }

    if (evolutionStage == EvolutionStage::Primary) {
        evolutionStage = EvolutionStage::Middle;
        maxHp += 50;
        baseAttack += 15;
        hp = maxHp;

        std::cout << "\n进化成功！\n";
        std::cout << "初级形态 进化为 中级形态。\n";
        std::cout << "最大生命值 +50，基础攻击力 +15。\n";
    } else if (evolutionStage == EvolutionStage::Middle) {
        evolutionStage = EvolutionStage::Ultimate;
        maxHp += 80;
        baseAttack += 25;
        hp = maxHp;

        std::cout << "\n进化成功！\n";
        std::cout << "中级形态 进化为 终极形态。\n";
        std::cout << "最大生命值 +80，基础攻击力 +25。\n";
    }

    std::cout << "进化后的成长结果如下：\n";
    showGrowthInfo();
}

void Character::loadData(
    const std::string& name,
    int level,
    int hp,
    int maxHp,
    int exp,
    int gold,
    int baseAttack,
    int equipmentAttackBonus,
    int evolutionStageValue,
    int personStyle,
    int dressStyle,
    int hairStyle,
    int shoeStyle
    ) {
    this->name = name.empty() ? "未命名" : name;
    this->level = std::max(1, level);
    this->maxHp = std::max(1, maxHp);
    this->hp = std::max(0, std::min(hp, this->maxHp));
    this->exp = std::max(0, exp);
    this->gold = std::max(0, gold);
    this->baseAttack = std::max(1, baseAttack);
    this->equipmentAttackBonus = std::max(0, equipmentAttackBonus);

    if (evolutionStageValue <= 0) {
        this->evolutionStage = EvolutionStage::Primary;
    } else if (evolutionStageValue == 1) {
        this->evolutionStage = EvolutionStage::Middle;
    } else {
        this->evolutionStage = EvolutionStage::Ultimate;
    }

    setAppearance(personStyle, dressStyle, hairStyle, shoeStyle);
}

void Character::checkLevelUp() {
    while (exp >= level * 100) {
        exp -= level * 100;
        level++;
        maxHp += 20;
        baseAttack += 5;
        hp = maxHp;

        std::cout << "\n========== 等级提升 ==========\n";
        std::cout << "恭喜升级！当前等级：" << level << std::endl;
        std::cout << "属性增长：最大生命值 +20，基础攻击力 +5。\n";
        std::cout << "当前最大生命值：" << maxHp << std::endl;
        std::cout << "当前基础攻击力：" << baseAttack << std::endl;
        std::cout << "当前形态：" << getEvolutionStageName() << std::endl;

        if (canEvolve()) {
            std::cout << "提示：你已经满足进化条件，可以在【成长训练】菜单中选择进化。\n";
        }
    }
}
void Character::setAppearance(int personStyle, int dressStyle, int hairStyle, int shoeStyle) {
    this->personStyle = personStyle;
    this->dressStyle = dressStyle;
    this->hairStyle = hairStyle;
    this->shoeStyle = shoeStyle;
}

int Character::getPersonStyle() const {
    return personStyle;
}

int Character::getDressStyle() const {
    return dressStyle;
}

int Character::getHairStyle() const {
    return hairStyle;
}

int Character::getShoeStyle() const {
    return shoeStyle;
}
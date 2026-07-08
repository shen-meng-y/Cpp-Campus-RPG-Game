#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include "Item.h"

enum class EvolutionStage {
    Primary = 0,   // 初级
    Middle = 1,    // 中级
    Ultimate = 2   // 终极
};

class Character {
private:
    std::string name;
    int level;
    int hp;
    int maxHp;
    int exp;
    int gold;
    int baseAttack;
    int equipmentAttackBonus;
    EvolutionStage evolutionStage;

    void checkLevelUp();

public:
    Character();
    explicit Character(const std::string& name);

    std::string getName() const;
    int getLevel() const;
    int getHp() const;
    int getMaxHp() const;
    int getExp() const;
    int getGold() const;
    int getBaseAttack() const;
    int getEquipmentAttackBonus() const;
    int getAttack() const;
    int getEvolutionStageValue() const;
    std::string getEvolutionStageName() const;

    bool isAlive() const;
    bool canEvolve() const;

    void setName(const std::string& name);
    void showInfo() const;
    void showGrowthInfo() const;

    void takeDamage(int damage);
    void heal(int value);
    void addExp(int value);
    void addGold(int value);
    bool spendGold(int value);
    void useItem(const Item& item);
    void revive();

    void train();
    void evolve();

    void loadData(
        const std::string& name,
        int level,
        int hp,
        int maxHp,
        int exp,
        int gold,
        int baseAttack,
        int equipmentAttackBonus,
        int evolutionStageValue
    );
};

#endif
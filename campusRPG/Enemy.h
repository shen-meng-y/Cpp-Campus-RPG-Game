#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include <vector>

class Enemy {
private:
    std::string name;
    int hp;
    int attack;
    int expReward;
    int goldReward;

public:
    Enemy();
    Enemy(const std::string& name, int hp, int attack, int expReward, int goldReward);

    std::string getName() const;
    int getHp() const;
    int getAttack() const;
    int getExpReward() const;
    int getGoldReward() const;

    bool isAlive() const;
    void takeDamage(int damage);
    void showInfo() const;

    static std::vector<Enemy> createDefaultEnemies();
};

#endif
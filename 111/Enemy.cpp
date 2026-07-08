#include "Enemy.h"
#include <iostream>
#include <algorithm>

Enemy::Enemy()
    : name("普通敌人"),
      hp(50),
      attack(8),
      expReward(20),
      goldReward(10) {}

Enemy::Enemy(const std::string& name, int hp, int attack, int expReward, int goldReward)
    : name(name),
      hp(hp),
      attack(attack),
      expReward(expReward),
      goldReward(goldReward) {}

std::string Enemy::getName() const {
    return name;
}

int Enemy::getHp() const {
    return hp;
}

int Enemy::getAttack() const {
    return attack;
}

int Enemy::getExpReward() const {
    return expReward;
}

int Enemy::getGoldReward() const {
    return goldReward;
}

bool Enemy::isAlive() const {
    return hp > 0;
}

void Enemy::takeDamage(int damage) {
    damage = std::max(0, damage);
    hp -= damage;

    if (hp < 0) {
        hp = 0;
    }
}

void Enemy::showInfo() const {
    std::cout << name
              << " | HP：" << hp
              << " | 攻击力：" << attack
              << " | 经验奖励：" << expReward
              << " | 金币奖励：" << goldReward
              << std::endl;
}

std::vector<Enemy> Enemy::createDefaultEnemies() {
    return {
        Enemy("操场小怪", 45, 8, 30, 15),
        Enemy("图书馆幽灵", 70, 12, 50, 25),
        Enemy("期末Boss", 120, 18, 100, 60)
    };
}
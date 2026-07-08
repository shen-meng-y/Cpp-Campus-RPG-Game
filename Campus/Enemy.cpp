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
    std::vector<Enemy> enemies;

    enemies.push_back(Enemy("800米小怪", 80, 12, 35, 20));
    enemies.push_back(Enemy("大作业幽灵", 140, 22, 70, 45));
    enemies.push_back(Enemy("期末周Boss", 260, 35, 150, 100));

    return enemies;
}
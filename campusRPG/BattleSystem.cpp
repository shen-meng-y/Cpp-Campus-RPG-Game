#include "BattleSystem.h"
#include <iostream>

bool BattleSystem::startBattle(Character& player, const Enemy& enemyTemplate) {
    Enemy enemy = enemyTemplate;

    if (!player.isAlive()) {
        player.revive();
    }

    std::cout << "\n战斗开始！敌人是：" << enemy.getName() << std::endl;

    while (player.isAlive() && enemy.isAlive()) {
        std::cout << "\n你的生命值：" << player.getHp() << "/" << player.getMaxHp()
            << " | 敌人生命值：" << enemy.getHp() << std::endl;

        std::cout << "1. 普通攻击\n";
        std::cout << "2. 逃跑\n";
        std::cout << "请选择：";

        int choice;
        std::cin >> choice;

        if (choice == 2) {
            std::cout << "你选择逃跑，战斗结束。\n";
            return false;
        }

        if (choice != 1) {
            std::cout << "输入无效，默认进行普通攻击。\n";
        }

        enemy.takeDamage(player.getAttack());

        std::cout << "你攻击了 " << enemy.getName()
                  << "，造成 " << player.getAttack()
                  << " 点伤害。\n";

        if (!enemy.isAlive()) {
            std::cout << "战斗胜利！获得经验 "
                      << enemy.getExpReward()
                      << "，金币 "
                      << enemy.getGoldReward()
                      << "。\n";

            player.addExp(enemy.getExpReward());
            player.addGold(enemy.getGoldReward());

            return true;
        }

        player.takeDamage(enemy.getAttack());

        std::cout << enemy.getName()
                  << " 发起反击，造成 "
                  << enemy.getAttack()
                  << " 点伤害。\n";
    }

    if (!player.isAlive()) {
        std::cout << "战斗失败，你被敌人击败了。\n";
        player.revive();
    }

    return false;
}
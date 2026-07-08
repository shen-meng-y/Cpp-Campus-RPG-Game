#include "GameSystem.h"
#include "BattleSystem.h"
#include "FileManager.h"

#include <iostream>
#include <limits>
#include <iomanip>

GameSystem::GameSystem()
    : enemies(Enemy::createDefaultEnemies()),
    quests(Quest::createDefaultQuests()),
    killCount(0),
    running(true),
    saveFileName("save.txt") {
    initMenu();
}

void GameSystem::run() {
    std::cout << "========== 校园 RPG 冒险游戏系统 ==========\n";
    std::cout << "1. 创建新角色\n";
    std::cout << "2. 读取存档\n";
    std::cout << "请选择：";

    int choice = readInt(1, 2);

    if (choice == 1) {
        createCharacter();
    } else {
        loadGame();
    }

    while (running) {
        refreshQuests();
        showMainMenu();

        int menuChoice = readInt(0, 8);

        switch (menuChoice) {
        case 1:
            characterMenu();
            break;

        case 2:
            backpackMenu();
            break;

        case 3:
            shopMenu();
            break;

        case 4:
            questMenu();
            break;

        case 5:
            battleMenu();
            break;

        case 6:
            growthMenu();
            break;

        case 7:
            petHospitalMenu();
            break;

        case 8:
            saveGame();
            break;

        case 0:
            saveGame();
            running = false;
            std::cout << "游戏结束，欢迎下次继续冒险。\n";
            break;
        }
    }
}

void GameSystem::initMenu() {
    menuText[1] = "查看角色信息";
    menuText[2] = "背包管理";
    menuText[3] = "商店系统";
    menuText[4] = "任务系统";
    menuText[5] = "战斗系统";
    menuText[6] = "成长训练";
    menuText[7] = "宠物医院";
    menuText[8] = "保存游戏";
    menuText[0] = "退出游戏";
}

int GameSystem::readInt(int minValue, int maxValue) const {
    int value;

    while (true) {
        if (std::cin >> value && value >= minValue && value <= maxValue) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }

        std::cout << "输入无效，请输入 "
                  << minValue
                  << " 到 "
                  << maxValue
                  << " 之间的整数：";

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void GameSystem::createCharacter() {
    std::string name;

    std::cout << "请输入角色名称：";
    std::getline(std::cin, name);

    player = Character(name);
    backpack.clear();
    quests = Quest::createDefaultQuests();
    killCount = 0;

    backpack.push_back(Item("面包", ItemType::Food, 20, 20, "恢复 20 点生命值"));
    backpack.push_back(Item("小药水", ItemType::Medicine, 40, 40, "恢复 40 点生命值"));

    std::cout << "角色创建成功！初始金币 100，并获得面包和小药水。\n";
}

void GameSystem::showMainMenu() const {
    std::cout << "\n========== 主菜单 ==========\n";
    std::cout << "1. " << menuText.at(1) << std::endl;
    std::cout << "2. " << menuText.at(2) << std::endl;
    std::cout << "3. " << menuText.at(3) << std::endl;
    std::cout << "4. " << menuText.at(4) << std::endl;
    std::cout << "5. " << menuText.at(5) << std::endl;
    std::cout << "6. " << menuText.at(6) << std::endl;
    std::cout << "7. " << menuText.at(7) << std::endl;
    std::cout << "8. " << menuText.at(8) << std::endl;
    std::cout << "0. " << menuText.at(0) << std::endl;
    std::cout << "请选择操作：";
}

void GameSystem::characterMenu() {
    player.showInfo();
    std::cout << "已击败敌人数：" << killCount << std::endl;
}

void GameSystem::backpackMenu() {
    while (true) {
        std::cout << "\n========== 背包管理 ==========\n";
        std::cout << "1. 查看背包\n";
        std::cout << "2. 使用物品\n";
        std::cout << "3. 删除物品\n";
        std::cout << "0. 返回主菜单\n";
        std::cout << "请选择：";

        int choice = readInt(0, 3);

        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            showBackpack();
        } else if (choice == 2) {
            showBackpack();

            if (backpack.empty()) {
                continue;
            }

            std::cout << "请输入要使用的物品编号：";
            int index = readInt(1, static_cast<int>(backpack.size())) - 1;

            player.useItem(backpack[index]);
            backpack.erase(backpack.begin() + index);
        } else if (choice == 3) {
            showBackpack();

            if (backpack.empty()) {
                continue;
            }

            std::cout << "请输入要删除的物品编号：";
            int index = readInt(1, static_cast<int>(backpack.size())) - 1;

            std::cout << "已删除物品：" << backpack[index].getName() << std::endl;
            backpack.erase(backpack.begin() + index);
        }
    }
}

void GameSystem::shopMenu() {
    while (true) {
        std::cout << "\n========== 商店系统 ==========\n";
        std::cout << "当前金币：" << player.getGold() << std::endl;
        std::cout << "1. 查看商品\n";
        std::cout << "2. 购买商品\n";
        std::cout << "3. 出售物品\n";
        std::cout << "0. 返回主菜单\n";
        std::cout << "请选择：";

        int choice = readInt(0, 3);

        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            shop.showGoods();
        } else if (choice == 2) {
            shop.showGoods();

            std::cout << "请输入要购买的商品编号：";
            int index = readInt(1, 5) - 1;

            shop.buyItem(index, player, backpack);
        } else if (choice == 3) {
            showBackpack();

            if (backpack.empty()) {
                continue;
            }

            std::cout << "请输入要出售的物品编号：";
            int index = readInt(1, static_cast<int>(backpack.size())) - 1;

            shop.sellItem(index, player, backpack);
        }
    }
}

void GameSystem::questMenu() {
    while (true) {
        refreshQuests();

        std::cout << "\n========== 任务系统 ==========\n";

        for (int i = 0; i < static_cast<int>(quests.size()); i++) {
            quests[i].showInfo(i + 1);
        }

        std::cout << "1. 接受任务\n";
        std::cout << "2. 领取奖励\n";
        std::cout << "0. 返回主菜单\n";
        std::cout << "请选择：";

        int choice = readInt(0, 2);

        if (choice == 0) {
            return;
        }

        std::cout << "请输入任务编号：";
        int index = readInt(1, static_cast<int>(quests.size())) - 1;

        if (choice == 1) {
            quests[index].accept();
        } else if (choice == 2) {
            quests[index].claimReward(player);
            refreshQuests();
        }
    }
}

void GameSystem::battleMenu() {
    std::cout << "\n========== 战斗系统 ==========\n";

    for (int i = 0; i < static_cast<int>(enemies.size()); i++) {
        std::cout << i + 1 << ". ";
        enemies[i].showInfo();
    }

    std::cout << "0. 返回主菜单\n";
    std::cout << "请选择敌人：";

    int choice = readInt(0, static_cast<int>(enemies.size()));

    if (choice == 0) {
        return;
    }

    bool win = BattleSystem::startBattle(player, enemies[choice - 1]);

    if (win) {
        killCount++;
        refreshQuests();
    }
}

void GameSystem::showBackpack() const {
    std::cout << "\n========== 背包 ==========\n";

    if (backpack.empty()) {
        std::cout << "背包为空。\n";
        return;
    }

    for (int i = 0; i < static_cast<int>(backpack.size()); i++) {
        std::cout << std::setw(2) << i + 1 << ". "
                  << backpack[i].getName()
                  << " | 类型：" << Item::typeToString(backpack[i].getType())
                  << " | 价格：" << backpack[i].getPrice()
                  << " | 效果值：" << backpack[i].getEffectValue()
                  << " | 描述：" << backpack[i].getDescription()
                  << std::endl;
    }
}

void GameSystem::refreshQuests() {
    for (Quest& quest : quests) {
        quest.checkComplete(player, killCount);
    }
}

void GameSystem::saveGame() {
    FileManager::saveGame(saveFileName, player, backpack, quests, killCount);
}

void GameSystem::loadGame() {
    bool success = FileManager::loadGame(saveFileName, player, backpack, quests, killCount);

    if (!success) {
        std::cout << "未找到有效存档，将创建新角色。\n";
        createCharacter();
    }
}
void GameSystem::growthMenu() {
    while (true) {
        std::cout << "\n========== 成长训练 ==========\n";
        std::cout << "1. 查看成长信息\n";
        std::cout << "2. 进行训练\n";
        std::cout << "3. 选择进化\n";
        std::cout << "0. 返回主菜单\n";
        std::cout << "请选择：";

        int choice = readInt(0, 3);

        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            player.showGrowthInfo();
        } else if (choice == 2) {
            player.train();
            refreshQuests();
        } else if (choice == 3) {
            player.evolve();
            refreshQuests();
        }
    }
}
void GameSystem::petHospitalMenu() {
    while (true) {
        std::cout << "\n========== 宠物医院 ==========\n";
        std::cout << "当前生命值：" << player.getHp() << "/" << player.getMaxHp() << std::endl;
        std::cout << "当前金币：" << player.getGold() << std::endl;
        std::cout << "1. 普通治疗：花费 20 金币，恢复 30 点生命值\n";
        std::cout << "2. 高级治疗：花费 50 金币，恢复 80 点生命值\n";
        std::cout << "3. 完全治疗：花费 100 金币，恢复全部生命值\n";
        std::cout << "0. 返回主菜单\n";
        std::cout << "请选择治疗方式：";

        int choice = readInt(0, 3);

        if (choice == 0) {
            return;
        }

        if (player.getHp() >= player.getMaxHp()) {
            std::cout << "当前生命值已满，不需要治疗。\n";
            continue;
        }

        int cost = 0;
        int healValue = 0;
        std::string treatmentName;

        if (choice == 1) {
            cost = 20;
            healValue = 30;
            treatmentName = "普通治疗";
        } else if (choice == 2) {
            cost = 50;
            healValue = 80;
            treatmentName = "高级治疗";
        } else if (choice == 3) {
            cost = 100;
            healValue = player.getMaxHp();
            treatmentName = "完全治疗";
        }

        if (!player.spendGold(cost)) {
            std::cout << "金币不足，无法进行" << treatmentName << "。\n";
            continue;
        }

        int oldHp = player.getHp();
        player.heal(healValue);
        int realHeal = player.getHp() - oldHp;

        std::cout << treatmentName << "成功！\n";
        std::cout << "花费金币：" << cost << std::endl;
        std::cout << "实际恢复生命值：" << realHeal << std::endl;
        std::cout << "当前生命值：" << player.getHp() << "/" << player.getMaxHp() << std::endl;
        std::cout << "剩余金币：" << player.getGold() << std::endl;
    }
}
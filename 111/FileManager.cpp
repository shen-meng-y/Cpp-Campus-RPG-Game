#include "FileManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

bool FileManager::saveGame(
    const std::string& fileName,
    const Character& player,
    const std::vector<Item>& backpack,
    const std::vector<Quest>& quests,
    int killCount
) {
    std::ofstream out(fileName);

    if (!out) {
        std::cout << "存档失败：无法打开文件。\n";
        return false;
    }

    out << "PLAYER\n";
    out << player.getName() << "\n";
    out << player.getLevel() << " "
        << player.getHp() << " "
        << player.getMaxHp() << " "
        << player.getExp() << " "
        << player.getGold() << " "
        << player.getBaseAttack() << " "
        << player.getEquipmentAttackBonus() << " "
        << player.getEvolutionStageValue() << "\n";

    out << "KILL\n";
    out << killCount << "\n";

    out << "ITEMS\n";
    out << backpack.size() << "\n";

    for (const Item& item : backpack) {
        out << item.getName() << "|"
            << Item::typeToString(item.getType()) << "|"
            << item.getPrice() << "|"
            << item.getEffectValue() << "|"
            << item.getDescription() << "\n";
    }

    out << "QUESTS\n";
    out << quests.size() << "\n";

    for (const Quest& quest : quests) {
        out << quest.isAccepted() << " "
            << quest.isCompleted() << " "
            << quest.isRewardClaimed() << "\n";
    }

    std::cout << "游戏已保存到文件：" << fileName << std::endl;
    return true;
}

bool FileManager::loadGame(
    const std::string& fileName,
    Character& player,
    std::vector<Item>& backpack,
    std::vector<Quest>& quests,
    int& killCount
) {
    std::ifstream in(fileName);

    if (!in) {
        return false;
    }

    std::string tag;
    std::getline(in, tag);

    if (tag != "PLAYER") {
        std::cout << "存档格式错误。\n";
        return false;
    }

    std::string name;
    std::getline(in, name);

    std::string playerDataLine;
    std::getline(in, playerDataLine);

    std::stringstream playerData(playerDataLine);

    int level = 1;
    int hp = 100;
    int maxHp = 100;
    int exp = 0;
    int gold = 100;
    int baseAttack = 15;
    int equipmentAttackBonus = 0;
    int evolutionStageValue = 0;

    playerData >> level
               >> hp
               >> maxHp
               >> exp
               >> gold
               >> baseAttack
               >> equipmentAttackBonus;

    if (!(playerData >> evolutionStageValue)) {
        evolutionStageValue = 0;
    }

    player.loadData(
        name,
        level,
        hp,
        maxHp,
        exp,
        gold,
        baseAttack,
        equipmentAttackBonus,
        evolutionStageValue
    );

    std::getline(in, tag);

    if (tag != "KILL") {
        std::cout << "存档格式错误。\n";
        return false;
    }

    in >> killCount;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::getline(in, tag);

    if (tag != "ITEMS") {
        std::cout << "存档格式错误。\n";
        return false;
    }

    int itemCount;
    in >> itemCount;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    backpack.clear();

    for (int i = 0; i < itemCount; i++) {
        std::string line;
        std::getline(in, line);

        std::stringstream ss(line);

        std::string itemName;
        std::string itemTypeText;
        std::string priceText;
        std::string effectText;
        std::string description;

        std::getline(ss, itemName, '|');
        std::getline(ss, itemTypeText, '|');
        std::getline(ss, priceText, '|');
        std::getline(ss, effectText, '|');
        std::getline(ss, description);

        backpack.push_back(
            Item(
                itemName,
                Item::stringToType(itemTypeText),
                std::stoi(priceText),
                std::stoi(effectText),
                description
            )
        );
    }

    std::getline(in, tag);

    if (tag != "QUESTS") {
        std::cout << "存档格式错误。\n";
        return false;
    }

    int questCount;
    in >> questCount;

    quests = Quest::createDefaultQuests();

    for (int i = 0; i < questCount && i < static_cast<int>(quests.size()); i++) {
        bool accepted;
        bool completed;
        bool rewardClaimed;

        in >> accepted >> completed >> rewardClaimed;

        quests[i].setStatus(accepted, completed, rewardClaimed);
    }

    std::cout << "存档读取成功，欢迎回来：" << player.getName() << std::endl;
    return true;
}
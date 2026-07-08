#ifndef GAMESYSTEM_H
#define GAMESYSTEM_H

#include <vector>
#include <map>
#include <string>
#include "Character.h"
#include "Item.h"
#include "Enemy.h"
#include "Shop.h"
#include "Quest.h"

class GameSystem {
private:
    Character player;
    std::vector<Item> backpack;
    std::vector<Enemy> enemies;
    std::vector<Quest> quests;
    Shop shop;
    std::map<int, std::string> menuText;

    int killCount;
    bool running;
    std::string saveFileName;

    void initMenu();
    int readInt(int minValue, int maxValue) const;

    void createCharacter();
    void showMainMenu() const;

    void characterMenu();
    void backpackMenu();
    void shopMenu();
    void questMenu();
    void battleMenu();
    void growthMenu();
    void petHospitalMenu();
    
    void showBackpack() const;
    void refreshQuests();

    void saveGame();
    void loadGame();

public:
    GameSystem();
    void run();
};

#endif
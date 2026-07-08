#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>
#include "Character.h"
#include "Item.h"
#include "Quest.h"

class FileManager {
public:
    static bool saveGame(
        const std::string& fileName,
        const Character& player,
        const std::vector<Item>& backpack,
        const std::vector<Quest>& quests,
        int killCount
        );

    static bool loadGame(
        const std::string& fileName,
        Character& player,
        std::vector<Item>& backpack,
        std::vector<Quest>& quests,
        int& killCount
        );
};

#endif
#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include "Character.h"
#include "Enemy.h"

class BattleSystem {
public:
    static bool startBattle(Character& player, const Enemy& enemyTemplate);
};

#endif
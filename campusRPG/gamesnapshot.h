#ifndef GAMESNAPSHOT_H
#define GAMESNAPSHOT_H

#include <QString>
#include <QMetaType>
#include <vector>

#include "character.h"
#include "Item.h"
#include "quest.h"

struct GameSnapshot
{
    QString fileName;

    Character player;
    std::vector<Item> backpack;
    std::vector<Quest> quests;

    int killCount = 0;
};

Q_DECLARE_METATYPE(GameSnapshot)

#endif // GAMESNAPSHOT_H
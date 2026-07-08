#include "autosaveworker.h"
#include "filemanager.h"

AutoSaveWorker::AutoSaveWorker(QObject *parent)
    : QObject(parent)
{
}

void AutoSaveWorker::saveSnapshot(GameSnapshot snapshot)
{
    Character playerCopy = snapshot.player;
    std::vector<Item> backpackCopy = snapshot.backpack;
    std::vector<Quest> questsCopy = snapshot.quests;
    int killCountCopy = snapshot.killCount;

    FileManager fileManager;

    bool success = fileManager.saveGame(
        snapshot.fileName.toStdString(),
        playerCopy,
        backpackCopy,
        questsCopy,
        killCountCopy
        );

    if (success) {
        emit autoSaveFinished(true, "后台自动存档成功");
    } else {
        emit autoSaveFinished(false, "后台自动存档失败");
    }
}
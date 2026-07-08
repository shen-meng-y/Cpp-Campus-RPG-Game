#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
class QPropertyAnimation;
#include "Character.h"
#include "Item.h"
#include "Enemy.h"
#include "Shop.h"
#include "Quest.h"
#include <QThread>
#include <QTimer>
#include "gametimerworker.h"
#include "randomeventworker.h"
#include "gamesnapshot.h"
#include "autosaveworker.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class PageMode {
    None,
    Backpack,
    Shop,
    Quest,
    Battle,
    Growth,
    Hospital
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void requestAutoSave(GameSnapshot snapshot);
    void stopGameTimer();
    void resetGameTimer();
    void stopRandomEvents();
private:
    Ui::MainWindow *ui;

    Character player;
    std::vector<Item> backpack;
    std::vector<Enemy> enemies;
    std::vector<Quest> quests;
    Shop shop;

    int killCount;
    int gameSeconds;
    int dailyStartKillCount;
    int dailyStartLevel;
    bool hasCreatedRole;
    bool battleInProgress;
    bool randomEventBusy;
    QString saveFileName;
    PageMode currentMode;
    QPropertyAnimation* hpAnimation;
    QPropertyAnimation* expAnimation;
    QThread *autoSaveThread = nullptr;
    AutoSaveWorker *autoSaveWorker = nullptr;
    QTimer *autoSaveTimer = nullptr;

    void initAutoSave();
    GameSnapshot createGameSnapshot() const;
    QThread *gameTimerThread = nullptr;
    GameTimerWorker *gameTimerWorker = nullptr;
    QThread *randomEventThread = nullptr;
    RandomEventWorker *randomEventWorker = nullptr;

    void initGameTimer();
    void initRandomEvents();

private:
    void initGameData();
    void initConnections();
    void updateRoleStatus();
    void addLog(const QString& text);
    void refreshQuests();

    void showBackpackList();
    void showShopList();
    void showQuestList();
    void showBattleList();
    void showGrowthList();
    void showHospitalList();
    void startCampusBattle(int enemyIndex);
    void handleAction1();
    void handleAction2();

    void createRole();
    void saveGame();
    void loadGame();
    void initStyle();
private slots:
    void doAutoSave();
    void onAutoSaveFinished(bool success, const QString &message);
    void onGameTimeChanged(int seconds);
    void handleRandomEvent(int eventType, int value, int enemyIndex, const QString &itemName);
};

#endif

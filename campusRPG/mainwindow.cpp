#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "FileManager.h"
#include "AvatarDressDialog.h"
#include "AvatarRenderer.h"
#include "backpackdialog.h"
#include "shopdialog.h"
#include "battlemapdialog.h"
#include "battlescenedialog.h"
#include "questdialog.h"
#include "firstaidcenterdialog.h"
#include "gametimerworker.h"
#include "randomeventworker.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QStringList>
#include <QPushButton>
#include <QList>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    player("昵称"),
    enemies(Enemy::createDefaultEnemies()),
    quests(Quest::createDefaultQuests()),
    killCount(0),
    gameSeconds(0),
    dailyStartKillCount(0),
    dailyStartLevel(1),
    hasCreatedRole(false),
    battleInProgress(false),
    randomEventBusy(false),
    saveFileName("save.txt"),
    currentMode(PageMode::None),
    hpAnimation(nullptr),
    expAnimation(nullptr) {
    ui->setupUi(this);
    initAutoSave();
    initGameTimer();
    initRandomEvents();
    hpAnimation = new QPropertyAnimation(ui->progressHp, "value", this);
    hpAnimation->setDuration(500);
    hpAnimation->setEasingCurve(QEasingCurve::OutCubic);

    expAnimation = new QPropertyAnimation(ui->progressExp, "value", this);
    expAnimation->setDuration(500);
    expAnimation->setEasingCurve(QEasingCurve::OutCubic);
    initStyle();

    initGameData();
    initConnections();
    updateRoleStatus();

    addLog("欢迎进入宠物养成 / RPG 游戏系统。");
}

MainWindow::~MainWindow()
{
    if (autoSaveTimer) {
        autoSaveTimer->stop();
    }

    if (autoSaveThread) {
        autoSaveThread->quit();
        autoSaveThread->wait();
    }

    if (gameTimerThread) {
        emit stopGameTimer();
        gameTimerThread->quit();
        gameTimerThread->wait();
    }

    if (randomEventThread) {
        emit stopRandomEvents();
        randomEventThread->quit();
        randomEventThread->wait();
    }

    delete ui;
}

void MainWindow::initGameData() {
    backpack.clear();

    backpack.push_back(Item("面包", ItemType::Food, 20, 20, "恢复 20 点生命值"));
    backpack.push_back(Item("牛奶", ItemType::Food, 30, 30, "恢复 30 点生命值"));
    backpack.push_back(Item("急救药水", ItemType::Medicine, 60, 60, "恢复 60 点生命值"));
    backpack.push_back(Item("木剑", ItemType::Equipment, 80, 8, "攻击力增加 8 点"));
    backpack.push_back(Item("学习宝典", ItemType::Equipment, 120, 12, "攻击力增加 12 点"));

    refreshQuests();

    ui->btnAction1->setText("选择操作");
    ui->btnAction2->setText("辅助操作");
}

void MainWindow::initConnections() {
    connect(ui->btnCreateRole, &QPushButton::clicked, this, &MainWindow::createRole);

    connect(ui->btnBackpack, &QPushButton::clicked, this, [this]() {
        currentMode = PageMode::Backpack;
        BackpackDialog dialog(backpack, player, this);

        connect(&dialog, &BackpackDialog::backpackChanged, this, [this]() {
            updateRoleStatus();
        });

        connect(&dialog, &BackpackDialog::logRequested, this, [this](const QString &message) {
            addLog(message);
        });

        dialog.exec();
        updateRoleStatus();
    });

    connect(ui->btnShop, &QPushButton::clicked, this, [this]() {
        currentMode = PageMode::Shop;
        showShopList();
    });

    connect(ui->btnQuest, &QPushButton::clicked, this, [this]() {
        currentMode = PageMode::Quest;
        refreshQuests();

        QuestDialog dialog(quests, player, killCount, this);

        connect(&dialog, &QuestDialog::questChanged, this, [this]() {
            refreshQuests();
            updateRoleStatus();
        });

        connect(&dialog, &QuestDialog::logRequested, this, [this](const QString& message) {
            addLog(message);
        });

        dialog.exec();

        refreshQuests();
        updateRoleStatus();
    });

    connect(ui->btnBattle, &QPushButton::clicked, this, [this]() {
        currentMode = PageMode::Battle;

        BattleMapDialog dialog(this);

        connect(&dialog, &BattleMapDialog::attackEnemyRequested,
                this, &MainWindow::startCampusBattle);

        dialog.exec();
    });

    connect(ui->btnGrowth, &QPushButton::clicked, this, [this]() {
        currentMode = PageMode::Growth;
        showGrowthList();
    });

    connect(ui->btnHospital, &QPushButton::clicked, this, [this]() {
        currentMode = PageMode::None;
        addLog("已进入校园急救中心，等待医生诊断。");

        FirstAidCenterDialog dialog(player, this);
        dialog.exec();

        if (dialog.treatmentPerformed()) {
            addLog(dialog.treatmentSummary());
        } else {
            addLog("已离开校园急救中心。");
        }

        updateRoleStatus();
    });

    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveGame);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::loadGame);

    connect(ui->btnAction1, &QPushButton::clicked, this, &MainWindow::handleAction1);
    connect(ui->btnAction2, &QPushButton::clicked, this, &MainWindow::handleAction2);
}

void MainWindow::updateRoleStatus() {
    ui->labelRoleName->setText(QString::fromStdString(player.getName()));
    ui->labelStage->setText(QString::fromStdString(player.getEvolutionStageName()));
    ui->labelLevel->setText(QString::number(player.getLevel()));

    ui->labelHp->setText(
        QString::number(player.getHp()) + " / " + QString::number(player.getMaxHp())
        );

    ui->labelExp->setText(
        QString::number(player.getExp()) + " / " + QString::number(player.getLevel() * 100)
        );
    // =========================
    // HP 血条
    // =========================
    // =========================
    // HP 血条：平滑动画 + 根据血量变色
    // =========================
    ui->progressHp->setMinimum(0);
    ui->progressHp->setMaximum(player.getMaxHp());
    ui->progressHp->setFormat(QString("HP %1 / %2")
                                  .arg(player.getHp())
                                  .arg(player.getMaxHp()));

    int hpPercent = 0;
    if (player.getMaxHp() > 0) {
        hpPercent = player.getHp() * 100 / player.getMaxHp();
    }

    QString hpColor;

    if (hpPercent >= 60) {
        hpColor = "#9bd69b";   // 绿色：健康
    } else if (hpPercent >= 30) {
        hpColor = "#f4c27a";   // 橙色：危险
    } else {
        hpColor = "#f28b82";   // 红色：濒危
    }

    ui->progressHp->setStyleSheet(QString(
                                      "QProgressBar {"
                                      "   border: 2px solid #d8c7f0;"
                                      "   border-radius: 10px;"
                                      "   background-color: #fffafa;"
                                      "   text-align: center;"
                                      "   font-size: 13px;"
                                      "   font-weight: bold;"
                                      "   color: #6b4f8a;"
                                      "}"

                                      "QProgressBar::chunk {"
                                      "   border-radius: 8px;"
                                      "   background-color: %1;"
                                      "}"
                                      ).arg(hpColor));

    hpAnimation->stop();
    hpAnimation->setStartValue(ui->progressHp->value());
    hpAnimation->setEndValue(player.getHp());
    hpAnimation->start();


    // =========================
    // EXP 经验条：平滑动画
    // =========================
    int expMax = player.getLevel() * 100;

    ui->progressExp->setMinimum(0);
    ui->progressExp->setMaximum(expMax);
    ui->progressExp->setFormat(QString("EXP %1 / %2")
                                   .arg(player.getExp())
                                   .arg(expMax));

    expAnimation->stop();
    expAnimation->setStartValue(ui->progressExp->value());
    expAnimation->setEndValue(player.getExp());
    expAnimation->start();
    ui->labelGold->setText(QString::number(player.getGold()));
    ui->labelAttack->setText(QString::number(player.getAttack()));
    ui->labelKillCount->setText(QString::number(killCount));
    QPixmap avatar = AvatarRenderer::render(
        player.getPersonStyle(),
        player.getDressStyle(),
        player.getHairStyle(),
        player.getShoeStyle(),
        ui->labelAvatar->width(),
        ui->labelAvatar->height()
        );

    ui->labelAvatar->setPixmap(avatar);
}

void MainWindow::addLog(const QString& text) {
    ui->textLog->append("◆ " + text);
}

void MainWindow::showSkillUnlockMessage(int stageValue) {
    if (stageValue == 1) {
        QMessageBox::information(
            this,
            "解锁新技能",
            "恭喜！你已经进化为【中级形态】。\n\n"
            "解锁新技能：【知识连击】\n"
            "技能效果：造成 1.5 倍攻击 + 10 点伤害。\n\n"
            "现在进入战斗时，除了【普通攻击】，还可以使用【知识连击】。"
            );

        addLog("解锁新技能：知识连击。");
    } else if (stageValue == 2) {
        QMessageBox::information(
            this,
            "解锁新技能",
            "恭喜！你已经进化为【终极形态】。\n\n"
            "解锁新技能：【期末爆发】\n"
            "技能效果：造成 2 倍攻击 + 30 点伤害。\n\n"
            "现在进入战斗时，可以使用【普通攻击】、【知识连击】和【期末爆发】。"
            );

        addLog("解锁新技能：期末爆发。");
    }
}

void MainWindow::refreshQuests() {
    for (Quest& quest : quests) {
        quest.checkComplete(player, killCount);
    }
}

void MainWindow::createRole() {
    bool ok = false;

    QString name = QInputDialog::getText(
        this,
        "创建角色",
        "请输入角色名称：",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (!ok || name.trimmed().isEmpty()) {
        return;
    }

    AvatarDressDialog dressDialog(this);

    if (dressDialog.exec() != QDialog::Accepted) {
        return;
    }

    player = Character(name.toStdString());
    hasCreatedRole = true;
    dailyStartKillCount = 0;
    dailyStartLevel = player.getLevel();

    player.setAppearance(
        dressDialog.getPersonStyle(),
        dressDialog.getDressStyle(),
        dressDialog.getHairStyle(),
        dressDialog.getShoeStyle()
        );

    backpack.clear();
    quests = Quest::createDefaultQuests();
    enemies = Enemy::createDefaultEnemies();
    killCount = 0;

    backpack.push_back(Item("面包", ItemType::Food, 20, 20, "恢复 20 点生命值"));
    backpack.push_back(Item("牛奶", ItemType::Food, 30, 30, "恢复 30 点生命值"));
    backpack.push_back(Item("急救药水", ItemType::Medicine, 60, 60, "恢复 60 点生命值"));
    backpack.push_back(Item("木剑", ItemType::Equipment, 80, 8, "攻击力增加 8 点"));
    backpack.push_back(Item("学习宝典", ItemType::Equipment, 120, 12, "攻击力增加 12 点"));

    currentMode = PageMode::None;
    ui->listWidget->clear();

    updateRoleStatus();

    addLog("创建角色成功：" + name);
    addLog("角色换装完成。");
    addLog("初始获得：面包、牛奶、急救药水、木剑、学习宝典。");
}

void MainWindow::showBackpackList() {
    ui->listWidget->clear();

    ui->btnAction1->setText("使用物品");
    ui->btnAction2->setText("删除物品");

    if (backpack.empty()) {
        ui->listWidget->addItem("背包为空");
        addLog("背包为空。");
        return;
    }

    for (int i = 0; i < static_cast<int>(backpack.size()); i++) {
        const Item& item = backpack[i];

        QString text = QString::number(i + 1) + ". "
                       + QString::fromStdString(item.getName())
                       + " | 类型：" + QString::fromStdString(Item::typeToString(item.getType()))
                       + " | 效果：" + QString::number(item.getEffectValue())
                       + " | 价格：" + QString::number(item.getPrice())
                       + " | 说明：" + QString::fromStdString(item.getDescription());

        ui->listWidget->addItem(text);
    }

    addLog("已打开背包管理。");
}

void MainWindow::showShopList() {
    ui->listWidget->clear();
    ui->listWidget->addItem("商店已升级为商品卡片与购物车窗口。");
    ui->btnAction1->setText("选择操作");
    ui->btnAction2->setText("辅助操作");

    addLog("已打开商品卡片商店。");

    ShopDialog dialog(shop, player, backpack, this);

    connect(&dialog, &ShopDialog::shopChanged, this, [this]() {
        updateRoleStatus();
    });

    connect(&dialog, &ShopDialog::logRequested,
            this, [this](const QString &message) {
                addLog(message);
            });

    dialog.exec();

    updateRoleStatus();
    currentMode = PageMode::None;
    addLog("已离开商店系统。");
}

void MainWindow::showQuestList() {
    refreshQuests();

    ui->listWidget->clear();

    ui->btnAction1->setText("接受任务");
    ui->btnAction2->setText("领取奖励");

    for (int i = 0; i < static_cast<int>(quests.size()); i++) {
        QString status;

        if (!quests[i].isAccepted()) {
            status = "未接受";
        } else if (quests[i].isRewardClaimed()) {
            status = "已领奖";
        } else if (quests[i].isCompleted()) {
            status = "已完成，可领奖";
        } else {
            status = "进行中";
        }

        QString text = QString::number(i + 1) + ". "
                       + QString::fromStdString(quests[i].getName())
                       + " | 状态：" + status;

        ui->listWidget->addItem(text);
    }

    addLog("已打开任务系统。");
}

void MainWindow::showBattleList() {
    ui->listWidget->clear();

    ui->btnAction1->setText("攻击敌人");
    ui->btnAction2->setText("逃跑");

    for (int i = 0; i < static_cast<int>(enemies.size()); i++) {
        const Enemy& enemy = enemies[i];

        QString text = QString::number(i + 1) + ". "
                       + QString::fromStdString(enemy.getName())
                       + " | HP：" + QString::number(enemy.getHp())
                       + " | 攻击：" + QString::number(enemy.getAttack())
                       + " | 经验：" + QString::number(enemy.getExpReward())
                       + " | 金币：" + QString::number(enemy.getGoldReward());

        ui->listWidget->addItem(text);
    }

    addLog("已打开战斗系统。");
}
void MainWindow::startCampusBattle(int enemyIndex) {
    if (enemyIndex < 0 || enemyIndex >= static_cast<int>(enemies.size())) {
        QMessageBox::warning(this, "战斗错误", "怪物数据不存在。");
        return;
    }

    battleInProgress = true;
    Enemy enemy = enemies[enemyIndex];

    QString sceneFileName;
    QString enemyFileName;

    if (enemyIndex == 0) {
        sceneFileName = "playground.png";
        enemyFileName = "lmp.png";
    } else if (enemyIndex == 1) {
        sceneFileName = "library.png";
        enemyFileName = "homework.png";
    } else {
        sceneFileName = "classroom.png";
        enemyFileName = "boss.png";
    }

    QPixmap playerPixmap = AvatarRenderer::render(
        player.getPersonStyle(),
        player.getDressStyle(),
        player.getHairStyle(),
        player.getShoeStyle(),
        220,
        220
        );

    BattleSceneDialog battleDialog(
        player,
        enemy,
        sceneFileName,
        enemyFileName,
        playerPixmap,
        this
        );

    battleDialog.exec();

    if (battleDialog.wasVictory()) {
        addLog("战斗胜利，击败了：" + battleDialog.enemyName());

        addLog("获得经验 "
               + QString::number(battleDialog.expReward())
               + "，金币 "
               + QString::number(battleDialog.goldReward())
               + "。");

        player.addExp(battleDialog.expReward());
        player.addGold(battleDialog.goldReward());
        killCount++;

        refreshQuests();
    } else if (battleDialog.wasDefeated()) {
        addLog("战斗失败，角色被击败。");

        player.revive();

        addLog("角色自动恢复部分生命值。");
    } else if (battleDialog.didEscape()) {
        addLog("你从战斗中逃跑了。");
    }

    updateRoleStatus();
    battleInProgress = false;
}
void MainWindow::showGrowthList() {
    ui->listWidget->clear();

    ui->btnAction1->setText("执行操作");
    ui->btnAction2->setText("查看成长");

    ui->listWidget->addItem("1. 训练：获得经验值");
    ui->listWidget->addItem("2. 进化：达到等级后提升形态并解锁技能");

    addLog("已打开成长训练。");
}

void MainWindow::showHospitalList() {
    ui->listWidget->clear();

    ui->btnAction1->setText("进行治疗");
    ui->btnAction2->setText("取消治疗");

    ui->listWidget->addItem("1. 普通治疗：花费 20 金币，恢复 30 点生命值");
    ui->listWidget->addItem("2. 高级治疗：花费 50 金币，恢复 80 点生命值");
    ui->listWidget->addItem("3. 完全治疗：花费 100 金币，恢复全部生命值");

    addLog("已打开宠物医院。");
}

void MainWindow::handleAction1() {
    int row = ui->listWidget->currentRow();

    if (row < 0) {
        QMessageBox::information(this, "提示", "请先在列表中选择一项。");
        return;
    }

    if (currentMode == PageMode::Backpack) {
        if (row >= static_cast<int>(backpack.size())) {
            return;
        }

        Item item = backpack[row];

        player.useItem(item);
        backpack.erase(backpack.begin() + row);

        addLog("使用物品：" + QString::fromStdString(item.getName()));

        showBackpackList();
    }

    else if (currentMode == PageMode::Shop) {
        const std::vector<Item>& goods = shop.getGoods();

        if (row >= static_cast<int>(goods.size())) {
            return;
        }

        const Item& item = goods[row];

        if (!player.spendGold(item.getPrice())) {
            QMessageBox::warning(this, "购买失败", "金币不足，无法购买该商品。");
            return;
        }

        backpack.push_back(item);

        addLog("购买商品成功：" + QString::fromStdString(item.getName()));

        showShopList();
    }

    else if (currentMode == PageMode::Quest) {
        if (row >= static_cast<int>(quests.size())) {
            return;
        }

        if (quests[row].isAccepted()) {
            QMessageBox::information(this, "任务提示", "该任务已经接受过。");
            return;
        }

        quests[row].accept();

        addLog("接受任务：" + QString::fromStdString(quests[row].getName()));

        showQuestList();
    }

    else if (currentMode == PageMode::Battle) {
        if (row >= static_cast<int>(enemies.size())) {
            return;
        }

        startCampusBattle(row);
    }

    else if (currentMode == PageMode::Growth) {
        if (row == 0) {
            int gainedExp = 30 + player.getLevel() * 10;

            player.addExp(gainedExp);

            addLog("训练完成，获得经验："
                   + QString::number(gainedExp));
        } else if (row == 1) {
            if (!player.canEvolve()) {
                QMessageBox::information(this, "进化失败", "当前等级不足，暂时无法进化。");
            } else {
                int oldStageValue = player.getEvolutionStageValue();
                QString oldStage = QString::fromStdString(player.getEvolutionStageName());

                player.evolve();

                int newStageValue = player.getEvolutionStageValue();
                QString newStage = QString::fromStdString(player.getEvolutionStageName());

                addLog("进化成功：" + oldStage + " → " + newStage);

                if (newStageValue != oldStageValue) {
                    showSkillUnlockMessage(newStageValue);
                }
            }
        }

        refreshQuests();
    }

    else if (currentMode == PageMode::Hospital) {
        int cost = 0;
        int healValue = 0;
        QString treatmentName;

        if (row == 0) {
            cost = 20;
            healValue = 30;
            treatmentName = "普通治疗";
        } else if (row == 1) {
            cost = 50;
            healValue = 80;
            treatmentName = "高级治疗";
        } else if (row == 2) {
            cost = 100;
            healValue = player.getMaxHp();
            treatmentName = "完全治疗";
        } else {
            return;
        }

        if (player.getHp() >= player.getMaxHp()) {
            QMessageBox::information(this, "宠物医院", "当前生命值已满，不需要治疗。");
            return;
        }

        if (!player.spendGold(cost)) {
            QMessageBox::warning(this, "治疗失败", "金币不足，无法治疗。");
            return;
        }

        int oldHp = player.getHp();

        player.heal(healValue);

        int realHeal = player.getHp() - oldHp;

        addLog(treatmentName
               + "成功，花费金币 "
               + QString::number(cost)
               + "，恢复生命值 "
               + QString::number(realHeal)
               + "。");
    }

    updateRoleStatus();
}

void MainWindow::handleAction2() {
    int row = ui->listWidget->currentRow();

    if (currentMode == PageMode::Backpack) {
        if (row < 0 || row >= static_cast<int>(backpack.size())) {
            QMessageBox::information(this, "提示", "请选择要删除的物品。");
            return;
        }

        QString itemName = QString::fromStdString(backpack[row].getName());

        backpack.erase(backpack.begin() + row);

        addLog("删除物品：" + itemName);

        showBackpackList();
    }

    else if (currentMode == PageMode::Shop) {
        if (backpack.empty()) {
            QMessageBox::information(this, "出售失败", "背包为空，无法出售物品。");
            return;
        }

        QStringList itemList;

        for (int i = 0; i < static_cast<int>(backpack.size()); i++) {
            itemList << QString::number(i + 1) + ". "
                            + QString::fromStdString(backpack[i].getName())
                            + " | 售价："
                            + QString::number(backpack[i].getPrice() / 2);
        }

        bool ok = false;

        QString selected = QInputDialog::getItem(
            this,
            "出售物品",
            "请选择要出售的物品：",
            itemList,
            0,
            false,
            &ok
            );

        if (!ok || selected.isEmpty()) {
            return;
        }

        int index = itemList.indexOf(selected);

        if (index < 0 || index >= static_cast<int>(backpack.size())) {
            return;
        }

        Item item = backpack[index];
        int sellPrice = item.getPrice() / 2;

        backpack.erase(backpack.begin() + index);
        player.addGold(sellPrice);

        addLog("出售物品："
               + QString::fromStdString(item.getName())
               + "，获得金币 "
               + QString::number(sellPrice)
               + "。");

        showShopList();
    }

    else if (currentMode == PageMode::Quest) {
        if (row < 0 || row >= static_cast<int>(quests.size())) {
            QMessageBox::information(this, "提示", "请选择任务。");
            return;
        }

        bool beforeClaimed = quests[row].isRewardClaimed();

        quests[row].claimReward(player);

        if (!beforeClaimed && quests[row].isRewardClaimed()) {
            addLog("领取任务奖励成功："
                   + QString::fromStdString(quests[row].getName()));
        } else {
            addLog("尝试领取任务奖励："
                   + QString::fromStdString(quests[row].getName()));
        }

        refreshQuests();
        showQuestList();
    }

    else if (currentMode == PageMode::Battle) {
        addLog("你选择逃跑，离开战斗。");
    }

    else if (currentMode == PageMode::Growth) {
        QString info;

        info += "等级：" + QString::number(player.getLevel()) + "\n";
        info += "经验：" + QString::number(player.getExp())
                + " / "
                + QString::number(player.getLevel() * 100)
                + "\n";
        info += "形态：" + QString::fromStdString(player.getEvolutionStageName()) + "\n";
        info += "最大生命值：" + QString::number(player.getMaxHp()) + "\n";
        info += "基础攻击力：" + QString::number(player.getBaseAttack()) + "\n";
        info += "总攻击力：" + QString::number(player.getAttack()) + "\n";

        info += "\n当前战斗技能：\n";
        info += "普通攻击：造成当前总攻击力伤害。\n";

        if (player.getEvolutionStageValue() >= 1) {
            info += "知识连击：造成 1.5 倍攻击 + 10 点伤害。\n";
        } else {
            info += "知识连击：中级形态解锁。\n";
        }

        if (player.getEvolutionStageValue() >= 2) {
            info += "期末爆发：造成 2 倍攻击 + 30 点伤害。\n";
        } else {
            info += "期末爆发：终极形态解锁。\n";
        }

        info += "\n";

        if (player.canEvolve()) {
            info += "当前可以进化。";
        } else {
            info += "当前暂时不能进化。";
        }

        QMessageBox::information(this, "成长信息", info);
    }

    else if (currentMode == PageMode::Hospital) {
        addLog("已取消治疗。");
    }

    updateRoleStatus();
}

void MainWindow::saveGame() {
    bool success = FileManager::saveGame(
        saveFileName.toStdString(),
        player,
        backpack,
        quests,
        killCount
        );

    if (success) {
        addLog("保存成功。");
    } else {
        QMessageBox::warning(this, "保存失败", "无法保存游戏。");
    }
}

void MainWindow::loadGame() {
    QStringList saveTypes;
    saveTypes << "读取手动存档" << "读取自动存档";

    bool ok = false;
    QString selectedType = QInputDialog::getItem(
        this,
        "读取存档",
        "请选择要读取的存档：",
        saveTypes,
        0,
        false,
        &ok
        );

    if (!ok) {
        return;
    }

    QString fileName;

    if (selectedType == "读取自动存档") {
        fileName = "autosave.txt";
    } else {
        fileName = saveFileName;
    }

    bool success = FileManager::loadGame(
        fileName.toStdString(),
        player,
        backpack,
        quests,
        killCount
        );

    if (success) {
        hasCreatedRole = true;
        dailyStartKillCount = killCount;
        dailyStartLevel = player.getLevel();

        updateRoleStatus();
        refreshQuests();

        currentMode = PageMode::None;
        ui->listWidget->clear();

        addLog(selectedType + "成功。");
    } else {
        QMessageBox::warning(this, "读取失败", "没有找到有效存档：" + fileName);
    }
}
void MainWindow::initStyle() {
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();

    for (QPushButton* button : buttons) {
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(40);
    }

    this->setStyleSheet(
        // =========================
        // 主窗口整体字体
        // =========================
        "QWidget {"
        "   font-family: 'Microsoft YaHei';"
        "}"

        // =========================
        // 普通按钮：淡蓝白风格
        // =========================
        "QPushButton {"
        "   font-size: 17px;"
        "   font-weight: 600;"
        "   color: #3f5f7f;"
        "   background-color: rgba(248, 252, 255, 235);"
        "   border: 2px solid #b7d3ee;"
        "   border-radius: 14px;"
        "   padding: 7px 16px;"
        "}"

        "QPushButton:hover {"
        "   color: #2f5f93;"
        "   background-color: rgba(230, 244, 255, 245);"
        "   border: 2px solid #8fbce6;"
        "}"

        "QPushButton:pressed {"
        "   color: #ffffff;"
        "   background-color: #9fc5e8;"
        "   border: 2px solid #7fa9d6;"
        "   padding-top: 9px;"
        "   padding-bottom: 5px;"
        "}"

        "QPushButton:disabled {"
        "   color: #999999;"
        "   background-color: #eeeeee;"
        "   border: 2px solid #cccccc;"
        "}"

        // =========================
        // 创建角色按钮：淡粉橘，稍微突出但不刺眼
        // =========================
        "#btnCreateRole {"
        "   color: #8a5a44;"
        "   background-color: rgba(255, 244, 232, 245);"
        "   border: 2px solid #e8bfa5;"
        "}"

        "#btnCreateRole:hover {"
        "   color: #7a4630;"
        "   background-color: rgba(255, 232, 211, 250);"
        "   border: 2px solid #dda987;"
        "}"

        "#btnCreateRole:pressed {"
        "   color: #ffffff;"
        "   background-color: #d9a07c;"
        "   border: 2px solid #c98c68;"
        "}"

        // =========================
        // 保存 / 读取按钮：淡绿色
        // =========================
        "#btnSave, #btnLoad {"
        "   color: #4d6f55;"
        "   background-color: rgba(240, 250, 242, 245);"
        "   border: 2px solid #b7d9bf;"
        "}"

        "#btnSave:hover, #btnLoad:hover {"
        "   color: #3f6849;"
        "   background-color: rgba(226, 245, 230, 250);"
        "   border: 2px solid #9dcca9;"
        "}"

        "#btnSave:pressed, #btnLoad:pressed {"
        "   color: #ffffff;"
        "   background-color: #8fbd99;"
        "   border: 2px solid #78a982;"
        "}"

        // =========================
        // 右侧主要操作按钮：淡蓝
        // =========================
        "#btnAction1 {"
        "   font-size: 16px;"
        "   color: #3f5f7f;"
        "   background-color: rgba(240, 248, 255, 245);"
        "   border: 2px solid #a8c9e8;"
        "   border-radius: 13px;"
        "}"

        "#btnAction1:hover {"
        "   background-color: rgba(224, 240, 255, 250);"
        "   border: 2px solid #86b7e2;"
        "}"

        "#btnAction1:pressed {"
        "   color: white;"
        "   background-color: #8fbce6;"
        "}"

        // =========================
        // 右侧次要操作按钮：淡灰紫，不用红色
        // =========================
        "#btnAction2 {"
        "   font-size: 16px;"
        "   color: #665f7f;"
        "   background-color: rgba(247, 245, 252, 245);"
        "   border: 2px solid #c8bfdc;"
        "   border-radius: 13px;"
        "}"

        "#btnAction2:hover {"
        "   background-color: rgba(238, 233, 248, 250);"
        "   border: 2px solid #b6a9d0;"
        "}"

        "#btnAction2:pressed {"
        "   color: white;"
        "   background-color: #aaa0c8;"
        "}"

        // =========================
        // 列表区域：顺便柔化一下
        // =========================
        "QListWidget {"
        "   font-size: 16px;"
        "   color: #3f5f7f;"
        "   background-color: rgba(255, 255, 255, 225);"
        "   border: 2px solid #c6dcef;"
        "   border-radius: 14px;"
        "   padding: 8px;"
        "}"

        "QListWidget::item {"
        "   padding: 8px;"
        "   border-radius: 8px;"
        "}"

        "QListWidget::item:selected {"
        "   color: #2f5f93;"
        "   background-color: rgba(218, 236, 252, 230);"
        "}"

        // =========================
        // 日志框
        // =========================
        "QTextEdit {"
        "   font-size: 15px;"
        "   color: #3f5f7f;"
        "   background-color: rgba(255, 255, 255, 225);"
        "   border: 2px solid #c6dcef;"
        "   border-radius: 14px;"
        "   padding: 8px;"
        "}"
        // =========================
        // 经验条：固定蓝色
        // =========================
        "#progressExp {"
        "   border: 2px solid #b7d3ee;"
        "   border-radius: 10px;"
        "   background-color: #f5fbff;"
        "   text-align: center;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "   color: #3f5f7f;"
        "}"

        "#progressExp::chunk {"
        "   border-radius: 8px;"
        "   background-color: #8fc8f7;"
        "}"
        );
}
void MainWindow::initAutoSave()
{
    qRegisterMetaType<GameSnapshot>("GameSnapshot");

    autoSaveThread = new QThread(this);
    autoSaveWorker = new AutoSaveWorker();

    autoSaveWorker->moveToThread(autoSaveThread);

    connect(autoSaveThread, &QThread::finished,
            autoSaveWorker, &QObject::deleteLater);

    connect(this, &MainWindow::requestAutoSave,
            autoSaveWorker, &AutoSaveWorker::saveSnapshot,
            Qt::QueuedConnection);

    connect(autoSaveWorker, &AutoSaveWorker::autoSaveFinished,
            this, &MainWindow::onAutoSaveFinished,
            Qt::QueuedConnection);

    autoSaveThread->start();

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(30000); // 30秒自动存档一次

    connect(autoSaveTimer, &QTimer::timeout,
            this, &MainWindow::doAutoSave);

    autoSaveTimer->start();

    qDebug() << "后台自动存档线程已启动";
}
void MainWindow::doAutoSave()
{
    GameSnapshot snapshot = createGameSnapshot();
    emit requestAutoSave(snapshot);
}
GameSnapshot MainWindow::createGameSnapshot() const
{
    GameSnapshot snapshot;

    snapshot.fileName = "autosave.txt";

    snapshot.player = player;
    snapshot.backpack = backpack;
    snapshot.quests = quests;
    snapshot.killCount = killCount;

    return snapshot;
}
void MainWindow::onAutoSaveFinished(bool success, const QString &message)
{
    qDebug() << message;

    if (success) {
        ui->textLog->append("【系统】" + message);
    } else {
        ui->textLog->append("【警告】" + message);
    }
}

void MainWindow::initGameTimer()
{
    gameTimerThread = new QThread(this);
    gameTimerWorker = new GameTimerWorker();

    gameTimerWorker->moveToThread(gameTimerThread);

    connect(gameTimerThread, &QThread::finished,
            gameTimerWorker, &QObject::deleteLater);

    connect(gameTimerThread, &QThread::started,
            gameTimerWorker, &GameTimerWorker::startTiming);

    connect(this, &MainWindow::stopGameTimer,
            gameTimerWorker, &GameTimerWorker::stopTiming,
            Qt::QueuedConnection);

    connect(this, &MainWindow::resetGameTimer,
            gameTimerWorker, &GameTimerWorker::resetTiming,
            Qt::QueuedConnection);

    connect(gameTimerWorker, &GameTimerWorker::gameTimeChanged,
            this, &MainWindow::onGameTimeChanged,
            Qt::QueuedConnection);

    gameTimerThread->start();
}

void MainWindow::onGameTimeChanged(int seconds)
{
    gameSeconds = seconds;

    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    QString timeText = QString("游戏时间：%1:%2:%3")
                           .arg(hours, 2, 10, QChar('0'))
                           .arg(minutes, 2, 10, QChar('0'))
                           .arg(secs, 2, 10, QChar('0'));

    ui->labelGameTime->setText(timeText);
}

void MainWindow::initRandomEvents()
{
    randomEventThread = new QThread(this);
    randomEventWorker = new RandomEventWorker();

    randomEventWorker->moveToThread(randomEventThread);

    connect(randomEventThread, &QThread::finished,
            randomEventWorker, &QObject::deleteLater);

    connect(randomEventThread, &QThread::started,
            randomEventWorker, &RandomEventWorker::startEvents);

    connect(this, &MainWindow::stopRandomEvents,
            randomEventWorker, &RandomEventWorker::stopEvents,
            Qt::QueuedConnection);

    connect(randomEventWorker, &RandomEventWorker::randomEventTriggered,
            this, &MainWindow::handleRandomEvent,
            Qt::QueuedConnection);

    randomEventThread->start();
    addLog("定时随机事件系统已启动，每 3 分钟触发一次。");
}

void MainWindow::handleRandomEvent(int eventType, int value, int enemyIndex, const QString &itemName)
{
    if (battleInProgress || randomEventBusy) {
        return;
    }

    randomEventBusy = true;

    if (eventType == 0) {
        player.addGold(value);
        QMessageBox::information(this, "随机事件", "你在路边捡到了 " + QString::number(value) + " 金币。");
        addLog("【随机事件】你捡到了 " + QString::number(value) + " 金币。");
    } else if (eventType == 1) {
        int realLoss = value;

        if (realLoss > player.getGold()) {
            realLoss = player.getGold();
        }

        if (realLoss > 0) {
            player.spendGold(realLoss);
        }

        QMessageBox::information(this, "随机事件", "一阵风吹过，你丢失了 " + QString::number(realLoss) + " 金币。");
        addLog("【随机事件】一阵风吹过，你丢失了 " + QString::number(realLoss) + " 金币。");
    } else if (eventType == 2) {
        backpack.push_back(Item(itemName.toStdString(), ItemType::Medicine, 60, value, "随机事件获得的恢复道具"));
        QMessageBox::information(this, "随机事件", "你在走廊里捡到了一瓶" + itemName + "。");
        addLog("【随机事件】获得物品：" + itemName + "。");
    } else if (eventType == 3) {
        player.addExp(value);
        QMessageBox::information(this, "随机事件", "你突然进入学习状态，获得经验 " + QString::number(value) + "。");
        addLog("【随机事件】灵感爆发，获得经验 " + QString::number(value) + "。");
    } else if (eventType == 4) {
        player.takeDamage(value);
        QMessageBox::information(this, "随机事件", "你被突如其来的小测吓了一跳，生命值减少 " + QString::number(value) + "。");
        addLog("【随机事件】突发小测，生命值减少 " + QString::number(value) + "。");

        if (!player.isAlive()) {
            player.revive();
            addLog("角色生命值过低，已自动恢复部分生命值。");
        }
    } else {
        QMessageBox::information(this, "随机事件", "你在校园里突然遇到了怪物，必须先完成战斗。");
        addLog("【随机事件】校园随机遇敌。");
        startCampusBattle(enemyIndex);
    }

    refreshQuests();
    updateRoleStatus();
    randomEventBusy = false;
}

#include "battlescenedialog.h"

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QCoreApplication>
#include <QCloseEvent>

BattleSceneDialog::BattleSceneDialog(
    Character& player,
    const Enemy& enemy,
    const QString& sceneFileName,
    const QString& enemyFileName,
    const QPixmap& playerPixmap,
    QWidget* parent
    )
    : QDialog(parent),
    player(player),
    enemy(enemy),
    enemyMaxHp(enemy.getHp()),
    sceneFileName(sceneFileName),
    enemyFileName(enemyFileName),
    playerPixmap(playerPixmap),
    victory(false),
    defeated(false),
    escaped(false),
    battleEnded(false),
    sceneLabel(nullptr),
    playerImageLabel(nullptr),
    enemyImageLabel(nullptr),
    playerHpText(nullptr),
    enemyHpText(nullptr),
    playerHpBar(nullptr),
    enemyHpBar(nullptr),
    battleLog(nullptr),
    attackButton(nullptr),
    escapeButton(nullptr) {
    setupUi();
    updateBattleStatus();
}

QString BattleSceneDialog::imagePath(const QString& fileName) const {
    return QCoreApplication::applicationDirPath()
    + "/assets/battle/"
        + fileName;
}

void BattleSceneDialog::setupUi() {
    setWindowTitle("战斗场景");
    setFixedSize(1000, 780);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // =========================
    // 上方场景区域
    // =========================
    QFrame* sceneFrame = new QFrame(this);
    sceneFrame->setFixedSize(968, 460);
    sceneFrame->setStyleSheet(
        "QFrame {"
        "   border: 2px solid #c6dcef;"
        "   border-radius: 18px;"
        "   background-color: white;"
        "}"
        );

    sceneLabel = new QLabel(sceneFrame);
    sceneLabel->setGeometry(8, 8, 952, 444);
    sceneLabel->setAlignment(Qt::AlignCenter);

    QPixmap scenePixmap(imagePath(sceneFileName));
    if (!scenePixmap.isNull()) {
        sceneLabel->setPixmap(
            scenePixmap.scaled(
                sceneLabel->size(),
                Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation
                )
            );
    } else {
        sceneLabel->setText("场景图片加载失败\n请检查 assets/battle/" + sceneFileName);
    }

    playerImageLabel = new QLabel(sceneFrame);
    playerImageLabel->setGeometry(110, 160, 210, 160);
    playerImageLabel->setAlignment(Qt::AlignCenter);
    playerImageLabel->setStyleSheet("border: none; background: transparent;");

    if (!playerPixmap.isNull()) {
        playerImageLabel->setPixmap(
            playerPixmap.scaled(
                playerImageLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    } else {
        playerImageLabel->setText("角色");
    }

    enemyImageLabel = new QLabel(sceneFrame);
    enemyImageLabel->setGeometry(620, 80, 250, 250);
    enemyImageLabel->setAlignment(Qt::AlignCenter);
    enemyImageLabel->setStyleSheet("border: none; background: transparent;");

    QPixmap enemyPixmap(imagePath(enemyFileName));
    if (!enemyPixmap.isNull()) {
        enemyImageLabel->setPixmap(
            enemyPixmap.scaled(
                enemyImageLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    } else {
        enemyImageLabel->setText("怪物图片加载失败");
    }

    sceneLabel->lower();
    playerImageLabel->raise();
    enemyImageLabel->raise();

    // =========================
    // 血量区域
    // =========================
    QFrame* statusFrame = new QFrame(this);
    statusFrame->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 240);"
        "   border: 2px solid #d8c7f0;"
        "   border-radius: 16px;"
        "}"
        );

    QHBoxLayout* statusLayout = new QHBoxLayout(statusFrame);
    statusLayout->setContentsMargins(16, 12, 16, 12);
    statusLayout->setSpacing(20);

    QVBoxLayout* playerStatusLayout = new QVBoxLayout();
    QVBoxLayout* enemyStatusLayout = new QVBoxLayout();

    playerHpText = new QLabel(this);
    enemyHpText = new QLabel(this);

    playerHpText->setStyleSheet("font-size: 16px; font-weight: bold; color: #5f4b8b; border: none;");
    enemyHpText->setStyleSheet("font-size: 16px; font-weight: bold; color: #5f4b8b; border: none;");

    playerHpBar = new QProgressBar(this);
    enemyHpBar = new QProgressBar(this);

    playerHpBar->setTextVisible(true);
    enemyHpBar->setTextVisible(true);

    playerHpBar->setFixedHeight(28);
    enemyHpBar->setFixedHeight(28);

    playerStatusLayout->addWidget(playerHpText);
    playerStatusLayout->addWidget(playerHpBar);

    enemyStatusLayout->addWidget(enemyHpText);
    enemyStatusLayout->addWidget(enemyHpBar);

    statusLayout->addLayout(playerStatusLayout);
    statusLayout->addLayout(enemyStatusLayout);

    // =========================
    // 日志 + 按钮
    // =========================
    battleLog = new QTextEdit(this);
    battleLog->setReadOnly(true);
    battleLog->setFixedHeight(130);
    battleLog->setStyleSheet(
        "QTextEdit {"
        "   font-size: 15px;"
        "   color: #3f5f7f;"
        "   background-color: rgba(255, 255, 255, 235);"
        "   border: 2px solid #c6dcef;"
        "   border-radius: 14px;"
        "   padding: 8px;"
        "}"
        );

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    attackButton = new QPushButton("攻击", this);
    escapeButton = new QPushButton("逃跑", this);

    attackButton->setFixedHeight(46);
    escapeButton->setFixedHeight(46);

    attackButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: white;"
        "   background-color: #8fbce6;"
        "   border: none;"
        "   border-radius: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7aaee0;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #649bd0;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #dddddd;"
        "   color: #999999;"
        "}"
        );

    escapeButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #665f7f;"
        "   background-color: #f3effa;"
        "   border: 2px solid #c8bfdc;"
        "   border-radius: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #ebe4f6;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #d8cee9;"
        "}"
        );

    connect(attackButton, &QPushButton::clicked, this, &BattleSceneDialog::handleAttack);
    connect(escapeButton, &QPushButton::clicked, this, &BattleSceneDialog::handleEscape);

    buttonLayout->addWidget(attackButton);
    buttonLayout->addWidget(escapeButton);

    mainLayout->addWidget(sceneFrame);
    mainLayout->addWidget(statusFrame);
    mainLayout->addWidget(battleLog);
    mainLayout->addLayout(buttonLayout);

    setStyleSheet(
        "QDialog {"
        "   background-color: #f7fbff;"
        "}"
        );

    appendBattleLog("战斗开始！你遇到了 " + QString::fromStdString(enemy.getName()) + "。");
}

void BattleSceneDialog::handleAttack() {
    if (battleEnded) {
        return;
    }

    enemy.takeDamage(player.getAttack());

    appendBattleLog(
        "你发动攻击，对 "
        + QString::fromStdString(enemy.getName())
        + " 造成 "
        + QString::number(player.getAttack())
        + " 点伤害。"
        );

    updateBattleStatus();

    if (!enemy.isAlive()) {
        victory = true;
        battleEnded = true;

        appendBattleLog("战斗胜利！你击败了 " + QString::fromStdString(enemy.getName()) + "。");

        attackButton->setEnabled(false);
        escapeButton->setText("结束战斗");
        return;
    }

    player.takeDamage(enemy.getAttack());

    appendBattleLog(
        QString::fromStdString(enemy.getName())
        + " 反击，对你造成 "
        + QString::number(enemy.getAttack())
        + " 点伤害。"
        );

    updateBattleStatus();

    if (!player.isAlive()) {
        defeated = true;
        battleEnded = true;

        appendBattleLog("战斗失败，你被击败了。");

        attackButton->setEnabled(false);
        escapeButton->setText("返回");
        return;
    }
}

void BattleSceneDialog::handleEscape() {
    if (battleEnded) {
        accept();
        return;
    }

    escaped = true;
    battleEnded = true;

    appendBattleLog("你选择了逃跑，战斗结束。");

    attackButton->setEnabled(false);
    escapeButton->setText("返回");
}

void BattleSceneDialog::updateBattleStatus() {
    int playerHp = player.getHp();
    int playerMaxHp = player.getMaxHp();

    if (playerHp < 0) {
        playerHp = 0;
    }

    int enemyHp = enemy.getHp();

    if (enemyHp < 0) {
        enemyHp = 0;
    }

    playerHpText->setText(
        "我的血量："
        + QString::number(playerHp)
        + " / "
        + QString::number(playerMaxHp)
        );

    enemyHpText->setText(
        QString::fromStdString(enemy.getName())
        + " 血量："
        + QString::number(enemyHp)
        + " / "
        + QString::number(enemyMaxHp)
        );

    playerHpBar->setMinimum(0);
    playerHpBar->setMaximum(playerMaxHp);
    playerHpBar->setValue(playerHp);
    playerHpBar->setFormat(QString("HP %1 / %2").arg(playerHp).arg(playerMaxHp));
    playerHpBar->setStyleSheet(hpBarStyle(playerHp, playerMaxHp));

    enemyHpBar->setMinimum(0);
    enemyHpBar->setMaximum(enemyMaxHp);
    enemyHpBar->setValue(enemyHp);
    enemyHpBar->setFormat(QString("HP %1 / %2").arg(enemyHp).arg(enemyMaxHp));
    enemyHpBar->setStyleSheet(hpBarStyle(enemyHp, enemyMaxHp));
}

QString BattleSceneDialog::hpBarStyle(int currentHp, int maxHp) const {
    int percent = 0;

    if (maxHp > 0) {
        percent = currentHp * 100 / maxHp;
    }

    QString color;

    if (percent >= 60) {
        color = "#9bd69b";
    } else if (percent >= 30) {
        color = "#f4c27a";
    } else {
        color = "#f28b82";
    }

    return QString(
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
               ).arg(color);
}

void BattleSceneDialog::appendBattleLog(const QString& text) {
    battleLog->append("◆ " + text);
}

bool BattleSceneDialog::wasVictory() const {
    return victory;
}

bool BattleSceneDialog::wasDefeated() const {
    return defeated;
}

bool BattleSceneDialog::didEscape() const {
    return escaped;
}

int BattleSceneDialog::expReward() const {
    return enemy.getExpReward();
}

int BattleSceneDialog::goldReward() const {
    return enemy.getGoldReward();
}

QString BattleSceneDialog::enemyName() const {
    return QString::fromStdString(enemy.getName());
}

void BattleSceneDialog::closeEvent(QCloseEvent* event) {
    if (!battleEnded) {
        escaped = true;
        battleEnded = true;
    }

    QDialog::closeEvent(event);
}
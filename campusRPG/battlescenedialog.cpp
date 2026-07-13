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
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <algorithm>

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
    animationRunning(false),
    sceneLabel(nullptr),
    playerImageLabel(nullptr),
    enemyImageLabel(nullptr),
    playerHpText(nullptr),
    enemyHpText(nullptr),
    playerHpBar(nullptr),
    enemyHpBar(nullptr),
    battleLog(nullptr),
    normalAttackButton(nullptr),
    middleSkillButton(nullptr),
    ultimateSkillButton(nullptr),
    escapeButton(nullptr),
    playerHpAnimation(nullptr),
    enemyHpAnimation(nullptr) {
    setupUi();
    updateBattleStatus(false);
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

    playerHomeGeometry = playerImageLabel->geometry();
    enemyHomeGeometry = enemyImageLabel->geometry();

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

    // 进度条数值改变时同步刷新文字与颜色。
    // QPropertyAnimation 修改 value 后，血条会连续变化，而不是直接跳到目标值。
    connect(playerHpBar, &QProgressBar::valueChanged, this, [this](int value) {
        const int maxHp = playerHpBar->maximum();
        playerHpBar->setFormat(QString("HP %1 / %2").arg(value).arg(maxHp));
        playerHpBar->setStyleSheet(hpBarStyle(value, maxHp));
    });

    connect(enemyHpBar, &QProgressBar::valueChanged, this, [this](int value) {
        const int maxHp = enemyHpBar->maximum();
        enemyHpBar->setFormat(QString("HP %1 / %2").arg(value).arg(maxHp));
        enemyHpBar->setStyleSheet(hpBarStyle(value, maxHp));
    });

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

    normalAttackButton = new QPushButton("普通攻击", this);
    middleSkillButton = new QPushButton("知识连击", this);
    ultimateSkillButton = new QPushButton("期末爆发", this);
    escapeButton = new QPushButton("逃跑", this);

    normalAttackButton->setFixedHeight(46);
    middleSkillButton->setFixedHeight(46);
    ultimateSkillButton->setFixedHeight(46);
    escapeButton->setFixedHeight(46);

    middleSkillButton->setToolTip("中级形态解锁：造成 1.5 倍攻击 + 10 点伤害。");
    ultimateSkillButton->setToolTip("终极形态解锁：造成 2 倍攻击 + 30 点伤害。");

    QString attackButtonStyle =
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
        "}";

    normalAttackButton->setStyleSheet(attackButtonStyle);
    middleSkillButton->setStyleSheet(attackButtonStyle);
    ultimateSkillButton->setStyleSheet(attackButtonStyle);

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
        "QPushButton:disabled {"
        "   color: #999999;"
        "   background-color: #e5e5e5;"
        "   border-color: #d0d0d0;"
        "}"
        );

    int stageValue = player.getEvolutionStageValue();
    middleSkillButton->setVisible(stageValue >= 1);
    ultimateSkillButton->setVisible(stageValue >= 2);

    connect(normalAttackButton, &QPushButton::clicked, this, &BattleSceneDialog::handleNormalAttack);
    connect(middleSkillButton, &QPushButton::clicked, this, &BattleSceneDialog::handleMiddleSkill);
    connect(ultimateSkillButton, &QPushButton::clicked, this, &BattleSceneDialog::handleUltimateSkill);
    connect(escapeButton, &QPushButton::clicked, this, &BattleSceneDialog::handleEscape);

    buttonLayout->addWidget(normalAttackButton);
    buttonLayout->addWidget(middleSkillButton);
    buttonLayout->addWidget(ultimateSkillButton);
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

    QString skillText = "普通攻击";
    if (player.getEvolutionStageValue() >= 1) {
        skillText += "、知识连击";
    }
    if (player.getEvolutionStageValue() >= 2) {
        skillText += "、期末爆发";
    }

    appendBattleLog("当前可用技能：" + skillText + "。");
}

void BattleSceneDialog::handleNormalAttack() {
    performPlayerSkill(
        "普通攻击",
        player.getAttack(),
        "稳定出手"
        );
}

void BattleSceneDialog::handleMiddleSkill() {
    if (player.getEvolutionStageValue() < 1) {
        appendBattleLog("当前形态还没有解锁【知识连击】。");
        return;
    }

    int damage = player.getAttack() * 3 / 2 + 10;

    performPlayerSkill(
        "知识连击",
        damage,
        "把复习资料连成一套组合拳"
        );
}

void BattleSceneDialog::handleUltimateSkill() {
    if (player.getEvolutionStageValue() < 2) {
        appendBattleLog("当前形态还没有解锁【期末爆发】。");
        return;
    }

    int damage = player.getAttack() * 2 + 30;

    performPlayerSkill(
        "期末爆发",
        damage,
        "进入终极冲刺状态"
        );
}

void BattleSceneDialog::performPlayerSkill(
    const QString& skillName,
    int damage,
    const QString& extraText
    ) {
    if (battleEnded || animationRunning) {
        return;
    }

    animationRunning = true;
    setBattleControlsEnabled(false);

    // 玩家向右前冲，再返回原位。
    QRect attackGeometry = playerHomeGeometry.translated(175, -6);

    QSequentialAnimationGroup* attackGroup = new QSequentialAnimationGroup(this);

    QPropertyAnimation* forwardAnimation =
        new QPropertyAnimation(playerImageLabel, "geometry", attackGroup);
    forwardAnimation->setDuration(180);
    forwardAnimation->setStartValue(playerHomeGeometry);
    forwardAnimation->setEndValue(attackGeometry);
    forwardAnimation->setEasingCurve(QEasingCurve::OutCubic);

    QPauseAnimation* hitPause = new QPauseAnimation(70, attackGroup);

    QPropertyAnimation* returnAnimation =
        new QPropertyAnimation(playerImageLabel, "geometry", attackGroup);
    returnAnimation->setDuration(220);
    returnAnimation->setStartValue(attackGeometry);
    returnAnimation->setEndValue(playerHomeGeometry);
    returnAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    attackGroup->addAnimation(forwardAnimation);
    attackGroup->addAnimation(hitPause);
    attackGroup->addAnimation(returnAnimation);

    // 到达前冲终点时才结算伤害，视觉动作与数值变化保持一致。
    connect(forwardAnimation, &QPropertyAnimation::finished, this,
            [this, skillName, damage, extraText]() {
                if (battleEnded) {
                    return;
                }

                enemy.takeDamage(damage);

                QString logText = "你使用【" + skillName + "】";
                if (!extraText.isEmpty()) {
                    logText += "，" + extraText;
                }

                logText += "，对 "
                           + QString::fromStdString(enemy.getName())
                           + " 造成 "
                           + QString::number(damage)
                           + " 点伤害。";

                appendBattleLog(logText);
                updateBattleStatus(true);
                playHitReaction(enemyImageLabel, enemyHomeGeometry);
            });

    connect(attackGroup, &QSequentialAnimationGroup::finished, this, [this]() {
        playerImageLabel->setGeometry(playerHomeGeometry);

        if (battleEnded) {
            return;
        }

        if (!enemy.isAlive()) {
            QTimer::singleShot(100, this, [this]() {
                if (!battleEnded) {
                    finishVictory();
                }
            });
            return;
        }

        // 留出少量停顿，使双方攻击不显得挤在一起。
        QTimer::singleShot(160, this, [this]() {
            if (!battleEnded) {
                handleEnemyCounterAttack();
            }
        });
    });

    attackGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void BattleSceneDialog::handleEnemyCounterAttack() {
    if (battleEnded) {
        return;
    }

    // 敌人向左前冲，再返回原位。
    QRect attackGeometry = enemyHomeGeometry.translated(-175, 6);

    QSequentialAnimationGroup* attackGroup = new QSequentialAnimationGroup(this);

    QPropertyAnimation* forwardAnimation =
        new QPropertyAnimation(enemyImageLabel, "geometry", attackGroup);
    forwardAnimation->setDuration(190);
    forwardAnimation->setStartValue(enemyHomeGeometry);
    forwardAnimation->setEndValue(attackGeometry);
    forwardAnimation->setEasingCurve(QEasingCurve::OutCubic);

    QPauseAnimation* hitPause = new QPauseAnimation(70, attackGroup);

    QPropertyAnimation* returnAnimation =
        new QPropertyAnimation(enemyImageLabel, "geometry", attackGroup);
    returnAnimation->setDuration(220);
    returnAnimation->setStartValue(attackGeometry);
    returnAnimation->setEndValue(enemyHomeGeometry);
    returnAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    attackGroup->addAnimation(forwardAnimation);
    attackGroup->addAnimation(hitPause);
    attackGroup->addAnimation(returnAnimation);

    connect(forwardAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (battleEnded) {
            return;
        }

        const int damage = enemy.getAttack();
        player.takeDamage(damage);

        appendBattleLog(
            QString::fromStdString(enemy.getName())
            + " 反击，对你造成 "
            + QString::number(damage)
            + " 点伤害。"
            );

        updateBattleStatus(true);
        playHitReaction(playerImageLabel, playerHomeGeometry);
    });

    connect(attackGroup, &QSequentialAnimationGroup::finished, this, [this]() {
        enemyImageLabel->setGeometry(enemyHomeGeometry);

        if (battleEnded) {
            return;
        }

        // 等待平滑血条基本结束后，再开放下一次操作。
        QTimer::singleShot(100, this, [this]() {
            if (battleEnded) {
                return;
            }

            if (!player.isAlive()) {
                finishDefeat();
                return;
            }

            animationRunning = false;
            setBattleControlsEnabled(true);
        });
    });

    attackGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void BattleSceneDialog::finishVictory() {
    if (battleEnded) {
        return;
    }

    victory = true;
    battleEnded = true;
    animationRunning = false;

    appendBattleLog("战斗胜利！你击败了 " + QString::fromStdString(enemy.getName()) + "。");

    setSkillButtonsEnabled(false);
    escapeButton->setEnabled(true);
    escapeButton->setText("结束战斗");
}

void BattleSceneDialog::finishDefeat() {
    if (battleEnded) {
        return;
    }

    defeated = true;
    battleEnded = true;
    animationRunning = false;

    appendBattleLog("战斗失败，你被击败了。");

    setSkillButtonsEnabled(false);
    escapeButton->setEnabled(true);
    escapeButton->setText("返回");
}

void BattleSceneDialog::handleEscape() {
    if (animationRunning) {
        return;
    }

    if (battleEnded) {
        accept();
        return;
    }

    escaped = true;
    battleEnded = true;

    appendBattleLog("你选择了逃跑，战斗结束。");

    setSkillButtonsEnabled(false);
    escapeButton->setText("返回");
}

void BattleSceneDialog::updateBattleStatus(bool animateBars) {
    const int playerMaxHp = std::max(0, player.getMaxHp());
    const int playerHp = std::clamp(player.getHp(), 0, playerMaxHp);
    const int safeEnemyMaxHp = std::max(0, enemyMaxHp);
    const int enemyHp = std::clamp(enemy.getHp(), 0, safeEnemyMaxHp);

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
        + QString::number(safeEnemyMaxHp)
        );

    playerHpBar->setRange(0, playerMaxHp);
    enemyHpBar->setRange(0, safeEnemyMaxHp);

    if (animateBars) {
        animateHpBar(playerHpBar, playerHp, playerMaxHp);
        animateHpBar(enemyHpBar, enemyHp, safeEnemyMaxHp);
    } else {
        playerHpBar->setValue(playerHp);
        enemyHpBar->setValue(enemyHp);

        playerHpBar->setFormat(QString("HP %1 / %2").arg(playerHp).arg(playerMaxHp));
        enemyHpBar->setFormat(QString("HP %1 / %2").arg(enemyHp).arg(safeEnemyMaxHp));

        playerHpBar->setStyleSheet(hpBarStyle(playerHp, playerMaxHp));
        enemyHpBar->setStyleSheet(hpBarStyle(enemyHp, safeEnemyMaxHp));
    }
}

void BattleSceneDialog::animateHpBar(QProgressBar* bar, int targetHp, int maxHp) {
    if (!bar) {
        return;
    }

    targetHp = std::clamp(targetHp, 0, std::max(0, maxHp));

    QPropertyAnimation*& currentAnimation =
        (bar == playerHpBar) ? playerHpAnimation : enemyHpAnimation;

    if (currentAnimation) {
        currentAnimation->stop();
        currentAnimation->deleteLater();
        currentAnimation = nullptr;
    }

    if (bar->value() == targetHp) {
        bar->setFormat(QString("HP %1 / %2").arg(targetHp).arg(maxHp));
        bar->setStyleSheet(hpBarStyle(targetHp, maxHp));
        return;
    }

    QPropertyAnimation* animation = new QPropertyAnimation(bar, "value", this);
    currentAnimation = animation;

    animation->setDuration(360);
    animation->setStartValue(bar->value());
    animation->setEndValue(targetHp);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    const bool isPlayerBar = (bar == playerHpBar);

    connect(animation, &QPropertyAnimation::finished, this,
            [this, animation, bar, targetHp, maxHp, isPlayerBar]() {
                bar->setValue(targetHp);
                bar->setFormat(QString("HP %1 / %2").arg(targetHp).arg(maxHp));
                bar->setStyleSheet(hpBarStyle(targetHp, maxHp));

                if (isPlayerBar) {
                    if (playerHpAnimation == animation) {
                        playerHpAnimation = nullptr;
                    }
                } else if (enemyHpAnimation == animation) {
                    enemyHpAnimation = nullptr;
                }

                animation->deleteLater();
            });

    animation->start();
}

void BattleSceneDialog::playHitReaction(QLabel* target, const QRect& homeGeometry) {
    if (!target) {
        return;
    }

    QSequentialAnimationGroup* reactionGroup = new QSequentialAnimationGroup(this);

    const QRect rightGeometry = homeGeometry.translated(9, 0);
    const QRect leftGeometry = homeGeometry.translated(-9, 0);

    auto addShakeStep = [reactionGroup, target](
                            const QRect& start,
                            const QRect& end,
                            int duration) {
        QPropertyAnimation* step = new QPropertyAnimation(target, "geometry", reactionGroup);
        step->setDuration(duration);
        step->setStartValue(start);
        step->setEndValue(end);
        step->setEasingCurve(QEasingCurve::InOutQuad);
        reactionGroup->addAnimation(step);
    };

    addShakeStep(homeGeometry, rightGeometry, 45);
    addShakeStep(rightGeometry, leftGeometry, 70);
    addShakeStep(leftGeometry, homeGeometry, 55);

    connect(reactionGroup, &QSequentialAnimationGroup::finished, this,
            [target, homeGeometry]() {
                target->setGeometry(homeGeometry);
            });

    reactionGroup->start(QAbstractAnimation::DeleteWhenStopped);
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

void BattleSceneDialog::setSkillButtonsEnabled(bool enabled) {
    if (normalAttackButton) {
        normalAttackButton->setEnabled(enabled);
    }

    if (middleSkillButton) {
        middleSkillButton->setEnabled(enabled && player.getEvolutionStageValue() >= 1);
    }

    if (ultimateSkillButton) {
        ultimateSkillButton->setEnabled(enabled && player.getEvolutionStageValue() >= 2);
    }
}

void BattleSceneDialog::setBattleControlsEnabled(bool enabled) {
    setSkillButtonsEnabled(enabled && !battleEnded);

    if (escapeButton) {
        escapeButton->setEnabled(enabled);
    }
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

    animationRunning = false;
    QDialog::closeEvent(event);
}

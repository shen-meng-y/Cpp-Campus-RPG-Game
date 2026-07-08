#ifndef BATTLESCENEDIALOG_H
#define BATTLESCENEDIALOG_H

#include <QDialog>
#include <QPixmap>

#include "Character.h"
#include "Enemy.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QTextEdit;
class QCloseEvent;

class BattleSceneDialog : public QDialog {
    Q_OBJECT

public:
    BattleSceneDialog(
        Character& player,
        const Enemy& enemy,
        const QString& sceneFileName,
        const QString& enemyFileName,
        const QPixmap& playerPixmap,
        QWidget* parent = nullptr
        );

    bool wasVictory() const;
    bool wasDefeated() const;
    bool didEscape() const;

    int expReward() const;
    int goldReward() const;
    QString enemyName() const;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Character& player;
    Enemy enemy;

    int enemyMaxHp;

    QString sceneFileName;
    QString enemyFileName;
    QPixmap playerPixmap;

    bool victory;
    bool defeated;
    bool escaped;
    bool battleEnded;

    QLabel* sceneLabel;
    QLabel* playerImageLabel;
    QLabel* enemyImageLabel;

    QLabel* playerHpText;
    QLabel* enemyHpText;

    QProgressBar* playerHpBar;
    QProgressBar* enemyHpBar;

    QTextEdit* battleLog;

    QPushButton* attackButton;
    QPushButton* escapeButton;

private:
    void setupUi();
    void updateBattleStatus();
    void handleAttack();
    void handleEscape();
    void appendBattleLog(const QString& text);

    QString imagePath(const QString& fileName) const;
    QString hpBarStyle(int currentHp, int maxHp) const;
};

#endif
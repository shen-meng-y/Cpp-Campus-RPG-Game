#ifndef BATTLEMAPDIALOG_H
#define BATTLEMAPDIALOG_H

#include <QDialog>
#include <QString>

class QLabel;
class QPushButton;
class QWidget;

class BattleMapDialog : public QDialog {
    Q_OBJECT

public:
    explicit BattleMapDialog(QWidget *parent = nullptr);

signals:
    // 告诉主窗口：玩家选择了哪只怪物
    void attackEnemyRequested(int enemyIndex);

private:
    int selectedEnemyIndex;

    QLabel* mapLabel;
    QLabel* monsterImageLabel;
    QLabel* infoLabel;
    QPushButton* attackButton;

    void setupUi();

    QString imagePath(const QString& fileName) const;

    QPushButton* createMapButton(
        const QString& text,
        QWidget* parent,
        int x,
        int y
        );

    void showMonster(
        int enemyIndex,
        const QString& placeName,
        const QString& monsterName,
        const QString& imageFileName,
        const QString& description
        );
};

#endif
#ifndef QUESTDIALOG_H
#define QUESTDIALOG_H

#include <QDialog>
#include <vector>
#include "Quest.h"
#include "Character.h"

class QTabWidget;
class QVBoxLayout;

class QuestDialog : public QDialog {
    Q_OBJECT

public:
    explicit QuestDialog(
        std::vector<Quest>& quests,
        Character& player,
        int& killCount,
        QWidget* parent = nullptr
        );

signals:
    void questChanged();
    void logRequested(const QString& message);

private:
    std::vector<Quest>& quests;
    Character& player;
    int& killCount;

    QTabWidget* tabWidget;

    void initUi();
    void rebuildTabs();
    QWidget* createQuestTab(QuestType type, const QString& emptyText);
    QWidget* createQuestCard(int questIndex);
    void updateAllQuestStatus();
    void handleQuestButton(int questIndex);

    QString statusText(const Quest& quest) const;
    QString statusColor(const Quest& quest) const;
    QString buttonText(const Quest& quest) const;
};

#endif

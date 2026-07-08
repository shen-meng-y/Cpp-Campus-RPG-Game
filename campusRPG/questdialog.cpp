#include "questdialog.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QFont>

QuestDialog::QuestDialog(
    std::vector<Quest>& quests,
    Character& player,
    int& killCount,
    QWidget* parent
    )
    : QDialog(parent),
    quests(quests),
    player(player),
    killCount(killCount),
    tabWidget(nullptr) {
    initUi();
    updateAllQuestStatus();
    rebuildTabs();
}

void QuestDialog::initUi() {
    setWindowTitle("任务中心");
    resize(860, 640);
    setModal(true);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(12);

    QLabel* titleLabel = new QLabel("任务中心", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("questTitleLabel");
    rootLayout->addWidget(titleLabel);

    QLabel* tipLabel = new QLabel("接受任务后，战斗、升级、金币变化会自动更新任务进度。完成后点击领取奖励。", this);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setWordWrap(true);
    tipLabel->setObjectName("questTipLabel");
    rootLayout->addWidget(tipLabel);

    tabWidget = new QTabWidget(this);
    rootLayout->addWidget(tabWidget, 1);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* refreshButton = new QPushButton("刷新进度", this);
    QPushButton* closeButton = new QPushButton("关闭", this);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(closeButton);
    rootLayout->addLayout(buttonLayout);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        updateAllQuestStatus();
        rebuildTabs();
        emit questChanged();
        QMessageBox::information(this, "任务进度", "任务进度已刷新。");
    });

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    setStyleSheet(
        "QDialog {"
        "   background-color: #f7f3ff;"
        "   font-family: 'Microsoft YaHei';"
        "}"

        "#questTitleLabel {"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   color: #5f4b8b;"
        "}"

        "#questTipLabel {"
        "   font-size: 14px;"
        "   color: #6f6680;"
        "}"

        "QTabWidget::pane {"
        "   border: 2px solid #d8c7f0;"
        "   border-radius: 14px;"
        "   background-color: rgba(255, 255, 255, 230);"
        "   top: -1px;"
        "}"

        "QTabBar::tab {"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "   color: #6b5b84;"
        "   background-color: #eee8fa;"
        "   border: 2px solid #d8c7f0;"
        "   border-bottom: none;"
        "   border-top-left-radius: 10px;"
        "   border-top-right-radius: 10px;"
        "   padding: 8px 20px;"
        "   margin-right: 4px;"
        "}"

        "QTabBar::tab:selected {"
        "   color: #4f3f77;"
        "   background-color: #ffffff;"
        "}"

        "QPushButton {"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "   color: #5f4b8b;"
        "   background-color: #ffffff;"
        "   border: 2px solid #cbb7eb;"
        "   border-radius: 12px;"
        "   padding: 7px 18px;"
        "}"

        "QPushButton:hover {"
        "   background-color: #f0e8ff;"
        "   border: 2px solid #bda2e8;"
        "}"

        "QPushButton:pressed {"
        "   color: white;"
        "   background-color: #a78bd8;"
        "}"

        "QPushButton:disabled {"
        "   color: #999999;"
        "   background-color: #eeeeee;"
        "   border: 2px solid #cccccc;"
        "}"

        "QProgressBar {"
        "   border: 2px solid #d8c7f0;"
        "   border-radius: 9px;"
        "   background-color: #fffafa;"
        "   text-align: center;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "   color: #5f4b8b;"
        "}"

        "QProgressBar::chunk {"
        "   border-radius: 7px;"
        "   background-color: #a9c7f5;"
        "}"
        );
}

void QuestDialog::updateAllQuestStatus() {
    for (Quest& quest : quests) {
        quest.checkComplete(player, killCount);
    }
}

void QuestDialog::rebuildTabs() {
    while (tabWidget->count() > 0) {
        QWidget* page = tabWidget->widget(0);
        tabWidget->removeTab(0);
        page->deleteLater();
    }

    tabWidget->addTab(createQuestTab(QuestType::KillEnemy, "暂无战斗任务。"), "战斗任务");
    tabWidget->addTab(createQuestTab(QuestType::ReachLevel, "暂无成长任务。"), "成长任务");
    tabWidget->addTab(createQuestTab(QuestType::GoldAmount, "暂无财富任务。"), "财富任务");
}

QWidget* QuestDialog::createQuestTab(QuestType type, const QString& emptyText) {
    QWidget* page = new QWidget(this);
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(12, 12, 12, 12);
    pageLayout->setSpacing(10);

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget* contentWidget = new QWidget(scrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(4, 4, 4, 4);
    contentLayout->setSpacing(12);

    bool hasQuest = false;

    for (int i = 0; i < static_cast<int>(quests.size()); ++i) {
        if (quests[i].getType() == type) {
            contentLayout->addWidget(createQuestCard(i));
            hasQuest = true;
        }
    }

    if (!hasQuest) {
        QLabel* emptyLabel = new QLabel(emptyText, contentWidget);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 16px; color: #8a7c99; padding: 40px;");
        contentLayout->addWidget(emptyLabel);
    }

    contentLayout->addStretch();
    contentWidget->setLayout(contentLayout);
    scrollArea->setWidget(contentWidget);

    pageLayout->addWidget(scrollArea);
    page->setLayout(pageLayout);

    return page;
}

QWidget* QuestDialog::createQuestCard(int questIndex) {
    Quest& quest = quests[questIndex];

    QFrame* card = new QFrame(this);
    card->setObjectName("questCard");
    card->setMinimumHeight(150);
    card->setStyleSheet(
        "QFrame#questCard {"
        "   background-color: rgba(255, 255, 255, 245);"
        "   border: 2px solid #d8c7f0;"
        "   border-radius: 16px;"
        "}"
        );

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(8);

    QHBoxLayout* topLayout = new QHBoxLayout();

    QLabel* nameLabel = new QLabel(QString::fromStdString(quest.getName()), card);
    nameLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4f3f77;");

    QLabel* statusLabel = new QLabel(statusText(quest), card);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setMinimumWidth(92);
    statusLabel->setStyleSheet(
        "font-size: 14px;"
        "font-weight: bold;"
        "color: white;"
        "border-radius: 10px;"
        "padding: 5px 10px;"
        "background-color: " + statusColor(quest) + ";"
        );

    topLayout->addWidget(nameLabel);
    topLayout->addStretch();
    topLayout->addWidget(statusLabel);
    cardLayout->addLayout(topLayout);

    QLabel* descLabel = new QLabel("目标：" + QString::fromStdString(quest.getDescription()), card);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 15px; color: #5f6680;");
    cardLayout->addWidget(descLabel);

    int target = quest.getConditionValue();
    if (target <= 0) {
        target = 1;
    }

    int progress = quest.getProgressValue(player, killCount);

    QProgressBar* progressBar = new QProgressBar(card);
    progressBar->setMinimum(0);
    progressBar->setMaximum(target);
    progressBar->setValue(progress);
    progressBar->setFormat(QString("进度 %1 / %2").arg(progress).arg(target));
    cardLayout->addWidget(progressBar);

    QHBoxLayout* bottomLayout = new QHBoxLayout();

    QLabel* rewardLabel = new QLabel(
        QString("奖励：经验 +%1    金币 +%2")
            .arg(quest.getRewardExp())
            .arg(quest.getRewardGold()),
        card
        );
    rewardLabel->setStyleSheet("font-size: 14px; color: #6f6680;");

    QPushButton* actionButton = new QPushButton(buttonText(quest), card);
    actionButton->setMinimumWidth(120);
    actionButton->setEnabled(!quest.isRewardClaimed());

    connect(actionButton, &QPushButton::clicked, this, [this, questIndex]() {
        handleQuestButton(questIndex);
    });

    bottomLayout->addWidget(rewardLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(actionButton);
    cardLayout->addLayout(bottomLayout);

    card->setLayout(cardLayout);
    return card;
}

void QuestDialog::handleQuestButton(int questIndex) {
    if (questIndex < 0 || questIndex >= static_cast<int>(quests.size())) {
        return;
    }

    Quest& quest = quests[questIndex];

    if (!quest.isAccepted()) {
        quest.accept();
        quest.checkComplete(player, killCount);

        emit logRequested("接受任务：" + QString::fromStdString(quest.getName()));
        emit questChanged();
        rebuildTabs();
        return;
    }

    if (!quest.isCompleted()) {
        quest.checkComplete(player, killCount);

        if (quest.isCompleted()) {
            QMessageBox::information(this, "任务完成", "任务已经完成，可以领取奖励了。");
            emit logRequested("任务完成：" + QString::fromStdString(quest.getName()));
        } else {
            QMessageBox::information(this, "任务进度", "当前任务还没有完成，请继续推进任务目标。");
        }

        emit questChanged();
        rebuildTabs();
        return;
    }

    if (!quest.isRewardClaimed()) {
        bool beforeClaimed = quest.isRewardClaimed();
        quest.claimReward(player);

        if (!beforeClaimed && quest.isRewardClaimed()) {
            updateAllQuestStatus();

            QMessageBox::information(
                this,
                "领取成功",
                QString("任务完成！\n\n%1\n获得奖励：经验 +%2，金币 +%3")
                    .arg(QString::fromStdString(quest.getName()))
                    .arg(quest.getRewardExp())
                    .arg(quest.getRewardGold())
                );

            emit logRequested("领取任务奖励：" + QString::fromStdString(quest.getName()));
            emit questChanged();
        }

        rebuildTabs();
    }
}

QString QuestDialog::statusText(const Quest& quest) const {
    if (!quest.isAccepted()) {
        return "未接受";
    }

    if (quest.isRewardClaimed()) {
        return "已完成";
    }

    if (quest.isCompleted()) {
        return "可领取";
    }

    return "进行中";
}

QString QuestDialog::statusColor(const Quest& quest) const {
    if (!quest.isAccepted()) {
        return "#8aa6c8";
    }

    if (quest.isRewardClaimed()) {
        return "#9a9a9a";
    }

    if (quest.isCompleted()) {
        return "#75b889";
    }

    return "#d2a65a";
}

QString QuestDialog::buttonText(const Quest& quest) const {
    if (!quest.isAccepted()) {
        return "接受任务";
    }

    if (quest.isRewardClaimed()) {
        return "已完成";
    }

    if (quest.isCompleted()) {
        return "领取奖励";
    }

    return "检测完成";
}

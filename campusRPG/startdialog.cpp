#include "startdialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QApplication>

StartDialog::StartDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("校园冒险");
    setFixedSize(900, 600);

    // 游戏标题
    QLabel *titleLabel = new QLabel("校园 RPG 冒险", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    color: #5b4b8a;"
        "    font-size: 48px;"
        "    font-weight: bold;"
        "    background-color: transparent;"
        "}"
        );

    // 游戏说明
    QLabel *descriptionLabel = new QLabel(
        "探索校园 · 完成任务 · 挑战怪物 · 培养角色",
        this
        );

    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setStyleSheet(
        "QLabel {"
        "    color: #74699b;"
        "    font-size: 20px;"
        "    background-color: transparent;"
        "}"
        );

    // 开始游戏按钮
    startButton = new QPushButton("开始游戏", this);
    startButton->setFixedSize(260, 65);
    startButton->setCursor(Qt::PointingHandCursor);

    // 退出游戏按钮
    exitButton = new QPushButton("退出游戏", this);
    exitButton->setFixedSize(260, 55);
    exitButton->setCursor(Qt::PointingHandCursor);

    startButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #9b8ac4;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 18px;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #8875b5;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #7562a2;"
        "}"
        );

    exitButton->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 220);"
        "    color: #625680;"
        "    border: 2px solid #a89ac9;"
        "    border-radius: 16px;"
        "    font-size: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #eee9f8;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #ddd5ee;"
        "}"
        );

    // 按钮水平居中
    QHBoxLayout *startButtonLayout = new QHBoxLayout;
    startButtonLayout->addStretch();
    startButtonLayout->addWidget(startButton);
    startButtonLayout->addStretch();

    QHBoxLayout *exitButtonLayout = new QHBoxLayout;
    exitButtonLayout->addStretch();
    exitButtonLayout->addWidget(exitButton);
    exitButtonLayout->addStretch();

    // 总布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(60, 60, 60, 50);
    mainLayout->addStretch(2);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addStretch(3);
    mainLayout->addLayout(startButtonLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(exitButtonLayout);
    mainLayout->addStretch(1);

    setLayout(mainLayout);

    // 窗口背景
    setStyleSheet(
        "StartDialog {"
        "    background-color: #f1edfa;"
        "}"
        );

    // 点击开始游戏，返回 Accepted
    connect(startButton, &QPushButton::clicked,
            this, &QDialog::accept);

    // 点击退出游戏
    connect(exitButton, &QPushButton::clicked,
            qApp, &QApplication::quit);
}
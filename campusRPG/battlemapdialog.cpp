#include "battlemapdialog.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPixmap>
#include <QMessageBox>
#include <QCoreApplication>

BattleMapDialog::BattleMapDialog(QWidget *parent)
    : QDialog(parent),
    selectedEnemyIndex(-1),
    mapLabel(nullptr),
    monsterImageLabel(nullptr),
    infoLabel(nullptr),
    attackButton(nullptr) {
    setupUi();
}

QString BattleMapDialog::imagePath(const QString& fileName) const {
    return QCoreApplication::applicationDirPath()
    + "/assets/battle/"
        + fileName;
}

void BattleMapDialog::setupUi() {
    setWindowTitle("校园探索战斗地图");
    setFixedSize(1180, 720);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // =========================
    // 左侧地图区域
    // =========================
    QFrame* mapFrame = new QFrame(this);
    mapFrame->setFixedSize(820, 660);
    mapFrame->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 235);"
        "   border: 2px solid #c6dcef;"
        "   border-radius: 18px;"
        "}"
        );

    mapLabel = new QLabel(mapFrame);
    mapLabel->setGeometry(10, 10, 800, 640);
    mapLabel->setAlignment(Qt::AlignCenter);

    QPixmap mapPixmap(imagePath("map.png"));
    if (!mapPixmap.isNull()) {
        mapLabel->setPixmap(
            mapPixmap.scaled(
                mapLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    } else {
        mapLabel->setText("地图图片加载失败\n请检查 assets/battle/map.png 是否在程序运行目录下");
    }

    // 地图上的探索按钮
    QPushButton* btnTrack = createMapButton("体育场", mapFrame, 535, 180);
    QPushButton* btnLibrary = createMapButton("图书馆", mapFrame, 350, 395);
    QPushButton* btnTeaching = createMapButton("教学区", mapFrame, 235, 230);

    connect(btnTrack, &QPushButton::clicked, this, [this]() {
        showMonster(
            0,
            "体育场 / 田径场",
            "800米小怪",
            "lmp.png",
            "它常年出没在操场，擅长消耗你的体力。虽然单次攻击不高，但会让人越跑越累。"
            );
    });

    connect(btnLibrary, &QPushButton::clicked, this, [this]() {
        showMonster(
            1,
            "图书馆 / 自习区",
            "大作业幽灵",
            "homework.png",
            "它背着厚厚的大作业资料，擅长制造 ddl 压力。难度中等，适合升级后挑战。"
            );
    });

    connect(btnTeaching, &QPushButton::clicked, this, [this]() {
        showMonster(
            2,
            "教学区 / 考试周核心区",
            "期末周Boss",
            "boss.png",
            "校园里最强的压力怪物。血量高、攻击强，建议血量充足、攻击力提升后再挑战。"
            );
    });

    // =========================
    // 右侧信息区域
    // =========================
    QFrame* infoFrame = new QFrame(this);
    infoFrame->setFixedSize(310, 660);
    infoFrame->setStyleSheet(
        "QFrame {"
        "   background-color: rgba(255, 255, 255, 238);"
        "   border: 2px solid #d8c7f0;"
        "   border-radius: 18px;"
        "}"
        );

    QVBoxLayout* infoLayout = new QVBoxLayout(infoFrame);
    infoLayout->setContentsMargins(18, 18, 18, 18);
    infoLayout->setSpacing(14);

    QLabel* titleLabel = new QLabel("校园探索", infoFrame);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #5f4b8b;"
        "   border: none;"
        "}"
        );

    monsterImageLabel = new QLabel(infoFrame);
    monsterImageLabel->setFixedSize(270, 270);
    monsterImageLabel->setAlignment(Qt::AlignCenter);
    monsterImageLabel->setText("未发现怪物");
    monsterImageLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #fbf8ff;"
        "   border: 2px solid #d8c7f0;"
        "   border-radius: 14px;"
        "   color: #7a6b8f;"
        "   font-size: 16px;"
        "}"
        );

    infoLabel = new QLabel(
        "点击地图上的地点进行探索。\n\n"
        "探索后会显示该地点出现的怪物，然后可以选择攻击。"
        ,
        infoFrame
        );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   color: #4f5f7f;"
        "   border: none;"
        "}"
        );

    attackButton = new QPushButton("攻击这个怪物", infoFrame);
    attackButton->setEnabled(false);
    attackButton->setMinimumHeight(46);
    attackButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 17px;"
        "   font-weight: bold;"
        "   color: #ffffff;"
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
        "   color: #999999;"
        "   background-color: #dddddd;"
        "}"
        );

    connect(attackButton, &QPushButton::clicked, this, [this]() {
        if (selectedEnemyIndex < 0) {
            QMessageBox::information(this, "提示", "请先探索一个地点。");
            return;
        }

        emit attackEnemyRequested(selectedEnemyIndex);
        accept();
    });

    infoLayout->addWidget(titleLabel);
    infoLayout->addWidget(monsterImageLabel);
    infoLayout->addWidget(infoLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(attackButton);

    mainLayout->addWidget(mapFrame);
    mainLayout->addWidget(infoFrame);

    setStyleSheet(
        "QDialog {"
        "   background-color: #f7fbff;"
        "}"
        );
}

QPushButton* BattleMapDialog::createMapButton(
    const QString& text,
    QWidget* parent,
    int x,
    int y
    ) {
    QPushButton* button = new QPushButton(text, parent);
    button->setGeometry(x, y, 92, 34);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(
        "QPushButton {"
        "   font-size: 15px;"
        "   font-weight: bold;"
        "   color: #ffffff;"
        "   background-color: rgba(95, 75, 139, 210);"
        "   border: 2px solid rgba(255, 255, 255, 230);"
        "   border-radius: 17px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(126, 104, 175, 235);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(78, 60, 120, 240);"
        "}"
        );

    return button;
}

void BattleMapDialog::showMonster(
    int enemyIndex,
    const QString& placeName,
    const QString& monsterName,
    const QString& imageFileName,
    const QString& description
    ) {
    selectedEnemyIndex = enemyIndex;

    QPixmap monsterPixmap(imagePath(imageFileName));
    if (!monsterPixmap.isNull()) {
        monsterImageLabel->setPixmap(
            monsterPixmap.scaled(
                monsterImageLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    } else {
        monsterImageLabel->setText("怪物图片加载失败\n请检查 assets/battle/" + imageFileName);
    }

    infoLabel->setText(
        "探索地点：" + placeName + "\n\n"
                                   "发现怪物：" + monsterName + "\n\n"
                        "怪物说明：" + description
        );

    attackButton->setEnabled(true);

    QMessageBox::information(
        this,
        "探索结果",
        "你在【" + placeName + "】发现了【" + monsterName + "】！"
        );
}
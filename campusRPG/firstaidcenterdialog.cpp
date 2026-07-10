#include "firstaidcenterdialog.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>
#include <random>

FirstAidCenterDialog::FirstAidCenterDialog(Character &player, QWidget *parent)
    : QDialog(parent),
    player(player),
    hpValueLabel(nullptr),
    goldValueLabel(nullptr),
    hpProgressBar(nullptr),
    diagnosisLabel(nullptr),
    recommendationLabel(nullptr),
    treatmentCombo(nullptr),
    costPreviewLabel(nullptr),
    gameStepLabel(nullptr),
    gameStatusLabel(nullptr),
    discountLabel(nullptr),
    startGameButton(nullptr),
    treatButton(nullptr),
    diagnosed(false),
    miniGameStarted(false),
    miniGameFinished(false),
    treatmentDone(false),
    currentStep(0),
    errorCount(0),
    discountPercent(0)
{
    setWindowTitle("校园急救中心");
    resize(780, 620);
    setMinimumSize(720, 560);
    setModal(true);

    buildUi();
    applyStyle();
    updateCharacterDisplay();
    resetMiniGame();
}

bool FirstAidCenterDialog::treatmentPerformed() const
{
    return treatmentDone;
}

QString FirstAidCenterDialog::treatmentSummary() const
{
    return summaryText;
}

void FirstAidCenterDialog::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(22, 18, 22, 18);
    mainLayout->setSpacing(14);

    auto *titleLabel = new QLabel("🏥 校园急救中心", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(14);

    auto *healthGroup = new QGroupBox("角色健康信息", this);
    auto *healthLayout = new QVBoxLayout(healthGroup);

    auto *nameLabel = new QLabel(
        "角色：" + QString::fromStdString(player.getName())
            + "　|　" + QString::fromStdString(player.getEvolutionStageName()),
        healthGroup);
    nameLabel->setObjectName("importantLabel");

    hpValueLabel = new QLabel(healthGroup);
    goldValueLabel = new QLabel(healthGroup);
    hpProgressBar = new QProgressBar(healthGroup);
    hpProgressBar->setTextVisible(true);
    hpProgressBar->setMinimum(0);

    healthLayout->addWidget(nameLabel);
    healthLayout->addWidget(hpValueLabel);
    healthLayout->addWidget(hpProgressBar);
    healthLayout->addWidget(goldValueLabel);
    healthLayout->addStretch();

    auto *diagnosisGroup = new QGroupBox("医生诊断", this);
    auto *diagnosisLayout = new QVBoxLayout(diagnosisGroup);

    diagnosisLabel = new QLabel("尚未诊断，请先点击“开始诊断”。", diagnosisGroup);
    diagnosisLabel->setWordWrap(true);
    recommendationLabel = new QLabel("医生建议：等待检查", diagnosisGroup);
    recommendationLabel->setWordWrap(true);
    recommendationLabel->setObjectName("recommendationLabel");

    auto *diagnoseButton = new QPushButton("开始诊断", diagnosisGroup);
    diagnoseButton->setObjectName("primaryButton");

    diagnosisLayout->addWidget(diagnosisLabel);
    diagnosisLayout->addWidget(recommendationLabel);
    diagnosisLayout->addStretch();
    diagnosisLayout->addWidget(diagnoseButton);

    topLayout->addWidget(healthGroup, 1);
    topLayout->addWidget(diagnosisGroup, 1);
    mainLayout->addLayout(topLayout);

    auto *treatmentGroup = new QGroupBox("选择治疗方案", this);
    auto *treatmentLayout = new QHBoxLayout(treatmentGroup);

    treatmentCombo = new QComboBox(treatmentGroup);
    treatmentCombo->addItem("简单包扎：恢复 30 HP，原价 20 金币");
    treatmentCombo->addItem("全面治疗：恢复 80 HP，原价 50 金币");
    treatmentCombo->addItem("紧急救治：恢复全部 HP，原价 100 金币");

    costPreviewLabel = new QLabel(treatmentGroup);
    costPreviewLabel->setObjectName("costLabel");
    costPreviewLabel->setMinimumWidth(220);

    treatmentLayout->addWidget(treatmentCombo, 1);
    treatmentLayout->addWidget(costPreviewLabel);
    mainLayout->addWidget(treatmentGroup);

    auto *gameGroup = new QGroupBox("急救小游戏：按正确顺序完成操作，可减少治疗费用", this);
    auto *gameLayout = new QVBoxLayout(gameGroup);

    gameStepLabel = new QLabel("正确顺序：清理伤口 → 消毒 → 上药 → 包扎", gameGroup);
    gameStepLabel->setAlignment(Qt::AlignCenter);
    gameStepLabel->setObjectName("importantLabel");

    auto *buttonGrid = new QGridLayout();
    buttonGrid->setHorizontalSpacing(12);
    buttonGrid->setVerticalSpacing(10);

    const QStringList stepNames = {"清理伤口", "消毒", "上药", "包扎"};
    for (int i = 0; i < stepNames.size(); ++i) {
        auto *button = new QPushButton(stepNames[i], gameGroup);
        button->setMinimumHeight(44);
        button->setProperty("gameButton", true);
        buttonGrid->addWidget(button, i / 2, i % 2);
        gameButtons.append(button);

        connect(button, &QPushButton::clicked, this, [this, button]() {
            handleGameButton(button);
        });
    }

    gameStatusLabel = new QLabel("完成诊断后即可开始小游戏。", gameGroup);
    gameStatusLabel->setAlignment(Qt::AlignCenter);
    gameStatusLabel->setWordWrap(true);

    discountLabel = new QLabel("当前费用减免：0%", gameGroup);
    discountLabel->setAlignment(Qt::AlignCenter);
    discountLabel->setObjectName("discountLabel");

    startGameButton = new QPushButton("开始急救小游戏", gameGroup);
    startGameButton->setObjectName("secondaryButton");

    gameLayout->addWidget(gameStepLabel);
    gameLayout->addLayout(buttonGrid);
    gameLayout->addWidget(gameStatusLabel);
    gameLayout->addWidget(discountLabel);
    gameLayout->addWidget(startGameButton);
    mainLayout->addWidget(gameGroup, 1);

    auto *bottomLayout = new QHBoxLayout();
    auto *closeButton = new QPushButton("暂时离开", this);
    treatButton = new QPushButton("确认治疗", this);
    treatButton->setObjectName("primaryButton");
    treatButton->setEnabled(false);

    bottomLayout->addStretch();
    bottomLayout->addWidget(closeButton);
    bottomLayout->addWidget(treatButton);
    mainLayout->addLayout(bottomLayout);

    connect(diagnoseButton, &QPushButton::clicked,
            this, &FirstAidCenterDialog::runDiagnosis);
    connect(startGameButton, &QPushButton::clicked,
            this, &FirstAidCenterDialog::startMiniGame);
    connect(treatButton, &QPushButton::clicked,
            this, &FirstAidCenterDialog::performTreatment);
    connect(closeButton, &QPushButton::clicked,
            this, &FirstAidCenterDialog::reject);
    connect(treatmentCombo, &QComboBox::currentIndexChanged,
            this, [this](int) {
                resetMiniGame();
                updateCostPreview();
            });
}

void FirstAidCenterDialog::applyStyle()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #f7f3ff;
            color: #3d3153;
            font-family: "Microsoft YaHei";
            font-size: 14px;
        }

        QLabel#titleLabel {
            color: #654ea3;
            font-size: 27px;
            font-weight: 700;
            padding: 4px;
        }

        QGroupBox {
            background-color: rgba(255, 255, 255, 225);
            border: 2px solid #cbb9ee;
            border-radius: 14px;
            margin-top: 12px;
            padding: 14px 12px 10px 12px;
            font-weight: 600;
            color: #5c4785;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 14px;
            padding: 0 7px;
            background-color: #f7f3ff;
        }

        QLabel#importantLabel,
        QLabel#recommendationLabel {
            color: #56407c;
            font-weight: 600;
        }

        QLabel#costLabel,
        QLabel#discountLabel {
            color: #8b4f9f;
            font-weight: 700;
        }

        QComboBox {
            min-height: 34px;
            padding: 3px 10px;
            border: 1px solid #bda8df;
            border-radius: 8px;
            background-color: white;
        }

        QPushButton {
            min-height: 36px;
            padding: 4px 18px;
            border: 1px solid #b8a4d8;
            border-radius: 9px;
            background-color: #eee7fa;
            color: #4d3a6c;
            font-weight: 600;
        }

        QPushButton:hover {
            background-color: #e2d6f6;
        }

        QPushButton:disabled {
            color: #a8a0b3;
            background-color: #ece9f0;
            border-color: #d8d3de;
        }

        QPushButton#primaryButton {
            background-color: #8064b3;
            color: white;
            border-color: #8064b3;
        }

        QPushButton#primaryButton:hover {
            background-color: #6d52a0;
        }

        QPushButton#secondaryButton {
            background-color: #dcecff;
            color: #375f8f;
            border-color: #a9c7e8;
        }

        QPushButton[gameButton="true"] {
            background-color: #fffaf0;
            border-color: #e4c995;
            color: #745724;
        }

        QProgressBar {
            min-height: 22px;
            border: 1px solid #bda8df;
            border-radius: 10px;
            background-color: #eeeaf4;
            text-align: center;
            color: #3d3153;
        }

        QProgressBar::chunk {
            border-radius: 9px;
            background-color: #83c9a5;
        }
    )");
}

void FirstAidCenterDialog::updateCharacterDisplay()
{
    hpValueLabel->setText(
        "生命值：" + QString::number(player.getHp())
        + " / " + QString::number(player.getMaxHp()));
    goldValueLabel->setText("持有金币：" + QString::number(player.getGold()));

    hpProgressBar->setMaximum(player.getMaxHp());
    hpProgressBar->setValue(player.getHp());
    hpProgressBar->setFormat("%v / %m HP");

    updateCostPreview();
}

void FirstAidCenterDialog::updateCostPreview()
{
    const TreatmentPlan plan = selectedTreatment();
    const int finalCost = discountedCost(plan.baseCost);

    costPreviewLabel->setText(
        "原价：" + QString::number(plan.baseCost)
        + "　实付：" + QString::number(finalCost) + " 金币");
}

void FirstAidCenterDialog::runDiagnosis()
{
    const int hp = player.getHp();
    const int maxHp = player.getMaxHp();
    const int missingHp = maxHp - hp;
    const double hpRatio = maxHp > 0
                               ? static_cast<double>(hp) / static_cast<double>(maxHp)
                               : 0.0;

    diagnosed = true;
    resetMiniGame();
    treatButton->setEnabled(hp < maxHp);
    startGameButton->setEnabled(hp < maxHp);

    if (missingHp <= 0) {
        diagnosisLabel->setText("检查结果：各项生命体征正常，当前生命值已满。");
        recommendationLabel->setText("医生建议：无需治疗，注意休息即可。");
        return;
    }

    if (hpRatio >= 0.75) {
        diagnosisLabel->setText("检查结果：轻微擦伤，精神状态良好。");
    } else if (hpRatio >= 0.45) {
        diagnosisLabel->setText("检查结果：中度受伤，需要及时处理伤口。");
    } else if (hpRatio >= 0.20) {
        diagnosisLabel->setText("检查结果：伤势较重，建议立即接受全面治疗。");
    } else {
        diagnosisLabel->setText("检查结果：生命值过低，当前处于危急状态！");
    }

    if (missingHp <= 30) {
        treatmentCombo->setCurrentIndex(0);
        recommendationLabel->setText("医生建议：简单包扎即可恢复基本状态。");
    } else if (missingHp <= 80) {
        treatmentCombo->setCurrentIndex(1);
        recommendationLabel->setText("医生建议：选择全面治疗，避免伤势加重。");
    } else {
        treatmentCombo->setCurrentIndex(2);
        recommendationLabel->setText("医生建议：进行紧急救治，恢复全部生命值。");
    }

    updateCostPreview();
}

void FirstAidCenterDialog::startMiniGame()
{
    if (!diagnosed) {
        QMessageBox::information(this, "急救小游戏", "请先完成医生诊断。");
        return;
    }

    if (player.getHp() >= player.getMaxHp()) {
        QMessageBox::information(this, "急救小游戏", "当前生命值已满，不需要治疗。");
        return;
    }

    miniGameStarted = true;
    miniGameFinished = false;
    currentStep = 0;
    errorCount = 0;
    discountPercent = 0;

    QStringList labels = {"清理伤口", "消毒", "上药", "包扎"};
    std::mt19937 engine(QRandomGenerator::global()->generate());
    std::shuffle(labels.begin(), labels.end(), engine);

    for (int i = 0; i < gameButtons.size(); ++i) {
        gameButtons[i]->setText(labels[i]);
        gameButtons[i]->setEnabled(true);
    }

    gameStatusLabel->setText("第 1 步：请选择“清理伤口”。错误次数：0");
    discountLabel->setText("当前费用减免：0%（完成后结算）");
    startGameButton->setEnabled(false);
    updateCostPreview();
}

void FirstAidCenterDialog::handleGameButton(QPushButton *button)
{
    if (!miniGameStarted || miniGameFinished || !button) {
        return;
    }

    static const QStringList correctOrder = {
        "清理伤口", "消毒", "上药", "包扎"
    };

    const QString selectedStep = button->text();
    const QString expectedStep = correctOrder[currentStep];

    if (selectedStep == expectedStep) {
        button->setEnabled(false);
        ++currentStep;

        if (currentStep >= correctOrder.size()) {
            miniGameFinished = true;

            if (errorCount == 0) {
                discountPercent = 30;
            } else if (errorCount == 1) {
                discountPercent = 20;
            } else if (errorCount == 2) {
                discountPercent = 10;
            } else {
                discountPercent = 0;
            }

            for (QPushButton *gameButton : gameButtons) {
                gameButton->setEnabled(false);
            }

            gameStatusLabel->setText(
                "急救操作完成！共出现 " + QString::number(errorCount)
                + " 次错误。");
            discountLabel->setText(
                "小游戏结算：治疗费用减免 "
                + QString::number(discountPercent) + "%");
            updateCostPreview();
            return;
        }

        gameStatusLabel->setText(
            "操作正确！第 " + QString::number(currentStep + 1)
            + " 步：请选择“" + correctOrder[currentStep]
            + "”。错误次数：" + QString::number(errorCount));
    } else {
        ++errorCount;
        gameStatusLabel->setText(
            "顺序错误！当前应先进行“" + expectedStep
            + "”。错误次数：" + QString::number(errorCount));
    }
}

void FirstAidCenterDialog::resetMiniGame()
{
    miniGameStarted = false;
    miniGameFinished = false;
    currentStep = 0;
    errorCount = 0;
    discountPercent = 0;

    for (QPushButton *button : gameButtons) {
        button->setEnabled(false);
    }

    gameStatusLabel->setText(
        diagnosed ? "可直接治疗，也可以先完成小游戏获得费用减免。"
                  : "完成诊断后即可开始小游戏。");
    discountLabel->setText("当前费用减免：0%");
    startGameButton->setEnabled(diagnosed && player.getHp() < player.getMaxHp());
    updateCostPreview();
}

void FirstAidCenterDialog::performTreatment()
{
    if (!diagnosed) {
        QMessageBox::information(this, "治疗提示", "请先完成医生诊断。");
        return;
    }

    if (player.getHp() >= player.getMaxHp()) {
        QMessageBox::information(this, "治疗提示", "当前生命值已满，不需要治疗。");
        return;
    }

    const TreatmentPlan plan = selectedTreatment();
    const int finalCost = discountedCost(plan.baseCost);

    if (!player.spendGold(finalCost)) {
        QMessageBox::warning(
            this,
            "治疗失败",
            "当前金币不足。\n需要 " + QString::number(finalCost)
                + " 金币，你只有 " + QString::number(player.getGold()) + " 金币。");
        return;
    }

    const int oldHp = player.getHp();
    player.heal(plan.fullHeal ? player.getMaxHp() : plan.healValue);
    const int realHeal = player.getHp() - oldHp;

    QString discountDescription = "未参加小游戏";
    if (miniGameStarted) {
        discountDescription = miniGameFinished
                                  ? "小游戏减免 " + QString::number(discountPercent) + "%"
                                  : "小游戏未完成，无费用减免";
    }

    summaryText = plan.name + "成功：" + discountDescription
                  + "，花费 " + QString::number(finalCost)
                  + " 金币，恢复 " + QString::number(realHeal) + " HP。";
    treatmentDone = true;

    updateCharacterDisplay();

    QMessageBox::information(
        this,
        "治疗完成",
        "治疗成功！\n\n治疗项目：" + plan.name
            + "\n费用：" + QString::number(finalCost) + " 金币"
            + "\n恢复生命值：" + QString::number(realHeal)
            + "\n" + discountDescription);

    accept();
}

FirstAidCenterDialog::TreatmentPlan FirstAidCenterDialog::selectedTreatment() const
{
    switch (treatmentCombo ? treatmentCombo->currentIndex() : 0) {
    case 1:
        return {"全面治疗", 50, 80, false};
    case 2:
        return {"紧急救治", 100, 0, true};
    case 0:
    default:
        return {"简单包扎", 20, 30, false};
    }
}

int FirstAidCenterDialog::discountedCost(int baseCost) const
{
    const int safeDiscount = std::clamp(discountPercent, 0, 100);
    return (baseCost * (100 - safeDiscount) + 99) / 100;
}

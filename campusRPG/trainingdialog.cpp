#include "trainingdialog.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

TrainingDialog::TrainingDialog(int expReward, QWidget *parent)
    : QDialog(parent),
    expReward(expReward)
{
    generateQuestions();
    buildUi();

    countdownTimer = new QTimer(this);
    countdownTimer->setInterval(1000);

    connect(countdownTimer, &QTimer::timeout,
            this, &TrainingDialog::updateCountdown);

    countdownTimer->start();
}

bool TrainingDialog::trainingSucceeded() const
{
    return succeeded;
}

void TrainingDialog::generateQuestions()
{
    QRandomGenerator *random = QRandomGenerator::global();

    for (int index = 0; index < QuestionCount; ++index) {
        // 依次覆盖加、减、乘、除，再随机生成一道，保证题型不会过于单一。
        int operation = index < 4 ? index : random->bounded(4);
        int left = 0;
        int right = 0;

        switch (operation) {
        case 0: // 加法
            left = random->bounded(1, 21);
            right = random->bounded(1, 21);
            questions[index] = {QString("%1 ＋ %2").arg(left).arg(right),
                                left + right};
            break;

        case 1: // 减法，结果不出现负数
            left = random->bounded(6, 31);
            right = random->bounded(1, left + 1);
            questions[index] = {QString("%1 － %2").arg(left).arg(right),
                                left - right};
            break;

        case 2: // 九九乘法范围
            left = random->bounded(2, 10);
            right = random->bounded(2, 10);
            questions[index] = {QString("%1 × %2").arg(left).arg(right),
                                left * right};
            break;

        default: { // 整除题，答案始终为整数
            int quotient = random->bounded(2, 10);
            right = random->bounded(2, 10);
            left = quotient * right;
            questions[index] = {QString("%1 ÷ %2").arg(left).arg(right),
                                quotient};
            break;
        }
        }
    }
}

void TrainingDialog::buildUi()
{
    setWindowTitle("成长训练 · 限时口算");
    setModal(true);
    setMinimumSize(560, 590);
    resize(590, 620);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(16);

    auto *titleLabel = new QLabel("成长训练", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto *ruleLabel = new QLabel(
        QString("请在 30 秒内完成 5 道口算题。五题全部正确可获得 %1 点经验。")
            .arg(expReward),
        this);
    ruleLabel->setObjectName("ruleLabel");
    ruleLabel->setWordWrap(true);
    ruleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(ruleLabel);

    auto *timerLayout = new QHBoxLayout;
    timerLayout->setSpacing(12);

    timeLabel = new QLabel(QString("剩余时间：%1 秒").arg(TrainingSeconds), this);
    timeLabel->setObjectName("timeLabel");
    timeLabel->setMinimumWidth(130);

    timeProgress = new QProgressBar(this);
    timeProgress->setRange(0, TrainingSeconds);
    timeProgress->setValue(TrainingSeconds);
    timeProgress->setTextVisible(false);
    timeProgress->setFixedHeight(16);

    timerLayout->addWidget(timeLabel);
    timerLayout->addWidget(timeProgress, 1);
    mainLayout->addLayout(timerLayout);

    auto *questionFrame = new QFrame(this);
    questionFrame->setObjectName("questionFrame");

    auto *questionLayout = new QGridLayout(questionFrame);
    questionLayout->setContentsMargins(24, 22, 24, 22);
    questionLayout->setHorizontalSpacing(16);
    questionLayout->setVerticalSpacing(14);
    questionLayout->setColumnStretch(1, 1);

    for (int index = 0; index < QuestionCount; ++index) {
        auto *numberLabel = new QLabel(QString::number(index + 1) + ".", questionFrame);
        numberLabel->setObjectName("numberLabel");
        numberLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *questionLabel = new QLabel(questions[index].expression + " ＝", questionFrame);
        questionLabel->setObjectName("questionLabel");
        questionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto *answerEdit = new QLineEdit(questionFrame);
        answerEdit->setObjectName("answerEdit");
        answerEdit->setAlignment(Qt::AlignCenter);
        answerEdit->setPlaceholderText("答案");
        answerEdit->setMaxLength(4);
        answerEdit->setValidator(new QIntValidator(-999, 999, answerEdit));
        answerEdit->setMinimumHeight(42);
        answerEdit->setMaximumWidth(150);
        answerEdits[index] = answerEdit;

        connect(answerEdit, &QLineEdit::returnPressed,
                this, &TrainingDialog::checkAnswers);

        connect(answerEdit, &QLineEdit::textChanged,
                this, [answerEdit]() {
                    answerEdit->setProperty("answerState", "normal");
                    answerEdit->style()->unpolish(answerEdit);
                    answerEdit->style()->polish(answerEdit);
                });

        questionLayout->addWidget(numberLabel, index, 0);
        questionLayout->addWidget(questionLabel, index, 1);
        questionLayout->addWidget(answerEdit, index, 2);
    }

    mainLayout->addWidget(questionFrame, 1);

    resultLabel = new QLabel("填写完成后点击“检查答案”，答错后可在倒计时内继续修改。", this);
    resultLabel->setObjectName("resultLabel");
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setWordWrap(true);
    mainLayout->addWidget(resultLabel);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

    auto *cancelButton = new QPushButton("放弃训练", this);
    cancelButton->setObjectName("cancelButton");

    submitButton = new QPushButton("检查答案", this);
    submitButton->setObjectName("submitButton");
    submitButton->setDefault(true);

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(submitButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(submitButton, &QPushButton::clicked,
            this, &TrainingDialog::checkAnswers);

    setStyleSheet(R"(
        TrainingDialog {
            background-color: #f8f4ff;
        }

        QLabel#titleLabel {
            color: #694b8f;
            font-size: 28px;
            font-weight: 700;
        }

        QLabel#ruleLabel {
            color: #6f647b;
            font-size: 14px;
            padding: 4px 12px;
        }

        QLabel#timeLabel {
            color: #684d85;
            font-size: 15px;
            font-weight: 700;
        }

        QProgressBar {
            border: 1px solid #d9c9ed;
            border-radius: 7px;
            background-color: #eee7f7;
        }

        QProgressBar::chunk {
            border-radius: 7px;
            background-color: #a98bd0;
        }

        QFrame#questionFrame {
            border: 2px solid #ddcff0;
            border-radius: 18px;
            background-color: rgba(255, 255, 255, 235);
        }

        QLabel#numberLabel,
        QLabel#questionLabel {
            color: #4f405f;
            font-size: 20px;
            font-weight: 600;
        }

        QLineEdit#answerEdit {
            border: 2px solid #cdb9e7;
            border-radius: 10px;
            background-color: white;
            color: #443650;
            font-size: 18px;
            font-weight: 600;
            padding: 5px 10px;
        }

        QLineEdit#answerEdit:focus {
            border-color: #8e67bd;
        }

        QLineEdit#answerEdit[answerState="correct"] {
            border-color: #75b78b;
            background-color: #f0faf3;
        }

        QLineEdit#answerEdit[answerState="wrong"] {
            border-color: #d98181;
            background-color: #fff2f2;
        }

        QLineEdit#answerEdit[answerState="empty"] {
            border-color: #d4a24c;
            background-color: #fff9eb;
        }

        QLabel#resultLabel {
            color: #74667f;
            font-size: 14px;
            min-height: 34px;
        }

        QPushButton {
            min-width: 118px;
            min-height: 40px;
            border-radius: 10px;
            padding: 4px 18px;
            font-size: 15px;
            font-weight: 600;
        }

        QPushButton#submitButton {
            border: none;
            background-color: #8f6cba;
            color: white;
        }

        QPushButton#submitButton:hover {
            background-color: #7f5daa;
        }

        QPushButton#cancelButton {
            border: 1px solid #cbb8df;
            background-color: white;
            color: #71578e;
        }

        QPushButton#cancelButton:hover {
            background-color: #f0e8f8;
        }
    )");

    answerEdits[0]->setFocus();
}

void TrainingDialog::updateCountdown()
{
    if (finished) {
        return;
    }

    --remainingSeconds;

    if (remainingSeconds < 0) {
        remainingSeconds = 0;
    }

    timeLabel->setText(QString("剩余时间：%1 秒").arg(remainingSeconds));
    timeProgress->setValue(remainingSeconds);

    if (remainingSeconds <= 10) {
        timeLabel->setStyleSheet("color: #c45858; font-weight: 700;");
    }

    if (remainingSeconds == 0) {
        finishByTimeout();
    }
}

void TrainingDialog::checkAnswers()
{
    if (finished) {
        return;
    }

    bool allCorrect = true;
    int correctCount = 0;
    QLineEdit *firstProblemEdit = nullptr;

    for (int index = 0; index < QuestionCount; ++index) {
        QLineEdit *edit = answerEdits[index];
        const QString input = edit->text().trimmed();

        if (input.isEmpty()) {
            allCorrect = false;
            edit->setProperty("answerState", "empty");

            if (!firstProblemEdit) {
                firstProblemEdit = edit;
            }
        } else {
            bool conversionOk = false;
            const int answer = input.toInt(&conversionOk);

            if (conversionOk && answer == questions[index].answer) {
                ++correctCount;
                edit->setProperty("answerState", "correct");
            } else {
                allCorrect = false;
                edit->setProperty("answerState", "wrong");

                if (!firstProblemEdit) {
                    firstProblemEdit = edit;
                }
            }
        }

        edit->style()->unpolish(edit);
        edit->style()->polish(edit);
    }

    if (allCorrect) {
        finishSuccessfully();
        return;
    }

    resultLabel->setText(
        QString("当前答对 %1 / %2 题。请修改标记的答案，倒计时仍在继续。")
            .arg(correctCount)
            .arg(QuestionCount));
    resultLabel->setStyleSheet("color: #a26535; font-weight: 600;");

    if (firstProblemEdit) {
        firstProblemEdit->setFocus();
        firstProblemEdit->selectAll();
    }
}

void TrainingDialog::finishSuccessfully()
{
    if (finished) {
        return;
    }

    finished = true;
    succeeded = true;
    countdownTimer->stop();
    setInputsEnabled(false);

    resultLabel->setText(
        QString("训练完成，五题全部正确，将获得 %1 点经验。")
            .arg(expReward));
    resultLabel->setStyleSheet("color: #4e9163; font-weight: 700;");

    QMessageBox::information(
        this,
        "训练成功",
        QString("你在规定时间内答对了全部题目，获得 %1 点经验。")
            .arg(expReward));

    accept();
}

void TrainingDialog::finishByTimeout()
{
    if (finished) {
        return;
    }

    finished = true;
    succeeded = false;
    countdownTimer->stop();
    setInputsEnabled(false);

    resultLabel->setText("时间已到。本次训练未完成，不获得经验。");
    resultLabel->setStyleSheet("color: #b05757; font-weight: 700;");

    QMessageBox::warning(
        this,
        "训练超时",
        "30 秒倒计时已结束。由于五道题未全部答对，本次不获得经验。"
        );

    reject();
}

void TrainingDialog::setInputsEnabled(bool enabled)
{
    for (QLineEdit *edit : answerEdits) {
        edit->setEnabled(enabled);
    }

    submitButton->setEnabled(enabled);
}
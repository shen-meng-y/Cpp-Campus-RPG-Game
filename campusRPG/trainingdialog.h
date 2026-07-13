#ifndef TRAININGDIALOG_H
#define TRAININGDIALOG_H

#include <QDialog>
#include <QString>
#include <array>

class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QTimer;

class TrainingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrainingDialog(int expReward, QWidget *parent = nullptr);

    bool trainingSucceeded() const;

private slots:
    void updateCountdown();
    void checkAnswers();

private:
    static constexpr int QuestionCount = 5;
    static constexpr int TrainingSeconds = 30;

    struct Question {
        QString expression;
        int answer = 0;
    };

    void generateQuestions();
    void buildUi();
    void finishSuccessfully();
    void finishByTimeout();
    void setInputsEnabled(bool enabled);

    std::array<Question, QuestionCount> questions;
    std::array<QLineEdit *, QuestionCount> answerEdits{};

    QLabel *timeLabel = nullptr;
    QLabel *resultLabel = nullptr;
    QProgressBar *timeProgress = nullptr;
    QPushButton *submitButton = nullptr;
    QTimer *countdownTimer = nullptr;

    int remainingSeconds = TrainingSeconds;
    int expReward = 0;
    bool succeeded = false;
    bool finished = false;
};

#endif // TRAININGDIALOG_H
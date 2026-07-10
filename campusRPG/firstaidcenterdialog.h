#ifndef FIRSTAIDCENTERDIALOG_H
#define FIRSTAIDCENTERDIALOG_H

#include <QDialog>
#include <QString>
#include <QVector>

#include "Character.h"

class QLabel;
class QProgressBar;
class QComboBox;
class QPushButton;

class FirstAidCenterDialog : public QDialog
{
public:
    explicit FirstAidCenterDialog(Character &player, QWidget *parent = nullptr);

    bool treatmentPerformed() const;
    QString treatmentSummary() const;

private:
    struct TreatmentPlan {
        QString name;
        int baseCost;
        int healValue;
        bool fullHeal;
    };

    Character &player;

    QLabel *hpValueLabel;
    QLabel *goldValueLabel;
    QProgressBar *hpProgressBar;

    QLabel *diagnosisLabel;
    QLabel *recommendationLabel;

    QComboBox *treatmentCombo;
    QLabel *costPreviewLabel;

    QLabel *gameStepLabel;
    QLabel *gameStatusLabel;
    QLabel *discountLabel;
    QPushButton *startGameButton;
    QPushButton *treatButton;
    QVector<QPushButton *> gameButtons;

    bool diagnosed;
    bool miniGameStarted;
    bool miniGameFinished;
    bool treatmentDone;
    int currentStep;
    int errorCount;
    int discountPercent;
    QString summaryText;

    void buildUi();
    void applyStyle();
    void updateCharacterDisplay();
    void updateCostPreview();
    void runDiagnosis();
    void startMiniGame();
    void handleGameButton(QPushButton *button);
    void resetMiniGame();
    void performTreatment();

    TreatmentPlan selectedTreatment() const;
    int discountedCost(int baseCost) const;
};

#endif

#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

class QPushButton;

class StartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartDialog(QWidget *parent = nullptr);

private:
    QPushButton *startButton;
    QPushButton *exitButton;
};

#endif // STARTDIALOG_H
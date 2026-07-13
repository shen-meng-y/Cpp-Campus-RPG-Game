#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>

#include "Character.h"
#include "gamestatistics.h"

class StatisticsDialog : public QDialog
{
public:
    explicit StatisticsDialog(const GameStatistics &statistics,
                              const Character &player,
                              QWidget *parent = nullptr);
};

#endif
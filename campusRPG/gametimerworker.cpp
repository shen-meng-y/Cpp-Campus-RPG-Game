#include "gametimerworker.h"

GameTimerWorker::GameTimerWorker(QObject *parent)
    : QObject(parent)
{
}

void GameTimerWorker::startTiming()
{
    if (!timer) {
        timer = new QTimer(this);
        timer->setInterval(1000);

        connect(timer, &QTimer::timeout, this, [this]() {
            elapsedSeconds++;
            emit gameTimeChanged(elapsedSeconds);
        });
    }

    if (!timer->isActive()) {
        timer->start();
    }
}

void GameTimerWorker::stopTiming()
{
    if (timer) {
        timer->stop();
    }
}

void GameTimerWorker::resetTiming()
{
    elapsedSeconds = 0;
    emit gameTimeChanged(elapsedSeconds);
}

#include "randomeventworker.h"

#include <QRandomGenerator>

RandomEventWorker::RandomEventWorker(QObject *parent)
    : QObject(parent)
{
}

void RandomEventWorker::startEvents()
{
    if (!timer) {
        timer = new QTimer(this);
        timer->setInterval(180000); // 3分钟触发一次随机事件

        connect(timer, &QTimer::timeout,
                this, &RandomEventWorker::generateRandomEvent);
    }

    if (!timer->isActive()) {
        timer->start();
    }
}

void RandomEventWorker::stopEvents()
{
    if (timer) {
        timer->stop();
    }
}

void RandomEventWorker::generateRandomEvent()
{
    int eventType = QRandomGenerator::global()->bounded(6);
    int value = 0;
    int enemyIndex = -1;
    QString itemName;

    if (eventType == 0) {
        value = QRandomGenerator::global()->bounded(15, 61);
    } else if (eventType == 1) {
        value = QRandomGenerator::global()->bounded(10, 41);
    } else if (eventType == 2) {
        value = 60;
        itemName = "药水";
    } else if (eventType == 3) {
        value = QRandomGenerator::global()->bounded(20, 51);
    } else if (eventType == 4) {
        value = QRandomGenerator::global()->bounded(8, 26);
    } else {
        enemyIndex = QRandomGenerator::global()->bounded(0, 3);
    }

    emit randomEventTriggered(eventType, value, enemyIndex, itemName);
}

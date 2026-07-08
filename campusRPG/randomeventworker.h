#ifndef RANDOMEVENTWORKER_H
#define RANDOMEVENTWORKER_H

#include <QObject>
#include <QString>
#include <QTimer>

class RandomEventWorker : public QObject
{
    Q_OBJECT

public:
    explicit RandomEventWorker(QObject *parent = nullptr);

public slots:
    void startEvents();
    void stopEvents();

signals:
    void randomEventTriggered(int eventType, int value, int enemyIndex, QString itemName);

private:
    QTimer *timer = nullptr;

    void generateRandomEvent();
};

#endif // RANDOMEVENTWORKER_H

#ifndef GAMETIMERWORKER_H
#define GAMETIMERWORKER_H

#include <QObject>
#include <QTimer>

class GameTimerWorker : public QObject
{
    Q_OBJECT

public:
    explicit GameTimerWorker(QObject *parent = nullptr);

public slots:
    void startTiming();
    void stopTiming();
    void resetTiming();

signals:
    void gameTimeChanged(int seconds);

private:
    QTimer *timer = nullptr;
    int elapsedSeconds = 0;
};

#endif // GAMETIMERWORKER_H

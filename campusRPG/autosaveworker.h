#ifndef AUTOSAVEWORKER_H
#define AUTOSAVEWORKER_H

#include <QObject>
#include <QString>

#include "gamesnapshot.h"

class AutoSaveWorker : public QObject
{
    Q_OBJECT

public:
    explicit AutoSaveWorker(QObject *parent = nullptr);

public slots:
    void saveSnapshot(GameSnapshot snapshot);

signals:
    void autoSaveFinished(bool success, QString message);
};

#endif // AUTOSAVEWORKER_H
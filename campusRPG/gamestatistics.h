#ifndef GAMESTATISTICS_H
#define GAMESTATISTICS_H

#include <QMap>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "Character.h"

struct GrowthRecord
{
    qint64 playSeconds = 0;
    int level = 1;
    int experience = 0;
    int maxHp = 100;
    int attack = 15;
    QString recordedAt;
    QString reason;
};

class GameStatistics
{
public:
    QString characterName;
    qint64 totalPlaySeconds = 0;

    int totalBattles = 0;
    int battleWins = 0;
    int battleDefeats = 0;
    int battleEscapes = 0;

    int totalKills = 0;
    int questsCompleted = 0;

    int goldEarned = 0;
    int goldSpent = 0;

    QMap<QString, int> enemyKillCounts;
    QVector<GrowthRecord> growthHistory;

    void reset(const QString &name);
    void recordGrowth(const Character &player,
                      qint64 playSeconds,
                      const QString &reason);

    double winRate() const;
    QString favoriteEnemy() const;

    bool save(QString *errorMessage = nullptr) const;
    static GameStatistics loadForCharacter(const QString &name,
                                           bool *loaded = nullptr,
                                           QString *errorMessage = nullptr);

private:
    static QString storageFilePath(const QString &characterName);
};

#endif

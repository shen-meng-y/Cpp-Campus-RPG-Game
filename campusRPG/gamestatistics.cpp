#include "gamestatistics.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>

namespace {
constexpr int MaxGrowthRecords = 500;

QJsonObject growthRecordToJson(const GrowthRecord &record)
{
    QJsonObject object;
    object["playSeconds"] = static_cast<double>(record.playSeconds);
    object["level"] = record.level;
    object["experience"] = record.experience;
    object["maxHp"] = record.maxHp;
    object["attack"] = record.attack;
    object["recordedAt"] = record.recordedAt;
    object["reason"] = record.reason;
    return object;
}

GrowthRecord growthRecordFromJson(const QJsonObject &object)
{
    GrowthRecord record;
    record.playSeconds = static_cast<qint64>(object.value("playSeconds").toDouble());
    record.level = std::max(1, object.value("level").toInt(1));
    record.experience = std::max(0, object.value("experience").toInt());
    record.maxHp = std::max(1, object.value("maxHp").toInt(100));
    record.attack = std::max(1, object.value("attack").toInt(15));
    record.recordedAt = object.value("recordedAt").toString();
    record.reason = object.value("reason").toString();
    return record;
}
}

void GameStatistics::reset(const QString &name)
{
    *this = GameStatistics();
    characterName = name.trimmed();
}

void GameStatistics::recordGrowth(const Character &player,
                                  qint64 playSeconds,
                                  const QString &reason)
{
    GrowthRecord record;
    record.playSeconds = std::max<qint64>(0, playSeconds);
    record.level = player.getLevel();
    record.experience = player.getExp();
    record.maxHp = player.getMaxHp();
    record.attack = player.getAttack();
    record.recordedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    record.reason = reason;

    if (!growthHistory.isEmpty()) {
        const GrowthRecord &last = growthHistory.constLast();
        const bool valuesUnchanged =
            last.level == record.level
            && last.experience == record.experience
            && last.maxHp == record.maxHp
            && last.attack == record.attack;

        if (valuesUnchanged) {
            return;
        }
    }

    growthHistory.append(record);

    while (growthHistory.size() > MaxGrowthRecords) {
        growthHistory.removeFirst();
    }
}

double GameStatistics::winRate() const
{
    if (totalBattles <= 0) {
        return 0.0;
    }

    return static_cast<double>(battleWins) * 100.0
           / static_cast<double>(totalBattles);
}

QString GameStatistics::favoriteEnemy() const
{
    QString result;
    int highestCount = 0;

    for (auto it = enemyKillCounts.constBegin();
         it != enemyKillCounts.constEnd(); ++it) {
        if (it.value() > highestCount) {
            highestCount = it.value();
            result = it.key();
        }
    }

    return result;
}

bool GameStatistics::save(QString *errorMessage) const
{
    if (characterName.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "角色名称为空，无法保存统计数据。";
        }
        return false;
    }

    QJsonObject root;
    root["version"] = 1;
    root["characterName"] = characterName;
    root["totalPlaySeconds"] = static_cast<double>(totalPlaySeconds);
    root["totalBattles"] = totalBattles;
    root["battleWins"] = battleWins;
    root["battleDefeats"] = battleDefeats;
    root["battleEscapes"] = battleEscapes;
    root["totalKills"] = totalKills;
    root["questsCompleted"] = questsCompleted;
    root["goldEarned"] = goldEarned;
    root["goldSpent"] = goldSpent;

    QJsonObject enemyKills;
    for (auto it = enemyKillCounts.constBegin();
         it != enemyKillCounts.constEnd(); ++it) {
        enemyKills[it.key()] = it.value();
    }
    root["enemyKillCounts"] = enemyKills;

    QJsonArray growthArray;
    for (const GrowthRecord &record : growthHistory) {
        growthArray.append(growthRecordToJson(record));
    }
    root["growthHistory"] = growthArray;

    const QString filePath = storageFilePath(characterName);
    QSaveFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = "无法打开统计文件：" + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = "统计文件写入失败：" + file.errorString();
        }
        return false;
    }

    return true;
}

GameStatistics GameStatistics::loadForCharacter(const QString &name,
                                                bool *loaded,
                                                QString *errorMessage)
{
    GameStatistics statistics;
    statistics.reset(name);

    if (loaded) {
        *loaded = false;
    }

    QFile file(storageFilePath(name));
    if (!file.exists()) {
        return statistics;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "无法读取统计文件：" + file.errorString();
        }
        return statistics;
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError
        || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = "统计文件格式错误：" + parseError.errorString();
        }
        return statistics;
    }

    const QJsonObject root = document.object();
    statistics.characterName = root.value("characterName").toString(name);
    statistics.totalPlaySeconds =
        std::max<qint64>(0, static_cast<qint64>(
                                root.value("totalPlaySeconds").toDouble()));
    statistics.totalBattles = std::max(0, root.value("totalBattles").toInt());
    statistics.battleWins = std::max(0, root.value("battleWins").toInt());
    statistics.battleDefeats = std::max(0, root.value("battleDefeats").toInt());
    statistics.battleEscapes = std::max(0, root.value("battleEscapes").toInt());
    statistics.totalKills = std::max(0, root.value("totalKills").toInt());
    statistics.questsCompleted =
        std::max(0, root.value("questsCompleted").toInt());
    statistics.goldEarned = std::max(0, root.value("goldEarned").toInt());
    statistics.goldSpent = std::max(0, root.value("goldSpent").toInt());

    const QJsonObject enemyKills = root.value("enemyKillCounts").toObject();
    for (auto it = enemyKills.constBegin(); it != enemyKills.constEnd(); ++it) {
        statistics.enemyKillCounts[it.key()] = std::max(0, it.value().toInt());
    }

    const QJsonArray growthArray = root.value("growthHistory").toArray();
    for (const QJsonValue &value : growthArray) {
        if (value.isObject()) {
            statistics.growthHistory.append(
                growthRecordFromJson(value.toObject()));
        }
    }

    while (statistics.growthHistory.size() > MaxGrowthRecords) {
        statistics.growthHistory.removeFirst();
    }

    const int recordedOutcomes = statistics.battleWins
                                 + statistics.battleDefeats
                                 + statistics.battleEscapes;
    statistics.totalBattles = std::max(statistics.totalBattles,
                                       recordedOutcomes);

    int recordedKills = 0;
    for (int count : statistics.enemyKillCounts) {
        recordedKills += count;
    }
    statistics.totalKills = std::max(statistics.totalKills, recordedKills);

    if (loaded) {
        *loaded = true;
    }

    return statistics;
}

QString GameStatistics::storageFilePath(const QString &characterName)
{
    QString basePath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (basePath.isEmpty()) {
        basePath = QDir::currentPath();
    }

    QDir directory(basePath);
    directory.mkpath("statistics");
    directory.cd("statistics");

    const QByteArray digest =
        QCryptographicHash::hash(characterName.trimmed().toUtf8(),
                                 QCryptographicHash::Sha256)
            .toHex()
            .left(20);

    return directory.filePath(QString("statistics_%1.json")
                                  .arg(QString::fromLatin1(digest)));
}
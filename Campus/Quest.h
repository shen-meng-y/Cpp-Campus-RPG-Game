#ifndef QUEST_H
#define QUEST_H

#include <string>
#include <vector>
#include "Character.h"

enum class QuestType {
    KillEnemy,
    ReachLevel,
    GoldAmount
};

class Quest {
private:
    std::string name;
    std::string description;
    QuestType type;
    int conditionValue;
    int rewardExp;
    int rewardGold;
    bool accepted;
    bool completed;
    bool rewardClaimed;

public:
    Quest();

    Quest(
        const std::string& name,
        const std::string& description,
        QuestType type,
        int conditionValue,
        int rewardExp,
        int rewardGold
        );

    std::string getName() const;
    bool isAccepted() const;
    bool isCompleted() const;
    bool isRewardClaimed() const;

    void showInfo(int index) const;
    void accept();
    void checkComplete(const Character& player, int killCount);
    void claimReward(Character& player);
    void setStatus(bool accepted, bool completed, bool rewardClaimed);

    static std::vector<Quest> createDefaultQuests();
};

#endif
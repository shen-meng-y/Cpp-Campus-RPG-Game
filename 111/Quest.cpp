#include "Quest.h"
#include <iostream>

Quest::Quest()
    : name("默认任务"),
      description("无"),
      type(QuestType::KillEnemy),
      conditionValue(1),
      rewardExp(10),
      rewardGold(10),
      accepted(false),
      completed(false),
      rewardClaimed(false) {}

Quest::Quest(
    const std::string& name,
    const std::string& description,
    QuestType type,
    int conditionValue,
    int rewardExp,
    int rewardGold
)
    : name(name),
      description(description),
      type(type),
      conditionValue(conditionValue),
      rewardExp(rewardExp),
      rewardGold(rewardGold),
      accepted(false),
      completed(false),
      rewardClaimed(false) {}

std::string Quest::getName() const {
    return name;
}

bool Quest::isAccepted() const {
    return accepted;
}

bool Quest::isCompleted() const {
    return completed;
}

bool Quest::isRewardClaimed() const {
    return rewardClaimed;
}

void Quest::showInfo(int index) const {
    std::cout << index << ". " << name << std::endl;
    std::cout << "   任务描述：" << description << std::endl;
    std::cout << "   奖励内容：经验 " << rewardExp
              << "，金币 " << rewardGold << std::endl;

    std::cout << "   当前状态：";

    if (!accepted) {
        std::cout << "未接受";
    } else if (rewardClaimed) {
        std::cout << "已领奖";
    } else if (completed) {
        std::cout << "已完成，可领奖";
    } else {
        std::cout << "进行中";
    }

    std::cout << "\n";
}

void Quest::accept() {
    if (accepted) {
        std::cout << "该任务已经接受过。\n";
        return;
    }

    accepted = true;
    std::cout << "任务接受成功：" << name << std::endl;
}

void Quest::checkComplete(const Character& player, int killCount) {
    if (!accepted || completed) {
        return;
    }

    switch (type) {
        case QuestType::KillEnemy:
            if (killCount >= conditionValue) {
                completed = true;
            }
            break;

        case QuestType::ReachLevel:
            if (player.getLevel() >= conditionValue) {
                completed = true;
            }
            break;

        case QuestType::GoldAmount:
            if (player.getGold() >= conditionValue) {
                completed = true;
            }
            break;
    }
}

void Quest::claimReward(Character& player) {
    if (!accepted) {
        std::cout << "请先接受该任务。\n";
        return;
    }

    if (!completed) {
        std::cout << "任务尚未完成，不能领取奖励。\n";
        return;
    }

    if (rewardClaimed) {
        std::cout << "该任务奖励已经领取过。\n";
        return;
    }

    player.addExp(rewardExp);
    player.addGold(rewardGold);
    rewardClaimed = true;

    std::cout << "领取奖励成功！获得经验 "
              << rewardExp
              << "，金币 "
              << rewardGold
              << "。\n";
}

void Quest::setStatus(bool accepted, bool completed, bool rewardClaimed) {
    this->accepted = accepted;
    this->completed = completed;
    this->rewardClaimed = rewardClaimed;
}

std::vector<Quest> Quest::createDefaultQuests() {
    return {
        Quest("清理校园", "击败 1 个敌人。", QuestType::KillEnemy, 1, 40, 20),
        Quest("努力成长", "角色等级达到 2 级。", QuestType::ReachLevel, 2, 60, 30),
        Quest("金币积累", "金币数量达到 150。", QuestType::GoldAmount, 150, 30, 40)
    };
}
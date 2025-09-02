#include "quest.h"

// 更新任务目标进度
bool QuestObjective::update(int amount) {
    if (is_completed) {
        return false;
    }

    current_amount += amount;
    if (current_amount >= required_amount) {
        current_amount = required_amount;
        is_completed = true;
    }
    return true;
}

// 获取目标进度文本
std::string QuestObjective::getProgressText() const {
    return description + " (" + std::to_string(current_amount) + "/" + std::to_string(required_amount) + ")";
}

// 构造函数
Quest::Quest(const std::string& questId, const std::string& questName, const std::string& questDesc)
    : id(questId), name(questName), description(questDesc), status(QuestStatus::NOT_STARTED), 
      reward(""), exp_reward(0), gold_reward(0) {
}

// 添加任务目标
void Quest::addObjective(const QuestObjective& objective) {
    objectives.push_back(objective);
}

// 更新任务目标进度
bool Quest::updateObjective(int objectiveIndex, int amount) {
    if (status != QuestStatus::IN_PROGRESS) {
        return false;
    }

    if (objectiveIndex < 0 || objectiveIndex >= static_cast<int>(objectives.size())) {
        return false;
    }

    return objectives[objectiveIndex].update(amount);
}

// 检查任务是否完成
bool Quest::isCompleted() const {
    if (status == QuestStatus::COMPLETED) {
        return true;
    }

    if (status != QuestStatus::IN_PROGRESS) {
        return false;
    }

    for (const auto& objective : objectives) {
        if (!objective.is_completed) {
            return false;
        }
    }

    return true;
}

// 获取任务状态字符串
std::string Quest::getStatusString() const {
    switch (status) {
        case QuestStatus::NOT_STARTED:
            return "未开始";
        case QuestStatus::IN_PROGRESS:
            return "进行中";
        case QuestStatus::COMPLETED:
            return "已完成";
        case QuestStatus::FAILED:
            return "已失败";
        default:
            return "未知状态";
    }
}

// 开始任务
void Quest::start() {
    if (status == QuestStatus::NOT_STARTED) {
        status = QuestStatus::IN_PROGRESS;
    }
}

// 完成任务
bool Quest::complete() {
    if (!isCompleted()) {
        return false;
    }

    status = QuestStatus::COMPLETED;
    return true;
}

// 失败任务
void Quest::fail() {
    if (status == QuestStatus::IN_PROGRESS) {
        status = QuestStatus::FAILED;
    }
}
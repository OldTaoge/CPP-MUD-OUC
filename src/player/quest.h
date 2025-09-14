#pragma once
#include <string>
#include <vector>

// 任务状态枚举
enum class QuestStatus {
    NOT_STARTED,
    IN_PROGRESS,
    COMPLETED,
    FAILED
};

// 任务目标类
struct QuestObjective {
    std::string description;     // 目标描述
    int current_amount;          // 当前进度
    int required_amount;         // 所需数量
    bool is_completed;           // 是否完成

    QuestObjective(const std::string& desc, int required = 1) 
        : description(desc), current_amount(0), required_amount(required), is_completed(false) {}

    // 更新目标进度
    bool update(int amount = 1);

    // 获取目标进度文本
    std::string getProgressText() const;
};

// 任务类
class Quest {
public:
    std::string id;              // 任务ID
    std::string name;            // 任务名称
    std::string description;     // 任务描述
    QuestStatus status;          // 任务状态
    std::vector<QuestObjective> objectives;  // 任务目标列表
    std::string reward;          // 任务奖励
    int exp_reward;              // 经验奖励
    int gold_reward;             // 金币奖励

    Quest(const std::string& questId, const std::string& questName, const std::string& questDesc);

    // 添加任务目标
    void addObjective(const QuestObjective& objective);

    // 更新任务目标进度
    bool updateObjective(int objectiveIndex, int amount = 1);

    // 检查任务是否完成
    bool isCompleted() const;

    // 获取任务状态字符串
    std::string getStatusString() const;

    // 开始任务
    void start();

    // 完成任务
    bool complete();

    // 失败任务
    void fail();
};
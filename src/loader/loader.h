#ifndef CPP_MUD_OUC_LOADER_H
#define CPP_MUD_OUC_LOADER_H

#include <string>
#include <vector>
#include <map>

namespace game {

    struct Skill {
        int id;
        std::string name;
        double damage_multiplier;
        int cooldown;
    };

    struct Character {
        int id;
        std::string name;
        int level;
        int hp;
        int attack;
        int defense;
        std::vector<Skill> skills;
    };

    struct ItemDrop {
        int item_id;
        std::string name;
        double chance;
    };

    struct Enemy {
        int id;
        std::string name;
        int level;
        int hp;
        int attack;
        int defense;
        std::vector<ItemDrop> drops;
    };

    struct Weapon {
        int id;
        std::string name;
        std::string type;
        int attack_bonus;
    };

    struct QuestObjective {
        std::string type;
        std::string target;
        int target_id;
        int count;
        std::string description;
    };
    
    struct QuestRewardItem {
        int item_id;
        std::string name;
        int count;
    };

    struct QuestRewards {
        int exp;
        std::vector<QuestRewardItem> items;
    };

    struct Quest {
        int id;
        std::string title;
        std::string description;
        std::vector<QuestObjective> objectives;
        QuestRewards rewards;
    };

    class GameDataLoader {
    public:
        GameDataLoader(const std::string& data_path);

        bool loadAllData();

        const std::vector<Character>& getCharacters() const;
        const std::vector<Enemy>& getEnemies() const;
        const std::vector<Weapon>& getWeapons() const;
        const std::vector<Quest>& getQuests() const;

    private:
        std::string data_path_;
        std::vector<Character> characters_;
        std::vector<Enemy> enemies_;
        std::vector<Weapon> weapons_;
        std::vector<Quest> quests_;

        template<typename T>
        bool loadJsonData(const std::string& filename, T& data_container);
    };

} // namespace game

#endif //CPP_MUD_OUC_LOADER_H

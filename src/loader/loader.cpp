#include "loader.h"
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace game {

    // JSON to C++ struct conversions
    void from_json(const json& j, Skill& s) {
        j.at("id").get_to(s.id);
        j.at("name").get_to(s.name);
        j.at("damage_multiplier").get_to(s.damage_multiplier);
        if (j.contains("cooldown")) {
            j.at("cooldown").get_to(s.cooldown);
        } else {
            s.cooldown = 0;
        }
    }

    void from_json(const json& j, Character& c) {
        j.at("id").get_to(c.id);
        j.at("name").get_to(c.name);
        j.at("level").get_to(c.level);
        j.at("hp").get_to(c.hp);
        j.at("attack").get_to(c.attack);
        j.at("defense").get_to(c.defense);
        j.at("skills").get_to(c.skills);
    }

    void from_json(const json& j, ItemDrop& d) {
        j.at("item_id").get_to(d.item_id);
        j.at("name").get_to(d.name);
        j.at("chance").get_to(d.chance);
    }

    void from_json(const json& j, Enemy& e) {
        j.at("id").get_to(e.id);
        j.at("name").get_to(e.name);
        j.at("level").get_to(e.level);
        j.at("hp").get_to(e.hp);
        j.at("attack").get_to(e.attack);
        j.at("defense").get_to(e.defense);
        j.at("drops").get_to(e.drops);
    }

    void from_json(const json& j, Weapon& w) {
        j.at("id").get_to(w.id);
        j.at("name").get_to(w.name);
        j.at("type").get_to(w.type);
        j.at("attack_bonus").get_to(w.attack_bonus);
    }

    void from_json(const json& j, QuestObjective& o) {
        j.at("type").get_to(o.type);
        if (j.contains("target")) {
            j.at("target").get_to(o.target);
        }
        if (j.contains("target_id")) {
            j.at("target_id").get_to(o.target_id);
        } else {
            o.target_id = 0;
        }
        if (j.contains("count")) {
            j.at("count").get_to(o.count);
        } else {
            o.count = 0;
        }
        j.at("description").get_to(o.description);
    }
    
    void from_json(const json& j, QuestRewardItem& i) {
        j.at("item_id").get_to(i.item_id);
        j.at("name").get_to(i.name);
        j.at("count").get_to(i.count);
    }

    void from_json(const json& j, QuestRewards& r) {
        j.at("exp").get_to(r.exp);
        j.at("items").get_to(r.items);
    }

    void from_json(const json& j, Quest& q) {
        j.at("id").get_to(q.id);
        j.at("title").get_to(q.title);
        j.at("description").get_to(q.description);
        j.at("objectives").get_to(q.objectives);
        j.at("rewards").get_to(q.rewards);
    }


    GameDataLoader::GameDataLoader(const std::string& data_path) : data_path_(data_path) {}

    template<typename T>
    bool GameDataLoader::loadJsonData(const std::string& filename, T& data_container) {
        std::ifstream file(data_path_ + "/" + filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }
        try {
            json j;
            file >> j;
            data_container = j.get<T>();
        } catch (json::parse_error& e) {
            std::cerr << "Error: Failed to parse " << filename << ". " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool GameDataLoader::loadAllData() {
        bool success = true;
        success &= loadJsonData("characters.json", characters_);
        success &= loadJsonData("enemies.json", enemies_);
        success &= loadJsonData("weapons.json", weapons_);
        success &= loadJsonData("quests.json", quests_);
        return success;
    }

    const std::vector<Character>& GameDataLoader::getCharacters() const {
        return characters_;
    }

    const std::vector<Enemy>& GameDataLoader::getEnemies() const {
        return enemies_;
    }

    const std::vector<Weapon>& GameDataLoader::getWeapons() const {
        return weapons_;
    }

    const std::vector<Quest>& GameDataLoader::getQuests() const {
        return quests_;
    }

} // namespace game

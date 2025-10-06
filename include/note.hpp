#pragma once

#include <iostream> // for cin
#include <stdio.h>
#include <string>
#include <unordered_set>
#include <vector>
#include <fstream>

#include <nlohmann/json.hpp>

struct Note {
    std::string title;
    std::string content;
    std::vector<std::string> tags;
    std::vector<std::string> children;
    bool show_note{false};
    bool edit_text{false};
};

struct NoteData {
    std::string content;
    std::vector<std::string> tags;
};

class NoteDataManager {
private:
    std::unordered_map<std::string, NoteData> data;
    std::unordered_map<std::string, std::vector<std::string>> children;
public:
    NoteDataManager() {
        std::ifstream file("notes.json");
        if (!file.is_open()) {
            std::cerr << "Failed to open file.\n";
        }

        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << '\n';
        }

        for (const auto& note : j) {
            const auto& title = note["title"];
            const auto& content = note["content"];
            const auto& tags = note["tags"];
            data[title] = {content, tags};
            for (const auto& tag : tags) {
                children[tag].emplace_back(title);
            }
        }
    }

    Note get_note(std::string title, bool visible = false) {
        std::string content;
        std::vector<std::string> tags;
        std::vector<std::string> offspring;
        if (const auto& search = data.find(title); search != data.end()) {
            content = search->second.content;
            tags = search->second.tags;
        }
        if (const auto& search = children.find(title); search != children.end()) {
            offspring = search->second;
        }
        return {title, content, tags, offspring, visible};
    }
};

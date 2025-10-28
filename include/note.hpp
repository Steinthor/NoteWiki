#pragma once

#include <algorithm> // for iter_swap
#include <iostream> // for cin
#include <stdio.h>
#include <string>
#include <unordered_set>
#include <vector>
#include <fstream>

#include <nlohmann/json.hpp>

#include "logger.h"

namespace note {

/**
 * represents raw note data.
 * The title of a note is the key in 'data' object of NoteStore
 */
struct NoteData {
    std::string content;
    std::vector<std::string> tags;
};

std::ostream& operator<<(std::ostream& os, const NoteData& note) {
    std::string tags;
    for(auto& tag: note.tags) tags += tag + ", ";
    return os << "  content: \n    " << note.content << "\n"
              << "  tags: " << tags << std::endl;
}

/**
 * represents a note that should be visible in a UI
 */
struct Note {
    std::string title;
    std::string content;
    std::vector<std::string> tags;
    std::vector<std::string> children;
    bool show_note{false};
    bool edit_text{false};
};

std::ostream& operator<<(std::ostream& os, const Note& note) {
    std::string tags;
    for(auto& tag: note.tags) tags += tag + ", ";
    std::string kids;
    for(auto& kid: note.children) kids += kid + ", ";
    return os << "title: " << note.title << "\n"
              << "  tags: " << tags << "\n"
              << "  content: \n    " << note.content << "\n"
              << "  kids: " << kids << std::endl;
}

/**
 * A placeholder for the strings when editing a note
 */
struct EditingNote {
    std::string title;
    std::string content;
    std::string tags;
    std::string children;
};

/**
 * Copies the content of the input note to ImGuiViewers' editNote.
 */
void copyToEditNote(const Note& note, EditingNote& editNote) {
    editNote.title = note.title;
    editNote.content = note.content;
    editNote.tags.clear();
    for (const auto& tag : note.tags) {
        editNote.tags = editNote.tags + " " + tag;
    }
    editNote.children.clear();
    for (const auto& child : note.children) {
        editNote.children = editNote.children + " " + child;
    }
}

void copyFromEditNote(Note& note, EditingNote editNote) {
    note.title = editNote.title;
    note.content = editNote.content;
    note.tags.clear();
    std::string tag;
    size_t start = 0, next;
    while ((next = editNote.tags.find_first_of(" ", start)) != std::string::npos) {
        tag = editNote.tags.substr(start, next - start);
        if (!tag.empty()) note.tags.emplace_back(tag);
        start = next + 1;
    }
    // check for a final tag
    tag = editNote.tags.substr(start, editNote.tags.size() - start);
    if (!tag.empty()) note.tags.emplace_back(tag);

    note.children.clear();
    std::string child;
    start = 0;
    while ((next = editNote.children.find_first_of(" ", start)) != std::string::npos) {
        child = editNote.children.substr(start, next - start);
        if (!child.empty()) note.children.emplace_back(child);
        start = next + 1;
    }
    // check for final child
    child = editNote.children.substr(start, editNote.children.size() - start);
    if (!child.empty()) note.children.emplace_back(child);
}

class NoteStore {
private:
    std::unordered_map<std::string, NoteData> data;
    std::unordered_map<std::string, std::vector<std::string>> children;
public:
    NoteStore(std::string storage_path) {
        if (!load_json_file(storage_path)) {
            generate_default();
        }
    }

    bool load_json_file(std::string json_file) {
        std::ifstream file(json_file);
        if (!file.is_open()) {
            LOG_ERROR() << "Failed to open file: " << json_file;
            return false;
        }

        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::parse_error& e) {
            LOG_ERROR() << "Parse error: " << e.what();
            return false;
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

        return true;
    }

    void generate_default() {
        std::string title = "NoteWiki";
        std::vector<std::string> tags = {"default"};
        std::string content = "This is the NoteWiki app.\n  Tag any note 'default' to show them on startup.";
        add_note(title, content, tags);
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

    void add_note(std::string title,
                  std::string content,
                  std::vector<std::string> tags) {
        data[title] = {content, tags};
        for (const auto& tag : tags) {
            children[tag].emplace_back(title);
        }
    }

    void update_note(const std::string& old_title,
                     const std::string& new_title,
                     const std::string& content,
                     const std::vector<std::string>& tags) {
        if ((old_title == new_title) && (data[old_title].content == content) && (data[old_title].tags) == tags) {
            return;
        }

        data.erase(old_title);
        data[new_title] = {content, tags};
        LOG_DEBUG() << "update_note: \n" << data[new_title];
        // update the children of a tag, removing old_title, adding the new_title
        for (const auto& tag : tags) {
            // if (children.find(tag) == children.end())

            // continue;
            auto it = std::find(children[tag].begin(), children[tag].end(), old_title);
            if (it != children.at(tag).end()) {
                std::iter_swap(it, children[tag].end() - 1); // Move target to end
                children[tag].pop_back();                    // Remove it
            }
            children[tag].emplace_back(new_title);
        }
    }
};

} // namespace note

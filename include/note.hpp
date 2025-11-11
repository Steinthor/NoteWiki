#pragma once
#include "logger.h"

#include <nlohmann/json.hpp>

#include <algorithm> // for iter_swap
#include <iostream> // for cin
#include <stdio.h>
#include <string>
#include <unordered_set>
#include <vector>
#include <fstream>

namespace note {

using NoteId = uint32_t;

/**
 * represents a note datanode.
 * 'tags' and 'kids' represent a note graph where 'tags' are parents nodes
 * and 'kids' are children nodes.
 */
struct NoteData {
    std::string title;
    std::string content{};
    std::vector<NoteId> tags{};
    std::vector<NoteId> kids{};
};

/**
 * represents a note with all data as strings
 */
struct NoteDataStrings {
    std::string title;
    std::string content{};
    std::vector<std::string> tags{};
    std::vector<std::string> kids{};
};

std::ostream& operator<<(std::ostream& os, const NoteDataStrings& note) {
    std::string tags;
    for(const auto& tag: note.tags) tags += tag + ", ";
    std::string kids;
    for(auto& kid: note.kids) kids += kid + ", ";
    return os << "Title: " << note.title << "\n"
              << "  tags: " << tags << "\n"
              << "  content: \n    " << note.content << "\n"
              << "  kids: " << kids << std::endl;
}

/**
 * Class to manage note data objects.
 * next_id: 'NoteId's start from 1 so we can use 0 as an empty value.
 */
class NoteStore {
private:
    std::unordered_map<NoteId, NoteData> data;
    std::unordered_map<std::string, NoteId> title_to_id;
    // std::unordered_map<NoteId, std::vector<std::string>> kids;
    NoteId next_id {1};

    // ensure a stable NoteId for notes and tags
    NoteId getId(const std::string& title) {
        if (auto it = title_to_id.find(title); it != title_to_id.end()) {
            LOG_DEBUG() << "found id for: " << title << ", id: " << it->second;
            return it->second;
        }
        NoteId id = next_id++;
        LOG_DEBUG() << "adding title: " << title << ", id: " << id;
        title_to_id.emplace(title, id);
        data[id] = {title, ""};  // placeholder
        return id;
    };
public:
    NoteStore(std::string storage_path) {
        LOG_DEBUG() << "loading NoteStore from path: " << storage_path;
        if (!load_json_file(storage_path)) {
            LOG_DEBUG() << "could not load filepath, generating defaults";
            generateDefault();
        }
    }

    NoteId getId(const std::string& title) const {
        if (auto it = title_to_id.find(title); it != title_to_id.end()) {
            return it->second;
        }
        throw std::out_of_range("Error!  Could not find: " + title + " in title_to_id!");
    };

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

        // import notes into 'data'
        NoteId this_id;
        for (const auto& note : j) {
            auto title = note["title"].get<std::string>();
            auto content = note["content"].get<std::string>();
            auto tags = note["tags"].get<std::vector<std::string>>();

            this_id = getId(title);
            std::vector<NoteId> tag_ids;
            for (const auto& tag : tags) {
                tag_ids.emplace_back(getId(tag));
            }
            auto [it, inserted] = data.try_emplace(this_id);
            auto& n = it->second;
            n.title = title;
            n.content = content;
            n.tags = tag_ids;
            title_to_id[title] = this_id;
            LOG_DEBUG() << "imported note: \n" << getNoteStrings(this_id);
            // add this title to the tags' kids
            for (const auto& tag : tags) {
                LOG_DEBUG() << "adding " << title << " as a kid to " << tag;
                NoteId tag_id = getId(tag);
                data[tag_id].kids.emplace_back(this_id);
            }
        }

        return true;
    }

    void save_json_file(std::string json_file) {
        std::ofstream outfile(json_file);
        if (!outfile.is_open()) {
            LOG_ERROR() << "Could not open file for writing!\n";
            return;
        }

        nlohmann::json json_array = nlohmann::json::array();
        for (const auto& value : data) {
             nlohmann::json json_note = {
                {"title", value.second.title},
                {"content", value.second.content},
            };

            std::vector<std::string> tags;
            NoteData tag;
            for (const auto& tag_id : value.second.tags) {
                tag = getNote(tag_id);
                tags.emplace_back(tag.title);
            }
            json_note["tags"] = tags;

            json_array.push_back(json_note);
        }

        outfile << std::setw(2) << json_array;
        if (!outfile) {
            LOG_ERROR() << "Write failed\n";
        }
    }

    /**
     * In case no file was found, generate default data.
     */
    void generateDefault() {
        std::string title = "NoteWiki";
        std::vector<std::string> tags = {"default"};
        std::string content = "This is the NoteWiki app.\n  Tag any note 'default' to show them on startup.";
        addNote(title, content, tags, {});
    }

    const NoteData& getNote(const NoteId& id) const {return data.at(id);}
    NoteData& getNote(const NoteId& id) {return data.at(id);}
    const NoteData& getNote(const std::string& title) const {
        NoteId id = getId(title);
        return getNote(id);
    }
    NoteData& getNote(const std::string& title) {
        NoteId id = getId(title);
        return getNote(id);
    }

    NoteDataStrings getNoteStrings(const NoteId& id) {
        std::string title;
        std::string content;
        std::vector<std::string> tags;
        std::vector<std::string> kids;
        if (const auto& search = data.find(id); search != data.end()) {
            title = search->second.title;
            content = search->second.content;
            for (const auto& tag: search->second.tags) tags.emplace_back(getNote(tag).title);
            for (const auto& kid: search->second.kids) kids.emplace_back(getNote(kid).title);
        }
        return {title, content, tags, kids};
    }

    NoteDataStrings getNoteStrings(const std::string& title) {
        NoteId id = getId(title);
        return getNoteStrings(id);
    }

    void addNote(std::string title,
                 std::string content,
                 std::vector<std::string> tags,
                 std::vector<std::string> kids) {
        NoteId id = getId(title);
        LOG_DEBUG() << "adding note: " << title << ", id: " << id;

        //convert tags and kids into NoteIds
        std::vector<NoteId> tag_ids;
        for (const auto& tag : tags) tag_ids.emplace_back(getId(tag));
        std::vector<NoteId> kid_ids;
        for (const auto& kid: kids) kid_ids.emplace_back(getId(kid));
        data[id] = NoteData{title, content, tag_ids, kid_ids};

        // add 'this' as a kid to its tags
        NoteId tag_id;
        for (const auto& tag : tags) {
            tag_id = getId(tag);
            data[tag_id].kids.emplace_back(id);
            LOG_DEBUG() << "added: " << id << ", as a kid to note: " << getNote(tag_id).title;
            LOG_DEBUG() << "kids[0]: " << getNote(tag_id).kids[0];
        }
    }

    void updateNote(const NoteId id,
                    const std::string& new_title,
                    const std::string& content,
                    const std::vector<std::string>& tags,
                    const std::vector<std::string>& kids) {
        auto& note = getNote(id);
        bool title_changed = (note.title != new_title ? true : false);

        std::vector<NoteId> tag_ids;
        for (const auto& tag : tags) tag_ids.emplace_back(getId(tag));
        std::vector<NoteId> kid_ids;
        for (const auto& kid : kids) kid_ids.emplace_back(getId(kid));
        // check if new data is exactly the same as the old
        if ((note.title == new_title) && (note.content == content) && 
            (note.tags == tag_ids) && (note.kids == kid_ids)) {
            return;
        }

        LOG_DEBUG() << "update_note: \n" << getNoteStrings(id);
        // update the children of a tag, removing old_title, adding the new_title
        if (title_changed) {
            for (const auto& tag_id : tag_ids) {
                auto it = std::find(data[tag_id].kids.begin(), data[tag_id].kids.end(), id);
                if (it != data.at(tag_id).kids.end()) {
                    std::iter_swap(it, data[tag_id].kids.end() - 1); // Move target to end
                    data[tag_id].kids.pop_back();                    // Remove it
                }
                data[tag_id].kids.emplace_back(id);
            }
        }
        data[id] = {new_title, content, tag_ids, kid_ids};
    }

    std::vector<NoteId>& getKids(std::string title) {
        LOG_DEBUG() << "getting kids from: " << getNote(title).title << " kids size: " << getNote(title).kids.size();
        return getNote(title).kids;
    }
};

} // namespace note

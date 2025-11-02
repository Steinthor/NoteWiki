#pragma once

#include "note.hpp"
#include "parser.h"

#include <algorithm> // for iter_swap

using namespace note;

/**
 * A placeholder for the strings when editing a note
 */
struct EditNote {
    std::string title;
    std::string content;
    std::string tags;
    std::string kids;
};

/**
 * represents a light-weight note reference for the ViewState
 */
struct NoteView {
    NoteId id;
    bool show{false};
    bool edit{false};
};

std::ostream& operator<<(std::ostream& os, const NoteView& note) {
    return os << "id: " << note.id << std::endl;
}

/**
 * 'ViewState' is a light-weight object that manages the hierarchy of the
 * notes independent of the renderer.
 * It keeps a list of the notes visible in the UI, their current state, and
 * allows manipulation of the list through actions on 'Command' objects.
 */
class ViewState {
private:
    std::vector<NoteView> visible;
    EditNote editNote;
    bool editMode {false};
    bool dirty {false};
public:
    const std::vector<NoteView>& view() const noexcept { return visible; }
    EditNote& getEditNote() { return editNote; }

    const NoteView& getNote(NoteId id) const {
        for (auto& note : visible) {
            if (note.id == id) return note;
        }
        throw std::out_of_range("Error!  Could not find: " + std::to_string(id) + " in 'visible'!");
    }

    NoteView& getNote(NoteId id) {
        for (auto& note : visible) {
            if (note.id == id) return note;
        }
        throw std::out_of_range("Error!  Could not find: " + std::to_string(id) + " in 'visible'!");
    }
    
    void addFromKids(const std::vector<NoteId>& kids, const NoteStore& store) {
        for (const auto& kid : kids) {
            visible.emplace_back(kid);
        }
    }

    void moveUp(NoteId id) {
        if (id == visible.front().id)
            return;
        for (auto it = visible.begin(); it != visible.end(); ++it) {
            if (id == it->id) {
                std::iter_swap(it, std::prev(it));
                break;
            }
        }
    }

    void moveDown(NoteId id) {
        if (id == visible.back().id)
            return;
        for (auto it = visible.begin(); it != visible.end(); ++it) {
            if (id == it->id) {
                std::iter_swap(it, std::next(it));
                break;
            }
        }
    }
    
    /**
     * Adds 'id' into 'visible'
     */
    void addId(NoteId id, NoteId after_id) {
        LOG_DEBUG() << "opening: " << id << ", after: " << after_id;
        for (auto it = visible.begin(); it != visible.end(); ++it) {
            if (it->id == id) {
                LOG_DEBUG() << "found id: " << id << ", returning";
                return;
            }
        }
        
        for (auto it = visible.begin(); it != visible.end(); ++it) {
            if (it->id == after_id) {
                LOG_DEBUG() << "found after_id: " << after_id;
                visible.insert(std::next(it), {id});
                return;
            }
        }
        LOG_DEBUG() << "adding id: " << id;
        visible.emplace_back(id);
    }

    bool getEditMode() const {return editMode;}
    
    /**
     * Sets the the specific note to edit mode
     */
    void startEdit(NoteId id, const NoteDataStrings& note = {}) {
        LOG_DEBUG() << "starting edit of id: " << id << ", editMode: " << (editMode ? "true" : "false");
        if (note.title.empty())
            return;

        for (auto it = visible.begin(); it != visible.end(); ++it) {
            if (id == it->id) {
                it->edit = true;
                LOG_DEBUG() << "setting edit to: " << (it->edit ? "true" : "false");
            }
        }
        editMode = true;
        setEditNote(note);
        LOG_DEBUG() << "final editMode: " << (editMode ? "true" : "false");
    }

    /**
     * Copies the content of the input note to ImGuiViewers' editNote.
     */
    void setEditNote(const NoteDataStrings& note, std::string delimiter = " ") {
        editNote.title = note.title;
        LOG_DEBUG() << "setting editNote title: " << editNote.title;
        editNote.content = note.content;
        editNote.tags.clear();
        for (const auto& tag : note.tags) {
            editNote.tags = editNote.tags + delimiter + tag;
        }
        editNote.kids.clear();
        for (const auto& kid : note.kids) {
            editNote.kids = editNote.kids + delimiter + kid;
        }
    }

    void copyFromEdit(NoteDataStrings& note) {
        note.title = editNote.title;
        note.content = editNote.content;
        note.tags = parseWords(editNote.tags);
        note.kids = parseWords(editNote.kids);
    }

    void stopEdit(NoteId id) {
        for (auto it = visible.begin(); it != visible.end(); ++it) {
            if (id == it->id) {
                it->edit = false;
            }
        }
        editMode = false;
    }
};

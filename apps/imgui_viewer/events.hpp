#pragma once

#include <optional>

#include "note.hpp"

enum class EventType {
    BeginEdit, CancelEdit, SubmitEdit,
    OpenId, MoveUp, MoveDown,
};

struct Event {
    EventType type;
    note::NoteId id{};
    std::optional<note::NoteId> insertAfter;
};

/**
 * EventQueue allows protecting the container object, defining behaviour, and 
 * change internals in the future
 */
class EventQueue {
public:
    void push(Event a) { q_.push_back(std::move(a)); }
    
    /**
     * returns bool to allow use in a while loop iterating over the queue
     */
    bool try_pop(Event& out) {
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop_front();
        return true;
    }
private:
    std::deque<Event> q_;
};
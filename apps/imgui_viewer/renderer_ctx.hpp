#pragma once
#include "note.hpp"
#include "viewstate.hpp"
#include "events.hpp"

/**
 * Object to send to the renderer so it can:
 * i) get the order of notes from 'view'
 * ii) get the notes from 'store'
 * iii) push events to the event queue.
 */
struct RenderCtx {
    const NoteStore& store;
    ViewState& view;
    EventQueue&      events;
};
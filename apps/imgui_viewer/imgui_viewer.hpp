#include "note.hpp"
#include "options.h"
#include "parser.h"
#include "renderer.hpp"
#include "renderer_ctx.hpp"
#include "viewstate.hpp"

using namespace note;

class NoteAppUI {
private:
    Options opts_;
    NoteStore store;
    ViewState view;
    ImguiRenderer renderer;
    EventQueue events;
    bool isRunning{false};
public:
    NoteAppUI(Options opts) : opts_(std::move(opts)),
                              store(NoteStore(opts_.storage_path)) {
        LOG_DEBUG() << "initializing NoteAppUI";
        const auto kids = store.getKids("default");
        view.addFromKids(kids, store);
    }

    int run() {
        isRunning = true;
        if (renderer.setup(opts_.app_name) != 0) return -1;

        while (isRunning) {
            // - render a frame
            isRunning = renderer.render(RenderCtx{store, view, events});
            // - process events in event queue
            Event e;
            while (events.try_pop(e)) {
                switch (e.type) {
                    case EventType::MoveUp: {
                        view.moveUp(e.id);
                        break;
                    }
                    case EventType::MoveDown: {
                        view.moveDown(e.id);
                        break;
                    }
                    case EventType::BeginEdit: {
                        view.startEdit(e.id, store.getNoteStrings(e.id));
                        break;
                    }
                    case EventType::SubmitEdit: {
                        auto note = store.getNoteStrings(e.id);
                        view.copyFromEdit(note);
                        store.updateNote(e.id, note.title, note.content, note.tags, note.kids);
                        view.stopEdit(e.id);
                        break;
                    }
                    case EventType::CancelEdit: {
                        view.stopEdit(e.id);
                        break;
                    }
                    case EventType::OpenId: {
                        if (e.insertAfter.has_value())
                            view.addId(e.id, *e.insertAfter);
                        break;
                    }
                }
            }
        }
        store.save_json_file("notes.json");
        return renderer.tearDown();
    }
};

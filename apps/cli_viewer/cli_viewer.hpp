#include "note.hpp"
#include "options.h"

using namespace note;

class CliViewer {
private:
    Options opts_;
    NoteStore noteStore;
    std::vector<NoteData> visible;
    std::string input;
public:
    CliViewer(Options opts) : opts_(std::move(opts)),
                              noteStore(NoteStore(opts_.storage_path)) {
        NoteData default_note = noteStore.getNote("default");
        for (const auto& kid : default_note.kids) {
            visible.emplace_back(noteStore.getNote(kid));
        }
    }

    int run() {
        while (input != "q") {
            input.clear();
            for (const auto& note : visible) {
                std::cout << "=======\n";
                std::cout << "Title: " << note.title << "\n";
                std::cout << "  tags: ";
                std::string separator;
                if (note.tags.size() > 0) {
                    for (const auto& tag : note.tags) {
                        std::cout << separator << tag;
                        separator = ", ";
                    }
                }
                std::cout << "\n";
                std::cout << "  ----\n";
                std::cout << "  " << note.content << "\n";
                std::cout << "  ----\n";
                if (note.kids.size() > 0) {
                    std::cout << "  children: ";
                    separator.clear();
                    for (const auto& child : note.kids) {
                        std::cout << separator << child;
                        separator = ", ";
                    }
                }
                std::cout << std::endl;
            }

            std::cin >> input;
        }
        return 0;
    }
};
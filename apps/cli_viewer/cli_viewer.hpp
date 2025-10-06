#include "note.hpp"

class CliViewer {
private:
    std::string app_name{"NoteWiki 0.0"};
    NoteDataManager noteDataManager;
    std::vector<Note> visible;
    std::string input;
public:
    CliViewer() {
        Note default_note = noteDataManager.get_note("default");
        for (const auto& kid : default_note.children) {
            visible.emplace_back(noteDataManager.get_note(kid));
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
                if (note.children.size() > 0) {
                    std::cout << "  children: ";
                    separator.clear();
                    for (const auto& child : note.children) {
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
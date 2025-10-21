#include "imgui_viewer.hpp"
#include "logger.h"
#include "note.hpp"
#include "options.h"

int main(int argc, char* argv[]) {
    auto parsed = parse_options(argc, argv);
    if (!parsed.value) {
        if (!parsed.error.empty()) std::cout << parsed.error << "\n";
        return parsed.exit_code;
    }
    ImGuiViewer viewer(*parsed.value);

    return viewer.run();
}

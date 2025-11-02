#include "imgui_viewer.hpp"
#include "logger.h"
#include "options.h"

int main(int argc, char* argv[]) {
    auto parsed = parse_options(argc, argv);
    if (!parsed.value) {
        if (!parsed.error.empty()) std::cout << parsed.error << "\n";
        return parsed.exit_code;
    }

    Logger& log = Logger::getInstance();
    if (parsed.value->verbose) log.setLogLevel(LogLevel::INFO);

    NoteAppUI viewer(*parsed.value);

    LOG_INFO() << "Starting ui";
    return viewer.run();
}

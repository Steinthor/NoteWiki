#include "cli_viewer.hpp"
#include "logger.h"
#include "note.hpp"

int main(int argc, char* argv[]) {
    Logger& log = Logger::getInstance();
    log.enableConsoleLogging(false);
    log.enableFileLogging("notewiki.log");
    LOG_INFO() << "Starting viewer";

    auto parsed = parse_options(argc, argv);
    if (!parsed.value) {
        if (!parsed.error.empty()) std::cout << parsed.error << "\n";
        return parsed.exit_code;
    }

    CliViewer viewer(*parsed.value);

    return viewer.run();
}

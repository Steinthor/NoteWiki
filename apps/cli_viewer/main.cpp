#include "cli_viewer.hpp"
#include "logger.h"
#include "note.hpp"

int main() {
    Logger& log = Logger::getInstance();
    // log.enableConsoleLogging(false);
    log.enableFileLogging("notewiki.log");
    LOG_INFO() << "Starting viewer";

    CliViewer viewer;

    return viewer.run();
}

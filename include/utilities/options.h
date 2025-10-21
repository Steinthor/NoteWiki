#pragma once
#include <string>
#include <optional>
#include <vector>

#include <cxxopts.hpp>
#include "logger.h"

// object to contain common parameters
struct Options {
    std::string app_name{"NoteWiki 0.0"};
    std::string storage_path; // file to load/save notes
    bool        verbose = false;
};

// parse without side effects; no I/O except returning an error
struct ParseResult {
    std::optional<Options> value;
    std::string            error;   // non-empty if error/help
    int                    exit_code = 0; // e.g. 0 for --help, 1 for bad args
};

ParseResult parse_options(int argc, char* argv[]) {
    ParseResult r;

    try {
        cxxopts::Options opts("notewiki", "NoteWiki options");
        opts.add_options()
            ("f,file", "Storage file to load", cxxopts::value<std::string>())
            ("v,verbose", "Verbose output")
            ("h,help", "Show help");

        auto result = opts.parse(argc, argv);

        if (result.count("help")) {
            r.error = opts.help();
            r.exit_code = 0;
            return r;
        }

        Options o;
        if (result.count("file")) o.storage_path = result["file"].as<std::string>();
        // optional: default if not provided
        if (o.storage_path.empty()) {
            // r.error = "Storage file is required (use -f <path>)\n\n" + opts.help();
            o.storage_path = "notes.json";
            LOG_INFO() << "No storage filepath given, setting to '" << o.storage_path << "'";
        }

        o.verbose = result.count("verbose") > 0;
        r.value = std::move(o);
        return r;

    } catch (const cxxopts::exceptions::exception& e) {
        r.error = std::string("Argument error: ") + e.what();
        r.exit_code = 1;
        return r;
    }
}
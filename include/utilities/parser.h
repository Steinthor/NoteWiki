#pragma once

#include <algorithm>
#include <string>
#include <vector>

/**
 * Turn a string 'words' with words separated by any character in 'delimiter' into
 * a vector of strings containing the words
 */
std::vector<std::string> parseWords(std::string_view words, std::string delimiter = " ,") {
    std::string word;
    std::vector<std::string> keys;
    size_t start = 0, next;
    while ((next = words.find_first_of(delimiter, start)) != std::string::npos) {
        word = words.substr(start, next - start);
        if (!word.empty()) keys.emplace_back(word);
        start = next + 1;
    }
    // check for a final word
    word = words.substr(start, words.size() - start);
    if (!word.empty()) keys.emplace_back(word);

    // sort words and remove duplicates
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    return keys;
}
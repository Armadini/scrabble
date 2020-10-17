#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <unordered_set>

class Dictionary {
public:
    // Reads each word from file, adds it to a Dictionary, and returns the Dictionary.
    static Dictionary read(const std::string& file_path);

    bool is_word(const std::string& word) const;

private:
    std::unordered_set<std::string> words;
};

#endif

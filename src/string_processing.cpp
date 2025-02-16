#include "string_processing.h"


namespace string_processing {

std::vector<std::string> SplitIntoWords(std::string_view text) {
    std::vector<std::string> words;
    for (auto word : text | std::views::split(' ')) {
        words.emplace_back(word.begin(), word.end());
    }
    return words;
}

}; // namespace string_processing
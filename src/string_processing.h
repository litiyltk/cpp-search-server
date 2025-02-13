#pragma once

#include <unordered_set>
#include <string>
#include <vector>

std::vector<std::string> SplitIntoWords (const std::string &text);

template <typename StringContainer>
std::unordered_set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::unordered_set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    
    return non_empty_strings;
}
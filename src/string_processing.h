#pragma once

#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>


namespace string_processing {

std::vector<std::string> SplitIntoWords(std::string_view text);

template <typename StringContainer>
std::unordered_set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::unordered_set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.emplace(str);
        }
    }
    
    return non_empty_strings;
}

}; // namespace string_processing
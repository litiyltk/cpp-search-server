#pragma once

#include <iostream>


enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct DocumentData {
    int rating;
    DocumentStatus status;
};

struct Document {
    static constexpr double EPSILON = 1.0E-6;

    Document() = default;
    Document(int id, double relevance, int rating);

    int id = 0;
    double relevance = 0;
    int rating = 0;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;
};

std::ostream& operator<<(std::ostream& os, const Document& document);


#pragma once

#include <iostream>


namespace document {

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
    static constexpr double EPSILON = 1.0E-6; // Точность сравнения релевантности документов

    Document() = default;
    Document(int doc_id, double doc_relevance, int doc_rating)
        : id(doc_id)
        , relevance(doc_relevance)
        , rating(doc_rating) {
    }

    int id = 0;
    double relevance = 0;
    int rating = 0;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;
};

std::ostream& operator<<(std::ostream& os, const Document& document);

}; // namespace document
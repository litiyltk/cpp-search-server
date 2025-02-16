#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <string>
#include <vector>

#include "document.h"
#include "string_processing.h"


namespace search_server {

using namespace std::string_literals;
using namespace document;
using namespace string_processing;

constexpr int MAX_RESULT_DOCUMENT_COUNT = 5; // Количество выводимых документов

class SearchServer {
public:
    SearchServer() = default;

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)) {
    }

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    // Фильтрация по пользовательскому предикату int document_id, DocumentStatus status, int rating
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;

    // Простая фильтрация, только актуальные документы DocumentStatus::ACTUAL
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    // Фильтрация по статусу
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus find_status) const;

    int GetDocumentCount() const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

    int GetDocumentId(int index) const;

private:
    // данные слова запроса (слово, флаги для типа)
    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    // поисковый запрос (плюс-слова, минус-слова)
    struct Query {
        std::unordered_set<std::string> plus_words;
        std::unordered_set<std::string> minus_words;
    };

    std::unordered_set<std::string> stop_words_; // множество стоп-слов
    std::unordered_map<std::string, std::unordered_map<int, double>> word_to_document_freqs_; // слово : словарь(ID : TF)
    std::unordered_map<int, DocumentData> documents_; // ID : данные документа
    std::vector<int> added_ids_; // вектор ID в хронологическом порядке добавления документа

    bool IsValidDocumentID(int document_id);
    bool IsStopWord(const std::string& word) const;

    // Разбивает строку по пробелам на слова, исключив стоп-слова
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static bool IsValidWord(const std::string& word);
    static bool IsValidMinusWord(const std::string& word);

    static int ComputeAverageRating(const std::vector<int>& ratings);

    //разделяет строку запроса на плюс- и минус-слова
    QueryWord ParseQueryWord(std::string text) const;
    Query ParseQuery(const std::string& text) const;

    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};


template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Incorrect stop words"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
    Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < std::numeric_limits<double>::epsilon()) {
                return lhs.rating > rhs.rating;
            }
            return lhs.relevance > rhs.relevance;
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
    std::unordered_map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (!word_to_document_freqs_.contains(word)) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto& [ document_id, term_freq ] : word_to_document_freqs_.at(word)) {
            if (document_predicate(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (!word_to_document_freqs_.contains(word)) {
            continue;
        }
        std::erase_if(document_to_relevance, [&](const auto& pair) {
            return word_to_document_freqs_.at(word).contains(pair.first);
        });
    }

    std::vector<Document> matched_documents;
    for (const auto& [ document_id, relevance ] : document_to_relevance) {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }

    return matched_documents;
}

}; // namespace search_server
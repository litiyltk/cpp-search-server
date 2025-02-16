#include "search_server.h"


namespace search_server {

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    std::vector<std::string> words = SplitIntoWordsNoStop(document);
    if (IsValidDocumentID(document_id)) {
        const double inv_word_count = 1.0 / static_cast<int>(words.size());
        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
            added_ids_.push_back(document_id);
            documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    } else {
        throw std::invalid_argument("Incorrect document ID: "s + std::to_string(document_id));
    }
}
    
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus find_status) const {
    return FindTopDocuments(raw_query,
        [find_status]([[maybe_unused]] int document_id, DocumentStatus status, [[maybe_unused]] int rating) {
            return status == find_status;
        });
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    std::tuple<std::vector<std::string>, DocumentStatus> result;
    Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (!word_to_document_freqs_.contains(word)) {
            continue;
        }
        if (word_to_document_freqs_.at(word).contains(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (!word_to_document_freqs_.contains(word)) {
            continue;
        }
        if (word_to_document_freqs_.at(word).contains(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status }; 
}

int SearchServer::GetDocumentId(int index) const {
    return added_ids_.at(static_cast<size_t>(index));
}

bool SearchServer::IsValidDocumentID(int document_id) {
    return (document_id >= 0 && !documents_.contains(document_id));
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.contains(word);
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (IsValidWord(word)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        } else {
            words.clear();
            throw std::invalid_argument("Word "s + word + " is invalid"s); 
        }            
    }
    return words;
}

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

bool SearchServer::IsValidMinusWord(const std::string& word) {
    return !((word.size() == 1u && word[0] == '-') || (word.size() > 1u && word[0] == '-' && word[1] == '-'));
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    return accumulate(ratings.begin(),ratings.end(),0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
        if (IsValidWord(word) && IsValidMinusWord(word)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.emplace(query_word.data);
                } else {
                    query.plus_words.emplace(query_word.data);
                }
            }
        } else {
            throw std::invalid_argument("Incorrect query: "s + text + ", invalid word: "s + word);
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / static_cast<int>(word_to_document_freqs_.at(word).size()));
}

}; // namespace search_server
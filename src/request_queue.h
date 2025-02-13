#pragma once

#include <deque>
#include <string>
#include <vector>

#include "search_server.h"


class RequestQueue {
public:
    static constexpr int MINUTES_IN_DAY = 1440;

    explicit RequestQueue(const SearchServer& search_server);

    // Фильтрация по пользовательскому предикату int document_id, DocumentStatus status, int rating
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    // Фильтрация по статусу
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    // Простая фильтрация, только актуальные документы DocumentStatus::ACTUAL
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        int time;
        int results;
    };
    
    const SearchServer& search_server_;
    int no_results_requests_;
    int current_time_;
    
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = MINUTES_IN_DAY;

    bool IsNextDay();
    void AddRequest(int results_num);
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
    AddRequest(result.size());
    return result;
}
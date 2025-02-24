#include "request_queue.h"


namespace request_queue {

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    const auto result = search_server_.FindTopDocuments(raw_query, status);
    AddRequest(static_cast<int>(result.size()));
    return result;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    const auto result = search_server_.FindTopDocuments(raw_query);
    AddRequest(static_cast<int>(result.size()));
    return result;
}

int RequestQueue::GetNoResultRequests() const {
    return no_results_requests_;
}

bool RequestQueue::IsNextDay() {
    return current_time_ - requests_.front().time >= min_in_day_;
}

void RequestQueue::AddRequest(int results_num) {
    ++current_time_;
    while (!requests_.empty() && IsNextDay()) {
        if (requests_.front().results == 0) {
            --no_results_requests_;
        }
        requests_.pop_front();
    }
    requests_.push_back({current_time_, results_num});
    if (results_num == 0) {
        ++no_results_requests_;
    }
}

}; // namespace request_queue
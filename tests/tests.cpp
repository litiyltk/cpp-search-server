#include "../src/paginator.h"
#include "../src/search_server.h"
#include "../src/request_queue.h"
#include "../src/string_processing.h"

#include "log_duration.h"
#include "test_framework.h"

#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>


namespace tests {

using namespace std::string_literals;

using namespace test_framework;
using namespace search_server;
using namespace request_queue;
using namespace paginator;

void TestDocumentsComparison() {
    Document doc1(1, 0.9, 5);
    Document doc2(1, 0.9000001, 5);
    Document doc3(2, 0.8, 4);

    ASSERT(doc1 == doc2);
    ASSERT(!(doc1 == doc3));
    ASSERT(doc1 != doc3);
}

void TestWithoutDocuments() {
    SearchServer server;

    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0, "There should be no documents"s);
    
    try {
        server.GetDocumentId(0);
        ASSERT_HINT(false, "There should be no documents"s);
    } catch (const std::exception& ex) {
        ASSERT(true);
    }
}

void TestWithInvalidIdDocument() {
    SearchServer server;
    
    try {
        server.AddDocument(-1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
        ASSERT_HINT(false, "Document with id < 0 cannot be added"s);
    } catch (const std::exception& ex) {
        ASSERT(true);
    }
}

void TestAddTwoEqualDocuments() {
    SearchServer server;
    server.AddDocument(11, "cat"s, DocumentStatus::ACTUAL, {});
    
    try {
        server.AddDocument(11, "cat"s, DocumentStatus::ACTUAL, {});
        ASSERT_HINT(false, "Document with previously added id cannot be added"s);
    } catch (const std::exception& ex) {
        ASSERT(true);
    }
}

void TestAddTwoDocuments() {
    SearchServer server;
    server.AddDocument(11, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});

    ASSERT_EQUAL(server.GetDocumentCount(), 1);
    ASSERT_EQUAL(server.GetDocumentId(0), 11);

    server.AddDocument(12, "curly dog and fancy collar"s, DocumentStatus::BANNED, {1, 2, 3});

    ASSERT_EQUAL(server.GetDocumentCount(), 2);
    ASSERT_EQUAL(server.GetDocumentId(1), 12);
}

void TestCalculateDocumentRating() {
    SearchServer server;

    const std::vector<int> ratings = { 8, 12, 5 };
    int average_rating = accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());

    server.AddDocument(0, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, ratings);
    const std::vector<Document> results = server.FindTopDocuments("dog"s);

    ASSERT_EQUAL(results.size(), 1);
    ASSERT_EQUAL(results[0].rating, average_rating);
}

void TestCalculateRelevance() {
    SearchServer server;
    server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(3, "big cat fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 8});
    server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    const std::vector<Document> results = server.FindTopDocuments("curly big cat"s);

    constexpr int total_documents = 5;
    constexpr int count_docs_with_curly = 2;
    constexpr int count_docs_with_big = 3;
    constexpr int count_docs_with_cat = 2;

    // Вычисление IDF
    double idf_curly = log(total_documents / (1.0 * count_docs_with_curly));
    double idf_big = log(total_documents / (1.0 * count_docs_with_big));
    double idf_cat = log(total_documents / (1.0 * count_docs_with_cat));

    // Вычисляем релевантность для каждого документа
    std::vector<double> expected_relevances;

    // id 1: "curly cat curly tail"
    expected_relevances.push_back(
        idf_curly * (2.0 / 4.0) + idf_big * (0.0 / 4.0) + idf_cat * (1.0 / 4.0));

    // id 3: "big cat fancy collar"
    expected_relevances.push_back(
        idf_curly * (0.0 / 4.0) + idf_big * (1.0 / 4.0) + idf_cat * (1.0 / 4.0));

    // id 2: "curly dog and fancy collar"
    expected_relevances.push_back(
        idf_curly * (1 / 5.0) + idf_big * (0.0 / 5.0) + idf_cat * (0.0 / 5.0));

    // id 4: "big dog sparrow Eugene"
    expected_relevances.push_back(
        idf_curly * (0.0 / 4.0) + idf_big * (1.0 / 4.0) + idf_cat * (0.0 / 4.0));

    // id 5: "big dog sparrow Vasiliy"
    expected_relevances.push_back(
        idf_curly * (0.0 / 4.0) + idf_big * (1.0 / 4.0) + idf_cat * (0.0 / 4.0));

    ASSERT_EQUAL(results.size(), total_documents);

    for (size_t i = 0; i < results.size(); ++i) {
        ASSERT_HINT(std::abs(results[i].relevance - expected_relevances[i]) < std::numeric_limits<double>::epsilon(),
                "Wrong calculation relevance for document ID = "s +
                std::to_string(results[i].id) + ": relevance "s +
                std::to_string(results[i].relevance) + ", but expected "s +
                std::to_string(expected_relevances[i]));
    }
}

void TestSortResultsByRelevance() {
    SearchServer server;
    server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});
 
    const std::vector<Document> results = server.FindTopDocuments("dog cat big"s);
    ASSERT_EQUAL(results.size(), 5);
    for (size_t i = 1; i < results.size(); ++i) {
        ASSERT(results[i - 1].relevance >= results[i].relevance);
    }
}

void TestMatchDocument() {
    SearchServer server("a in at"s);
    server.AddDocument(1, "a small cat and a big dog in the park"s, DocumentStatus::REMOVED, {1, 2, 3});
    
    auto [words, status] = server.MatchDocument("big dog"s, 1);
    
    ASSERT_EQUAL(words.size(), 2);
    std::sort(words.begin(), words.end()); // search_server::Query - содержит std::unordered_set

    ASSERT_EQUAL(words[0], "big"s);
    ASSERT_EQUAL(words[1], "dog"s);

    ASSERT_HINT(status == DocumentStatus::REMOVED, "Status must be REMOVED");
}

void TestMatchDocumentWithMinusWords() {
    SearchServer server("a in at"s);
    server.AddDocument(1, "a small cat and a big dog in the park"s, DocumentStatus::ACTUAL, {1, 2, 3});
    
    const auto [words, status] = server.MatchDocument("big dog -cat"s, 1);
    
    ASSERT_EQUAL(words.size(), 0);
    ASSERT(words.empty());
    
    ASSERT_HINT(status == DocumentStatus::ACTUAL, "Default status must be ACTUAL"s);
}

void TestMatchDocumentWithNoMatchingWords() {
    SearchServer server("a in at"s);
    server.AddDocument(1, "a small cat and a big dog in the park"s, DocumentStatus::ACTUAL, {1, 2, 3});
    
    const auto [words, status] = server.MatchDocument("elephant tiger"s, 1);
    
    ASSERT_EQUAL(words.size(), 0);
    ASSERT(words.empty());
    
    ASSERT_HINT(status == DocumentStatus::ACTUAL, "Default status must be ACTUAL"s);
}

void TestMatchDocumentWithOnlyMinusWords() {
    SearchServer server("a in at"s);
    server.AddDocument(1, "a small cat and a big dog in the park"s, DocumentStatus::ACTUAL, {1, 2, 3});
    
    const auto [words, status] = server.MatchDocument("-dog -big"s, 1);
    
    ASSERT_EQUAL(words.size(), 0);
    ASSERT(words.empty());
    
    ASSERT_HINT(status == DocumentStatus::ACTUAL, "Default status must be ACTUAL"s);
}

void TestMatchDocumentWithOnlyStopWords() {
    SearchServer server("a in at the"s);
    server.AddDocument(1, "a small cat and a big dog in the park"s, DocumentStatus::ACTUAL, {1, 2, 3});
    
    // Все слова в запросе являются стоп-словами
    const auto [words, status] = server.MatchDocument("a in the"s, 1);
    
    ASSERT_EQUAL(words.size(), 0);
    ASSERT(words.empty());
    
    ASSERT_HINT(status == DocumentStatus::ACTUAL, "Default status must be ACTUAL"s);
}

void TestMatchNonExistentDocument() {
    SearchServer server("a in at"s);
    server.AddDocument(10, "a small cat and a big dog in the park"s, DocumentStatus::ACTUAL, {1, 2, 3}); // Id = 10
    
    try {
        const auto [words, status] = server.MatchDocument("big dog"s, 22); // Ищем документ с Id = 22
        ASSERT_HINT(false, "There is no document with ID 22"s);
    } catch (const std::out_of_range&) {
        // Ожидаем только исключение std::out_of_range
    } catch (...) {
        ASSERT_HINT(false, "Expected std::out_of_range exception because document with ID 2 does not exist"s);
    }
}

void TestExcludeStopWordsFromAddedDocument() {
    SearchServer server("in"s);
    server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });

    const std::vector<Document> results = server.FindTopDocuments("in"s);

    ASSERT_HINT(results.empty(), "Documents containing stop words should be excluded by searching"s);

    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 1, "Document must be added"s);
    ASSERT_EQUAL_HINT(server.GetDocumentId(0), 42, "Document must be added"s);
}

void TestFindByOneWordTopDocuments() {
    SearchServer server("and in at"s);
    server.AddDocument(11, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(12, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    const std::vector<Document> results = server.FindTopDocuments("curly"s);

    ASSERT(!results.empty());
    ASSERT_EQUAL(results.size(), 2);

    ASSERT_EQUAL(results[0], Document(11, 0.202733, 5));
    ASSERT_EQUAL(results[1], Document(12, 0.101366, 2));
}

void TestFindByTwoWordsTopDocuments() {
    SearchServer server("and in at"s);
    server.AddDocument(102, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(31, "small cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    server.AddDocument(2, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    server.AddDocument(53, "big dog cat Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    const std::vector<Document> results = server.FindTopDocuments("cat big -collar"s);

    ASSERT(!results.empty());
    ASSERT_EQUAL(results.size(), 2);

    ASSERT_EQUAL(results[0], Document(53, 0.346574, 1));
    ASSERT_EQUAL(results[1], Document(2, 0.173287, 2));
}

void TestRequestQueue() {
    SearchServer server("and in at"s);
    RequestQueue queue(server);
    server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});

    ASSERT_EQUAL(queue.GetNoResultRequests(), 0);

    queue.AddFindRequest("curly"s);

    ASSERT_EQUAL(queue.GetNoResultRequests(), 0);

    queue.AddFindRequest("dog"s);

    ASSERT_EQUAL(queue.GetNoResultRequests(), 0);

    queue.AddFindRequest("empty request"s);

    ASSERT_EQUAL(queue.GetNoResultRequests(), 1);

    queue.AddFindRequest("tail"s);

    ASSERT_EQUAL(queue.GetNoResultRequests(), 1);

    queue.AddFindRequest("empty request"s);

    ASSERT_EQUAL(queue.GetNoResultRequests(), 2);
}

void TestPagination() {
    std::vector<int> data;
    data.reserve(10);

    std::ranges::copy(std::views::iota(1, 11), std::back_inserter(data));

    int page_size = 3;
    auto paginated = Paginate(data, page_size);

    ASSERT_EQUAL(paginated.size(), 4); // 3 страницы по 3 элемента и 1 с 1

    std::vector<std::vector<int>> expected_pages = {
        {1, 2, 3}, // 1 страница
        {4, 5, 6}, // 2 страница
        {7, 8, 9}, // 3 страница
        {10}       // 4 страница
    };

    size_t page_index = 0;
    for (const auto& page : paginated) {
        std::vector<int> actual_page(page.begin(), page.end());

        ASSERT_EQUAL_HINT(actual_page.size(), expected_pages[page_index].size(),
            "Invalid page size "s + std::to_string(page_index + 1));

        ASSERT_HINT(std::ranges::equal(actual_page, expected_pages[page_index]),
            "Mismatch on page "s + std::to_string(page_index + 1));

        ++page_index;
    }
}

void TestMakeUniqueNonEmptyStrings() {
    // Проверка работы с std::vector
    std::vector<std::string> vec = {"apple", "banana", "", "apple", "orange", ""};
    auto result1 = MakeUniqueNonEmptyStrings(vec);
    std::unordered_set<std::string> expected1 = {"apple", "banana", "orange"};
    ASSERT_EQUAL(result1.size(), expected1.size());
    for (const auto& str : expected1) {
        ASSERT_HINT(result1.contains(str), "Missing expected string: "s + str);
    }

    // Проверка работы с std::unordered_set
    std::unordered_set<std::string> uset = {"kiwi", "mango", "", "kiwi", "grape"};
    auto result2 = MakeUniqueNonEmptyStrings(uset);
    std::unordered_set<std::string> expected3 = {"kiwi", "mango", "grape"};
    ASSERT_EQUAL(result2.size(), expected3.size());
    for (const auto& str : expected3) {
        ASSERT_HINT(result2.contains(str), "Missing expected string: "s + str);
    }

    // Проверка пустого контейнера
    std::vector<std::string> empty_vec;
    auto result3 = MakeUniqueNonEmptyStrings(empty_vec);
    ASSERT(result3.empty());
    ASSERT_EQUAL(result3.size(), 0);

    // Проверка контейнера с только пустыми строками
    std::vector<std::string> only_empty = {"", "", ""};
    auto result4 = MakeUniqueNonEmptyStrings(only_empty);
    ASSERT(result4.empty());
    ASSERT_EQUAL(result4.size(), 0);
}

void TestSplitIntoWords() {
    // Проверка строки
    std::string_view text = "apple banana apple orange"sv;
    auto result = SplitIntoWords(text);
    std::vector<std::string> expected = {"apple"s, "banana"s, "apple"s, "orange"s};
    ASSERT_EQUAL(result.size(), expected.size());
    ASSERT_HINT(result == expected, "Mismatch in word splitting");

    // Проверка пустой строки
    std::string_view no_text = "";
    auto empty_result = SplitIntoWords(no_text);
    ASSERT(empty_result.empty());
    ASSERT_EQUAL(empty_result.size(), 0);
}


void RunTests() {
    LOG_DURATION("Testing time"s);

    RUN_TEST(TestDocumentsComparison);
    RUN_TEST(TestWithoutDocuments);
    RUN_TEST(TestWithInvalidIdDocument);
    RUN_TEST(TestAddTwoEqualDocuments);
    RUN_TEST(TestAddTwoDocuments);
    RUN_TEST(TestCalculateDocumentRating);
    RUN_TEST(TestCalculateRelevance);
    RUN_TEST(TestSortResultsByRelevance);

    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestMatchDocumentWithMinusWords);
    RUN_TEST(TestMatchDocumentWithNoMatchingWords);
    RUN_TEST(TestMatchDocumentWithOnlyMinusWords);
    RUN_TEST(TestMatchDocumentWithOnlyStopWords);
    RUN_TEST(TestMatchNonExistentDocument);

    RUN_TEST(TestExcludeStopWordsFromAddedDocument);
    RUN_TEST(TestFindByOneWordTopDocuments);
    RUN_TEST(TestFindByTwoWordsTopDocuments);
    RUN_TEST(TestRequestQueue);
    RUN_TEST(TestPagination);
    RUN_TEST(TestMakeUniqueNonEmptyStrings);
    RUN_TEST(TestSplitIntoWords);
}

} // namespace tests

int main() {
    std::cerr << "=== Tests are running ===\n";
    tests::RunTests();
    std::cerr << "=== All tests passed successfully! ===\n";
}
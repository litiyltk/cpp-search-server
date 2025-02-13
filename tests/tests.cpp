#include "../src/paginator.h"
#include "../src/search_server.h"
#include "../src/request_queue.h"
#include "test_framework.h"

#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>


namespace tests {

using namespace std;
using namespace test_framework;

constexpr double EPSILON = 1.0E-6;

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

    const vector<int> ratings = { 8, 12, 5 };
    int average_rating = accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());

    server.AddDocument(0, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, ratings);
    const vector<Document> results = server.FindTopDocuments("dog"s);

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

    const vector<Document> results = server.FindTopDocuments("curly big cat"s);

    constexpr int total_documents = 5;
    constexpr int count_docs_with_curly = 2;
    constexpr int count_docs_with_big = 3;
    constexpr int count_docs_with_cat = 2;

    // Вычисление IDF
    double idf_curly = log(total_documents / (1.0 * count_docs_with_curly));
    double idf_big = log(total_documents / (1.0 * count_docs_with_big));
    double idf_cat = log(total_documents / (1.0 * count_docs_with_cat));

    // Вычисляем релевантность для каждого документа
    vector<double> expected_relevances;

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
        ASSERT_HINT(std::abs(results[i].relevance - expected_relevances[i]) < EPSILON,
                "Wrong calculation relevance for document ID = "s +
                to_string(results[i].id) + ": relevance "s +
                to_string(results[i].relevance) + ", but expected "s +
                to_string(expected_relevances[i]));
    }
}

void TestSortResultsByRelevance() {
    SearchServer server;
    server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});
 
    const vector<Document> results = server.FindTopDocuments("dog cat big"s);
    ASSERT_EQUAL(results.size(), 5);
    for (size_t i = 1; i < results.size(); ++i) {
        ASSERT(results[i - 1].relevance >= results[i].relevance);
    }
}

void TestExcludeStopWordsFromAddedDocument() {
    SearchServer server("in"s);
    server.AddDocument(42, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });

    const vector<Document> results = server.FindTopDocuments("in"s);

    ASSERT_HINT(results.empty(), "Documents containing stop words should be excluded by searching"s);

    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 1, "Document must be added"s);
    ASSERT_EQUAL_HINT(server.GetDocumentId(0), 42, "Document must be added"s);
}

void TestFindByOneWordTopDocuments() {
    SearchServer server("and in at"s);
    server.AddDocument(11, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(12, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    const vector<Document> results = server.FindTopDocuments("curly"s);

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

    const vector<Document> results = server.FindTopDocuments("cat big -collar"s);

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
    vector<int> data;
    data.reserve(15);
    for (int i = 1; i <= 15; ++i) {
        data.push_back(i);
    }
    auto paginated = Paginate(data, 5);
    ASSERT_EQUAL(paginated.size(), 3);
}

void RunTests() {
    RUN_TEST(TestWithoutDocuments);
    RUN_TEST(TestWithInvalidIdDocument);
    RUN_TEST(TestAddTwoEqualDocuments);
    RUN_TEST(TestAddTwoDocuments);
    RUN_TEST(TestCalculateDocumentRating);
    RUN_TEST(TestCalculateRelevance);
    RUN_TEST(TestSortResultsByRelevance);
    RUN_TEST(TestExcludeStopWordsFromAddedDocument);
    RUN_TEST(TestFindByOneWordTopDocuments);
    RUN_TEST(TestFindByTwoWordsTopDocuments);
    RUN_TEST(TestRequestQueue);
    RUN_TEST(TestPagination);
}

} // namespace tests

int main() {
    std::cerr << "=== Tests are running ===\n";
    tests::RunTests();
    std::cerr << "=== All tests passed successfully! ===\n";
}
#include "paginator.h"
#include "search_server.h"
#include "request_queue.h"

using namespace std;

// ==================== для демонстрации функционала =========================

int main() {
    // Указанные стоп-слова игнорируются при поиске
    SearchServer search_server("and in at"s);

    // Очередь запросов для поиска
    RequestQueue request_queue(search_server);

    // Добавляем документы
    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::BANNED, {1, 1, 1});

    // Все документы добавлены, нет документов с пустым результатом поиска
    cout << "=== ALL documents successfully added ===\n";
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 1. Поиск без фильтрации по одному слову ***
    cout << "\n=== Search documents with 'big', default status is ACTUAL ===\n";
    for (const auto& document : request_queue.AddFindRequest("big"s)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 7. Поиск по двум словам без фильтрации ***
    cout << "\n=== Search excluding documents with 'big' or 'cat', default status is ACTUAL ===\n";
    for (const auto& document : request_queue.AddFindRequest("big cat"s)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 6. Поиск с минус-словами ***
    cout << "\n=== Search documents with 'big' or 'cat' or fancy' excluding documents with 'curly' or 'collar', default status is ACTUAL ===\n";
    for (const auto& document : request_queue.AddFindRequest("big cat fancy -curly -collar"s)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 2. Поиск по статусу ***
    cout << "\n=== Search only BANNED documents with 'big' ===\n";
    for (const auto& document : request_queue.AddFindRequest("big"s, DocumentStatus::BANNED)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 3. Поиск с использованием пользовательского предиката (id документа)***
    cout << "\n=== Search documents with 'big' and id == 4 ===\n";
    auto id_predicate = [](int id, DocumentStatus, int) {
        return id == 4;
    };
    for (const auto& document : request_queue.AddFindRequest("big"s, id_predicate)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 4. Поиск с использованием пользовательского предиката (рейтинг)***
    cout << "\n=== Search documents with 'big and rating > 2' ===\n";
    auto rating_predicate = [](int, DocumentStatus, int rating) {
        return rating > 2;
    };
    for (const auto& document : request_queue.AddFindRequest("big"s, rating_predicate)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // *** 5. Поиск с использованием пользовательского предиката (рейтинг и статус)
    cout << "\n=== Search documents with rating > 1 and ACTUAL documents ===\n";
    auto rating_status_predicate = []([[maybe_unused]] int document_id, DocumentStatus status, int rating) {
        return rating > 1 && status == DocumentStatus::ACTUAL; 
    };
    for (const auto& document : request_queue.AddFindRequest("dog"s, rating_status_predicate)) {
        cout << document << endl;
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;


    // Добавляем 1439 документов с нулевым результатом - одни сутки
    cout << "\n=== Add 1439 documents with zero result in one day ===\n";
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    
    // Всё ещё 1439 документов с нулевым результатом
    cout << "\n=== Add one valid document ===\n";
    request_queue.AddFindRequest("curly dog"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // Новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    cout << "\n=== Add one valid document ===\n";
    request_queue.AddFindRequest("big collar"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // Первый запрос удален, 1437 запросов с нулевым результатом
    cout << "\n=== Add one valid document ===\n";
    request_queue.AddFindRequest("sparrow"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    return 0;
}

// ==================== для демонстрации функционала =========================
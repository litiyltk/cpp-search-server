#include "paginator.h"
#include "search_server.h"
#include "request_queue.h"


using namespace std::string_literals;

using namespace search_server;
using namespace request_queue;
using namespace paginator;

// ==================== для демонстрации функционала =========================

template <typename T>
void PrintPaginatedResults(const std::vector<T>& results, int page_size = 2) {
    int page_number = 1;
    if (results.empty()) {
        std::cout << "Page "s << page_number << ": No results found"s << std::endl;
        std::cout << "Page break"s << std::endl;
        return;
    }
    const auto pages = Paginate(results, page_size);
    for (auto page = pages.begin(); page != pages.end(); ++page, ++page_number) {
        std::cout << "Page "s << page_number << ": "s << *page << std::endl;
        std::cout << "Page break"s << std::endl;
    }
}

int main() {
    // Указанные стоп-слова игнорируются при поиске
    SearchServer search_server("and at on in with"s);

    // Очередь запросов для поиска
    RequestQueue request_queue(search_server);

    // Добавляем документы
    search_server.AddDocument(1, "lost cat with blue collar"s, DocumentStatus::ACTUAL, {5, 3, 9});
    search_server.AddDocument(2, "small dog and red leash"s, DocumentStatus::ACTUAL, {6, 2, 8});
    search_server.AddDocument(3, "parrot in green cage at home"s, DocumentStatus::IRRELEVANT, {4, 5, 7});
    search_server.AddDocument(4, "small hamster with brown fur and white spot"s, DocumentStatus::ACTUAL, {3, 6, 5});
    search_server.AddDocument(5, "fluffy rabbit near the park"s, DocumentStatus::BANNED, {2, 2, 6});
    search_server.AddDocument(6, "ferret with missing tail in city"s, DocumentStatus::ACTUAL, {5, 4, 8});
    search_server.AddDocument(7, "turtle and lost pet sign"s, DocumentStatus::ACTUAL, {7, 2, 5});
    search_server.AddDocument(8, "black cat in alley at night"s, DocumentStatus::ACTUAL, {6, 3, 7});
    search_server.AddDocument(9, "dog with curly hair at shelter"s, DocumentStatus::IRRELEVANT, {4, 6, 9});
    search_server.AddDocument(10, "missing pigeon in the square"s, DocumentStatus::BANNED, {5, 5, 6});
    search_server.AddDocument(11, "parrot with blue feathers at cafe with hamster"s, DocumentStatus::ACTUAL, {3, 7, 5});
    search_server.AddDocument(12, "lost lizard on a warm rock"s, DocumentStatus::REMOVED, {6, 4, 7});
    search_server.AddDocument(13, "small kitten and big dog in shelter"s, DocumentStatus::ACTUAL, {5, 3, 8});
    search_server.AddDocument(14, "white rabbit and brown hamster together"s, DocumentStatus::IRRELEVANT, {4, 5, 9});
    search_server.AddDocument(15, "snake lost near the river and trees"s, DocumentStatus::BANNED, {3, 6, 7});
    search_server.AddDocument(16, "big golden retriever with red collar"s, DocumentStatus::ACTUAL, {7, 5, 8});
    search_server.AddDocument(17, "white parrot in the park lost"s, DocumentStatus::BANNED, {6, 4, 9});
    search_server.AddDocument(18, "hamster with tiny ears and white fur"s, DocumentStatus::IRRELEVANT, {5, 6, 7});
    search_server.AddDocument(19, "lost snake near the bushes"s, DocumentStatus::ACTUAL, {3, 7, 6});
    search_server.AddDocument(20, "black turtle in the pond missing"s, DocumentStatus::REMOVED, {4, 5, 8});

    // Все документы добавлены, нет документов с пустым результатом поиска
    std::cout << "=== ALL documents successfully added ===\n";
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;

    // *** 1. Поиск без фильтрации по одному слову ***
    {
        std::cout << "\n=== Search documents with 'dog', default status is ACTUAL ===\n";
        const auto results = request_queue.AddFindRequest("dog"s);
        PrintPaginatedResults(results, 1);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // *** 2. Поиск по двум словам без фильтрации ***
    {
        std::cout << "\n=== Search documents with 'cat' or 'parrot', default status is ACTUAL ===\n";
        const auto results = request_queue.AddFindRequest("cat parrot"s);
        PrintPaginatedResults(results, 2);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // *** 3. Поиск с минус-словами ***
    {
        std::cout << "\n=== Search documents with 'lost' or 'rabbit' excluding 'hamster' or 'collar', default status is ACTUAL ===\n";
        const auto results = request_queue.AddFindRequest("lost rabbit -hamster -collar"s);
        PrintPaginatedResults(results);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // *** 4. Поиск по статусу IRRELEVANT ***
    {
        std::cout << "\n=== Search only IRRELEVANT documents with 'parrot' ===\n";
        const auto results = request_queue.AddFindRequest("parrot"s, DocumentStatus::IRRELEVANT);
        PrintPaginatedResults(results);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // *** 5. Поиск с использованием пользовательского предиката (id документа)***
    {
        std::cout << "\n=== Search documents with 'parrot' and id == 11 ===\n";
        auto id_predicate = [](int id, DocumentStatus, int) {
            return id == 11;
        };
        const auto results = request_queue.AddFindRequest("parrot"s, id_predicate);
        PrintPaginatedResults(results);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // *** 6. Поиск с использованием пользовательского предиката (рейтинг)***
    {
        std::cout << "\n=== Search documents with 'hamster' and rating > 5 ===\n";
        auto rating_predicate = [](int, DocumentStatus, int rating) {
            return rating > 5;
        };
        const auto results = request_queue.AddFindRequest("hamster"s, rating_predicate);
        PrintPaginatedResults(results, 1);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // *** 7. Поиск с использованием пользовательского предиката (рейтинг и статус)***
    {
        std::cout << "\n=== Search documents with 'snake', rating > 3 and BANNED documents ===\n";
        auto rating_status_predicate = [](int, DocumentStatus status, int rating) {
            return rating > 3 && status == DocumentStatus::BANNED; 
        };
        const auto results = request_queue.AddFindRequest("snake"s, rating_status_predicate);
        PrintPaginatedResults(results, 3);
        std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    }

    // Добавляем 1439 документов с нулевым результатом - одни сутки
    std::cout << "\n=== Add 1439 documents with zero result in one day ===\n";
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    
    // Всё ещё 1439 документов с нулевым результатом
    std::cout << "\n=== Add one valid document ===\n";
    request_queue.AddFindRequest("curly dog"s);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;

    // Новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    std::cout << "\n=== Add one valid document ===\n";
    request_queue.AddFindRequest("big collar"s);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;

    // Первый запрос удален, 1437 запросов с нулевым результатом
    std::cout << "\n=== Add one valid document ===\n";
    request_queue.AddFindRequest("sparrow"s);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;

    return 0;
}

// ==================== для демонстрации функционала =========================
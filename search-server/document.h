#pragma once
#include <iostream>

//статусы документа
enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

//документ (ID, релавантность, рейтинг)
struct Document {
    Document() = default;

    Document(int id, double relevance, int rating);

    int id = 0;
    double relevance = 0;
    int rating = 0;
};

//перегрузка оператора вывода документа
std::ostream& operator<< (std::ostream& os, const Document& document);
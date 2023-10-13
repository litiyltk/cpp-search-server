#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <map>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

struct Document {
    int id;
    double relevance;
};

struct Query {
    set<string> plus_words;
    set<string> minus_words;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }
    
    void AddDocument(int document_id, const string& document) {
        ++document_count_;
        const vector<string> words = SplitIntoWordsNoStop(document);
        //изменил обозначение переменной tf на inc_tf
        const double inc_tf = CalcOneWordTF(words);
        for (const string& word: words) {
            if (!IsStopWord(word)) {
                word_to_document_freqs_[word][document_id] += inc_tf;
            }
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    
private:
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    
    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                query_words.minus_words.insert(word.substr(1));
            }
            if (!IsStopWord(word)) {
                query_words.plus_words.insert(word);
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> document_to_relevance; 
        vector<Document> matched_documents;
        /*в отдельном методе ComputeMapIDFs() вычисляются и сохраняются в словарь
        идф для каждого +слова*/
        map<string, double> word_to_idfs = ComputeMapIDFs(query_words.plus_words);
        for (const string& word : query_words.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            //идф вызывается из словаря по ключу +слову
            for (const auto& [document_id, tf] : word_to_document_freqs_.at(word)){
                document_to_relevance[document_id] += tf * word_to_idfs[word];
            }
        }
        
        for (const string& word : query_words.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, tf] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({document_id, relevance});
        }
        return matched_documents;
    }
    
    double CalcIdf(const string& word) const {
        return log(static_cast<double>(document_count_) / word_to_document_freqs_.at(word).size());
    }
    
    //добавил метод возвращающий словарь +слово-идф
    map<string, double> ComputeMapIDFs(const set<string>& plus_words) const {
        map<string, double> word_to_idfs;
        for (const string& word : plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            word_to_idfs.insert({word,CalcIdf(word)});
        }
        return word_to_idfs;
    }
    
    //изменил обозначение метода CalcTF на CalcOneWordTF
    double CalcOneWordTF (const vector<string>& words) const {
        return 1./ words.size();
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}


int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
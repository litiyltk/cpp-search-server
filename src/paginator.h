#pragma once

#include <iostream>
#include <string>
#include <vector>


namespace paginator {

using namespace std::string_literals;

template<typename Iterator> 
class IteratorRange {
public:
    explicit IteratorRange(Iterator begin, Iterator end)
        : begin_(begin)
        , end_(end)
        , size_of_page_(distance(begin_, end_)) {
    }
  
    auto begin() const{
        return begin_;
    } 
    
    auto end() const{
        return end_;
    }
    
    size_t size() const{
        return size_of_page_;
    }

private:
    Iterator begin_;
    Iterator end_;
    size_t size_of_page_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        for (size_t left = distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_size, left);
            const Iterator current_page_end = std::next(begin, current_page_size);
            pages_.push_back(IteratorRange{ begin, current_page_end });

            left -= current_page_size;
            begin = current_page_end;
        }
    }
    
    auto begin() const {
        return pages_.begin();
    } 
    
    auto end() const {
        return pages_.end();
    }
    
    size_t size() const {
        return pages_.size();
    }
    
private:
    std::vector<IteratorRange<Iterator>> pages_;
}; 

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

}; //namespace paginator
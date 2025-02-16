#include "document.h"


namespace document {

bool Document::operator==(const Document& other) const {
    return this->id == other.id && this->rating == other.rating && std::abs(this->relevance - other.relevance) <  EPSILON;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ "
        << "document_id = " << document.id << ", "
        << "relevance = " << document.relevance << ", "
        << "rating = " << document.rating << " }";
    return out;
}

}; // namespace document
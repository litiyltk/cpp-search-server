#pragma once

#include <functional>
#include <iostream>
#include <string>


// функции для тестирования
namespace test_framework {
    using namespace std::literals;

// принимает bool значение, выводит сообщение об ошибке при false и, если указана, то строку hint - тип ошибки
void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
                const std::string& hint) {
    if (!value) {
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

// принимает два значения произвольного типа, выводит сообщение об ошибке при false и, если указана, строку hint - тип ошибки
template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    //if (t != u) {
    if (std::not_equal_to{}(t, u)) {
        std::cerr  << std::boolalpha;
        std::cerr  << file << "("s << line << "): "s << func << ": "s;
        std::cerr  << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cerr  << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr  << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

// вызывает функцию для тестирования
template <typename TestFunc>
void RunTestImpl(const TestFunc& func, const std::string& test_name) {
    func();
    std::cerr << test_name << " OK"s << std::endl;
}

/* макросы для тестирования */

// принимает bool значение, выводит сообщение об ошибке при false
#define ASSERT(a) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, ""s)

// принимает bool значение, выводит сообщение об ошибке при false и строку hint - тип ошибки
#define ASSERT_HINT(a, hint) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, (hint))

// принимает два значения произвольного типа, выводит сообщение об ошибке при false
#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

// принимает два значения произвольного типа, выводит сообщение об ошибке при false и строку hint - тип ошибки
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

// вызывает функцию для тестирования
#define RUN_TEST(func) RunTestImpl(func, #func)

} // test_framework
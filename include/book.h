#pragma once
#include <string>
#include <vector>
#include <utility>

#include "MemoryRiver.h"
#include "session.h"

struct Book {
    char isbn[21];
    char name[61];
    char author[61];
    char keywords[61];
    double price;
    int quantity;

    Book();
    Book(const std::string &i, const std::string &n = "",
         const std::string &a = "", const std::string &k = "",
         double p = 0.0, int q = 0);
};

class BookManager {
public:
    BookManager();

    void show_all();
    void show_by_isbn(const std::string &isbn);
    void show_by_name(const std::string &name);
    void show_by_author(const std::string &author);
    void show_by_keyword(const std::string &keyword);

    bool buy(const std::string &isbn, int quantity, double &total_cost);

    bool select(const std::string &isbn, Session &session);

    bool modify(const std::string &selected_isbn,
                const std::vector<std::pair<std::string, std::string>> &modifications);

    bool import(const std::string &selected_isbn,
                int quantity, double total_cost);

private:
    MemoryRiver<Book, 1> book_file;

    bool find_by_isbn(const std::string &isbn_str, Book &book, int &index);
    std::vector<Book> get_all_books();
    void print_book(const Book &book);
    bool validate_isbn(const std::string &isbn);
    bool validate_string_no_quotes(const std::string &str);
    bool validate_keywords(const std::string &keywords_str);
};
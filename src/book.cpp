#include "include/book.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <set>
#include <string>

Book::Book() {
    std::memset(isbn, 0, sizeof(isbn));
    std::memset(name, 0, sizeof(name));
    std::memset(author, 0, sizeof(author));
    std::memset(keywords, 0, sizeof(keywords));
    price = 0.0;
    quantity = 0;
}

Book::Book(const std::string &i, const std::string &n,
           const std::string &a, const std::string &k,
           double p, int q) {
    std::strncpy(isbn, i.c_str(), 20);
    isbn[20] = '\0';
    std::strncpy(name, n.c_str(), 60);
    name[60] = '\0';
    std::strncpy(author, a.c_str(), 60);
    author[60] = '\0';
    std::strncpy(keywords, k.c_str(), 60);
    keywords[60] = '\0';
    price = p;
    quantity = q;
}

BookManager::BookManager()
    : book_file("books.dat") {
    std::ifstream fin("books.dat", std::ios::binary);
    bool need_init = false;
    if (!fin.good()) {
        need_init = true;
    } else {
        fin.seekg(0, std::ios::end);
        if (fin.tellg() < static_cast<std::streamoff>(sizeof(int))) need_init = true;
    }
    fin.close();
    if (need_init) book_file.initialise();
}

bool BookManager::validate_isbn(const std::string &isbn) {
    if (isbn.empty() || isbn.length() > 20) return false;
    for (char c : isbn) {
        if (c < 32 || c > 126) return false;
    }
    return true;
}

bool BookManager::validate_string_no_quotes(const std::string &str) {
    if (str.empty() || str.length() > 60) return false;
    for (char c : str) {
        if (c < 32 || c > 126 || c == '"') return false;
    }
    return true;
}

bool BookManager::validate_keywords(const std::string &keywords_str) {
    if (keywords_str.empty() || keywords_str.length() > 60) return false;

    std::istringstream iss(keywords_str);
    std::string keyword;
    std::set<std::string> unique_keywords;

    while (std::getline(iss, keyword, '|')) {
        if (keyword.empty()) return false;
        if (unique_keywords.find(keyword) != unique_keywords.end()) return false;
        unique_keywords.insert(keyword);
    }

    return true;
}

bool BookManager::find_by_isbn(const std::string &isbn_str, Book &book, int &index) {
    int n = 0;
    book_file.get_info(n, 1);

    for (int i = 1; i <= n; ++i) {
        int pos = sizeof(int) + (i - 1) * sizeof(Book);
        Book tmp;
        book_file.read(tmp, pos);
        if (tmp.isbn[0] == '\0') continue;

        if (std::strcmp(isbn_str.c_str(), tmp.isbn) == 0) {
            book = tmp;
            index = pos;
            return true;
        }
    }
    return false;
}

std::vector<Book> BookManager::get_all_books() {
    std::vector<Book> books;
    int n = 0;
    book_file.get_info(n, 1);

    for (int i = 1; i <= n; ++i) {
        int pos = sizeof(int) + (i - 1) * sizeof(Book);
        Book book;
        book_file.read(book, pos);
        if (book.isbn[0] != '\0') {
            books.push_back(book);
        }
    }

    std::sort(books.begin(), books.end(),
              [](const Book &a, const Book &b) {
                  return std::strcmp(a.isbn, b.isbn) < 0;
              });

    return books;
}

void BookManager::print_book(const Book &book) {
    std::cout << book.isbn << '\t'
              << book.name << '\t'
              << book.author << '\t'
              << book.keywords << '\t';
    std::cout << std::fixed << std::setprecision(2)
              << book.price << '\t'
              << book.quantity << '\n';
}

void BookManager::show_all() {
    auto books = get_all_books();
    for (const auto &book : books) {
        print_book(book);
    }
    if (books.empty()) {
        std::cout << '\n';
    }
}

void BookManager::show_by_isbn(const std::string &isbn) {
    auto books = get_all_books();
    bool found = false;
    for (const auto &book : books) {
        if (std::strcmp(book.isbn, isbn.c_str()) == 0) {
            print_book(book);
            found = true;
        }
    }
    if (!found) {
        std::cout << '\n';
    }
}

void BookManager::show_by_name(const std::string &name) {
    auto books = get_all_books();
    bool found = false;
    for (const auto &book : books) {
        if (std::strcmp(book.name, name.c_str()) == 0) {
            print_book(book);
            found = true;
        }
    }
    if (!found) {
        std::cout << '\n';
    }
}

void BookManager::show_by_author(const std::string &author) {
    auto books = get_all_books();
    bool found = false;
    for (const auto &book : books) {
        if (std::strcmp(book.author, author.c_str()) == 0) {
            print_book(book);
            found = true;
        }
    }
    if (!found) {
        std::cout << '\n';
    }
}

void BookManager::show_by_keyword(const std::string &keyword) {
    auto books = get_all_books();
    bool found = false;

    for (const auto &book : books) {
        std::string keywords_str(book.keywords);
        std::istringstream iss(keywords_str);
        std::string k;
        bool has_keyword = false;

        while (std::getline(iss, k, '|')) {
            if (k == keyword) {
                has_keyword = true;
                break;
            }
        }

        if (has_keyword) {
            print_book(book);
            found = true;
        }
    }

    if (!found) {
        std::cout << '\n';
    }
}

bool BookManager::buy(const std::string &isbn_str, int q, double &total_cost) {
    if (q <= 0) return false;

    Book book;
    int idx = 0;
    if (!find_by_isbn(isbn_str, book, idx)) return false;
    if (book.quantity < q) return false;

    book.quantity -= q;
    book_file.update(book, idx);

    total_cost = book.price * q;
    return true;
}

bool BookManager::select(const std::string &isbn_str, Session &session) {
    if (!validate_isbn(isbn_str)) return false;

    Book book;
    int idx = 0;
    if (!find_by_isbn(isbn_str, book, idx)) {
        // 创建新图书
        Book new_book(isbn_str);
        int n = 0;
        book_file.get_info(n, 1);

        const int HEADER = sizeof(int);
        idx = HEADER + n * static_cast<int>(sizeof(Book));

        book_file.write(new_book);
        book_file.write_info(n + 1, 1);
    }

    session.selected_pos = idx;
    return true;
}

bool BookManager::modify(const int &selected_pos,
                         const std::vector<std::pair<std::string, std::string>> &modifications) {
    if (modifications.empty()) return false;

    if (selected_pos < static_cast<int>(sizeof(int))) return false;

    Book book;
    book_file.read(book, selected_pos);

    if (book.isbn[0] == '\0') return false;

    // 检查是否有重复参数
    std::set<std::string> seen_keys;
    for (const auto& mod : modifications) {
        if (!seen_keys.insert(mod.first).second) {
            return false;
        }
    }

    // 检查是否修改ISBN为已有的ISBN
    for (const auto& mod : modifications) {
        if (mod.first == "ISBN") {
            if (!validate_isbn(mod.second)) return false;

            if (std::strcmp(book.isbn, mod.second.c_str()) == 0) return false;

            Book existing_book;
            int existing_idx = 0;
            if (find_by_isbn(mod.second, existing_book, existing_idx)) {
                if (existing_idx != selected_pos) return false;
            }
        }
    }

    // 应用修改
    for (const auto& mod : modifications) {
        const std::string& key = mod.first;
        const std::string& value = mod.second;

        if (value.empty()) return false;

        if (key == "ISBN") {
            if (!validate_isbn(value)) return false;
            if (std::strcmp(book.isbn, value.c_str()) == 0) return false;
            std::strncpy(book.isbn, value.c_str(), 20);
            book.isbn[20] = '\0';
        } else if (key == "name") {
            if (!validate_string_no_quotes(value)) return false;
            std::strncpy(book.name, value.c_str(), 60);
            book.name[60] = '\0';
        } else if (key == "author") {
            if (!validate_string_no_quotes(value)) return false;
            std::strncpy(book.author, value.c_str(), 60);
            book.author[60] = '\0';
        } else if (key == "keyword") {
            if (!validate_keywords(value)) return false;
            std::strncpy(book.keywords, value.c_str(), 60);
            book.keywords[60] = '\0';
        } else if (key == "price") {
            try {
                std::size_t p = 0;
                double price = std::stod(value, &p);
                if (p != value.size()) return false;
                if (price < 0) return false;
                book.price = price;
            } catch (...) {
                return false;
            }
        } else {
            return false;
        }
    }
    
    book_file.update(book, selected_pos);
    return true;
}

bool BookManager::import(const int &selected_pos,
                         int quantity, double total_cost) {
    if (quantity <= 0 || total_cost <= 0) return false;
    if (selected_pos < static_cast<int>(sizeof(int))) return false;

    Book book;
    book_file.read(book, selected_pos);
    if (book.isbn[0] == '\0') return false;

    book.quantity += quantity;
    book_file.update(book, selected_pos);
    return true;
}
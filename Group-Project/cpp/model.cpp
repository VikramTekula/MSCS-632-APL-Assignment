#include "model.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <stdexcept>

namespace {

// Display names in the same order as the enum, so the enum value indexes this table.
const std::vector<std::string> kCategoryNames = {
    "Food", "Transport", "Utilities", "Entertainment", "Other"
};

bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int days_in_month(int year, int month) {
    static const int lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return lengths[month - 1];
}

// Digits with at most one decimal point, and an optional sign. Nothing else is a sum of money.
bool is_decimal_number(const std::string& text) {
    std::size_t position = 0;
    if (position < text.size() && (text[position] == '+' || text[position] == '-')) {
        ++position;
    }
    int digits = 0;
    int points = 0;
    for (; position < text.size(); ++position) {
        const unsigned char letter = static_cast<unsigned char>(text[position]);
        if (std::isdigit(letter) != 0) {
            ++digits;
        } else if (text[position] == '.') {
            ++points;
        } else {
            return false;
        }
    }
    return digits > 0 && points <= 1;
}

bool all_digits(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    return std::all_of(text.begin(), text.end(),
                       [](unsigned char letter) { return std::isdigit(letter) != 0; });
}

}  // namespace

std::string to_lower(const std::string& text) {
    std::string lowered = text;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char letter) { return static_cast<char>(std::tolower(letter)); });
    return lowered;
}

const std::vector<Category>& all_categories() {
    static const std::vector<Category> categories = {
        Category::Food, Category::Transport, Category::Utilities,
        Category::Entertainment, Category::Other
    };
    return categories;
}

std::string category_name(Category category) {
    return kCategoryNames[static_cast<std::size_t>(category)];
}

std::string category_names_joined() {
    std::string joined;
    for (std::size_t position = 0; position < kCategoryNames.size(); ++position) {
        if (position > 0) {
            joined += ", ";
        }
        joined += kCategoryNames[position];
    }
    return joined;
}

ParseError parse_category(const std::string& text, Category& category) {
    const std::string wanted = to_lower(text);
    for (Category candidate : all_categories()) {
        if (to_lower(category_name(candidate)) == wanted) {
            category = candidate;
            return ParseError::None;
        }
    }
    return ParseError::UnknownCategory;
}

ParseError parse_date(const std::string& text, Date& date) {
    // The shape is fixed, so check it before reading any number out of the text.
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        return ParseError::DateForm;
    }
    const std::string year_text = text.substr(0, 4);
    const std::string month_text = text.substr(5, 2);
    const std::string day_text = text.substr(8, 2);
    if (!all_digits(year_text) || !all_digits(month_text) || !all_digits(day_text)) {
        return ParseError::DateForm;
    }

    const int year = std::stoi(year_text);
    const int month = std::stoi(month_text);
    const int day = std::stoi(day_text);
    // A month outside 1 to 12 is the wrong shape; a day outside the month is a day that never was.
    if (month < 1 || month > 12) {
        return ParseError::DateForm;
    }
    // Year 0 never happened, and it is also outside what the Python half can hold, so both
    // programs turn it away with the same message.
    if (year < 1) {
        return ParseError::DateNotReal;
    }
    if (day < 1 || day > days_in_month(year, month)) {
        return ParseError::DateNotReal;
    }

    date = Date{year, month, day};
    return ParseError::None;
}

ParseError parse_amount(const std::string& text, double& amount) {
    // std::stod also reads "nan", "inf" and hexadecimal, none of which is a sum of money, so the
    // shape is checked before the conversion rather than after it.
    if (!is_decimal_number(text)) {
        return ParseError::AmountForm;
    }

    double parsed = 0.0;
    std::size_t consumed = 0;
    try {
        parsed = std::stod(text, &consumed);
    } catch (const std::invalid_argument&) {
        return ParseError::AmountForm;
    } catch (const std::out_of_range&) {
        return ParseError::AmountForm;
    }
    // std::stod stops at the first character it cannot use, so "12abc" would pass unnoticed.
    if (consumed != text.size()) {
        return ParseError::AmountForm;
    }
    if (parsed <= 0.0) {
        return ParseError::AmountNotPositive;
    }

    amount = parsed;
    return ParseError::None;
}

bool date_on_or_before(const Date& left, const Date& right) {
    if (left.year != right.year) {
        return left.year < right.year;
    }
    if (left.month != right.month) {
        return left.month < right.month;
    }
    return left.day <= right.day;
}

std::string format_date(const Date& date) {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", date.year, date.month, date.day);
    return std::string(buffer);
}

std::string format_amount_plain(double amount) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", amount);
    return std::string(buffer);
}

std::string format_amount(double amount) {
    return "$" + format_amount_plain(amount);
}
